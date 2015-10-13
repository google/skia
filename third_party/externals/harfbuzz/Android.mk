##
## Copyright (C) 2012 The Android Open Source Project
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##      http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
##

#############################################################
#   Note:
#
#   This file is used to build HarfBuzz within the Android
#   platform itself.  If you need to compile HarfBuzz to
#   ship with your Android NDK app, you can use the autotools
#   build system to do so.  To do that you need to install a
#   "standalone" toolchain with the NDK, eg:
#
#       ndk/build/tools/make-standalone-toolchain.sh \
#           --platform=android-18 \
#           --install-dir=/prefix
#
#   Set PLATFORM_PREFIX eng var to that prefix and make sure
#   the cross-compile tools from PLATFORM_PREFIX are in path.
#   Configure and install HarfBuzz:
#
#       ./configure --host=arm-linux-androideabi \
#           --prefix=$PLATFORM_PREFIX \
#           --enable-static \
#           --with-freetype \
#           PKG_CONFIG_LIBDIR=$PLATFORM_PREFIX/lib/pkgconfig
#       make install
#
#   You can first build FreeType the same way:
#
#       ./configure --host=arm-linux-androideabi \
#           --prefix=$PLATFORM_PREFIX \
#           --enable-static \
#           --without-png \
#           PKG_CONFIG_LIBDIR=$PLATFORM_PREFIX/lib/pkgconfig
#       make install
#

LOCAL_PATH:= $(call my-dir)

HARFBUZZ_SRC_FILES = \
	src/hb-blob.cc \
	src/hb-buffer-serialize.cc \
	src/hb-buffer.cc \
	src/hb-common.cc \
	src/hb-face.cc \
	src/hb-font.cc \
	src/hb-ot-tag.cc \
	src/hb-set.cc \
	src/hb-shape.cc \
	src/hb-shape-plan.cc \
	src/hb-shaper.cc \
	src/hb-unicode.cc \
	src/hb-warning.cc \
	src/hb-ot-font.cc \
	src/hb-ot-layout.cc \
	src/hb-ot-map.cc \
	src/hb-ot-shape.cc \
	src/hb-ot-shape-complex-arabic.cc \
	src/hb-ot-shape-complex-default.cc \
	src/hb-ot-shape-complex-hangul.cc \
	src/hb-ot-shape-complex-hebrew.cc \
	src/hb-ot-shape-complex-indic.cc \
	src/hb-ot-shape-complex-indic-table.cc \
	src/hb-ot-shape-complex-myanmar.cc \
	src/hb-ot-shape-complex-sea.cc \
	src/hb-ot-shape-complex-thai.cc \
	src/hb-ot-shape-complex-tibetan.cc \
	src/hb-ot-shape-normalize.cc \
	src/hb-ot-shape-fallback.cc \
	$(NULL)

#############################################################
#   build the harfbuzz shared library
#
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES:= \
	$(HARFBUZZ_SRC_FILES) \
	src/hb-icu.cc
LOCAL_CPP_EXTENSION := .cc
LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libicuuc \
	libicui18n \
	libutils \
	liblog
LOCAL_C_INCLUDES += \
        $(LOCAL_PATH)/src \
        external/icu/icu4c/source/common
LOCAL_CFLAGS += -DHB_NO_MT -DHAVE_OT -DHAVE_ICU -DHAVE_ICU_BUILTIN
LOCAL_MODULE:= libharfbuzz_ng
include $(BUILD_SHARED_LIBRARY)
