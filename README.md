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

The OEM API communicates with the _Clarius Scanner_ directly, and makes use of Bluetooth and TCP/UDP technologies to create and maintain the wireless connection to the scanner. The Bluetooth communications is not built directly into the API and thus must be written by the developer using the library/platform of their choice. For demonstration, the Qt 5 Bluetooth module is used in the example provided in the repository - while this is cross-platform, it may not be the primary choice for all API developers.

                                      +-----------------------+
                                      |  Desktop Application  |
                                      |                       |
    +---------+                       |    +-------------+    |
    |         |   Images (via UDP)    |    |             |    |
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

Bluetooth Low Energy (BLE) is used to make the initial connection with the Clarius scanner. There are two custom services provided:
- Power Service (PWS)
- Wi-Fi Information Service (WIS)

The __PWS__ (UUID 0x8C853B6A-2297-44C1-8277-73627C8D2ABC) is a custom service built by clarius to read and manage the power status of the scanner.

Once ready, the device powered status will be published through the __Power Published__ characteristic (UUID 0x8C853B6A-2297-44C1-8277-73627C8D2ABD), and can be read at any point after a BLE connection, as well as subscribed to, and thus a notification will take place when the information has changed. The read and notifications should always be 1 byte, with 0 denoting an off state, and 1 denoting an on state.

To subscribe to the Power Published characteristic, one can write 0100 to the characteristic's Client Characteristic Configuration Descriptor (CCCD), allowing the scanner to send out notifications to the connected program.

To power on or off the device, the __Power Request__ characteristic (UUID 0x8C853B6A-2297-44C1-8277-73627C8D2ABE) can be written to. Writing 0x00 to the characteristic will power the device off, and writing 0x01 will power the device on.

The __WIS__ (UUID 0xF9EB3FAE-947A-4E5B-AB7C-C799E91ED780) is a custom service built by Clarius to read and manage the Wi-Fi network once the scanner is powered up and ready. A scanner is typically in a ready state when the LED has stopped flashing and is solid blue.

Once ready, the current Wi-Fi network information will be published through the __Wi-Fi Published__ characteristic (UUID 0xF9EB3FAE-947A-4E5B-AB7C-C799E91ED781), and can be read at any point after a BLE connection, as well as subscribed to, and thus a notification will take place when the information has changed. If the service reads back "N/A" it typically means the scanners has not finished booting to a ready state.

To subscribe to the Wi-Fi Published characteristic, one can write 0100 to the characteristic's Client Characteristic Configuration Descriptor (CCCD), allowing the scanner to send out notifications to the connected program.

The format of the read back text from the characteristic is as follows: _SSID,PASSWORD,V4_IP_ADDRESS,V6_IP_ADDRESS,TCP_CONNECTION_PORT_. Note that the password will only be sent if the network used is the scanner's own Access Point (AP). If connected to a router, the password will not be sent as it is assumed that the credentials are managed elsewhere.

To change network configurations, the __Wi-Fi Request__ characteristic (UUID 0xF9EB3FAE-947A-4E5B-AB7C-C799E91ED782) can be written to. Note that the probe must be in a ready state before the request will have any effect.

To request to put the scanner on a router (or any other external network), write to the characteristic in the following format: _SSID,PASSWORD_. To put the scanner on it's internal access point, simply send: _p2pRequest,auto_. Once the scanner has joined or launched the Wi-Fi network, the published characteristic will be subsequently written. In the event of an error, "N/A" will be published.

# Examples

Two example programs are provided, one written as a console program, the other using Qt and a graphical interface. The console program does not use any Bluetooth connectivity, therefore it is only useful if the IP address and port are already known, or if a Bluetooth script is run through the console (for example, using hcitool or gatttool on Linux).
