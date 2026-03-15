import Foundation
import Combine

/// GameLoop coordinates the execution of the Windows binary with macOS UI/Graphics updates.
public final class GameLoop: ObservableObject {
    public static let shared = GameLoop()
    
    @Published public var isRunning: Bool = false
    @Published public var framesProcessed: UInt64 = 0
    
    private var displayLink: CADisplayLink?
    private var engineThread: Thread?
    
    private init() {}
    
    public func start(entryPoint: UInt64) {
        guard !isRunning else { return }
        isRunning = true
        
        // 1. Setup Display Link for sync with screen refresh
        setupDisplayLink()
        
        // 2. Start Windows Execution on a background thread
        engineThread = Thread { [weak self] in
            self?.runWindowsCode(entryPoint: entryPoint)
        }
        engineThread?.name = "ProWin.EngineThread"
        engineThread?.qualityOfService = .userInteractive
        engineThread?.start()
    }
    
    public func stop() {
        isRunning = false
        displayLink?.invalidate()
        displayLink = nil
        // Thread termination would be more complex in a real scenario
    }
    
    private func setupDisplayLink() {
        displayLink = CADisplayLink(target: self, selector: #selector(onDisplayUpdate))
        displayLink?.add(to: .main, forMode: .common)
    }
    
    @objc private func onDisplayUpdate() {
        guard isRunning else { return }
        
        // Update Graphics
        GraphicsManager.shared.presentFrame()
        
        framesProcessed += 1
    }
    
    private func runWindowsCode(entryPoint: UInt64) {
        DispatchQueue.main.async {
            print("[ProWin] UI Bridge: Starting Engine via EngineBridge")
            EngineBridge.sharedInstance().startEngine(entryPoint)
        }
        
        // Monitor for termination
        while isRunning {
            if !EngineBridge.sharedInstance().isEngineRunning() {
                print("[ProWin] UI Bridge: Engine reported termination")
                DispatchQueue.main.async {
                    self.stop()
                }
                break
            }
            Thread.sleep(forTimeInterval: 0.1)
        }
    }
}
