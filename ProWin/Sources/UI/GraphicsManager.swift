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
    
    private override init() {
        self.device = MTLCreateSystemDefaultDevice()
        self.commandQueue = device?.makeCommandQueue()
        super.init()
        
        setupDefaultView()
    }
    
    private func setupDefaultView() {
        let view = MTKView(frame: .zero, device: device)
        view.clearColor = MTLClearColor(red: 0.1, green: 0.1, blue: 0.1, alpha: 1.0)
        view.preferredFramesPerSecond = 60
        view.enableSetNeedsDisplay = true
        self.mtkView = view
    }
    
    /// Called when the Windows game requests a frame presentation.
    public func presentFrame() {
        // TODO: Implement DirectX backbuffer to Metal texture copy
        mtkView?.draw()
    }
}
