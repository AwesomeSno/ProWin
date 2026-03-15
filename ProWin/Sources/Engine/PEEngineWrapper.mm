#import "PEEngineWrapper.h"
#include "PEParser.h"

@implementation PEInfoWrapper
@end

@implementation PEEngineWrapper

- (PEInfoWrapper *)getPEInfo:(NSString *)filePath {
    ProWin::PEParser::PEInfo info = ProWin::PEParser::getInfo([filePath UTF8String]);
    
    PEInfoWrapper *wrapper = [[PEInfoWrapper alloc] init];
    wrapper.isValid = info.isValid;
    wrapper.is64Bit = info.is64Bit;
    wrapper.imageBase = info.imageBase;
    wrapper.entryPointRVA = info.entryPointRVA;
    wrapper.sizeOfImage = info.sizeOfImage;
    
    return wrapper;
}

@end
