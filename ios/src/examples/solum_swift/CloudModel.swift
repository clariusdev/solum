//
//  CloudModel.swift
//  Solum Example
//
//  Class for communicating with Clarius cloud
//

import Foundation

/// Device struct that matches JSON format from Clarius cloud
struct DeviceResult: Codable {
    /// Scanner serial number
    var serial: String
    /// Scanner model ID
    var model: String
}

/// Scanner struct that matches JSON format from Clarius cloud
struct ScannerResult: Codable {
    /// Scanner cerificate, if available
    var crt: String?
    /// Device additional details
    var device: DeviceResult
}

/// Scanner query result struct that matches JSON format from Clarius cloud
struct ScannerResults: Codable {
    /// Number of results in this query
    var count: Int
    /// URL of the next query if the results don't fit on one page
    var next: String?
    /// Current results
    var results: [ScannerResult]
}

/// Simplified scanner information struct for results from Clarius cloud
struct ScannerCloud {
    /// Scanner serial number
    var serial: String
    /// Scanner model ID
    var model: String
    /// Scanner cerificate, if available
    var certificate: String?
}

/// Model for querying scanner results from the cloud
class CloudModel: ObservableObject {
    /// Key for caching the token for cloud queries
    private static let tokenKey = "apiToken"
    /// User storage for key caching
    let defaults = UserDefaults.standard
    /// List of scanners downloaded from the cloud
    private var downloaded: [ScannerCloud] = []
    /// Set of serials for scanners found over bluetooth
    private var found: Set<String> = []
    /// Token which will be used to query the cloud
    @Published var token: String {
        didSet {
            // Caching updated token in the storage
            defaults.set(token, forKey: CloudModel.tokenKey)
        }
    }
    /// Initialization to restore the token from storage and connect to notifications
    init() {
        token = defaults.string(forKey: CloudModel.tokenKey) ?? ""
        // Listen for notifications from the bluetooth model about found scanners
        NotificationCenter.default.addObserver(forName: .deviceFound, object: nil, queue: nil) { [weak self] notification in
            guard let self = self else {
                return
            }
            guard let userInfo = notification.userInfo else {
                return
            }
            guard let deviceFound = userInfo["device"] as? DeviceFound else {
                return
            }
            // Recording all devices found over bluetooth for later cloud queries
            self.found.insert(deviceFound.serial)
            // Send out any relevant cloud details for the scanner which was just found
            self.handleDeviceFound(serial: deviceFound.serial)
        }
    }
    /// When a scanner is found over bluetooth, notify the list model of any additional details which came from the cloud
    /// - Parameter serial: Serial of the scanner which was found over bluetooth
    private func handleDeviceFound(serial: String) {
        if let row = self.downloaded.firstIndex(where: {$0.serial == serial}) {
            let userInfo = ["scannerCloud": self.downloaded[row]]
            NotificationCenter.default.post(name: .scannerCloud, object: nil, userInfo: userInfo)
        }
    }
    /// Refresh the entire list of scanners which was
    func refreshScanners() {
        if (token.isEmpty) {
            print("No cloud token provided")
            return
        }
        // Clear out the list and begin the first download
        downloaded = []
        // Start the download at the default URL
        startDownload(location: "https://cloud.clarius.com/api/public/v0/devices/oem/?format=json")
    }
    /// Start downloading scanner details from a single URL
    /// - Parameter location: URL from which to download scanners
    private func startDownload(location: String)
    {
        guard let url = URL(string: location) else {
            print("Failed to format URL: \(location)")
            return
        }
        var request = URLRequest(url: url)
        request.httpMethod = "GET"
        // Provide the token as authorization for the request
        request.setValue("OEM-API-Key \(token)", forHTTPHeaderField: "Authorization")
        URLSession.shared.dataTask(with: request) { data, response, error in
            if let error = error {
                print("Failed to download scanners: \(error)")
            }
            guard let data = data else {
                print("Downloaded no data for scanners")
                return
            }
            // Try to parse the results from JSON
            guard let results = try? JSONDecoder().decode(ScannerResults.self, from: data) else {
                print("Failed to parse JSON from \(data)")
                return
            }
            // Informational messages (not strictly necessary)
            print("Downloaded \(results.results.count) scanners total")
            print("Downloaded \(results.results.filter { $0.crt != nil }.count) scanners with certificates")
            // Post back to the main thread for proper synchronization with the GUI
            DispatchQueue.main.async(execute: {
                // Append new results to the list
                self.downloaded.append(contentsOf: results.results.map{(scanner) -> ScannerCloud in
                    return ScannerCloud(serial: scanner.device.serial, model: scanner.device.model, certificate: scanner.crt)
                })
                // For any scanners that were already found over bluetooth, check if additional cloud details can be provided
                for serial in self.found {
                    self.handleDeviceFound(serial: serial)
                }
                // Check if any additonal results should be downloaded
                if let nextURL = results.next {
                    self.startDownload(location: nextURL)
                }
            })
        }.resume()
    }
}
