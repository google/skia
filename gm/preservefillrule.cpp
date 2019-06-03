/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkPath.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/ccpr/GrCCPathCache.h"
#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"
#include "tools/ToolUtils.h"

namespace skiagm {

/**
 * This test ensures that the ccpr path cache preserves fill rules properly, both in the case where
 * we copy paths into a8 literal coverage atlases, as well as in the case where we just reuse a
 * stashed fp16 coverage count atlas.
 */
class PreserveFillRuleGM : public GpuGM {
public:
    PreserveFillRuleGM(bool literalCoverageAtlas)
            : fLiteralCoverageAtlas(literalCoverageAtlas)
            , fStarSize((fLiteralCoverageAtlas) ? 200 : 20) {
    }
private:
    SkString onShortName() override {
        SkString name("preservefillrule");
        name += (fLiteralCoverageAtlas) ? "_big" : "_little";
        return name;
    }
    SkISize onISize() override { return SkISize::Make(fStarSize * 2, fStarSize * 2); }

    void modifyGrContextOptions(GrContextOptions* ctxOptions) override {
        ctxOptions->fGpuPathRenderers = GpuPathRenderers::kCoverageCounting;
        ctxOptions->fAllowPathMaskCaching = true;
    }

    DrawResult onDraw(GrContext* ctx, GrRenderTargetContext* rtc, SkCanvas* canvas,
                      SkString* errorMsg) override {
        auto* ccpr = ctx->priv().drawingManager()->getCoverageCountingPathRenderer();
        if (!ccpr) {
            errorMsg->set("ccpr only");
            return DrawResult::kSkip;
        }

        auto pathCache = ccpr->testingOnly_getPathCache();
        if (!pathCache) {
            errorMsg->set("ccpr is not in caching mode. "
                          "Are you using viewer? Launch with \"--cachePathMasks true\".");
            return DrawResult::kFail;
        }

        auto starRect = SkRect::MakeWH(fStarSize, fStarSize);
        SkPath star7_winding = ToolUtils::make_star(starRect, 7);
        star7_winding.setFillType(SkPath::kWinding_FillType);

        SkPath star7_evenOdd = star7_winding;
        star7_evenOdd.transform(SkMatrix::MakeTrans(0, fStarSize));
        star7_evenOdd.setFillType(SkPath::kEvenOdd_FillType);

        SkPath star5_winding = ToolUtils::make_star(starRect, 5);
        star5_winding.transform(SkMatrix::MakeTrans(fStarSize, 0));
        star5_winding.setFillType(SkPath::kWinding_FillType);

        SkPath star5_evenOdd = star5_winding;
        star5_evenOdd.transform(SkMatrix::MakeTrans(0, fStarSize));
        star5_evenOdd.setFillType(SkPath::kEvenOdd_FillType);

        SkPaint paint;
        paint.setColor(SK_ColorGREEN);
        paint.setAntiAlias(true);

        for (int i = 0; i < 3; ++i) {
            canvas->clear(SK_ColorWHITE);
            canvas->drawPath(star7_winding, paint);
            canvas->drawPath(star7_evenOdd, paint);
            canvas->drawPath(star5_winding, paint);
            canvas->drawPath(star5_evenOdd, paint);
            rtc->flush(SkSurface::BackendSurfaceAccess::kNoAccess, GrFlushInfo());

#ifdef SK_DEBUG
            // Verify the path cache is behaving as expected from API usage. As long as these
            // asserts pass, then we know the ccpr path cache is preserving fill rules correctly
            // when the images look right.
            bool shouldHaveCoverageAtlas = (i >= 1);
            auto expectedCoverageType = (fLiteralCoverageAtlas && i >= 2)
                    ? GrCCAtlas::CoverageType::kA8_LiteralCoverage
                    : GrCCAtlas::CoverageType::kFP16_CoverageCount;

            int numCachedPaths = 0;
            for (GrCCPathCacheEntry* entry : pathCache->testingOnly_getLRU()) {
                SkASSERT(SkToBool(entry->cachedAtlas()) == shouldHaveCoverageAtlas);
                if (shouldHaveCoverageAtlas) {
                    SkASSERT(expectedCoverageType == entry->cachedAtlas()->coverageType());
                }
                ++numCachedPaths;
            }
            SkASSERT(4 == numCachedPaths);
#endif
        }

        return DrawResult::kOk;
    }

public:
    const bool fLiteralCoverageAtlas;
    const int fStarSize;
};

DEF_GM( return new PreserveFillRuleGM(true); )
DEF_GM( return new PreserveFillRuleGM(false); )

}
