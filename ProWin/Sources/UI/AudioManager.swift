import AVFoundation
import Foundation

/// AudioManager handles the macOS audio subsystem.
/// It bridges Windows XAudio2/DirectSound calls to native AVAudioEngine.
public final class AudioManager: NSObject {
    
    public static let shared = AudioManager()
    
    private let audioEngine = AVAudioEngine()
    private let playerNode = AVAudioPlayerNode()
    
    private override init() {
        super.init()
        setupAudioEngine()
    }
    
    private func setupAudioEngine() {
        audioEngine.attach(playerNode)
        
        let mainMixer = audioEngine.mainMixerNode
        audioEngine.connect(playerNode, to: mainMixer, format: mainMixer.outputFormat(forBus: 0))
        
        do {
            try audioEngine.start()
            print("[ProWin] Audio Engine started successfully")
        } catch {
            print("[ProWin] Could not start Audio Engine: \(error.localizedDescription)")
        }
    }
    
    /// Called when the Windows game sends audio data.
    public func queueAudioBuffer(_ data: Data, channels: Int, sampleRate: Double) {
        // TODO: Implement PCM buffer queuing from raw memory data
        if !playerNode.isPlaying {
            playerNode.play()
        }
    }
    
    public func stopAllAudio() {
        playerNode.stop()
        audioEngine.stop()
    }
}
