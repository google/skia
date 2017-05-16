

LOCAL_PATH := $(call my-dir)

##
# Sets up a module for the skia shared object to be copied into the apk.
##

include $(CLEAR_VARS)

# Name for referencing this module in other modules
LOCAL_MODULE := skia_android

# Local filename of the skia shared object
 LOCAL_SRC_FILES := ../obj/local/$(TARGET_ARCH_ABI)/libskia_android.so

# Makes this module into shared object that is simply copied into the apk
include $(PREBUILT_SHARED_LIBRARY)


##
# Sets up the JNI module that our app calls into to draw things with skia.
##

include $(CLEAR_VARS) # clear out the variables of the previous module

# Name of the module that the app will reference with System.loadLibrary
LOCAL_MODULE := hello_skia_ndk

# List of the source files compiled for this module
LOCAL_SRC_FILES := ./helloskia.cpp \
					./renderer.cpp \
					./skiaDraw.cpp

# Makes the skia shared object get pulled in as a reference
LOCAL_SHARED_LIBRARIES := skia_android

# jnigraphics defines the function AndroidBitmap_lockPixels, which we need in order to draw into
# android.graphics.Bitmap
LOCAL_LDLIBS := -ljnigraphics  -lEGL -llog -landroid  -lGLESv2

# Allows the compiler to find the Skia header files
LOCAL_C_INCLUDES := $(SKIA_ROOT)/skia/include/config \
                    $(SKIA_ROOT)/skia/include/core \
                    $(SKIA_ROOT)/skia/i nclude/utils \
                    $(SKIA_ROOT)/skia/include/gpu
                    
LOCAL_CFLAGS  +=  -DSK_BUILD_FOR_ANDROID -std=c++11 


include $(BUILD_SHARED_LIBRARY)
