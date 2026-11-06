import AVFoundation
import AppKit
let args = CommandLine.arguments
let asset = AVURLAsset(url: URL(fileURLWithPath: args[1]))
let fps = Int(args[3])!
let count = Int(args[4])!
let generator = AVAssetImageGenerator(asset: asset)
generator.requestedTimeToleranceBefore = .zero
generator.requestedTimeToleranceAfter = .zero
for i in 0..<count {
    let time = CMTime(value: Int64(i), timescale: Int32(fps))
    let image = try generator.copyCGImage(at: time, actualTime: nil)
    let png = NSBitmapImageRep(cgImage: image).representation(using: .png, properties: [:])!
    try png.write(to: URL(fileURLWithPath: args[2]).appendingPathComponent(String(format: "%03d.png", i)))
}
