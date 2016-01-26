LOCAL_PATH := $(call my-dir)

$(call import-add-path,$(LOCAL_PATH)/libs)

include $(CLEAR_VARS)

LOCAL_MODULE := serial_static
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/libs
LOCAL_SRC_FILES := serial_jni.cc \
    jni_utility.cc \
    jni_main.cc

LOCAL_STATIC_LIBRARIES += nativehelper serialport

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := serial

LOCAL_STATIC_LIBRARIES += serial_static
LOCAL_LDLIBS := -llog -lz

include $(BUILD_SHARED_LIBRARY)

$(call import-module,serialport)
$(call import-module,nativehelper)

