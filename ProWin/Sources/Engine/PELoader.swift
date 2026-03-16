import Foundation

/// PEBinary defines the structure of a loaded Windows executable.
public struct PEBinary {
    public struct Section {
        public let name: String
        public let virtualAddress: UInt32
        public let virtualSize: UInt32
    }
    
    public let name: String
    public let baseAddress: UInt64
    public let entryPoint: UInt64
    public let is64Bit: Bool
    public let sections: [Section]
}

/// PELoadResult contains the result of a consolidated PE loading operation.
public struct PELoadResult {
    public let entryPointRVA: UInt64
    public let imageBase: UInt64
    public let absoluteEntryPoint: UInt64
    public let importCount: Int
    public let sectionCount: Int
    public var errorMessage: String?
}

/// PELoader is responsible for loading Windows PE32/PE32+ binaries into the macOS virtual address space.
public final class PELoader {
    
    public enum LoaderError: Error {
        case fileNotFound
        case invalidPEHeader
        case unsupportedArchitecture
        case relocationFailed
    }
    
    public init() {}
    
    /// Loads and maps a PE binary from the given file URL.
    /// - Parameter url: The URL of the .exe or .dll file.
    /// - Returns: A PELoadResult containing loading metadata.
    public func load(from url: URL) throws -> PELoadResult {
        let wrapper = PEEngineWrapper()
        
        // 1. Get PE Info
        guard let info = wrapper.getPEInfo(url.path), info.isValid else {
            throw LoaderError.invalidPEHeader
        }
        
        // 2. Map Binary into Memory
        let absoluteEntryPoint = wrapper.loadImage(url.path)
        guard absoluteEntryPoint != 0 else {
            throw LoaderError.relocationFailed
        }
        
        // 3. Resolve Imports
        // Imports are now resolved in C++ PEImage::load which is called by wrapper.loadImage
        // We'll keep this method for any Swift-side resolution if needed later.
        try self.resolveImports(for: url)
        
        return PELoadResult(
            entryPointRVA: UInt64(info.entryPointRVA),
            imageBase: info.imageBase,
            absoluteEntryPoint: absoluteEntryPoint,
            importCount: 0, // Handled in C++ logging for now
            sectionCount: info.sections.count,
            errorMessage: nil
        )
    }
    
    /// Internal helper to resolve imports for a mapped binary.
    private func resolveImports(for url: URL) throws {
        // IAT resolution now happens automatically in the C++ layer.
        // We log details there. 
        print("[PELoader] IAT resolution triggered via C++ engine path.")
    }
    
    public func resolveImports(for binary: PEBinary) throws {
        // Obsolete: Transitioning to PELoadResult
    }
}
