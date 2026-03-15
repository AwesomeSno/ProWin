#ifndef InputBridge_h
#define InputBridge_h

#import <Foundation/Foundation.h>

typedef struct {
    uint32_t dwPacketNumber;
    uint16_t wButtons;
    uint8_t  bLeftTrigger;
    uint8_t  bRightTrigger;
    int16_t  sThumbLX;
    int16_t  sThumbLY;
    int16_t  sThumbRX;
    int16_t  sThumbRY;
} WIN_XINPUT_GAMEPAD;

typedef struct {
    uint32_t dwPacketNumber;
    WIN_XINPUT_GAMEPAD Gamepad;
} WIN_XINPUT_STATE;

@interface InputBridge : NSObject

+ (instancetype)sharedInstance;
- (BOOL)getXInputState:(uint32_t)userIndex state:(WIN_XINPUT_STATE*)state;

@end

#endif /* InputBridge_h */
