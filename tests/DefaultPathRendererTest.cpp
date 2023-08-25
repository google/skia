/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRect.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/SkColorData.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ContextType.h"

#include <memory>
#include <utility>

static void only_allow_default(GrContextOptions* options) {
    options->fGpuPathRenderers = GpuPathRenderers::kNone;
}

static SkBitmap read_back(GrDirectContext* dContext,
                          skgpu::ganesh::SurfaceDrawContext* sdc,
                          int width,
                          int height) {
    SkImageInfo dstII = SkImageInfo::MakeN32Premul(width, height);

    SkBitmap bm;
    bm.allocPixels(dstII);

    sdc->readPixels(dContext, bm.pixmap(), {0, 0});

    return bm;
}

static SkPath make_path(const SkRect& outer, int inset, SkPathFillType fill) {
    SkPath p;

    p.addRect(outer, SkPathDirection::kCW);
    p.addRect(outer.makeInset(inset, inset), SkPathDirection::kCCW);
    p.setFillType(fill);
    return p;
}


static const int kBigSize = 64; // This should be a power of 2
static const int kPad = 3;

// From crbug.com/769898:
//   create an approx fit render target context that will have extra space (i.e., npot)
//   draw an inverse wound concave path into it - forcing use of the stencil-using path renderer
//   throw the RTC away so the backing GrSurface/GrStencilBuffer can be reused
//   create a new render target context that will reuse the prior GrSurface
//   draw a normally wound concave path that touches outside of the approx fit RTC's content rect
//
// When the bug manifests the DefaultPathRenderer/GrMSAAPathRenderer is/was leaving the stencil
// buffer outside of the first content rect in a bad state and the second draw would be incorrect.

static void run_test(GrDirectContext* dContext, skiatest::Reporter* reporter) {
    SkPath invPath = make_path(SkRect::MakeXYWH(0, 0, kBigSize, kBigSize),
                               kBigSize/2-1, SkPathFillType::kInverseWinding);
    SkPath path = make_path(SkRect::MakeXYWH(0, 0, kBigSize, kBigSize),
                            kPad, SkPathFillType::kWinding);

    GrStyle style(SkStrokeRec::kFill_InitStyle);

    {
        auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(dContext,
                                                           GrColorType::kRGBA_8888,
                                                           nullptr,
                                                           SkBackingFit::kApprox,
                                                           {kBigSize / 2 + 1, kBigSize / 2 + 1},
                                                           SkSurfaceProps(),
                                                           /*label=*/{});

        sdc->clear(SK_PMColor4fBLACK);

        GrPaint paint;

        const SkPMColor4f color = { 1.0f, 0.0f, 0.0f, 1.0f };
        auto fp = GrFragmentProcessor::MakeColor(color);
        paint.setColorFragmentProcessor(std::move(fp));

        sdc->drawPath(nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(), invPath, style);

        dContext->priv().flushSurface(sdc->asSurfaceProxy());
    }

    {
        auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(dContext,
                                                           GrColorType::kRGBA_8888,
                                                           nullptr,
                                                           SkBackingFit::kExact,
                                                           {kBigSize, kBigSize},
                                                           SkSurfaceProps(),
                                                           /*label=*/{});

        sdc->clear(SK_PMColor4fBLACK);

        GrPaint paint;

        const SkPMColor4f color = { 0.0f, 1.0f, 0.0f, 1.0f };
        auto fp = GrFragmentProcessor::MakeColor(color);
        paint.setColorFragmentProcessor(std::move(fp));

        sdc->drawPath(nullptr, std::move(paint), GrAA::kNo,
                      SkMatrix::I(), path, style);

        SkBitmap bm = read_back(dContext, sdc.get(), kBigSize, kBigSize);

        bool correct = true;
        for (int y = kBigSize/2+1; y < kBigSize-kPad-1 && correct; ++y) {
            for (int x = kPad+1; x < kBigSize-kPad-1 && correct; ++x) {
                correct = bm.getColor(x, y) == SK_ColorBLACK;
                REPORTER_ASSERT(reporter, correct);
            }
        }
    }
}

DEF_GANESH_TEST_FOR_CONTEXTS(DefaultPathRendererTest,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             only_allow_default,
                             CtsEnforcement::kApiLevel_T) {
    auto ctx = ctxInfo.directContext();

    run_test(ctx, reporter);
}
