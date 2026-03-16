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
        let width = EngineBridge.sharedInstance().getDisplayWidth()
        let height = EngineBridge.sharedInstance().getDisplayHeight()
        
        let descriptor = MTLTextureDescriptor.texture2DDescriptor(pixelFormat: .bgra8Unorm,
                                                                   width: Int(width),
                                                                   height: Int(height),
                                                                   mipmapped: false)
        descriptor.usage = [.shaderRead, .shaderWrite]
        self.vramTexture = device?.makeTexture(descriptor: descriptor)
    }
    
    /// Called when the Windows game requests a frame presentation.
    public func presentFrame() {
        guard let texture = vramTexture,
              let vramPtr = EngineBridge.sharedInstance().getVRAMPointer(),
              let commandBuffer = commandQueue?.makeCommandBuffer(),
              let view = mtkView,
              let currentDrawable = view.currentDrawable else {
            return
        }
        
        let width = Int(EngineBridge.sharedInstance().getDisplayWidth())
        let height = Int(EngineBridge.sharedInstance().getDisplayHeight())
        let bytesPerRow = width * 4
        
        // Copy VRAM to Texture
        texture.replace(region: MTLRegionMake2D(0, 0, width, height),
                        mipmapLevel: 0,
                        withBytes: vramPtr,
                        bytesPerRow: bytesPerRow)
        
        // Render texture to screen
        let renderPassDescriptor = view.currentRenderPassDescriptor
        if let renderPassDescriptor = renderPassDescriptor {
             // For a simple blit, we could use a blit encoder, but MTKView is easier 
             // if we just copy to its current texture.
             // However, for maximum performance "better than windows", we'll eventually use a shader.
             // For now, let's just use the blit encoder.
             let blitEncoder = commandBuffer.makeBlitCommandEncoder()
             blitEncoder?.copy(from: texture, 
                              sourcePosition: MTLOrigin(x: 0, y: 0, z: 0), 
                              sourceSize: MTLSize(width: width, height: height, depth: 1), 
                              to: currentDrawable.texture, 
                              destinationPosition: MTLOrigin(x: 0, y: 0, z: 0))
             blitEncoder?.endEncoding()
        }
        
        commandBuffer.present(currentDrawable)
        commandBuffer.commit()
    }
}
