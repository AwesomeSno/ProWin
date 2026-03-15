import SwiftUI
import MetalKit

struct MetalViewRepresentable: NSViewRepresentable {
    func makeNSView(context: Context) -> MTKView {
        return GraphicsManager.shared.mtkView ?? MTKView()
    }
    
    func updateNSView(_ nsView: MTKView, context: Context) {
    }
}

struct GameContentView: View {
    @ObservedObject var graphicsManager = GraphicsManager.shared
    
    var body: some View {
        ZStack {
            MetalViewRepresentable()
                .edgesIgnoringSafeArea(.all)
            
            VStack {
                Text("ProWin Metal Graphics Engine")
                    .font(.caption)
                    .padding(8)
                    .background(Color.black.opacity(0.5))
                    .cornerRadius(8)
                    .padding()
                Spacer()
            }
        }
        .frame(minWidth: 800, minHeight: 600)
    }
}
