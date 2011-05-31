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
    $(LOCAL_PATH)/skia_extra/include/views \
    $(LOCAL_PATH)/skia_extra/samplecode \
    $(LOCAL_PATH)/skia_extra/include/xml \
    external/skia/include/gpu \
    external/skia/src/core \
    external/skia/gpu/include \
    frameworks/base/core/jni/android/graphics \
    frameworks/base/native/include/android \
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
    skia_extra/src/ports/SkXMLParser_empty.cpp \
    jni/sample-jni.cpp

include $(LOCAL_PATH)/skia_extra/src/views/views_files.mk
LOCAL_SRC_FILES += $(addprefix skia_extra/src/views/, $(SOURCE))

include $(LOCAL_PATH)/skia_extra/src/xml/xml_files.mk
LOCAL_SRC_FILES += $(addprefix skia_extra/src/xml/, $(SOURCE))

include $(LOCAL_PATH)/skia_extra/samplecode/samplecode_files.mk
LOCAL_SRC_FILES += $(addprefix skia_extra/samplecode/, $(SOURCE))

include $(BUILD_SHARED_LIBRARY)
