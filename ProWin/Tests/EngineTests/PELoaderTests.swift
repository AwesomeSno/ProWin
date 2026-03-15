import XCTest
@testable import ProWin

final class PELoaderTests: XCTestCase {
    
    func testLoadMinimalPE() throws {
        let fileManager = FileManager.default
        let currentPath = fileManager.currentDirectoryPath
        let testExeURL = URL(fileURLWithPath: currentPath).appendingPathComponent("test.exe")
        
        XCTAssertTrue(fileManager.fileExists(atPath: testExeURL.path), "Test executable should exist")
        
        let loader = PELoader()
        let binary = try loader.load(from: testExeURL)
        
        XCTAssertEqual(binary.name, "test.exe")
        XCTAssertEqual(binary.baseAddress, 0x00400000)
        XCTAssertEqual(binary.entryPoint, 0x00401000)
        XCTAssertTrue(binary.is64Bit)
        
        XCTAssertEqual(binary.sections.count, 2)
        
        let textSection = binary.sections.first { $0.name == ".text" }
        XCTAssertNotNil(textSection)
        XCTAssertEqual(textSection?.virtualAddress, 0x1000)
        
        let dataSection = binary.sections.first { $0.name == ".data" }
        XCTAssertNotNil(dataSection)
        XCTAssertEqual(dataSection?.virtualAddress, 0x2000)
    }
}
