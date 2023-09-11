/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file
*/

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/gpu/ganesh/image/SkSpecialImage_Ganesh.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <initializer_list>
struct GrContextOptions;

static const int kSurfaceSize = 10;

// Exercise the public API of SkSpecialSurface (e.g., getCanvas, newImageSnapshot)
static void test_surface(const sk_sp<SkSpecialSurface>& surf,
                         skiatest::Reporter* reporter,
                         int offset) {

    const SkIRect surfSubset = surf->subset();
    REPORTER_ASSERT(reporter, offset == surfSubset.fLeft);
    REPORTER_ASSERT(reporter, offset == surfSubset.fTop);
    REPORTER_ASSERT(reporter, kSurfaceSize == surfSubset.width());
    REPORTER_ASSERT(reporter, kSurfaceSize == surfSubset.height());

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT_RELEASE(canvas);

    canvas->clear(SK_ColorRED);

    sk_sp<SkSpecialImage> img(surf->makeImageSnapshot());
    REPORTER_ASSERT(reporter, img);

    const SkIRect imgSubset = img->subset();
    REPORTER_ASSERT(reporter, surfSubset == imgSubset);

    // the canvas was invalidated by the newImageSnapshot call
    REPORTER_ASSERT(reporter, !surf->getCanvas());
}

DEF_TEST(SpecialSurface_Raster, reporter) {

    SkImageInfo info = SkImageInfo::MakeN32(kSurfaceSize, kSurfaceSize, kOpaque_SkAlphaType);
    sk_sp<SkSpecialSurface> surf(SkSpecialSurfaces::MakeRaster(info, SkSurfaceProps()));

    test_surface(surf, reporter, 0);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SpecialSurface_Gpu1,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();

    for (auto colorType : { kRGBA_8888_SkColorType, kRGBA_1010102_SkColorType }) {
        if (!dContext->colorTypeSupportedAsSurface(colorType)) {
            continue;
        }

        SkImageInfo ii = SkImageInfo::Make({ kSurfaceSize, kSurfaceSize }, colorType,
                                           kPremul_SkAlphaType);

        auto surf(SkSpecialSurfaces::MakeRenderTarget(dContext, ii, SkSurfaceProps(),
                                                      kTopLeft_GrSurfaceOrigin));
        test_surface(surf, reporter, 0);
    }
}

#if defined(SK_GRAPHITE)

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/SpecialImage_Graphite.h"

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SpecialSurface_Graphite, reporter, context,
                                         CtsEnforcement::kNextRelease) {
    using namespace skgpu::graphite;

    auto caps = context->priv().caps();
    auto recorder = context->makeRecorder();

    for (auto colorType : { kRGBA_8888_SkColorType, kRGBA_1010102_SkColorType }) {
        TextureInfo info = caps->getDefaultSampledTextureInfo(colorType,
                                                              skgpu::Mipmapped::kNo,
                                                              skgpu::Protected::kNo,
                                                              skgpu::Renderable::kYes);
        if (!info.isValid()) {
            continue;
        }

        SkImageInfo ii = SkImageInfo::Make({ kSurfaceSize, kSurfaceSize }, colorType,
                                           kPremul_SkAlphaType);

        auto surf(SkSpecialSurfaces::MakeGraphite(recorder.get(), ii, SkSurfaceProps()));
        test_surface(surf, reporter, 0);
    }
}

#endif // SK_GRAPHITE
