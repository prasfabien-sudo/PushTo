// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "DobsonNativePlugin",
    platforms: [.iOS(.v15)],
    products: [
        .library(name: "DobsonCapacitorDobson", targets: ["DobsonNativePlugin"])
    ],
    dependencies: [
        .package(url: "https://github.com/ionic-team/capacitor-swift-pm.git", exact: "8.6.0")
    ],
    targets: [
        .target(
            name: "DobsonNativePlugin",
            dependencies: [
                .product(name: "Capacitor", package: "capacitor-swift-pm")
            ],
            path: "ios/Sources/DobsonNativePlugin"
        )
    ]
)
