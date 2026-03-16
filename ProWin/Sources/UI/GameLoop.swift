import Foundation
import Combine
import QuartzCore

/// GameLoop coordinates the execution of the Windows binary with macOS UI/Graphics updates.
public final class GameLoop: ObservableObject {
    public static let shared = GameLoop()
    
    @Published public var rax: UInt64 = 0
    @Published public var errorMessage: String?
    @Published public var showErrorAlert: Bool = false
    @Published public var isLoaded: Bool = false
    
    private var entryPoint: UInt64 = 0
    
    private var displayLink: CADisplayLink?
    private var engineThread: Thread?
    
    private init() {}
    
    public func start(url: URL) {
        guard !isRunning else { return }
        
        do {
            // 1. Consolidated Load
            let result = try PELoader().load(from: url)
            self.entryPoint = result.absoluteEntryPoint
            
            // 2. Set Entry Point in Engine
            EngineBridge.sharedInstance().setEntryPoint(self.entryPoint)
            
            DispatchQueue.main.async {
                self.isLoaded = true
                self.isRunning = true
                self.setupDisplayLink()
                
                // 3. Start Windows Execution on a background thread
                self.engineThread = Thread { [weak self] in
                    self?.runWindowsCode(entryPoint: result.absoluteEntryPoint)
                }
                self.engineThread?.name = "ProWin.EngineThread"
                self.engineThread?.qualityOfService = .userInteractive
                self.engineThread?.start()
            }
        } catch {
            print("[ProWin] GameLoop: Failed to load PE: \(error)")
            self.errorMessage = "Failed to load PE: \(error.localizedDescription)"
            self.showErrorAlert = true
        }
    }
    
    public func stop() {
        isRunning = false
        displayLink?.invalidate()
        displayLink = nil
    }
    
    private func setupDisplayLink() {
        displayLink = CADisplayLink(target: self, selector: #selector(onDisplayUpdate))
        displayLink?.add(to: .main, forMode: .common)
    }
    
    @objc private func onDisplayUpdate() {
        guard isRunning else { return }
        
        // Update Graphics
        GraphicsManager.shared.presentFrame()
        
        // Update Input (Bridge native controller state to Engine)
        if let state = InputManager.shared.getControllerState(index: 0) {
            EngineBridge.sharedInstance().updateInputState(
                0, 
                buttons: state.buttons, 
                leftStickX: state.leftStickX, 
                leftStickY: state.leftStickY, 
                rightStickX: state.rightStickX, 
                rightStickY: state.rightStickY
            )
        }
        
        // Sync Register State for UI
        self.rax = EngineBridge.sharedInstance().getRegisterRAX()
        
        framesProcessed += 1
    }
    
    private func runWindowsCode(entryPoint: UInt64) {
        print("[ProWin] UI Bridge: Starting Engine via EngineBridge")
        let success = EngineBridge.sharedInstance().startEngine(entryPoint)
        
        if (!success) {
            let error = EngineBridge.sharedInstance().getErrorState() ?? "Unknown Engine Error"
            print("[ProWin] UI Bridge: Engine failed to start: \(error)")
            DispatchQueue.main.async {
                self.errorMessage = error
                self.showErrorAlert = true
                self.stop()
            }
            return
        }
        
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
