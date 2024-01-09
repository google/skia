/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "tools/ToolUtils.h"
#include "tools/viewer/Slide.h"

#if defined(SK_GANESH)
#include "include/android/SkImageAndroid.h"
#include "include/android/SkSurfaceAndroid.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/android/graphite/SurfaceAndroid.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/RecorderPriv.h"

#else
namespace skgpu::graphite {
    class Recorder;
}
#endif

#include <android/hardware_buffer.h>

using namespace skgpu::graphite;

namespace {

static void release_buffer(AHardwareBuffer* buffer) {
    if (buffer) {
        AHardwareBuffer_release(buffer);
    }
}

sk_sp<SkSurface> wrap_buffer(GrDirectContext* dContext,
                             Recorder* recorder,
                             AHardwareBuffer* buffer) {
#if defined(SK_GANESH)
    if (dContext) {
        return SkSurfaces::WrapAndroidHardwareBuffer(dContext,
                                                     buffer,
                                                     kTopLeft_GrSurfaceOrigin,
                                                     /* colorSpace= */ nullptr,
                                                     /* surfaceProps= */ nullptr);
    }
#endif

#if defined(SK_GRAPHITE)
    if (recorder) {
        return SkSurfaces::WrapAndroidHardwareBuffer(recorder,
                                                     buffer,
                                                     /* colorSpace= */ nullptr,
                                                     /* surfaceProps= */ nullptr);
    }
#endif

    return nullptr;
}

sk_sp<SkSurface> create_protected_render_target(GrDirectContext* dContext,
                                                Recorder* recorder,
                                                const SkImageInfo& ii) {
#if defined(SK_GANESH)
    if (dContext) {
        return SkSurfaces::RenderTarget(dContext,
                                        skgpu::Budgeted::kYes,
                                        ii,
                                        /* sampleCount= */ 1,
                                        kTopLeft_GrSurfaceOrigin,
                                        /* surfaceProps= */ nullptr,
                                        /* shouldCreateWithMips= */ false,
                                        /* isProtected= */ true);
    }
#endif

#if defined(SK_GRAPHITE)
    if (recorder) {
        // Protected-ness is pulled off of the recorder
        return SkSurfaces::RenderTarget(recorder,
                                        ii,
                                        skgpu::Mipmapped::kNo,
                                        /* props= */ nullptr);
    }
#endif

    return nullptr;
}

AHardwareBuffer* create_protected_buffer(int width, int height) {

    AHardwareBuffer* buffer = nullptr;

    AHardwareBuffer_Desc hwbDesc;
    hwbDesc.width = width;
    hwbDesc.height = height;
    hwbDesc.layers = 1;
    hwbDesc.usage = AHARDWAREBUFFER_USAGE_CPU_READ_NEVER |
                    AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER |
                    AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
                    AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT;

    hwbDesc.usage |= AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;

    hwbDesc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    // The following three are not used in the allocate
    hwbDesc.stride = 0;
    hwbDesc.rfu0= 0;
    hwbDesc.rfu1= 0;

    if (int error = AHardwareBuffer_allocate(&hwbDesc, &buffer)) {
        SkDebugf("Failed to allocated hardware buffer, error: %d\n", error);
        release_buffer(buffer);
        return nullptr;
    }

    return buffer;
}

sk_sp<SkImage> create_protected_AHB_image(GrDirectContext* dContext,
                                          Recorder* recorder,
                                          AHardwareBuffer* buffer,
                                          SkColor color) {

    sk_sp<SkSurface> surf = wrap_buffer(dContext, recorder, buffer);
    if (!surf) {
        SkDebugf("Failed to make SkSurface.\n");
        return nullptr;
    }

    ToolUtils::draw_checkerboard(surf->getCanvas(), color, SK_ColorTRANSPARENT, 32);

    return surf->makeImageSnapshot();
}

sk_sp<SkImage> create_protected_skia_image(GrDirectContext* dContext,
                                           Recorder* recorder,
                                           int width, int height,
                                           SkColor color) {
    SkImageInfo ii = SkImageInfo::Make(width, height, kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);

    sk_sp<SkSurface> tmpSurface = create_protected_render_target(dContext, recorder, ii);
    if (!tmpSurface) {
        return nullptr;
    }

    ToolUtils::draw_checkerboard(tmpSurface->getCanvas(), color, SK_ColorTRANSPARENT, 32);

    return tmpSurface->makeImageSnapshot();
}

} // anonymous namespace

class ProtectedSlide : public Slide {
public:
    ProtectedSlide() { fName = "Protected"; }

    SkISize getDimensions() const override { return {kSize, 2*kSize}; }

    void draw(SkCanvas* origCanvas) override {
        origCanvas->clear(SK_ColorDKGRAY);

        skgpu::graphite::Recorder* recorder = origCanvas->recorder();
        GrDirectContext* dContext = GrAsDirectContext(origCanvas->recordingContext());

#if defined(SK_GANESH)
        if (dContext && !dContext->supportsProtectedContent()) {
            origCanvas->clear(SK_ColorGREEN);
            return;
        }
#endif

#if defined(SK_GRAPHITE)
        if (recorder && recorder->priv().isProtected() == skgpu::Protected::kNo) {
            origCanvas->clear(SK_ColorBLUE);
            return;
        }
#endif

        if (!dContext && !recorder) {
            origCanvas->clear(SK_ColorRED);
            return;
        }

        AHardwareBuffer* buffer = create_protected_buffer(kSize, kSize);

        sk_sp<SkImage> protectedAHBImage = create_protected_AHB_image(dContext, recorder, buffer,
                                                                      SK_ColorRED);
        sk_sp<SkImage> protectedSkImage = create_protected_skia_image(dContext, recorder,
                                                                      kSize, kSize, SK_ColorBLUE);

        // Pick one of the two protected images to draw. Only the protected AHB-backed image will
        // reproduce the bug (b/242266174).
        SkImage* imgToUse = protectedAHBImage.get();
//        SkImage* imgToUse = protectedSkImage.get();

        sk_sp<SkImage> indirectImg;

        {
            SkImageInfo ii = SkImageInfo::Make(kSize, kSize, kRGBA_8888_SkColorType,
                                               kPremul_SkAlphaType);
            sk_sp<SkSurface> tmpS = create_protected_render_target(dContext, recorder, ii);

            tmpS->getCanvas()->clear(SK_ColorMAGENTA);
            tmpS->getCanvas()->drawCircle(64, 64, 32, SkPaint());

            // For protected AHB-backed images this draw seems to poison all above the draws too
            tmpS->getCanvas()->drawImage(imgToUse, 0, 0);
            indirectImg = tmpS->makeImageSnapshot();
        }

        origCanvas->drawImage(imgToUse, 0, 0);
        origCanvas->drawImage(indirectImg, 0, kSize);

        protectedAHBImage.reset();
        protectedSkImage.reset();
        release_buffer(buffer);
    }

private:
    static const int kSize = 128;
};

DEF_SLIDE( return new ProtectedSlide(); )

#endif
