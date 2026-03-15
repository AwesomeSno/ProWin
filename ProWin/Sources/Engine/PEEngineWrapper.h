#import <Foundation/Foundation.h>

@interface PESectionWrapper : NSObject
@property (nonatomic, copy) NSString *name;
@property (nonatomic, assign) uint32_t virtualAddress;
@property (nonatomic, assign) uint32_t virtualSize;
@property (nonatomic, assign) uint32_t rawDataSize;
@property (nonatomic, assign) uint32_t rawDataPtr;
@property (nonatomic, assign) uint32_t characteristics;
@end

@interface PEInfoWrapper : NSObject
@property (nonatomic, assign) BOOL isValid;
@property (nonatomic, assign) BOOL is64Bit;
@property (nonatomic, assign) uint64_t imageBase;
@property (nonatomic, assign) uint32_t entryPointRVA;
@property (nonatomic, assign) uint32_t sizeOfImage;
@property (nonatomic, strong) NSArray<PESectionWrapper *> *sections;
@end

@interface PEEngineWrapper : NSObject
- (PEInfoWrapper *)getPEInfo:(NSString *)filePath;
@end
