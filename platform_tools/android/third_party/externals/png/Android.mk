LOCAL_PATH:= $(call my-dir)

# We need to build this for both the device (as a shared library)
# and the host (as a static library for tools to use).

common_SRC_FILES := \
	png.c \
	pngerror.c \
	pnggccrd.c \
	pngget.c \
	pngmem.c \
	pngpread.c \
	pngread.c \
	pngrio.c \
	pngrtran.c \
	pngrutil.c \
	pngset.c \
	pngtrans.c \
	pngvcrd.c \
	pngwio.c \
	pngwrite.c \
	pngwtran.c \
	pngwutil.c

common_CFLAGS := -std=gnu89 -fvisibility=hidden ## -fomit-frame-pointer

ifeq ($(HOST_OS),windows)
  ifeq ($(USE_MINGW),)
    # Case where we're building windows but not under linux (so it must be cygwin)
    # In this case, gcc cygwin doesn't recognize -fvisibility=hidden
    $(info libpng: Ignoring gcc flag $(common_CFLAGS) on Cygwin)
    common_CFLAGS := 
  endif
endif

common_C_INCLUDES += 

common_COPY_HEADERS_TO := libpng
common_COPY_HEADERS := png.h pngconf.h pngusr.h

# For the host
# =====================================================

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(common_SRC_FILES)
LOCAL_CFLAGS += $(common_CFLAGS)
LOCAL_C_INCLUDES += $(common_C_INCLUDES) external/zlib

LOCAL_MODULE:= libpng

LOCAL_COPY_HEADERS_TO := $(common_COPY_HEADERS_TO)
LOCAL_COPY_HEADERS := $(common_COPY_HEADERS)

include $(BUILD_HOST_STATIC_LIBRARY)


# For the device
# =====================================================

include $(CLEAR_VARS)
LOCAL_CLANG := true
LOCAL_SRC_FILES := $(common_SRC_FILES)
LOCAL_CFLAGS += $(common_CFLAGS) -ftrapv
LOCAL_C_INCLUDES += $(common_C_INCLUDES) \
	external/zlib
LOCAL_SHARED_LIBRARIES := \
	libz

LOCAL_MODULE:= libpng

LOCAL_COPY_HEADERS_TO := $(common_COPY_HEADERS_TO)
LOCAL_COPY_HEADERS := $(common_COPY_HEADERS)

include $(BUILD_STATIC_LIBRARY)

# For testing
# =====================================================

include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= $(common_C_INCLUDES) external/zlib
LOCAL_SRC_FILES:= $(common_SRC_FILES) pngtest.c
LOCAL_MODULE := pngtest
LOCAL_SHARED_LIBRARIES:= libz
LOCAL_MODULE_TAGS := debug
include $(BUILD_EXECUTABLE)
