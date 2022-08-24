Clarius Solum API
===============

The Solum API allows third party applications to be developed for desktop systems that wish to connect to Clarius scanners for imaging functionality. Applications can be executed without the Clarius App running, unlike the Cast API and Mobile API tools. The API only provides connectivity and imaging control; there is no functionality for Clarius Cloud, DICOM, measurements, and other user interface technologies that are integrated into the Clarius App.

# Prerequisites

- [Clarius Ultrasound Scanner](https://clarius.com/)
- A valid "oem" license (managed per scanner)

# Supported Platforms

- Windows 10 (64 bit)
- Ubuntu Linux (20.04 / 22.04)
- macOS 10.15 / 11.0

# Architecture

The API communicates with the _Clarius Scanner_ directly, and makes use of Bluetooth and TCP/UDP technologies to create and maintain the wireless connection to the scanner. The Bluetooth communications is not built directly into the API and thus must be written by the developer using the library/platform of their choice. For demonstration, the Qt Bluetooth module is used in the example provided in the repository - while this is cross-platform, it may not be the primary choice for all API developers.

                                      +-----------------------+
                                      |  Desktop Application  |
                                      |                       |
    +---------+                       |    +-------------+    |
    |         |   Images (via UDP)    |    |             |    |
    |  Probe  +--------------------------->+  Solum API  |    |
    |         |   Control (via TCP)   |    |             |    |
    |         +<-------------------------->+             |    |
    |         | Wi-Fi / Power Control |    |             |    |
    |         +<--------------+       |    +-------------+    |
    |         |               |       |                       |
    +---------+               +-------|--->+-------------+    |
                                      |    |  BLE Module |    |
                                      |    +-------------+    |
                                      |                       |
                                      +-----------------------+

# Bluetooth

Bluetooth Low Energy (BLE) is used to make the initial connection with the Clarius scanner. There are two custom services provided, which are further broken down into specific characteristics:
* Power Service __(PWS)__ _(UUID 0x8C853B6A-2297-44C1-8277-73627C8D2ABC)_
  * Power Published Characteristic _(UUID 0x8C853B6A-2297-44C1-8277-73627C8D2ABD)_
  * Power Request Characteristic _(UUID 0x8C853B6A-2297-44C1-8277-73627C8D2ABE)_
* Wi-Fi Information Service __(WIS)__ _(UUID 0xF9EB3FAE-947A-4E5B-AB7C-C799E91ED780)_
  * Wi-Fi Published Characteristic _(UUID 0xF9EB3FAE-947A-4E5B-AB7C-C799E91ED781)_
  * Wi-Fi Request Characteristic _(UUID 0xF9EB3FAE-947A-4E5B-AB7C-C799E91ED782)_  

## Power Service

The __PWS__ is a custom service built by clarius to read and manage the power status of the scanner.

### Power Published Characteristic
Once ready, the device powered status will be published through the _Power Published_ characteristic, and can be read at any point after a BLE connection, as well as subscribed to, and thus a notification will take place when the information has changed. The read and notifications should always be 1 byte, with 0 denoting an off state, and 1 denoting an on state.

To subscribe to the Power Published characteristic, one can write 0100 to the characteristic's Client Characteristic Configuration Descriptor (CCCD), allowing the scanner to send out notifications to the connected program.

### Power Request Characteristic
To power on or off the device, the _Power Request_ characteristic can be written to. Writing 0x00 to the characteristic will power the device off, and writing 0x01 will power the device on.

## Wi-Fi Information Service
The __WIS__ is a custom service built by Clarius to read and manage the Wi-Fi network once the scanner is powered up and ready. A scanner is typically in a ready state when the LED has stopped flashing and is solid blue.

### Wi-Fi Published Characteristic
Once ready, the current Wi-Fi network information will be published through the _Wi-Fi Published_ characteristic, and can be read at any point after a BLE connection, as well as subscribed to, and thus a notification will take place when the information has changed. If the service reads back "N/A" it typically means the scanners has not finished booting to a ready state.

To subscribe to the Wi-Fi Published characteristic, one can write 0100 to the characteristic's Client Characteristic Configuration Descriptor (CCCD), allowing the scanner to send out notifications to the connected program.

The format of the read back text from the characteristic is as follows:

If the scanner is still booting up and the Wi-Fi network is not yet ready:
```
state: disabled
```

If the Wi-Fi network is in AP mode and ready for connection:
```
state: connected
ap: true
ssid: DIRECT-<probe serial number>
pw: <probe network password>
ip4: 192.168.1.1
ctl: <control port>
cast: <casting port>
avail: <'available' if available for control, 'listen' if available for casting, 'not available' if not available for any connection>
channel: <wifi channel>
mac: <mac address>
```
Note that the password will only be sent if the network used is the scanner's own Access Point (AP). If connected to a router, the password will not be sent as it is assumed that the credentials are managed elsewhere.

### Wi-Fi Request Characteristic
To change network configurations, the _Wi-Fi Request_ characteristic can be written to. Note that the probe must be in a ready state before the request will have any effect.

To put the scanner on it's internal access point, simply send:
```
ap: true
ch: <'auto' or channel number>
```

To request to put the scanner on a router (or any other external network), write to the characteristic in the following format:
```
ap: false
ssid: <network ssid>
password: <network password>
```

Once the scanner has joined or launched the Wi-Fi network, the published characteristic will be subsequently written.

# Authentication

The Solum API requires the scanner be authenticated once a connection has been made, otherwise loading applications, firmware updates, and imaging cannot be performed. Clarius Cloud offers a new mechansim for retrieving authentication certificates for each scanner.

The first step is to generate a token which will allow software to access a dedicated endpoint. These tokens should be treated securely, as they essentially provide similar credentials to a user login. To generate a new token:
* Login to Clarius Cloud with an Administrator account
* Go to **Institution Settings**
* Choose **Policies** from the menu
* Choose **OEM API Keys**
* Choose **Add New Key**
* Store that key somewhere safe

Keys can also be revoked through the same interface. If you feel your token was compromised, revoking, creating a new key, and then issuing a security update for your software is the proper pathway to address the situation.

Valid tokens should be used to access a the [REST API endpoint](https://cloud.clarius.com/api/public/v0/devices/oem/) built for certificate (and other meta data) retrieval for each probe within your institution. Only "oem" licensed probes will have a certificate attached. Once parsed, supply the certificate corresponding to the connected probe through the __cusOemSetCert__ function.

# Core Functionality

The API provides all the functionality required to builidl a fully capable ultrasound interface, including the ability to: select imaging modes, change parameters, access images and raw data in real-time, obtain device status information, and change device settings.

## Imaging Modes

The following imaging modes are currently supported through the API:
* B-mode: base grayscale imaging for the majority of use cases
* Spatial Compounding: multi-frame grayscale acquisition to enhance certain anatomy and smooth the image
* Color Doppler: directional flow information that is overlaid on the grayscale image
* Power Doppler: similar to color Dopploer, but without directional information, showing just the power (relative intensity) of the flow
* Needle Enhance: multi-frame grayscale acquisition that will try and highlight an in-plane needle shaft when detected through special algorithms
* RF Capture: raw radiofrequency signals interleaved with grayscale frames

## Imaging Parameters

The following imaging parameters are supported for various imaging modes:
* Gain/Brightness: overall brightness of the image, adjusts a post-processing gamma curve
* Auto Gain: option to use the automated gain algorithm for adjusting the analog circuitry over the depth of the image to normalize the tissue brightness
* TGC: three sliders that control digital gain applied to the raw signal over the depth of the image
* Dynamic Range: compression curve adjustment for a more or less contrast image
* Chroma: colorized map to apply to the grayscale image
* Smooth: option to change the smooth filtering
* Trapezoidal: option to engage a wider field of view for linear array scanners
* Color ROI: the region of interest for obtaining the Doppler signal within the grayscale image
* Color Gain: analog gain adjustment for the Doppler signal
* Color PRF: velocity range adjustment for the Doppler signal
* Color Steer: steering angle adjustment for optimizing flow profiles
* Color Invert: option to reverse the red and blue correlation to flow direction
* Needle Side: option for changing the side for which the algorithm is analyzing the needle
* Eco Mode: option for engaging a power saving mode while imaging

## Status Information

The probe status can be retrieved at any time and will provide the following information:
* Battery level
* Temperature level
* Frame rate (if imaging)
* Fan status (attached, detached, running)
* Charging status

# Examples

Two example programs are provided, one written as a console program, the other using Qt and a graphical interface. The console program does not use any Bluetooth connectivity, therefore it is only useful if the IP address and port are already known, or if a Bluetooth script is run through the console (for example, using hcitool or gatttool on Linux).
