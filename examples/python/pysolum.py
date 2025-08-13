#!/usr/bin/env python

import argparse
import ctypes
import os.path
import pathlib
import sys

if sys.platform.startswith("linux"):
    libsolum_handle = ctypes.CDLL("./libsolum.so", ctypes.RTLD_GLOBAL)._handle  # load the libsolum.so shared library
    pysolum = ctypes.cdll.LoadLibrary("./pysolum.so")  # load the pysolum.so shared library

import pysolum
from PIL import Image

printStream = True
isRunning = False


## called when a new processed image is streamed
# @param image the scan-converted image data
# @param width width of the image in pixels
# @param height height of the image in pixels
# @param sz full size of image
# @param micronsPerPixel microns per pixel
# @param timestamp the image timestamp in nanoseconds
# @param angle acquisition angle for volumetric data
# @param imu inertial data tagged with the frame
def newProcessedImage(image, width, height, sz, micronsPerPixel, timestamp, angle, imu):
    bpp = sz / (width * height)
    if printStream:
        print(
            f"image: {timestamp}, {width}x{height} @ {bpp} bpp, {micronsPerPixel:.2f} um/px, imu: {len(imu)} pts",
            end="\r",
        )
    if bpp == 4:
        _img = Image.frombytes("RGBA", (width, height), image)
    else:
        _img = Image.frombytes("L", (width, height), image)
    # img.save("processed_image.png")


## called when a new raw image is streamed
# @param image the raw pre scan-converted image data, uncompressed 8-bit or jpeg compressed
# @param lines number of lines in the data
# @param samples number of samples in the data
# @param bps bits per sample
# @param axial microns per sample
# @param lateral microns per line
# @param timestamp the image timestamp in nanoseconds
# @param jpg jpeg compression size if the data is in jpeg format
# @param rf flag for if the image received is radiofrequency data
# @param angle acquisition angle for volumetric data
def newRawImage(image, lines, samples, bps, axial, lateral, timestamp, jpg, rf, angle):
    # check the rf flag for radiofrequency data vs raw grayscale
    # raw grayscale data is non scan-converted and in polar co-ordinates
    # print(
    #    "raw image: {0}, {1}x{2} @ {3} bps, {4:.2f} um/s, {5:.2f} um/l, rf: {6}".format(
    #        timestamp, lines, samples, bps, axial, lateral, rf
    #    ), end = "\r"
    # )
    # if jpg == 0:
    #    img = Image.frombytes("L", (samples, lines), image, "raw")
    # else:
    #    # note! this probably won't work unless a proper decoder is written
    #    img = Image.frombytes("L", (samples, lines), image, "jpg")
    # img.save("raw_image.jpg")
    return


## called when a new spectrum image is streamed
# @param image the spectral image
# @param lines number of lines in the spectrum
# @param samples number of samples per line
# @param bps bits per sample
# @param period line repetition period of spectrum
# @param micronsPerSample microns per sample for an m spectrum
# @param velocityPerSample velocity per sample for a pw spectrum
# @param pw flag that is true for a pw spectrum, false for an m spectrum
def newSpectrumImage(image, lines, samples, bps, period, micronsPerSample, velocityPerSample, pw):
    return


## called when a new imu data is streamed
# @param imu inertial data tagged with the frame
def newImuData(imu):
    return


## called when the connection state changes
# @param res the connection result
# @param port the udp connection port
# @param msg informative message
def connectFn(res, port, msg):
    print(f"connection state changed: {res} [{msg}]")


## called after a certificate was sent for validation
# @param daysValid the # of days the passed certificate is valid for
def certFn(daysValid):
    print(f"certificate is valid for {daysValid} days")


## called when the power down state changes
# @param state the power down state
# @param tm the time the probe will shut down in
def powerDownFn(state, tm):
    print(f"power down state changed: {state}, in {tm}s]")


## called after imaging state has changed
# @param state the imaging state
# @param imaging the imaging flag, 0=stopped, 1=running
def imagingFn(state, imaging):
    print(f"imaging state changed: {state}, running: {imaging}")
    global isRunning
    isRunning = imaging


## called when a button is pressed
# @param button the button that was pressed
# @param clicks number of clicks performed
def buttonFn(button, clicks):
    print(f"button pressed: {button}, clicks: {clicks}")


## called when an error occurs
# @param code the error code
# @param msg corresponding error message
def errorFn(code, msg):
    print(f"error: {code}, {msg}")


