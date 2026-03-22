#import "AudioBridge.h"
#import <AVFoundation/AVFoundation.h>

@interface AudioBridge ()
@property (nonatomic, strong) AVAudioEngine *engine;
@property (nonatomic, strong) AVAudioPlayerNode *playerNode;
@property (nonatomic, assign) BOOL isSetup;
@end

@implementation AudioBridge

+ (instancetype)sharedInstance {
    static AudioBridge *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[AudioBridge alloc] init];
    });
    return instance;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _isSetup = NO;
        [self setupEngine];
    }
    return self;
}

- (void)setupEngine {
    _engine = [[AVAudioEngine alloc] init];
    _playerNode = [[AVAudioPlayerNode alloc] init];
    
    [_engine attachNode:_playerNode];
    
    AVAudioMixerNode *mixer = [_engine mainMixerNode];
    [_engine connect:_playerNode to:mixer format:[mixer outputFormatForBus:0]];
    
    NSError *error = nil;
    [_engine startAndReturnError:&error];
    if (error) {
        NSLog(@"[ProWin] AudioBridge: Failed to start engine: %@", error);
    } else {
        _isSetup = YES;
        NSLog(@"[ProWin] AudioBridge: AVAudioEngine started");
    }
}

- (void)playPCMData:(const void *)data length:(NSUInteger)length channels:(NSUInteger)channels sampleRate:(double)sampleRate {
    if (!_isSetup || !data || length == 0) return;
    
    AVAudioFormat *format = [[AVAudioFormat alloc] initWithCommonFormat:AVAudioPCMFormatFloat32
                                                            sampleRate:sampleRate
                                                              channels:(AVAudioChannelCount)channels
                                                           interleaved:NO];
    
    AVAudioFrameCount frameCount = (AVAudioFrameCount)(length / (channels * sizeof(float)));
    AVAudioPCMBuffer *buffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:format frameCapacity:frameCount];
    buffer.frameLength = frameCount;
    
    memcpy(buffer.floatChannelData[0], data, length);
    
    if (![_playerNode isPlaying]) {
        [_playerNode play];
    }
    
    [_playerNode scheduleBuffer:buffer completionHandler:nil];
}

- (void)playSound {
    NSLog(@"[ProWin] AudioBridge: playSound called");
}

- (void)stopSound {
    [_playerNode stop];
    [_engine stop];
    NSLog(@"[ProWin] AudioBridge: Audio stopped");
}

@end
