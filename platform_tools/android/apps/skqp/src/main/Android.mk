# Copyright (C) 2018 The Android Open Source Project
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

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

# don't include this package in any target
LOCAL_MODULE_TAGS := tests optional

LOCAL_MODULE_PATH := $(TARGET_OUT_DATA_APPS)

# Tag this module as a cts test artifact
LOCAL_COMPATIBILITY_SUITE := cts vts general-tests

LOCAL_JAVA_LIBRARIES := android.test.runner.stubs

LOCAL_JNI_SHARED_LIBRARIES := libskqp_app

LOCAL_USE_AAPT2 := true

LOCAL_STATIC_ANDROID_LIBRARIES := \
    android-support-design

LOCAL_STATIC_JAVA_LIBRARIES := \
    ctstestrunner

LOCAL_SRC_FILES := $(call all-java-files-under, java)

LOCAL_PACKAGE_NAME := CtsSkQPTestCases

# Enforce public / test api only
LOCAL_SDK_VERSION := test_current

include $(BUILD_CTS_PACKAGE)
