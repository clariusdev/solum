Clarius OEM API
===============

The OEM API allows third party applications to be developed for desktop systems that wish to connect to Clarius scanners for imaging functionality. Applications can be executed without the Clarius App running, unlike the Cast API and Mobile API tools. The API only provides connectivity and imaging control; there is no functionality for Clarius Cloud, DICOM, measurements, and other user interface technologies that are integrated into the Clarius App.

# Prerequisites

- [Clarius Ultrasound Scanner](https://clarius.com/)
- A valid OEM API license (managed per scanner)

# Supported Platforms

- Windows 10 (64 bit)
- Ubuntu Linux (20.04)
- macOS 10.15 / 11.0

# Architecture

The OEM API communicates with the _Clarius Scanner_ directly, and makes use of Bluetooth and TCP technologies to create and maintain the wireless connection to the scanner. The Bluetooth communications is not built directly into the API and thus must be written by the developer using the library/platform of their choice. For demonstration, the Qt 5 Bluetooth module is used in the example provided in the repository - while this is cross-platform, it may not be the primary choice for all API developers.

                                      +-----------------------+
                                      |  Desktop Application  |
                                      |                       |
    +---------+                       |    +-------------+    |
    |         |   Images (via TCP)    |    |             |    |
    |  Probe  +--------------------------->+   OEM API   |    |
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

Bluetooth Low Energy (BLE) is used to make the initial connection with the Clarius scanner. There are two services provided, one of which Clarius 'hijacks' from the Bluetooth SIG, these include:
- Immediate Alert Service (IAS)
- Wi-Fi Information Service (WIS)

The __IAS__ (UUID 0x1802) is typically used to send an alert to a BLE device, with an __Alert Level__ characteristic (UUID 0x2A06) that can be written to either 01 or 02 depending on the alert level. The OEM API hijacks this service to control the power on/off of the scanner. Writing 02 to the Alert Level will turn the device on, and writing 01 will power the device off.

The __WIS__ (UUID 0xF9EB3FAE-947A-4E5B-AB7C-C799E91ED780) is a custom service built by Clarius to read and manage the Wi-Fi network once the scanner is powered up and ready. A scanner is typically in a ready state when the LED has stopped flashing and is solid blue.

Once ready, the current Wi-Fi network information will be published through the __Wi-Fi Published__ characteristic (UUID 0xF9EB3FAE-947A-4E5B-AB7C-C799E91ED781), and can be read at any point after a BLE connection, as well as subscribed to, and thus a notification will take place when the information has changed. If the service reads back "N/A" it typically means the scanners has not finished booting to a ready state.

To subscribe to the Wi-Fi Published characteristic, one can write 0100 to the characteristic's Client Characteristic Configuration Descriptor (CCCD), allowing the scanner to send out notifications to the connected program.

The format of the read back text from the characteristic is as follows: _SSID,PASSWORD,V4_IP_ADDRESS,V6_IP_ADDRESS,TCP_CONNECTION_PORT_. Note that the password will only be sent if the network used is the scanner's own Access Point (AP). If connected to a router, the password will not be sent as it is assumed that the credentials are managed elsewhere.

To change network configurations, the __Wi-Fi Request__ characteristic (UUID 0xF9EB3FAE-947A-4E5B-AB7C-C799E91ED782) can be written to. Note that the probe must be in a ready state before the request will have any effect.

To request to put the scanner on a router (or any other external network), write to the characteristic in the following format: _SSID,PASSWORD_. To put the scanner on it's internal access point, simply send: _p2pRequest,auto_. Once the scanner has joined or launched the Wi-Fi network, the published characteristic will be subsequently written. In the event of an error, "N/A" will be published.

# Examples

Two example programs are provided, one written as a console program, the other using Qt and a graphical interface. The console program does not use any Bluetooth connectiviity, therefore it is only useful if the IP address and port are already known, or if a Bluetooth script is run through the console (for example, using hcitool or gatttool on Linux).
