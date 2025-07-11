# Clarius Solum API

The Solum API allows third party applications to be developed for desktop or mobile systems that wish to connect to Clarius scanners for imaging functionality. Applications can be executed without the Clarius App running, unlike the Cast API and Mobile API tools. The API only provides connectivity and imaging control; there is no functionality for Clarius Cloud, DICOM, measurements, and other user interface technologies that are integrated into the Clarius App.

## Prerequisites

- [Clarius Ultrasound Scanner](https://clarius.com/)
- Must be a **commercial partner** purchasing bulk units on a quarterly basis
- A valid "oem" license (managed per probe)

## Supported Platforms

- Windows
- Linux
- macOS
- iOS
- Android

### Example Overview

Two desktop example programs are provided, one written as a console program, the other using Qt and a graphical interface. The console program does not use any Bluetooth connectivity, therefore it is only useful if the IP address and port are already known, or if a Bluetooth script is run through the console (for example, using hcitool or gatttool on Linux).

The iOS example program is a simple SwiftUI program that demonstrates some of the features of the framework. To build, the full iOS framework zip must be extracted to the ../../Library/Frameworks/ path or the path must be adjusted in the project settings. A signing certificate must be specified in the project settings. Ensure that the build target is iOS 64-bit arm to match the downloaded framework. The program demonstrates download certificates from Clarius cloud, populating scanner details via bluetooth, and image streaming.

### Documentation

- [Specifications](specifications.md)
- [Design](design.md)

To create a PDF from one of our markdown documents, try the pandoc program: `pandoc document.md -o document.pdf`.
In cases where mermaid is used for diagramming, also pass in: `-F mermaid-filter`, which can be installed via an npm package

## Probe Certificate

A valid probe certificate obtained from Clarius must be set before being able to connect to the probe.

How to obtain a probe certificate:

1. Create an OEM API key from the Clarius Cloud institution admin page: in institution settings > policies > OEM API keys > create a new key and write it down, it will not be shown again. This step needs to be done only once.
2. Download probes certificates at https://cloud.clarius.com/api/public/v0/devices/oem/, use the OEM API key as an authorization token:

```
curl -H "Authorization: OEM-API-Key <your key>" "https://cloud.clarius.com/api/public/v0/devices/oem/"
```


## Deploy with all dependencies of QT

[windeployqt](https://doc.qt.io/qt-6/windows-deployment.html) needs to be executed in order to copy all required QT dependencies into the directory of the executable:


For example, open a terminal where the solum.exe was build and enter the following command:

```
C:\Qt\6.6.0\msvc2019_64\bin\windeployqt.exe solum.exe
```

This copies all required dependencies into the same folder. **The solum.dll file though needs to be copied manually in addition.**

## Developer startup


### Windows

1. Download and install Qt creator
2. Ensure you use a msvc compiler (Qt.bluetooth is not supported with the mingw
   compiler)
    - I non is installed on your machine download it via the microsoft build
    tools
3. Open the `solum_qt.pro` with qt creator
4. Run the application
5. Make sure that you add the necessary certificates. You can find the
   necessary token to claim them on the clarius user website
