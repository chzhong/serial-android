LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := serialport
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include
	
LOCAL_SRC_FILES := serial.cc \
    serial_unix.cc \
    list_ports_linux.cc

LOCAL_EXPORT_CPPFLAGS := -I$(LOCAL_PATH)/include

include $(BUILD_STATIC_LIBRARY)
