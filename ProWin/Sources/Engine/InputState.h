#ifndef InputState_h
#define InputState_h

#include <cstdint>

namespace ProWin {

/**
 * Shared input state between the macOS frontend and the Windows stubs.
 * Based on the XINPUT_GAMEPAD structure.
 */
struct WinXInputState {
    uint32_t packetNumber;
    uint16_t buttons;
    uint8_t  leftTrigger;
    uint8_t  rightTrigger;
    int16_t  leftStickX;
    int16_t  leftStickY;
    int16_t  rightStickX;
    int16_t  rightStickY;
};

// Singleton to hold the current input state
class InputStateManager {
public:
    static InputStateManager& getInstance() {
        static InputStateManager instance;
        return instance;
    }

    void updateState(uint32_t playerIndex, const WinXInputState& state) {
        if (playerIndex < 4) {
            m_states[playerIndex] = state;
        }
    }

    WinXInputState getState(uint32_t playerIndex) const {
        if (playerIndex < 4) {
            return m_states[playerIndex];
        }
        return {};
    }

private:
    InputStateManager() {
        for (int i = 0; i < 4; ++i) {
            m_states[i] = {0};
        }
    }
    WinXInputState m_states[4];
};

} // namespace ProWin

#endif /* InputState_h */
