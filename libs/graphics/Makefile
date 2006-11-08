LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	effects/Sk1DPathEffect.cpp \
	effects/Sk2DPathEffect.cpp \
	effects/SkBlurMask.cpp \
	effects/SkBlurMaskFilter.cpp \
	effects/SkCamera.cpp \
	effects/SkColorFilters.cpp \
	effects/SkCornerPathEffect.cpp \
	effects/SkDashPathEffect.cpp \
	effects/SkDiscretePathEffect.cpp \
	effects/SkEmbossMask.cpp \
	effects/SkEmbossMaskFilter.cpp \
	effects/SkGradientShader.cpp \
	effects/SkLayerRasterizer.cpp \
	effects/SkNinePatch.cpp \
	effects/SkShaderExtras.cpp \
	effects/SkTransparentShader.cpp \
	images/SkBitmapRef.cpp \
	images/SkImageDecoder.cpp \
	images/SkImageDecoder_libgif.cpp \
	images/SkImageDecoder_libjpeg.cpp \
	images/SkImageDecoder_libpng.cpp \
	images/SkImageDecoder_libico.cpp \
	images/SkImageDecoder_zbm.cpp \
	images/SkStream.cpp \
	sgl/SkAlphaRuns.cpp \
	sgl/SkBitmap.cpp \
	sgl/SkBitmapSampler.cpp \
	sgl/SkBitmapShader.cpp \
	sgl/SkBlitter.cpp \
	sgl/SkBlitter_A1.cpp \
	sgl/SkBlitter_A8.cpp \
	sgl/SkBlitter_ARGB32.cpp \
	sgl/SkBlitter_RGB16.cpp \
	sgl/SkBlitter_Sprite.cpp \
	sgl/SkCanvas.cpp \
	sgl/SkColor.cpp \
	sgl/SkColorFilter.cpp \
	sgl/SkColorTable.cpp \
	sgl/SkDeque.cpp \
	sgl/SkDraw.cpp \
	sgl/SkEdge.cpp \
	sgl/SkFilterProc.cpp \
	sgl/SkGeometry.cpp \
	sgl/SkGlobals.cpp \
	sgl/SkGlyphCache.cpp \
	sgl/SkGraphics.cpp \
	sgl/SkMaskFilter.cpp \
	sgl/SkPaint.cpp \
	sgl/SkPath.cpp \
	sgl/SkPathEffect.cpp \
	sgl/SkPathMeasure.cpp \
	sgl/SkProcSpriteBlitter.cpp \
	sgl/SkRasterizer.cpp \
	sgl/SkRefCnt.cpp \
	sgl/SkRegion_path.cpp \
	sgl/SkScalerContext.cpp \
	sgl/SkScan.cpp \
	sgl/SkScan_AntiPath.cpp \
	sgl/SkScan_Antihair.cpp \
	sgl/SkScan_Hairline.cpp \
	sgl/SkScan_Path.cpp \
	sgl/SkShader.cpp \
	sgl/SkSpriteBlitter_ARGB32.cpp \
	sgl/SkSpriteBlitter_RGB16.cpp \
	sgl/SkString.cpp \
	sgl/SkStroke.cpp \
	sgl/SkStrokerPriv.cpp \
	sgl/SkTSearch.cpp \
	sgl/SkUtils.cpp \
	sgl/SkXfermode.cpp \
	ports/SkImageDecoder_Factory.cpp \
	ports/SkFontHost_android.cpp \
	ports/SkFontHost_FreeType.cpp \
	ports/SkGlobals_global.cpp \
	ports/SkOSFile_stdio.cpp \
	ports/SkTime_Unix.cpp

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcorecg \
	libpng \
	libgif \
	libjpeg \
	libft2 \
	libexpat \
	libz

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/animator \
	$(LOCAL_PATH)/sgl \
	$(LOCAL_PATH)/images \
	$(LOCAL_PATH)/ports \
	include/graphics \
	include/corecg \
	libs/corecg \
	extlibs/freetype-2.1.10/include \
	extlibs/zlib-1.2.3 \
	extlibs/libpng-1.2.8 \
	extlibs/libgif-4.0 \
	extlibs/jpeg-6b \
	extlibs/expat-2.0.0/lib

ifeq ($(DEVICE_OS),darwin)
	LOCAL_CFLAGS += -fPIC
else
	LOCAL_CFLAGS += -fpic
endif

LOCAL_CFLAGS += -fstrict-aliasing

ifeq ($(DEVICE_ARCH),arm)
	LOCAL_CFLAGS += -DFMS_ARCH_ANDROID_ARM -fomit-frame-pointer
endif

# TODO RELEASE_BUILD isn't right
ifeq ($(RELEASE_BUILD),true)
	LOCAL_CFLAGS += -O2
endif

LOCAL_LDLIBS += -lpthread

LOCAL_TARGET:= libsgl

include $(BUILD_SHARED_LIBRARY)
