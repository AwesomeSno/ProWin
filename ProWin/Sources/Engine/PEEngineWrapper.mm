#import "PEEngineWrapper.h"
#include "PEParser.h"
#include "PEImage.h"

@implementation PESectionWrapper
@end

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
    
    NSMutableArray<PESectionWrapper *> *sections = [NSMutableArray array];
    for (const auto& s : info.sections) {
        PESectionWrapper *sw = [[PESectionWrapper alloc] init];
        sw.name = [NSString stringWithUTF8String:s.name.c_str()];
        sw.virtualAddress = s.virtualAddress;
        sw.virtualSize = s.virtualSize;
        sw.rawDataSize = s.rawDataSize;
        sw.rawDataPtr = s.rawDataPtr;
        sw.characteristics = s.characteristics;
        [sections addObject:sw];
    }
    wrapper.sections = sections;
    
    return wrapper;
}

- (uint64_t)loadImage:(NSString *)filePath {
    auto image = new ProWin::PEImage();
    if (image->load([filePath UTF8String])) {
        return image->getEntryPoint();
    }
    delete image;
    return 0;
}

@end
