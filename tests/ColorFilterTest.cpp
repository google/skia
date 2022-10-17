/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkColorMatrix.h" // IWYU pragma: keep
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrDirectContext.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <cstddef>
#include <utility>

class SkArenaAlloc;
struct GrContextOptions;
struct SkStageRec;

static sk_sp<SkColorFilter> reincarnate_colorfilter(SkFlattenable* obj) {
    SkBinaryWriteBuffer wb;
    wb.writeFlattenable(obj);

    size_t size = wb.bytesWritten();
    SkAutoSMalloc<1024> storage(size);
    // make a copy into storage
    wb.writeToMemory(storage.get());

    SkReadBuffer rb(storage.get(), size);
    return rb.readColorFilter();
}

///////////////////////////////////////////////////////////////////////////////

#define ILLEGAL_MODE    ((SkBlendMode)-1)

DEF_TEST(ColorFilter, reporter) {
    SkRandom rand;

    for (int mode = 0; mode < kSkBlendModeCount; mode++) {
        SkColor color = rand.nextU();

        // ensure we always get a filter, by avoiding the possibility of a
        // special case that would return nullptr (if color's alpha is 0 or 0xFF)
        color = SkColorSetA(color, 0x7F);

        auto cf = SkColorFilters::Blend(color, (SkBlendMode)mode);

        // allow for no filter if we're in Dst mode (its a no op)
        if (SkBlendMode::kDst == (SkBlendMode)mode && nullptr == cf) {
            continue;
        }

        REPORTER_ASSERT(reporter, cf);

        SkColor c = ~color;
        SkBlendMode m = ILLEGAL_MODE;

        SkColor expectedColor = color;
        SkBlendMode expectedMode = (SkBlendMode)mode;

//        SkDebugf("--- mc [%d %x] ", mode, color);

        REPORTER_ASSERT(reporter, cf->asAColorMode(&c, (SkBlendMode*)&m));
        // handle special-case folding by the factory
        if (SkBlendMode::kClear == (SkBlendMode)mode) {
            if (c != expectedColor) {
                expectedColor = 0;
            }
            if (m != expectedMode) {
                expectedMode = SkBlendMode::kSrc;
            }
        }

//        SkDebugf("--- got [%d %x] expected [%d %x]\n", m, c, expectedMode, expectedColor);

        REPORTER_ASSERT(reporter, c == expectedColor);
        REPORTER_ASSERT(reporter, m == expectedMode);

        {
            auto cf2 = reincarnate_colorfilter(cf.get());
            REPORTER_ASSERT(reporter, cf2);

            SkColor c2 = ~color;
            SkBlendMode m2 = ILLEGAL_MODE;
            REPORTER_ASSERT(reporter, cf2->asAColorMode(&c2, (SkBlendMode*)&m2));
            REPORTER_ASSERT(reporter, c2 == expectedColor);
            REPORTER_ASSERT(reporter, m2 == expectedMode);
        }
    }
}

DEF_TEST(WorkingFormatFilterFlags, r) {
    {
        // A matrix with final row 0,0,0,1,0 shouldn't change alpha.
        sk_sp<SkColorFilter> cf = SkColorFilters::Matrix({1,0,0,0,0,
                                                          0,1,0,0,0,
                                                          0,0,1,0,0,
                                                          0,0,0,1,0});
        REPORTER_ASSERT(r, cf->isAlphaUnchanged());

        // No working format change will itself change alpha.
        SkAlphaType unpremul = kUnpremul_SkAlphaType;
        cf = SkColorFilterPriv::WithWorkingFormat(std::move(cf),
                                                  &SkNamedTransferFn::kLinear,
                                                  &SkNamedGamut::kDisplayP3,
                                                  &unpremul);
        REPORTER_ASSERT(r, cf->isAlphaUnchanged());
    }

    {
        // Here's a matrix that definitely does change alpha.
        sk_sp<SkColorFilter> cf = SkColorFilters::Matrix({1,0,0,0,0,
                                                          0,1,0,0,0,
                                                          0,0,1,0,0,
                                                          0,0,0,0,1});
        REPORTER_ASSERT(r, !cf->isAlphaUnchanged());

        SkAlphaType unpremul = kUnpremul_SkAlphaType;
        cf = SkColorFilterPriv::WithWorkingFormat(std::move(cf),
                                                  &SkNamedTransferFn::kLinear,
                                                  &SkNamedGamut::kDisplayP3,
                                                  &unpremul);
        REPORTER_ASSERT(r, !cf->isAlphaUnchanged());
    }
}

struct FailureColorFilter final : public SkColorFilterBase {
    skvm::Color onProgram(skvm::Builder*,
                          skvm::Color c,
                          const SkColorInfo&,
                          skvm::Uniforms*,
                          SkArenaAlloc*) const override {
        return {};
    }

    bool onAppendStages(const SkStageRec&, bool) const override { return false; }

    // Only created here, should never be flattened / unflattened.
    Factory getFactory() const override { return nullptr; }
    const char* getTypeName() const override { return "FailureColorFilter"; }
};

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(ComposeFailureWithInputElision,
                                 r,
                                 ctxInfo,
                                 CtsEnforcement::kApiLevel_T) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(8, 8);
    auto surface = SkSurface::MakeRenderTarget(ctxInfo.directContext(), SkBudgeted::kNo, info);
    SkPaint paint;

    // Install a non-trivial shader, so the color filter isn't just applied to the paint color:
    const SkPoint pts[] = {{0, 0}, {100, 100}};
    const SkColor colors[] = {SK_ColorWHITE, SK_ColorBLACK};
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp));

    // Our inner (first) color filter does a "blend" (kSrc) against green, *discarding* the input:
    auto inner = SkColorFilters::Blend(SK_ColorGREEN, SkBlendMode::kSrc);

    // The outer (second) color filter then fails to generate an FP. There are ways to do this with
    // the public API, (eg: image shader with a non-invertible local matrix, wrapped in a runtime
    // color filter). That's significant boilerplate, so we use a helpful "always fail" filter:
    auto outer = sk_make_sp<FailureColorFilter>();
    paint.setColorFilter(outer->makeComposed(inner));

    // At one time, this would trigger a use-after-free / crash, when converting the paint to FPs:
    surface->getCanvas()->drawPaint(paint);
}
