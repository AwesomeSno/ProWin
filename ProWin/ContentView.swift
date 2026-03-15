//
//  ContentView.swift
//  ProWin
//
//  Created by Harinandan J V on 15/03/26.
//

import SwiftUI

struct ContentView: View {
    @State private var statusMessage = "ProWin v0.13.0 Ready"
    
    var body: some View {
        VStack(spacing: 20) {
            Image(systemName: "cpu")
                .imageScale(.large)
                .foregroundStyle(.tint)
            Text(statusMessage)
                .font(.headline)
            
            Button("Run Smoke Test") {
                runSmokeTest()
            }
            .buttonStyle(.borderedProminent)
            .disabled(GameLoop.shared.isRunning)
            
            if GameLoop.shared.isRunning {
                ProgressView("Executing test.exe...")
                    .padding()
            }
        }
        .padding()
        .frame(minWidth: 300, minHeight: 250)
    }
    
    private func runSmokeTest() {
        let fileManager = FileManager.default
        let currentPath = fileManager.currentDirectoryPath
        let testExeURL = URL(fileURLWithPath: currentPath).appendingPathComponent("test.exe")
        
        statusMessage = "Loading test.exe..."
        
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
