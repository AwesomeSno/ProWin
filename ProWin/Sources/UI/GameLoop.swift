import Foundation
import Combine
import QuartzCore

/// GameLoop coordinates the execution of the Windows binary with macOS UI/Graphics updates.
public final class GameLoop: ObservableObject {
    public static let shared = GameLoop()
    
    @Published public var isRunning: Bool = false
    @Published public var framesProcessed: UInt64 = 0
    @Published public var rax: UInt64 = 0
    
    private var timer: Timer?
    private var engineThread: Thread?
    
    private init() {}
    
    public func start(entryPoint: UInt64) {
        guard !isRunning else { return }
        isRunning = true
        
        // 1. Setup Timer for sync with screen refresh (approx 60 FPS)
        setupTimer()
        
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
        timer?.invalidate()
        timer = nil
    }
    
    private func setupTimer() {
        timer = Timer.scheduledTimer(withTimeInterval: 1.0/60.0, repeats: true) { [weak self] _ in
            self?.onDisplayUpdate()
        }
    }
    
    @objc private func onDisplayUpdate() {
        guard isRunning else { return }
        
        // Update Graphics
        GraphicsManager.shared.presentFrame()
        
        // Sync Register State for UI
        self.rax = EngineBridge.sharedInstance().getRegisterRAX()
        
        framesProcessed += 1
    }
    
    private func runWindowsCode(entryPoint: UInt64) {
        print("[ProWin] UI Bridge: Starting Engine via EngineBridge")
        EngineBridge.sharedInstance().startEngine(entryPoint)
        
        // Wait a bit to ensure the thread has actually started
        Thread.sleep(forTimeInterval: 0.01)
        
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
