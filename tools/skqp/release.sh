#!/bin/sh
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cd "$(dirname "$0")/../.."

set -e -x

[ -f platform_tools/android/apps/skqp/src/main/assets/files.checksum       ] || exit 1
[ -f platform_tools/android/apps/skqp/src/main/assets/skqp/rendertests.txt ] || exit 1
[ -f platform_tools/android/apps/skqp/src/main/assets/skqp/unittests.txt   ] || exit 1

python tools/skqp/gn_to_bp.py
python tools/skqp/download_model
python tools/skqp/setup_resources

touch MODULE_LICENSE_BSD

cat > platform_tools/android/apps/skqp/src/main/Android.mk <<- "EOM"
	# Copyright 2019 Google LLC.
	# Use of this source code is governed by a BSD-style license that can be
	# found in the LICENSE file.
	LOCAL_PATH:= $(call my-dir)
	include $(CLEAR_VARS)
	LOCAL_MODULE_TAGS := tests optional
	LOCAL_MODULE_PATH := $(TARGET_OUT_DATA_APPS)
	LOCAL_COMPATIBILITY_SUITE := cts vts general-tests
	LOCAL_JAVA_LIBRARIES := android.test.runner.stubs
	LOCAL_JNI_SHARED_LIBRARIES := libskqp_app
	LOCAL_MULTILIB := both
	LOCAL_USE_AAPT2 := true
	LOCAL_STATIC_ANDROID_LIBRARIES := android-support-design
	LOCAL_STATIC_JAVA_LIBRARIES := ctstestrunner-axt
	LOCAL_SRC_FILES := $(call all-java-files-under, java)
	LOCAL_PACKAGE_NAME := CtsSkQPTestCases
	LOCAL_SDK_VERSION := test_current
	include $(BUILD_CTS_PACKAGE)
EOM

cat > include/config/SkUserConfigManual.h <<- "EOM"
	// Copyright 2019 Google LLC.
	// Use of this source code is governed by a BSD-style license that can be
	// found in the LICENSE file.
	#ifndef SkUserConfigManual_DEFINED
	#define SkUserConfigManual_DEFINED
	// DON'T DEFINE THINGS HERE AS IT WILL RESULT IN DIFFERENCES WITH
	// THE VERSION OF SKQP PUBLISHED ON SKIA.ORG
	#endif // SkUserConfigManual_DEFINED
EOM

cat > platform_tools/android/apps/skqp/src/main/AndroidTest.xml <<- "EOM"
	<?xml version="1.0" encoding="utf-8"?>
	<!--
	Copyright 2019 Google LLC.
	Use of this source code is governed by a BSD-style license that can be
	found in the LICENSE file.
	-->
	<configuration description="Config for CTS SkQP test cases">
	<option name="test-suite-tag" value="cts" />
	<option name="not-shardable" value="true" />
	<option name="config-descriptor:metadata" key="component" value="uitoolkit" />
	<option name="config-descriptor:metadata" key="parameter" value="not_instant_app" />
	<option name="config-descriptor:metadata" key="parameter" value="multi_abi" />
	<target_preparer class="com.android.tradefed.targetprep.suite.SuiteApkInstaller">
	<option name="cleanup-apks" value="true" />
	<option name="test-file-name" value="CtsSkQPTestCases.apk" />
	</target_preparer>
	<test class="com.android.tradefed.testtype.AndroidJUnitTest" >
	<option name="package" value="org.skia.skqp" />
	<option name="runtime-hint" value="7m" />
	</test>
	</configuration>
EOM

[ -f platform_tools/android/apps/skqp/src/main/assets/.gitignore ] && \
    git rm platform_tools/android/apps/skqp/src/main/assets/.gitignore

git add                                                       \
    Android.bp                                                \
    MODULE_LICENSE_BSD                                        \
    include/config/SkUserConfig.h                             \
    include/config/SkUserConfigManual.h                       \
    platform_tools/android/apps/skqp/src/main/Android.mk      \
    platform_tools/android/apps/skqp/src/main/AndroidTest.xml \
    platform_tools/android/apps/skqp/src/main/assets
