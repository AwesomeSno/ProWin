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
                
                VStack(alignment: .leading) {
                    Text("Register State")
                        .font(.caption)
                        .foregroundColor(.gray)
                    Text("RAX: 0x\(String(format: "%016llX", gameLoop.rax))")
                        .font(.system(.body, design: .monospaced))
                }
                .padding()
                .background(Color.black.opacity(0.1))
                .cornerRadius(8)
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
            // Simply delegate to GameLoop which handles loading and execution
            GameLoop.shared.start(url: testExeURL)
            statusMessage = "Executing..."
        } catch {
            statusMessage = "Test Failed: \(error.localizedDescription)"
        }
    }
}

#Preview {
    ContentView()
}
