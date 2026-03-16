import Metal
import MetalKit
import SwiftUI
import Combine

/// GraphicsManager handles the macOS Metal graphics subsystem.
/// It provides the bridge between Windows DirectX/GDI calls and native Metal rendering.
public final class GraphicsManager: NSObject, ObservableObject {
    
    public static let shared = GraphicsManager()
    
    public let device: MTLDevice?
    public let commandQueue: MTLCommandQueue?
    
    @Published public var mtkView: MTKView?
    private var vramTexture: MTLTexture?
    private var vramBuffer: MTLBuffer?
    
    private override init() {
        self.device = MTLCreateSystemDefaultDevice()
        self.commandQueue = device?.makeCommandQueue()
        super.init()
        
        setupDefaultView()
        setupVRAMTexture()
    }
    
    private func setupDefaultView() {
        let view = MTKView(frame: .zero, device: device)
        view.clearColor = MTLClearColor(red: 0.1, green: 0.1, blue: 0.1, alpha: 1.0)
        view.preferredFramesPerSecond = 60
        view.enableSetNeedsDisplay = true
        self.mtkView = view
    }
    
    private func setupVRAMTexture() {
        guard let bridge = EngineBridge.sharedInstance() else { return }
        let width = bridge.getDisplayWidth()
        let height = bridge.getDisplayHeight()
        
        let descriptor = MTLTextureDescriptor.texture2DDescriptor(pixelFormat: .bgra8Unorm,
                                                                   width: Int(width),
                                                                   height: Int(height),
                                                                   mipmapped: false)
        descriptor.usage = [.shaderRead, .shaderWrite]
        self.vramTexture = device?.makeTexture(descriptor: descriptor)
    }

    private func ensureVRAMBuffer() -> MTLBuffer? {
        guard let device = device, let bridge = EngineBridge.sharedInstance() else { return nil }
        
        // Re-create buffer if invalid or missing
        if vramBuffer == nil || !bridge.isVRAMValid() {
            vramBuffer = bridge.getVRAMBuffer(with: device)
            if vramBuffer != nil {
                print("[ProWin] Graphics: VRAM Buffer established (Shared Memory)")
            }
        }
        return vramBuffer
    }
    
    /// Called when the Windows game requests a frame presentation.
    public func presentFrame() {
        guard let bridge = EngineBridge.sharedInstance(),
              bridge.isVRAMValid(),
              let device = self.device,
              let texture = vramTexture,
              let buffer = ensureVRAMBuffer(),
              let commandBuffer = commandQueue?.makeCommandBuffer(),
              let view = mtkView,
              let currentDrawable = view.currentDrawable else {
            return
        }
        
        let width = Int(bridge.getDisplayWidth())
        let height = Int(bridge.getDisplayHeight())
        
        // Copy Shared VRAM Buffer to Texture using Blit Encoder
        let blit = commandBuffer.makeBlitCommandEncoder()
        blit?.copy(from: buffer, 
                  sourceOffset: 0, 
                  sourceBytesPerRow: width * 4, 
                  sourceBytesPerImage: width * height * 4, 
                  sourceSize: MTLSize(width: width, height: height, depth: 1), 
                  to: texture, 
                  destinationSlice: 0, 
                  destinationLevel: 0, 
                  destinationOrigin: MTLOrigin(x: 0, y: 0, z: 0))
        blit?.endEncoding()
        
        // Render texture to screen
        let renderPassDescriptor = view.currentRenderPassDescriptor
        if let _ = renderPassDescriptor {
             let blitEncoder = commandBuffer.makeBlitCommandEncoder()
             blitEncoder?.copy(from: texture, 
                               sourceSlice: 0, 
                               sourceLevel: 0, 
                               sourceOrigin: MTLOrigin(x: 0, y: 0, z: 0), 
                               sourceSize: MTLSize(width: width, height: height, depth: 1), 
                               to: currentDrawable.texture, 
                               destinationSlice: 0, 
                               destinationLevel: 0, 
                               destinationOrigin: MTLOrigin(x: 0, y: 0, z: 0))
             blitEncoder?.endEncoding()
        }
        
        commandBuffer.present(currentDrawable)
        commandBuffer.commit()
    }
}
