#!/usr/bin/env python

import ctypes
import os.path
import sys
from pathlib import Path
from typing import Final

from PySide6 import QtCore, QtGui, QtWidgets
from PySide6.QtCore import Slot

isRunning = False

if sys.platform.startswith("linux"):
    libsolum_handle = ctypes.CDLL("./libsolum.so", ctypes.RTLD_GLOBAL)._handle  # load the libsolum.so shared library
    pysolum = ctypes.cdll.LoadLibrary("./pysolum.so")  # load the pysolum.so shared library

import pysolum

DEPTH: Final = 0
GAIN: Final = 1


# custom event for handling change in connect state
class ConnectEvent(QtCore.QEvent):
    def __init__(self, res, port, msg):
        super().__init__(QtCore.QEvent.User)
        self.res = res
        self.port = port
        self.msg = msg


# custom event for handling certificate validations
class CertEvent(QtCore.QEvent):
    def __init__(self, daysValid):
        super().__init__(QtCore.QEvent.Type(QtCore.QEvent.User + 1))
        self.daysValid = daysValid


# custom event for handling power downs
class PowerDownEvent(QtCore.QEvent):
    def __init__(self, state, tm):
        super().__init__(QtCore.QEvent.Type(QtCore.QEvent.User + 2))
        self.state = state
        self.tm = tm


# custom event for handling change in imaging states
class ImagingEvent(QtCore.QEvent):
    def __init__(self, state, imaging):
        super().__init__(QtCore.QEvent.Type(QtCore.QEvent.User + 3))
        self.state = state
        self.imaging = imaging


# custom event for handling button presses
class ButtonEvent(QtCore.QEvent):
    def __init__(self, btn, clicks):
        super().__init__(QtCore.QEvent.Type(QtCore.QEvent.User + 4))
        self.btn = btn
        self.clicks = clicks


# custom event for handling error codes
class ErrorEvent(QtCore.QEvent):
    def __init__(self, code, msg):
        super().__init__(QtCore.QEvent.Type(QtCore.QEvent.User + 5))
        self.code = code
        self.msg = msg


# custom event for handling new images
class ImageEvent(QtCore.QEvent):
    def __init__(self):
        super().__init__(QtCore.QEvent.Type(QtCore.QEvent.User + 6))


# manages custom events posted from callbacks, then relays as signals to the main widget
class Signaller(QtCore.QObject):
    conn = QtCore.Signal(int, int, str)
    cert = QtCore.Signal(int)
    powerdown = QtCore.Signal(int, int)
    imaging = QtCore.Signal(int, int)
    button = QtCore.Signal(int, int)
    err = QtCore.Signal(int, str)
    image = QtCore.Signal(QtGui.QImage)

    def __init__(self):
        QtCore.QObject.__init__(self)
        self.usimage = QtGui.QImage()

    def event(self, evt):
        if evt.type() == QtCore.QEvent.User:
            self.conn.emit(evt.res, evt.port, evt.msg)
        elif evt.type() == QtCore.QEvent.Type(QtCore.QEvent.User + 1):
            self.cert.emit(evt.daysValid)
        elif evt.type() == QtCore.QEvent.Type(QtCore.QEvent.User + 2):
            self.powerdown.emit(evt.state, evt.tm)
        elif evt.type() == QtCore.QEvent.Type(QtCore.QEvent.User + 3):
            self.imaging.emit(evt.state, evt.imaging)
        elif evt.type() == QtCore.QEvent.Type(QtCore.QEvent.User + 4):
            self.button.emit(evt.btn, evt.clicks)
        elif evt.type() == QtCore.QEvent.Type(QtCore.QEvent.User + 5):
            self.err.emit(evt.code, evt.msg)
        elif evt.type() == QtCore.QEvent.Type(QtCore.QEvent.User + 6):
            self.image.emit(self.usimage)
        return True


# global required for the solum api callbacks
signaller = Signaller()


