LOCAL_PATH := $(call my-dir)
TOP_LOCAL_PATH := $(LOCAL_PATH)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
	jni/hans 
LOCAL_CFLAGS := -DHAVE_ANDROID
LOCAL_MODULE    := hans_java
LOCAL_SRC_FILES := com_lantern_launcher_vpnservice_EchoUtils.cpp
LOCAL_LDLIBS    := -llog

include $(BUILD_SHARED_LIBRARY)