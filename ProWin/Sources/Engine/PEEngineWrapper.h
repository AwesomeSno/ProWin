#import <Foundation/Foundation.h>

@interface PEInfoWrapper : NSObject
@property (nonatomic, assign) BOOL isValid;
@property (nonatomic, assign) BOOL is64Bit;
@property (nonatomic, assign) uint64_t imageBase;
@property (nonatomic, assign) uint32_t entryPointRVA;
@property (nonatomic, assign) uint32_t sizeOfImage;
@end

@interface PEEngineWrapper : NSObject
- (PEInfoWrapper *)getPEInfo:(NSString *)filePath;
@end
