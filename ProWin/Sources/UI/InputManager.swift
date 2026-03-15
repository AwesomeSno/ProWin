import GameController
import Foundation
import Combine

/// InputManager handles macOS game controllers and bridges them to Windows XInput.
public final class InputManager: NSObject, ObservableObject {
    
    public static let shared = InputManager()
    
    @Published public var isControllerConnected: Bool = false
    private var controllers: [GCController] = []
    
    private override init() {
        super.init()
        setupControllerNotifications()
    }
    
    private func setupControllerNotifications() {
        NotificationCenter.default.addObserver(self, selector: #selector(controllerDidConnect), name: .GCControllerDidConnect, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(controllerDidDisconnect), name: .GCControllerDidDisconnect, object: nil)
        
        // Check for existing controllers
        controllers = GCController.controllers()
        isControllerConnected = !controllers.isEmpty
    }
    
    @objc private func controllerDidConnect(_ notification: Notification) {
        guard let controller = notification.object as? GCController else { return }
        controllers.append(controller)
        isControllerConnected = true
        print("[ProWin] Controller connected: \(controller.vendorName ?? "Generic")")
    }
    
    @objc private func controllerDidDisconnect(_ notification: Notification) {
        guard let controller = notification.object as? GCController else { return }
        controllers.removeAll { $0 === controller }
        isControllerConnected = !controllers.isEmpty
        print("[ProWin] Controller disconnected")
    }
    
    public func getControllerState(index: Int) -> XInputState? {
        guard index < controllers.count else { return nil }
        let controller = controllers[index]
        
        guard let gamepad = controller.extendedGamepad else { return nil }
        
        var state = XInputState()
        state.packetNumber += 1
        
        // Map buttons
        if gamepad.buttonA.isPressed { state.buttons |= 0x1000 } // XINPUT_GAMEPAD_A
        if gamepad.buttonB.isPressed { state.buttons |= 0x2000 } // XINPUT_GAMEPAD_B
        if gamepad.buttonX.isPressed { state.buttons |= 0x4000 } // XINPUT_GAMEPAD_X
        if gamepad.buttonY.isPressed { state.buttons |= 0x8000 } // XINPUT_GAMEPAD_Y
        
        // Map sticks
        state.leftStickX = Int16(gamepad.leftThumbstick.xAxis.value * 32767)
        state.leftStickY = Int16(gamepad.leftThumbstick.yAxis.value * 32767)
        state.rightStickX = Int16(gamepad.rightThumbstick.xAxis.value * 32767)
        state.rightStickY = Int16(gamepad.rightThumbstick.yAxis.value * 32767)
        
        return state
    }
}

/// Simplified XInput state structure
public struct XInputState {
    public var packetNumber: UInt32 = 0
    public var buttons: UInt16 = 0
    public var leftTrigger: UInt8 = 0
    public var rightTrigger: UInt8 = 0
    public var leftStickX: Int16 = 0
    public var leftStickY: Int16 = 0
    public var rightStickX: Int16 = 0
    public var rightStickY: Int16 = 0
}
