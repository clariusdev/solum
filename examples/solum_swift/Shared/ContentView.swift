//
//  ContentView.swift
//  Shared
//
//  Main application view
//

import SwiftUI

struct ContentView: View {
    // Class instances
    @EnvironmentObject private var solum: SolumModel
    @EnvironmentObject private var bluetooth: BluetoothModel
    @EnvironmentObject private var cloud: CloudModel
    @EnvironmentObject private var scannerModel: ScannersModel
    // Current Wi-Fi request details
    @State private var directRequest: Bool = true
    @State private var ssidRequest: String = ""
    @State private var passwordRequest: String = ""
    // Display the latest image received, if any
    func displayImage() -> Image {
        if solum.image != nil {
            return Image(solum.image!, scale: 1.0, label: Text("Ultrasound"))
        } else {
            return Image("BlankImage")
        }
    }
    var body: some View {
        ScrollView {
            VStack(spacing: 0) {
                GroupBox(label: Label("Cloud", systemImage: "cloud").font(.title3)) {
                    HStack {
                        Text("API Token")
                        TextField("API Token", text: $cloud.token)
                    }
                    Button(action: cloud.refreshScanners) {
                        Label("Download Scanner Info", systemImage: "icloud.and.arrow.down")
                    }
                    Button(action: {() -> Void in
                        solum.getFirmwareVersion(platformType: PlatformType(rawValue: solum.scannerType))
                    }) {
                        Label("Download Matching Firmware", systemImage: "icloud.and.arrow.down")
                    }
                }.padding()
                GroupBox(label: Label("Bluetooth", systemImage: "antenna.radiowaves.left.and.right").font(.title3)) {
                    Toggle(isOn: $bluetooth.scanActive) {
                        Label("Searching", systemImage: "magnifyingglass")
                    }
                    ScannerList(scanners: scannerModel.scanners, selectedSerial: bluetooth.selectedSerial)
                    HStack {
                        Button(action: bluetooth.powerToggleScanner) {
                            Label("Toggle Selected Scanner Power", systemImage: "power")
                        }
                        Button(action: {
                            if bluetooth.selectedSerial.isEmpty {
                                print("Cannot load details when no scanner selected")
                                return
                            }
                            scannerModel.sendDetails(serial: bluetooth.selectedSerial)
                        }) {
                            Label("Load Selected Scanner Details", systemImage: "square.and.arrow.down.on.square")
                        }
                    }
                    GroupBox(label: Label("Wi-Fi Request", systemImage: "wifi")) {
                        Toggle("Wi-Fi Direct", isOn: $directRequest)
                        if (!directRequest) {
                            HStack {
                                Text("SSID")
                                TextField("SSID", text: $ssidRequest)
                            }
                            HStack {
                                Text("Password")
                                TextField("Password", text: $passwordRequest)
                            }
                        }
                        Button(action: {
                            let ssid: String? = directRequest ? ssidRequest : nil
                            let password: String? = directRequest ? passwordRequest : nil
                            bluetooth.requestWifi(wifiDirect: directRequest, ssid: ssid, password: password)
                        }) {
                            Text("Send Request Over Bluetooth")
                        }

                    }
                }.padding()
                GroupBox(label: Label("Wi-Fi", systemImage: "wifi").font(.title3)) {
                    VStack {
                        HStack {
                            Text("SSID")
                            TextField("SSID", text: $solum.ssid)
                        }
                        HStack {
                            Text("Password")
                            TextField("Password", text: $solum.password)
                        }
                        HStack {
                            Text("IP Address")
                            TextField("IP Address", text: $solum.address)
                        }
                        HStack {
                            Text("Port")
                            TextField("Port", value: $solum.port, format: .number)
                        }
                        HStack {
                            Text("Certificate")
                            TextField("Certificate", text: $solum.certificate)
                        }
                        HStack {
                            Button(action: solum.connectToScanner) {
                                Text("Connect")
                            }
                            Button(action: solum.disconnect) {
                                Text("Disconnect")
                            }
                        }
                    }
                }.padding()
                GroupBox(label: Label("Status", systemImage: "wrench").font(.title3)) {
                    VStack {
                        HStack {
                            Text("Firmware Update is\(solum.updateRequired ? "" : " Not") Required")
                            Button(action: solum.updateSoftware) {
                                Text("Update Firmware")
                            }
                        }
                        ProgressView(value: solum.updateProgress)
                        Button(action: solum.optimizeWifi) {
                                Text("Optimize Wifi")
                        }
                        Button(action: solum.factoryReset) {
                                Text("Factory Reset")
                        }
                    }
                }.padding()
                GroupBox(label: Label("Imaging", systemImage: "waveform").font(.title3)) {
                    VStack {
                        HStack {
                            Text("Parameter")
                            Picker("", selection: $solum.parameter) {
                                ForEach(ParameterType.allCases, id: \.self) { paramterType in
                                    Text(String(describing: paramterType))
                                }
                            }
                            TextField("", value: $solum.parameterValue, format: .number)
                            Button(action: solum.getParam) {
                                Text("Get")
                            }
                            Button(action: solum.setParam) {
                                Text("Set")
                            }
                        }
                        HStack {
                            Text("Probe Model")
                            Picker("", selection: $solum.scannerType) {
                                ForEach(solum.scannerTypes, id: \.self) { scannerType in
                                    Text(scannerType)
                                }
                            }
                        }
                        HStack {
                            Text("Application")
                            Picker("", selection: $solum.applicationID) {
                                ForEach(solum.applications, id: \.self) { applicationID in
                                    Text(applicationID)
                                }
                            }
                        }
                        HStack {
                            Button(action: solum.loadApplication) {
                                Text("Load Application")
                            }
                            Button(action: solum.runImaging) {
                                Text("Run Imaging")
                            }
                            Button(action: solum.stopImaging) {
                                Text("Stop Imaging")
                            }
                        }
                        displayImage()
                            .resizable()
                            .aspectRatio(contentMode: .fit)
                    }
                }.padding()
            }
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
            .environmentObject(SolumModel())
            .environmentObject(BluetoothModel())
            .environmentObject(CloudModel())
            .environmentObject(ScannersModel())
    }
}