## main function
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--address", "-a", dest="ip", help="ip address of probe.", required=True)
    parser.add_argument("--port", "-p", dest="port", type=int, help="port of the probe", required=True)
    parser.add_argument("--width", "-w", dest="width", type=int, help="image output width in pixels")
    parser.add_argument("--height", "-ht", dest="height", type=int, help="image output height in pixels")
    parser.set_defaults(ip=None)
    parser.set_defaults(port=None)
    parser.set_defaults(width=640)
    parser.set_defaults(height=480)
    args = parser.parse_args()

    # uncomment to get documentation for pysolum module
    # print(help(pysolum))

    if not args.ip or not args.port or args.port < 0:
        print("one or more arguments are invalid")
        parser.print_usage()
        return

    # get home path
    path = os.path.expanduser("~/")

    # initialize
    solum = pysolum.Solum(
        newProcessedImage,
        newRawImage,
        newSpectrumImage,
        newImuData,
        connectFn,
        certFn,
        powerDownFn,
        imagingFn,
        buttonFn,
        errorFn,
    )
    ret = solum.init(path, args.width, args.height)
    if ret:
        print("initialization succeeded")
        ret = solum.connect(args.ip, args.port)
        if ret:
            print(f"trying to connect to {args.ip} on port {args.port}")
        else:
            print("connection failed")
    else:
        print("initialization failed")
        return

    # input loop
    key = ""
    while key != "q" and key != "Q":
        key = input(
            "options:\n(c)->connect\n(d)->disconnect\n(f)->firmware version\n(p)->list (p)robes\n(a)->list (a)pplications\n(t)->load cer(t)ificate\n"
            "(l)->load application\n(r)->run/stop\n(g)->get parameter value\n(v)->set parameter (v)alue\n(#)->toggle console imaging stream\n(q)->quit\n",
        )
        if key == "c" or key == "C":
            inp = input("enter: {ip address} {port}").split()
            if len(inp) != 2:
                print("please format as: {ip address} {port}")
            else:
                ret = solum.connect(inp[0], int(inp[1]))
                if ret:
                    print("trying to connect")
                else:
                    print("connection failed")
        elif key == "d" or key == "D":
            ret = solum.disconnect()
            if ret:
                print("successful disconnect")
            else:
                print("disconnection failed")
        elif key == "f" or key == "F":
            ret = solum.firmwareVersion(2)
            print(f"firmware version: {ret}")
        elif key == "p" or key == "P":
            ret = solum.probes()
            print(f"probes: {ret}")
        elif key == "a" or key == "A":
            inp = input("provide probe name:").split()
            if len(inp) != 1:
                print("please provide a probe name")
            else:
                ret = solum.applications(inp[0])
                print(f"applications: {ret}")
        elif key == "t" or key == "T":
            inp = input("provide certificate file:").split()
            if len(inp) != 1:
                print("please provide a certificate file")
            else:
                f = pathlib.Path(inp[0])
                if f.exists():
                    file = open(f)
                    cert = file.read()
                    print(f"certificate:\n{cert}")
                    file.close()
                    solum.setCertificate(cert)
        elif key == "l" or key == "L" or key == "l" or key == "L":
            inp = input("enter: {probe} {application}").split()
            if len(inp) != 2:
                print("please format as: {probe} {application}")
            else:
                solum.loadApplication(inp[0], inp[1])
        elif key == "r" or key == "R":
            global isRunning
            if isRunning:
                solum.stop()
            else:
                solum.run()
        elif key == "v" or key == "V":
            inp = input("enter: {parameter id} {value [float/true/false]}").split()
            if len(inp) != 2:
                print("please format as: {parameter id} {value [float/true/false]}")
            elif inp[1] == "true" or inp[1] == "false":
                solum.setParameter(int(inp[0]), 1 if inp[1] == "true" else 0)
            else:
                solum.setParameter(int(inp[0]), float(inp[1]))
        elif key == "g" or key == "G":
            inp = input("enter: {parameter id}").split()
            if len(inp) != 1:
                print("please format as: {parameter id}")
            else:
                print(f"value: {solum.getParameter(int(inp[0]))}")
        elif key == "#":
            global printStream
            printStream = not printStream

    if sys.platform.startswith("linux"):
        # unload the shared library before destroying the solum object
        ctypes.CDLL("libc.so.6").dlclose(libsolum_handle)
    solum.destroy()


if __name__ == "__main__":
    main()
