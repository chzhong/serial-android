LOCAL_PATH := $(call my-dir)

$(call import-add-path,$(LOCAL_PATH)/libs)

include $(CLEAR_VARS)

LOCAL_MODULE := serial
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/libs
	
LOCAL_SRC_FILES := serial_jni.cc \
    jni_utility.cc \
    jni_main.cc

LOCAL_STATIC_LIBRARIES += nativehelper serial_static
LOCAL_LDLIBS := -llog -lz

include $(BUILD_SHARED_LIBRARY)

$(call import-module,serial_static)
$(call import-module,nativehelper)

