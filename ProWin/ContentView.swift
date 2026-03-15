//
//  ContentView.swift
//  ProWin
//
//  Created by Harinandan J V on 15/03/26.
//

import SwiftUI

struct ContentView: View {
    @ObservedObject private var gameLoop = GameLoop.shared
    @State private var statusMessage = "ProWin v0.14.0 Ready"
    
    var body: some View {
        VStack(spacing: 20) {
            Image(systemName: "cpu")
                .imageScale(.large)
                .foregroundStyle(.tint)
            
            Text(gameLoop.isRunning ? "Executing..." : statusMessage)
                .font(.headline)
            
            Button("Run Smoke Test") {
                runSmokeTest()
            }
            .buttonStyle(.borderedProminent)
            .disabled(gameLoop.isRunning)
            
            if gameLoop.isRunning {
                ProgressView("Executing test.exe...")
                    .padding()
            } else if statusMessage == "Executing..." {
                Text("Execution Finished")
                    .foregroundColor(.green)
                    .font(.subheadline)
            }
        }
        .padding()
        .frame(minWidth: 300, minHeight: 250)
        .onChange(of: gameLoop.isRunning) { running in
            if !running && statusMessage == "Executing..." {
                statusMessage = "Execution Finished"
            }
        }
    }
    
    private func runSmokeTest() {
        // Use absolute path for development/smoke test on this machine
        let testExePath = "/Users/harinandanjv/Documents/ProWIn/test.exe"
        let testExeURL = URL(fileURLWithPath: testExePath)
        
        statusMessage = "Loading test.exe..."
        
        if !FileManager.default.fileExists(atPath: testExePath) {
            statusMessage = "File not found at: \(testExePath)"
            return
        }
        
        do {
            let loader = PELoader()
            // 1. Initial Load (Metadata)
            _ = try loader.load(from: testExeURL)
            
            // 2. Execute Load (Memory Mapping)
            let entryPoint = try loader.executeLoad(from: testExeURL)
            
            // 3. Start Game Loop
            GameLoop.shared.start(entryPoint: entryPoint)
            
            statusMessage = "Executing..."
        } catch {
            statusMessage = "Test Failed: \(error.localizedDescription)"
        }
    }
}

#Preview {
    ContentView()
}
