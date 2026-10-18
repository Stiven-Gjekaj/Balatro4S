// Draws an app icon as iOS 6 shows it: the corners rounded, and the gloss
// over the top half. The PNG then looks the same in Cydia and on the home
// screen, so Info.plist sets UIPrerenderedIcon and iOS adds no second gloss.
// Usage: swift scripts/icon-gloss.swift SOURCE.png OUTPUT.png SIZE
import CoreGraphics
import Foundation
import ImageIO
import UniformTypeIdentifiers

let args = CommandLine.arguments
guard args.count == 4, let size = Int(args[3]),
      let source = CGImageSourceCreateWithURL(URL(fileURLWithPath: args[1]) as CFURL, nil),
      let image = CGImageSourceCreateImageAtIndex(source, 0, nil) else {
    FileHandle.standardError.write("usage: swift icon-gloss.swift SOURCE.png OUTPUT.png SIZE\n".data(using: .utf8)!)
    exit(1)
}

let s = CGFloat(size)
let space = CGColorSpaceCreateDeviceRGB()
let context = CGContext(data: nil, width: size, height: size, bitsPerComponent: 8, bytesPerRow: 0, space: space,
                        bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue)!
context.interpolationQuality = .high

// The mask of iOS 6: a radius of 10 points on a 57 point icon.
let rect = CGRect(x: 0, y: 0, width: s, height: s)
let radius = s * 10 / 57
context.addPath(CGPath(roundedRect: rect, cornerWidth: radius, cornerHeight: radius, transform: nil))
context.clip()
context.draw(image, in: rect)

// The gloss: a wide ellipse over the top half, white at the top that
// fades down to its lower edge. The y axis of CoreGraphics goes up.
context.saveGState()
let ellipse = CGRect(x: -s * 0.5, y: s * 0.48, width: s * 2, height: s * 1.4)
context.addEllipse(in: ellipse)
context.clip()
let colours = [CGColor(gray: 1, alpha: 0.55), CGColor(gray: 1, alpha: 0.12)] as CFArray
let gradient = CGGradient(colorsSpace: CGColorSpaceCreateDeviceGray(), colors: colours, locations: [0, 1])!
context.drawLinearGradient(gradient, start: CGPoint(x: 0, y: s), end: CGPoint(x: 0, y: s * 0.48), options: [])
context.restoreGState()

let output = context.makeImage()!
let destination = CGImageDestinationCreateWithURL(URL(fileURLWithPath: args[2]) as CFURL,
                                                  UTType.png.identifier as CFString, 1, nil)!
CGImageDestinationAddImage(destination, output, nil)
CGImageDestinationFinalize(destination)
