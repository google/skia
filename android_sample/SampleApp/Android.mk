######################################
# Build the app.
######################################

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
        $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := SampleApp

LOCAL_JNI_SHARED_LIBRARIES := libskia-sample

LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_PACKAGE)

######################################
# Build the shared library.
######################################

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES += \
    external/skia/include/core \
    external/skia/include/config \
    external/skia/include/effects \
    external/skia/include/images \
    external/skia/include/utils \
    external/skia/include/utils/android \
    external/skia/include/views \
    external/skia/samplecode \
    external/skia/include/xml \
    external/skia/include/gpu \
    external/skia/src/core \
    external/skia/gpu/include \
    frameworks/base/opengl/include/GLES2 \
    $(LOCAL_PATH)/jni

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libskia \
    libandroid_runtime \
    libGLESv2

LOCAL_STATIC_LIBRARIES := \
    libskiagpu

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libskia-sample

LOCAL_SRC_FILES := \
    ../../src/ports/SkXMLParser_empty.cpp \
    jni/sample-jni.cpp

include external/skia/src/views/views_files.mk
LOCAL_SRC_FILES += $(addprefix ../../src/views/, $(SOURCE))

include external/skia/src/xml/xml_files.mk
LOCAL_SRC_FILES += $(addprefix ../../src/xml/, $(SOURCE))

include external/skia/samplecode/samplecode_files.mk
LOCAL_SRC_FILES += $(addprefix ../../samplecode/, $(SOURCE))

include $(BUILD_SHARED_LIBRARY)
