# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    :=  hans
LOCAL_SRC_FILES :=  hans/utility.cpp \
					hans/exception.cpp \
					hans/time.cpp \
					hans/sha1.cpp \
					hans/tun_dev.c \
					hans/echo.cpp \
					hans/tun.cpp \
					hans/client.cpp \
					hans/auth.cpp \
					hans/worker.cpp \
					hans/server.cpp \
					hans/main.cpp
					

		
LOCAL_C_INCLUDES := $(LOCAL_PATH)/hans/libpcap
					
LOCAL_CFLAGS := -c -g -DHAVE_LINUX_IF_TUN_H
LOCAL_LDFLAGS := -fPIE -pie

LOCAL_STATIC_LIBRARIES := libpcap  

LOCAL_LDLIBS    := -lm -llog


include $(BUILD_EXECUTABLE)

include $(LOCAL_PATH)/hans/libpcap/Android.mk  