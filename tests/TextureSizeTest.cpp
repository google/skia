/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#endif

#if defined(SK_GANESH)
#include "include/gpu/ganesh/SkImageGanesh.h"
#endif

#include <cstddef>
#include <functional>
#include <initializer_list>

class GrDirectContext;
struct GrContextOptions;

namespace {

void run_test(skiatest::Reporter* reporter,
              std::function<sk_sp<SkImage>(SkImage*)> convert2gpu) {

    for (auto ct : { kRGBA_8888_SkColorType, kRGBA_8888_SkColorType }) {
        SkImageInfo ii = SkImageInfo::Make(16, 16, ct, kPremul_SkAlphaType);

        SkBitmap src;
        src.allocPixels(ii);
        src.eraseColor(SK_ColorWHITE);

        sk_sp<SkImage> raster = src.asImage();

        sk_sp<SkImage> gpu = convert2gpu(raster.get());

        int bytesPerPixel = SkColorTypeBytesPerPixel(ct);

        size_t expectedSize = bytesPerPixel * gpu->width() * gpu->height();

        size_t actualSize = gpu->textureSize();

        REPORTER_ASSERT(reporter, actualSize == expectedSize,
                        "Expected: %zu  Actual: %zu", expectedSize, actualSize);
    }
}

} // anonymous namespace

#if defined(SK_GANESH)

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(ImageSizeTest_Ganesh,
                                 reporter,
                                 ctxInfo,
                                 CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();

    run_test(reporter,
             [&](SkImage* src) -> sk_sp<SkImage> {
                 return SkImages::TextureFromImage(dContext, src);
             });

}

#endif // SK_GANESH

#if defined(SK_GRAPHITE)

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ImageSizeTest_Graphite, reporter, context) {
    using namespace skgpu::graphite;

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    run_test(reporter,
             [&](SkImage* src) -> sk_sp<SkImage> {
                 sk_sp<SkImage> tmp = SkImages::TextureFromImage(recorder.get(), src, {});
                 std::unique_ptr<Recording> recording = recorder->snap();
                 context->insertRecording({ recording.get() });
                 context->submit(SyncToCpu::kYes);
                 return tmp;
             });
}

#endif // SK_GRAPHITE
