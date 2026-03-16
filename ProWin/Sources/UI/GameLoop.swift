import Foundation
import Combine
import QuartzCore
import CoreVideo

/// GameLoop coordinates the execution of the Windows binary with macOS UI/Graphics updates.
public final class GameLoop: ObservableObject {
    public static let shared = GameLoop()
    
    @Published public var rax: UInt64 = 0
    @Published public var rip: UInt64 = 0
    @Published public var rflags: UInt32 = 0
    @Published public var errorMessage: String?
    @Published public var showErrorAlert: Bool = false
    @Published public var isLoaded: Bool = false
    @Published public var isRunning: Bool = false
    @Published public var isPaused: Bool = false
    
    private var entryPoint: UInt64 = 0
    private var framesProcessed: UInt64 = 0
    
    private var displayLink: CVDisplayLink?
    private var engineThread: Thread?
    
    private init() {}
    
    public func start(url: URL) {
        guard !isRunning else { return }
        
        do {
            // 1. Consolidated Load
            let result = try PELoader().load(from: url)
            self.entryPoint = result.absoluteEntryPoint
            
            // 2. Set Entry Point in Engine
            let bridge = EngineBridge.sharedInstance()
            bridge?.setEntryPoint(self.entryPoint)
            
            // 3. Setup Listener
            bridge?.onEngineEvent = { [weak self] event in
                guard let event = event else { return }
                print("[ProWin] Engine Event: \(event)")
                if event == "halt" || event == "finish" {
                    self?.stop()
                } else if event.hasPrefix("error:") {
                    let msg = String(event.dropFirst(6))
                    self?.errorMessage = msg
                    self?.showErrorAlert = true
                    self?.stop()
                }
            }
            
            DispatchQueue.main.async {
                self.isLoaded = true
                self.isRunning = true
                self.setupCVDisplayLink()
                
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
        if let dl = displayLink {
            CVDisplayLinkStop(dl)
            displayLink = nil
        }
    }
    
    private func setupCVDisplayLink() {
        CVDisplayLinkCreateWithActiveCGDisplays(&displayLink)
        
        guard let dl = displayLink else {
            print("[ProWin] Error: Could not create CVDisplayLink")
            return
        }
        
        CVDisplayLinkSetOutputCallback(dl, { _, _, _, _, _, userInfo in
            let gameLoop = Unmanaged<GameLoop>.fromOpaque(userInfo!).takeUnretainedValue()
            gameLoop.tick()
            return kCVReturnSuccess
        }, Unmanaged.passUnretained(self).toOpaque())
        
        CVDisplayLinkStart(dl)
    }
    
    private func tick() {
        guard isRunning else { return }
        
        // 1. Update Input (Bridge native controller state to Engine)
        if let state = InputManager.shared.getControllerState(index: 0),
           let bridge = EngineBridge.sharedInstance() {
            bridge.updateInputState(
                0, 
                buttons: state.buttons, 
                leftStickX: state.leftStickX, 
                leftStickY: state.leftStickY, 
                rightStickX: state.rightStickX, 
                rightStickY: state.rightStickY
            )
        }
        
        // 2. Sync State for UI (Atomic Snapshot)
        if let bridge = EngineBridge.sharedInstance() {
            let snapshot = bridge.getSnapshot()
            DispatchQueue.main.async {
                self.rax = snapshot.rax
                self.rip = snapshot.rip
                self.rflags = snapshot.rflags
                self.isRunning = snapshot.isRunning.boolValue
                self.isPaused = snapshot.isPaused.boolValue
                
                // 3. Present Graphics (Metal presentation MUST be synced to DisplayLink)
                GraphicsManager.shared.presentFrame()
            }
        }
        
        framesProcessed += 1
    }
    
    private func runWindowsCode(entryPoint: UInt64) {
        guard let bridge = EngineBridge.sharedInstance() else { return }
        print("[ProWin] UI Bridge: Starting Engine via EngineBridge")
        let success = bridge.startEngine(entryPoint)
        
        if (!success) {
            let error = bridge.getErrorState() ?? "Unknown Engine Error"
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
            if !bridge.isEngineRunning() {
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
