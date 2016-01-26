LOCAL_PATH := $(call my-dir)

$(call import-add-path,$(LOCAL_PATH)/libs)

SERIAL_SRC_FILES := serial_jni.cc \
    jni_utility.cc \
    jni_main.cc

SERIAL_C_INCLUDES := \
    $(LOCAL_PATH)/include

include $(CLEAR_VARS)

LOCAL_MODULE := serial_static
LOCAL_C_INCLUDES := $(SERIAL_C_INCLUDES)
LOCAL_SRC_FILES := $(SERIAL_SRC_FILES)

LOCAL_STATIC_LIBRARIES += nativehelper serialport

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := serial
LOCAL_C_INCLUDES := $(SERIAL_C_INCLUDES)
LOCAL_SRC_FILES := $(SERIAL_SRC_FILES)

LOCAL_STATIC_LIBRARIES += nativehelper serialport
LOCAL_LDLIBS := -llog -lz

include $(BUILD_SHARED_LIBRARY)

$(call import-module,serialport)
$(call import-module,nativehelper)