# draws the ultrasound image
class ImageView(QtWidgets.QGraphicsView):
    def __init__(self, solum):
        QtWidgets.QGraphicsView.__init__(self)
        self.solum = solum
        self.setScene(QtWidgets.QGraphicsScene())

    # set the new image and redraw
    def updateImage(self, img):
        self.image = img
        self.scene().invalidate()

    # saves a local image
    def saveImage(self):
        self.image.save(str(Path.home() / "Pictures/clarius_image.png"))

    # resize the scan converter, image, and scene
    def resizeEvent(self, evt):
        w = evt.size().width()
        h = evt.size().height()
        self.solum.setOutputSize(w, h)
        self.image = QtGui.QImage(w, h, QtGui.QImage.Format_ARGB32)
        self.image.fill(QtCore.Qt.black)
        self.setSceneRect(0, 0, w, h)

    # black background
    def drawBackground(self, painter, rect):
        painter.fillRect(rect, QtCore.Qt.black)

    # draws the image
    def drawForeground(self, painter, rect):
        if not self.image.isNull():
            painter.drawImage(rect, self.image)


# main widget with controls and ui
class MainWidget(QtWidgets.QMainWindow):
    def __init__(self, solum, parent=None):
        QtWidgets.QMainWindow.__init__(self, parent)

        self.solum = solum
        self.setWindowTitle("Clarius Solum Demo")

        # create central widget within main window
        central = QtWidgets.QWidget()
        self.setCentralWidget(central)

        ip = QtWidgets.QLineEdit("192.168.1.1")
        ip.setInputMask("000.000.000.000")
        port = QtWidgets.QLineEdit("5000")
        port.setInputMask("00000")

        self.connButton = QtWidgets.QPushButton("Connect")
        cert = QtWidgets.QPushButton("Certify")
        self.runButton = QtWidgets.QPushButton("Run")
        quit = QtWidgets.QPushButton("Quit")
        depthUp = QtWidgets.QPushButton("< Depth")
        depthDown = QtWidgets.QPushButton("> Depth")
        gainInc = QtWidgets.QPushButton("> Gain")
        gainDec = QtWidgets.QPushButton("< Gain")

        # try to connect/disconnect to/from the probe
        def tryConnect():
            if not solum.isConnected():
                if not solum.connect(ip.text(), int(port.text())):
                    self.statusBar().showMessage("Failed to connect")
            elif not solum.disconnect():
                self.statusBar().showMessage("Failed to disconnect")

        # try to load cert
        def tryCert():
            name = QtWidgets.QFileDialog.getOpenFileName(self, "Load Certificate", "", "Cert Files (*.crt *.txt)")
            if os.path.isfile(name[0]):
                file = open(name[0])
                cert = file.read()
                print(f"certificate:\n{cert}")
                file.close()
                solum.setCertificate(cert)

        # try to run/stop
        def tryRun():
            global isRunning
            if isRunning:
                solum.stop()
                self.runButton.setText("Run")
            else:
                solum.run()
                self.runButton.setText("Stop")

        # try depth up
        def tryDepthUp():
            solum.setParameter(DEPTH, solum.getParameter(DEPTH) - 1)

        # try depth down
        def tryDepthDown():
            solum.setParameter(DEPTH, solum.getParameter(DEPTH) + 1)

        # try gain down
        def tryGainDec():
            solum.setParameter(GAIN, solum.getParameter(GAIN) - 0.1)

        # try gain up
        def tryGainInc():
            solum.setParameter(GAIN, solum.getParameter(GAIN) + 0.1)

        self.connButton.clicked.connect(tryConnect)
        cert.clicked.connect(tryCert)
        self.runButton.clicked.connect(tryRun)
        quit.clicked.connect(self.shutdown)
        depthUp.clicked.connect(tryDepthUp)
        depthDown.clicked.connect(tryDepthDown)
        gainInc.clicked.connect(tryGainInc)
        gainDec.clicked.connect(tryGainDec)

        # add widgets to layout
        self.img = ImageView(solum)
        layout = QtWidgets.QVBoxLayout()
        layout.addWidget(self.img)

        inplayout = QtWidgets.QHBoxLayout()
        layout.addLayout(inplayout)
        inplayout.addWidget(ip)
        inplayout.addWidget(port)

        connlayout = QtWidgets.QHBoxLayout()
        layout.addLayout(connlayout)
        connlayout.addWidget(self.connButton)
        connlayout.addWidget(cert)
        connlayout.addWidget(self.runButton)
        connlayout.addWidget(quit)
        central.setLayout(layout)

        prmlayout = QtWidgets.QHBoxLayout()
        layout.addLayout(prmlayout)
        prmlayout.addWidget(depthUp)
        prmlayout.addWidget(depthDown)
        prmlayout.addWidget(gainDec)
        prmlayout.addWidget(gainInc)

        caplayout = QtWidgets.QHBoxLayout()
        layout.addLayout(caplayout)

        modelayout = QtWidgets.QHBoxLayout()
        layout.addLayout(modelayout)

        # connect signals
        signaller.conn.connect(self.conn)
        signaller.cert.connect(self.cert)
        signaller.powerdown.connect(self.powerdown)
        signaller.imaging.connect(self.imaging)
        signaller.button.connect(self.button)
        signaller.err.connect(self.err)
        signaller.image.connect(self.image)

        # get home path
        path = os.path.expanduser("~/")
        if solum.init(path, 640, 480):
            self.statusBar().showMessage("Initialized")
        else:
            self.statusBar().showMessage("Failed to initialize")

    # handles imaging state changes
    @Slot(int, int, str)
    def conn(self, res, port, msg):
        self.statusBar().showMessage(f"Connection state {res}, port: {port}, message: {msg}")
        if res == 0:
            self.connButton.setText("Disconnect")
        else:
            self.connButton.setText("Connect")

    # handles cert validations
    @Slot(int)
    def cert(self, daysValid):
        self.statusBar().showMessage(f"Cert valid for {daysValid} days")
        if daysValid:
            # load a default application which should be updated as necessary
            self.solum.loadApplication("L7HD", "smp")

    # handles power downs
    @Slot(int, int)
    def powerdown(self, state, tm):
        self.statusBar().showMessage(f"Power Down {state} in {tm}")

    # handles imaging state changes
    @Slot(int, int)
    def imaging(self, state, imaging):
        if not imaging:
            self.run.setText("Run")
            self.statusBar().showMessage("Image Stopped")
        else:
            self.run.setText("Freeze")
            self.statusBar().showMessage("Image Running (check firewall settings if no image seen)")

    # handles button messages
    @Slot(int, int)
    def button(self, btn, clicks):
        self.statusBar().showMessage(f"Button {btn} pressed w/ {clicks} clicks")

    # handles error messages
    @Slot(int, str)
    def err(self, code, msg):
        self.statusBar().showMessage(f"Error: {code}, {msg}")

    # handles new images
    @Slot(QtGui.QImage)
    def image(self, img):
        self.img.updateImage(img)

    # handles shutdown
    @Slot()
    def shutdown(self):
        if sys.platform.startswith("linux"):
            # unload the shared library before destroying the solum object
            ctypes.CDLL("libc.so.6").dlclose(libsolum_handle)
        self.solum.destroy()
        QtWidgets.QApplication.quit()


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
    if bpp == 4:
        img = QtGui.QImage(image, width, height, QtGui.QImage.Format_ARGB32)
    else:
        img = QtGui.QImage(image, width, height, QtGui.QImage.Format_Grayscale8)
    # a deep copy is important here, as the memory from 'image' won't be valid after the event posting
    signaller.usimage = img.copy()
    evt = ImageEvent()
    QtCore.QCoreApplication.postEvent(signaller, evt)


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
    evt = ConnectEvent(res, port, msg)
    QtCore.QCoreApplication.postEvent(signaller, evt)


