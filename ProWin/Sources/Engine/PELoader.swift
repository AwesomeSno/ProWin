import Foundation

/// PEBinary defines the structure of a loaded Windows executable.
public struct PEBinary {
    public let name: String
    public let baseAddress: UInt64
    public let entryPoint: UInt64
    public let is64Bit: Bool
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
    
    /// Loads a PE binary from the given file URL.
    /// - Parameter url: The URL of the .exe or .dll file.
    /// - Returns: A loaded PEBinary instance.
    public func load(from url: URL) throws -> PEBinary {
        let wrapper = PEEngineWrapper()
        guard let info = wrapper.getPEInfo(url.path) else {
            throw LoaderError.fileNotFound
        }
        
        guard info.isValid else {
            throw LoaderError.invalidPEHeader
        }
        
        return PEBinary(
            name: url.lastPathComponent,
            baseAddress: info.imageBase,
            entryPoint: info.imageBase + UInt64(info.entryPointRVA),
            is64Bit: info.is64Bit
        )
    }
    
    /// Resolves the Import Address Table for the given binary.
    /// - Parameter binary: The binary to resolve imports for.
    public func resolveImports(for binary: PEBinary) throws {
        // TODO: Implement IAT resolution
        fatalError("Not yet implemented")
    }
}
