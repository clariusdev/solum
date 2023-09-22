//
//  Model.swift
//  Solum Example
//
//  Class to wrap the Solum framework for Swift
//

import Foundation
import CoreGraphics

enum ParameterType: UInt32, CaseIterable {
    case imageDepth = 0, gain, autoGain
}

enum PlatformType: UInt32 {
    case V1 = 0, HD, HD3

    init(rawValue: String) {
        print(rawValue)
        if rawValue.contains("HD3") {
            self = .HD3
        } else if rawValue.contains("HD") {
            self = .HD
        } else {
            self = .V1
        }
    }
}

/// Class to wrap the Solum framework for Swift
class SolumModel: ObservableObject {

    /// Available scanner types (model names)
    @Published var scannerTypes: [String]
    /// Available application IDs for the current scanner type
    @Published var applications: [String]
    /// Current scanner type
    @Published var scannerType: String {
        didSet {
            // Update the list of available applications
            if scannerType.isEmpty {
                applications = []
            } else {
                applications = solum.applications(scannerType)
            }
            // Check if the current application needs to change
            if !applications.contains(applicationID) {
                // Choose the first if any are available
                applicationID = applications.isEmpty ? "" : applications[0]
            }
        }
    }
    /// Current selected application ID
    @Published var applicationID: String
    /// Last image received from the scanner
    @Published var image: CGImage?