## called after a certificate was sent for validation
# @param daysValid the # of days the passed certificate is valid for
def certFn(daysValid):
    evt = CertEvent(daysValid)
    QtCore.QCoreApplication.postEvent(signaller, evt)


## called when the power down state changes
# @param state the power down state
# @param tm the time the probe will shut down in
def powerDownFn(state, tm):
    evt = PowerDownEvent(state, tm)
    QtCore.QCoreApplication.postEvent(signaller, evt)


## called after imaging state has changed
# @param state the imaging state
# @param imaging the imaging flag, 0=stopped, 1=running
def imagingFn(state, imaging):
    _evt = ImagingEvent(state, imaging)
    global isRunning
    isRunning = imaging


## called when a button is pressed
# @param button the button that was pressed
# @param clicks number of clicks performed
def buttonFn(button, clicks):
    evt = ButtonEvent(button, clicks)
    QtCore.QCoreApplication.postEvent(signaller, evt)


## called when an error occurs
# @param code the error code
# @param msg corresponding error message
def errorFn(code, msg):
    evt = ErrorEvent(code, msg)
    QtCore.QCoreApplication.postEvent(signaller, evt)


## main function
def main():
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
    app = QtWidgets.QApplication(sys.argv)
    widget = MainWidget(solum)
    widget.resize(640, 480)
    widget.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
