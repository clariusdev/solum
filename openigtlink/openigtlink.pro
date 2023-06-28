TEMPLATE = lib
CONFIG += c++20 staticlib warn_off
CONFIG -= qt
CONFIG(release, debug|release): CONFIG += ltcg

include(openigtlink.pri)

# Needs to be kept in sync with all file lists in repo/Source/CMakeLists.txt.
SOURCES += \
    repo/Source/igtlBindMessage.cxx \
    repo/Source/igtlCapabilityMessage.cxx \
    repo/Source/igtlClientSocket.cxx \
    repo/Source/igtlColorTableMessage.cxx \
    repo/Source/igtlCommandMessage.cxx \
    repo/Source/igtlConditionVariable.cxx \
    repo/Source/igtlFastMutexLock.cxx \
    repo/Source/igtlGeneralSocket.cxx \
    repo/Source/igtlImageMessage.cxx \
    repo/Source/igtlImageMessage2.cxx \
    repo/Source/igtlImageMetaMessage.cxx \
    repo/Source/igtlLabelMetaMessage.cxx \
    repo/Source/igtlLightObject.cxx \
    repo/Source/igtlMath.cxx \
    repo/Source/igtlMessageBase.cxx \
    repo/Source/igtlMessageFactory.cxx \
    repo/Source/igtlMessageRTPWrapper.cxx \
    repo/Source/igtlMultiThreader.cxx \
    repo/Source/igtlMutexLock.cxx \
    repo/Source/igtlNDArrayMessage.cxx \
    repo/Source/igtlObject.cxx \
    repo/Source/igtlObjectFactoryBase.cxx \
    repo/Source/igtlOSUtil.cxx \
    repo/Source/igtlPointMessage.cxx \
    repo/Source/igtlPolyDataMessage.cxx \
    repo/Source/igtlPositionMessage.cxx \
    repo/Source/igtlQuaternionTrackingDataMessage.cxx \
    repo/Source/igtlQueryMessage.cxx \
    repo/Source/igtlSensorMessage.cxx \
    repo/Source/igtlServerSocket.cxx \
    repo/Source/igtlSessionManager.cxx \
    repo/Source/igtlSimpleFastMutexLock.cxx \
    repo/Source/igtlSocket.cxx \
    repo/Source/igtlStatusMessage.cxx \
    repo/Source/igtlStringMessage.cxx \
    repo/Source/igtlTimeStamp.cxx \
    repo/Source/igtlTrackingDataMessage.cxx \
    repo/Source/igtlTrajectoryMessage.cxx \
    repo/Source/igtlTransformMessage.cxx \
    repo/Source/igtlUDPClientSocket.cxx \
    repo/Source/igtlUDPServerSocket.cxx \
    repo/Source/igtlUnit.cxx \
    repo/Source/igtlutil/igtl_bind.c \
    repo/Source/igtlutil/igtl_capability.c \
    repo/Source/igtlutil/igtl_colortable.c \
    repo/Source/igtlutil/igtl_command.c \
    repo/Source/igtlutil/igtl_header.c \
    repo/Source/igtlutil/igtl_image.c \
    repo/Source/igtlutil/igtl_imgmeta.c \
    repo/Source/igtlutil/igtl_lbmeta.c \
    repo/Source/igtlutil/igtl_ndarray.c \
    repo/Source/igtlutil/igtl_point.c \
    repo/Source/igtlutil/igtl_polydata.c \
    repo/Source/igtlutil/igtl_position.c \
    repo/Source/igtlutil/igtl_qtdata.c \
    repo/Source/igtlutil/igtl_qtrans.c \
    repo/Source/igtlutil/igtl_query.c \
    repo/Source/igtlutil/igtl_sensor.c \
    repo/Source/igtlutil/igtl_status.c \
    repo/Source/igtlutil/igtl_string.c \
    repo/Source/igtlutil/igtl_tdata.c \
    repo/Source/igtlutil/igtl_trajectory.c \
    repo/Source/igtlutil/igtl_transform.c \
    repo/Source/igtlutil/igtl_unit.c \
    repo/Source/igtlutil/igtl_util.c \