    /// Scanner's SSID (informational)
    @Published var ssid: String = ""
    /// Scanner's Wi-Fi direct password (informational)
    @Published var password: String = ""
    /// Scanner's IP address
    @Published var address: String = ""
    /// Scanner's TCP port
    @Published var port: UInt32 = 0
    /// Certificate to allow scanner use (provide before connecting)
    @Published var certificate: String = ""
    /// Path to the firmware update file
    @Published var firmwarePath: String = ""
    /// True if the scanner reported that a firmware update is required
    @Published var updateRequired: Bool = false
    /// Update send progress from 0.0 to 1.0
    @Published var updateProgress: Double = 0.0
    /// Send the firmware update to the current connect scanner
    func updateSoftware() {
        solum.softwareUpdate(firmwarePath, callback: { (result: CusSwUpdate) -> Void in
            print("Software update result: \(result)")
        }) { (progress: Int32) -> Void in
            self.updateProgress = Double(progress) / 100.0
        }
    }
    func optimizeWifi() {
        solum.optimizeWifi(WifiOptSearch, callback: { (result: Bool) -> Void in
            print("Wifi optimization result: \(result)")
        });
    }
    func factoryReset() {
        solum.resetProbe(ResetFactory, callback: { (result: Bool) -> Void in
            print("Factory reset result: \(result)")
        });
    }
    @Published var parameter: ParameterType = ParameterType.imageDepth
    @Published var parameterValue: Double?
    func getParam(param: CusParam, callback: @escaping (NSNumber?) -> Void) {
        solum.getParam(callback, param: param)
    }
    func setParam(param: CusParam, value: Double) {
        solum.setParam(param, value: value)
    }
    func getParam() {
        getParam(param: CusParam(parameter.rawValue)) { (value: NSNumber?) -> Void in
            self.parameterValue = value as? Double
        }
    }
    func setParam() {
        guard let value = parameterValue else {
            return
        }
        setParam(param: CusParam(parameter.rawValue), value: value)
    }
    /// Connect to the scanner at the given IP address and port
    func connectToScanner() {
        solum.setCertificate(certificate)
        solum.connect(address, port: port)
    }
    /// Disconnect from the scanner
    func disconnect() {
        solum.disconnect()
    }
    /// Load the application for the given scanner model and application ID
    func loadApplication() {
        if scannerTypes.contains(scannerType) && applications.contains(applicationID) {
            solum.loadApplication(applicationID, probe: scannerType)
        } else {
            print("Invalid probe/application combination: \(scannerType.isEmpty ? "<no probe>" : scannerType) with \(applicationID.isEmpty ? "<no application>" : applicationID)")
        }
    }
    /// Start imaging (after loading application)
    func runImaging() {
        solum.run(true)
    }
    /// Stop imaging
    func stopImaging() {
        solum.run(false)
    }
    /// Get the version of the firmware
    func getFirmwareVersion(platformType: PlatformType) {
        solum.getFirmwareVersion(CusPlatform(platformType.rawValue)) {(version: String?) -> Void in
            guard let version else {
                print("Unable to retrieve the firmware version")
                return
            }
            let userInfo = ["firmwareVersion": version]
            NotificationCenter.default.post(name: .firmwareVersion, object: nil, userInfo: userInfo)
        }
    }
    /// Solum framework instance
    private let solum = Solum()
    /// Initialize the framework and set up callbacks
    init() {
        solum.setErrorCallback({ (errorString: String) -> Void in
            print(errorString)
        })
        solum.setImagingCallback({ (state: CusImagingState, imageRunning: Int32) -> Void in
            print("Solum is \(state): \(imageRunning)")
        })
        // The framework requires a directory for storing security keys.
        // Using the application support path for this application.
        let appSupportPaths = FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask)
        let appSupportPath = appSupportPaths[0].path
        solum.initialize(appSupportPath) { (result: Bool) -> Void in
            print("Initialization \(result ? "succeeded" : "failed")")
        }
        // Get the available probe models
        let types = solum.probes()
        scannerTypes = types
        // Select the first if any are available (more for the GUI's sake)
        scannerType = types.isEmpty ? "" : types[0]
        // Get the available applications for the current probe model
        let apps = types.isEmpty ? [] : solum.applications(types[0])
        applications = apps
        // Select the first if any are available (more for the GUI's sake)
        applicationID = apps.isEmpty ? "" : apps[0]
        solum.setConnectCallback({ (status: CusConnection, listenPort: Int32, message: String) -> Void in
            print("Solum is \(status) at \(listenPort): \(message)")
            if status == ProbeDisconnected {
                self.updateRequired = false
            }
            if status == SwUpdateRequired {
                self.updateRequired = true
            }
        })
        // New images calblack
        solum.setNewProcessedImageCallback({ (imageData: Data, imageInfo: UnsafePointer<CusProcessedImageInfo>, npos: Int32, positions: UnsafePointer<CusPosInfo>) -> Void in
            // Converting the raw image data into a CGImage which can be displayed
            let nfo = imageInfo.pointee
            let rowBytes = nfo.width * nfo.bitsPerPixel / 8
            let totalBytes = Int(nfo.height * rowBytes)
            let rawBytes = UnsafeMutableRawPointer.allocate(byteCount: totalBytes, alignment: 1)
            let bmpInfo = CGImageAlphaInfo.premultipliedFirst.rawValue | CGBitmapInfo.byteOrder32Little.rawValue
            imageData.copyBytes(to: UnsafeMutableRawBufferPointer(start: rawBytes, count: totalBytes))
            guard let colorspace = CGColorSpace(name: CGColorSpace.sRGB) else {
                return
            }
            guard let ctxt = CGContext(data: rawBytes, width: Int(nfo.width), height: Int(nfo.height), bitsPerComponent: 8, bytesPerRow: Int(rowBytes), space: colorspace, bitmapInfo: bmpInfo) else {
                return
            }
            self.image = ctxt.makeImage()
        })
        // Listen for notifications from the scanners model about scanner details
        NotificationCenter.default.addObserver(forName: .scannerDetails, object: nil, queue: nil) { [weak self] notification in
            guard let self = self else {
                return
            }
            guard let userInfo = notification.userInfo else {
                return
            }
            guard let scanner = userInfo["scanner"] as? Scanner else {
                return
            }
            self.ssid = scanner.ssid
            self.password = scanner.password
            self.address = scanner.ip
            self.port = scanner.tcpPort
            self.certificate = scanner.certificate ?? ""
            if self.scannerTypes.contains(scanner.model) {
                self.scannerType = scanner.model
            }
        }
        // Listen for notifications from the cloud model about the firmware path
        NotificationCenter.default.addObserver(forName: .firmwarePath, object: nil, queue: nil) { [weak self] notification in
            guard let self = self else {
                return
            }
            guard let userInfo = notification.userInfo else {
                return
            }
            guard let firmwarePath = userInfo["firmwarePath"] as? String else {
                return
            }
            DispatchQueue.main.async(execute: { [self] in
                self.firmwarePath = firmwarePath
            })
        }
   }
}
