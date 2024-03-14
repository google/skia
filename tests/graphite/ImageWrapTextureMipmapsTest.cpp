/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkCanvas.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "tools/gpu/ManagedBackendTexture.h"
#include "tools/graphite/GraphiteTestContext.h"

using namespace skgpu;
using namespace skgpu::graphite;

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ImageWrapTextureMipmapsTest,
                                               reporter,
                                               context,
                                               testContext,
                                               true,
                                               CtsEnforcement::kNextRelease) {
    auto recorder = context->makeRecorder();
    if (!recorder) {
        ERRORF(reporter, "Could not make recorder");
        return;
    }

    skgpu::Protected isProtected = skgpu::Protected(context->priv().caps()->protectedSupport());

    auto info = SkImageInfo::Make({2, 1}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkAutoPixmapStorage basePM, topPM;
    basePM.alloc(info);
    basePM.erase(SK_ColorGREEN);
    topPM.alloc(info.makeDimensions({1, 1}));
    topPM.erase(SK_ColorBLUE);

    SkPixmap levelPMs[]{basePM, topPM};
    auto mbet = sk_gpu_test::ManagedGraphiteTexture::MakeMipmappedFromPixmaps(recorder.get(),
                                                                              levelPMs,
                                                                              Renderable::kNo,
                                                                              isProtected);

    if (!mbet) {
        ERRORF(reporter, "Could not make backend texture");
        return;
    }

    std::unique_ptr<Recording> recording = recorder->snap();

    auto recordingFinishProc = [](void* context, CallbackResult) {
        std::unique_ptr<Recording>(static_cast<Recording*>(context));
    };

    skgpu::graphite::InsertRecordingInfo recordingInfo;
    recordingInfo.fFinishedProc = recordingFinishProc;
    recordingInfo.fRecording = recording.get();
    recordingInfo.fFinishedContext = recording.release();
    if (!context->insertRecording(recordingInfo)) {
        ERRORF(reporter, "Could not insert recording");
        return;
    }

    static constexpr struct TestCase {
        SkImages::GenerateMipmapsFromBase genMipmaps;
        SkColor expectedColor;
    } kTestCases[]{{SkImages::GenerateMipmapsFromBase::kNo , 0xFFFF0000},
                   {SkImages::GenerateMipmapsFromBase::kYes, 0XFF00FF00}};

    for (const auto& testCase : kTestCases) {
        recorder = context->makeRecorder();
        if (!recorder) {
            ERRORF(reporter, "Could not make recorder");
            return;
        }

        auto image = SkImages::WrapTexture(recorder.get(),
                                           mbet->texture(),
                                           info.colorType(),
                                           info.alphaType(),
                                           info.refColorSpace(),
                                           Origin::kTopLeft,
                                           testCase.genMipmaps,
                                           sk_gpu_test::ManagedGraphiteTexture::ImageReleaseProc,
                                           mbet->releaseContext());
        if (!recorder) {
            ERRORF(reporter, "Could not make image");
            return;
        }

        // We determe the contents of the image's top level by doing a downsampling draw to a
        // surface and then reading the surface's contents.
        auto surface = SkSurfaces::RenderTarget(recorder.get(), info.makeDimensions({1, 1}));
        if (!recorder) {
            ERRORF(reporter, "Could not make surface");
            return;
        }

        auto shader = image->makeShader(
                SkTileMode::kRepeat,
                SkTileMode::kRepeat,
                SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNearest));

        surface->getCanvas()->scale(0.05f, 0.05f);
        SkPaint paint;
        paint.setShader(std::move(shader));
        surface->getCanvas()->drawPaint(paint);

        recording = recorder->snap();
        recordingInfo.fRecording = recording.get();
        recordingInfo.fFinishedContext = recording.release();
        if (!context->insertRecording(recordingInfo)) {
            ERRORF(reporter, "Could not insert recording");
            return;
        }

        struct ReadContext {
            bool called = false;
            bool success = false;
            uint32_t color;
        };
        auto readPixelsCallback = [](SkImage::ReadPixelsContext context,
                                     std::unique_ptr<const SkImage::AsyncReadResult> result) {
            auto& readContext = *static_cast<ReadContext*>(context);
            readContext.called = true;
            if (result) {
                readContext.success = true;
                readContext.color = *static_cast<const uint32_t*>(result->data(0));
            }
        };
        ReadContext readContext;
        context->asyncRescaleAndReadPixels(surface.get(),
                                           surface->imageInfo(),
                                           SkIRect::MakeSize(surface->imageInfo().dimensions()),
                                           SkImage::RescaleGamma::kSrc,
                                           SkImage::RescaleMode::kNearest,
                                           readPixelsCallback,
                                           &readContext);
        context->submit();
        while (!readContext.called) {
            testContext->tick();
            context->checkAsyncWorkCompletion();
        }

        if (!readContext.success) {
            ERRORF(reporter, "Read pixels failed");
            return;
        }

        REPORTER_ASSERT(reporter, readContext.color == testCase.expectedColor);
    }
}
