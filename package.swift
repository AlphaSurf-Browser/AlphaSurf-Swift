// swift-tools-version:5.5
import PackageDescription

let package = Package(
    name: "AlphaSurf",
    platforms: [
        .iOS(.v15)
    ],
    products: [
        .executable(
            name: "AlphaSurf",
            targets: ["AlphaSurf"]),
    ],
    dependencies: [
        // Add any external dependencies here
    ],
    targets: [
        .executableTarget(
            name: "AlphaSurf",
            dependencies: [],
            path: "src"
        )
    ]
)
