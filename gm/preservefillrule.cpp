/*
 * Copyright 2019 Google LLC.
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

#define ERR_MSG_ASSERT(COND) \
    do { \
        if (!(COND)) { \
            errorMsg->printf("preservefillrule.cpp(%i): assert(%s)", \
                             __LINE__, #COND); \
            return DrawResult::kFail; \
        } \
    } while (false)


/**
 * This test ensures that the ccpr path cache preserves fill rules properly, both in the case where
 * we copy paths into a8 literal coverage atlases, as well as in the case where we just reuse a
 * stashed fp16 coverage count atlas.
 */
class PreserveFillRuleGM : public GpuGM {
public:
    // fStarSize affects whether ccpr copies the paths to an a8 literal coverage atlas, or just
    // leaves them stashed in an fp16 coverage count atlas. The threshold for copying to a8 is
    // currently 256x256 total pixels copied. If this ever changes, there is code in onDraw that
    // will detect the unexpected behavior and draw a failure message.
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
        using CoverageType = GrCCAtlas::CoverageType;

        if (rtc->numSamples() > 1) {
            errorMsg->set("ccpr is currently only used for coverage AA");
            return DrawResult::kSkip;
        }

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
        star7_winding.setFillType(SkPathFillType::kWinding);

        SkPath star7_evenOdd = star7_winding;
        star7_evenOdd.transform(SkMatrix::MakeTrans(0, fStarSize));
        star7_evenOdd.setFillType(SkPathFillType::kEvenOdd);

        SkPath star5_winding = ToolUtils::make_star(starRect, 5);
        star5_winding.transform(SkMatrix::MakeTrans(fStarSize, 0));
        star5_winding.setFillType(SkPathFillType::kWinding);

        SkPath star5_evenOdd = star5_winding;
        star5_evenOdd.transform(SkMatrix::MakeTrans(0, fStarSize));
        star5_evenOdd.setFillType(SkPathFillType::kEvenOdd);

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

            // Ensure the path cache is behaving in such a way that we are actually testing what we
            // think we are.
            int numCachedPaths = 0;
            for (GrCCPathCacheEntry* entry : pathCache->testingOnly_getLRU()) {
                if (0 == i) {
                    // We don't cache an atlas on the first hit.
                    ERR_MSG_ASSERT(!entry->cachedAtlas());
                } else {
                    // The stars should be cached in an atlas now.
                    ERR_MSG_ASSERT(entry->cachedAtlas());

                    CoverageType atlasCoverageType = entry->cachedAtlas()->coverageType();
                    if (i < 2) {
                        // We never copy to an a8 atlas before the second hit.
                        ERR_MSG_ASSERT(ccpr->coverageType() == atlasCoverageType);
                    } else if (fLiteralCoverageAtlas) {
                        // Verify fStarSize is large enough that the paths got copied to an a8
                        // atlas.
                        ERR_MSG_ASSERT(CoverageType::kA8_LiteralCoverage == atlasCoverageType);
                    } else {
                        // Verify fStarSize is small enough that the paths did *NOT* get copied to
                        // an a8 atlas.
                        ERR_MSG_ASSERT(ccpr->coverageType() == atlasCoverageType);
                    }
                }
                ++numCachedPaths;
            }
            // Verify all 4 paths are tracked by the path cache.
            ERR_MSG_ASSERT(4 == numCachedPaths);
        }

        return DrawResult::kOk;
    }

private:
    const bool fLiteralCoverageAtlas;
    const int fStarSize;
};

DEF_GM( return new PreserveFillRuleGM(true); )
DEF_GM( return new PreserveFillRuleGM(false); )

}
