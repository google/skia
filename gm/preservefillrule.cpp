/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkPath.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
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
 * This test originally ensured that the ccpr path cache preserved fill rules properly. CCRP is gone
 * now, but we decided to keep the test.
 */
class PreserveFillRuleGM : public GpuGM {
public:
    PreserveFillRuleGM(bool big) : fBig(big) , fStarSize((big) ? 200 : 20) {}

private:
    SkString onShortName() override {
        SkString name("preservefillrule");
        name += (fBig) ? "_big" : "_little";
        return name;
    }
    SkISize onISize() override { return SkISize::Make(fStarSize * 2, fStarSize * 2); }

    void modifyGrContextOptions(GrContextOptions* ctxOptions) override {
        ctxOptions->fAllowPathMaskCaching = true;
    }

    DrawResult onDraw(GrRecordingContext* rContext, SkCanvas* canvas, SkString* errorMsg) override {
        auto dContext = GrAsDirectContext(rContext);
        auto sfc = SkCanvasPriv::TopDeviceSurfaceFillContext(canvas);
        if (!dContext || !sfc) {
            *errorMsg = "Requires a direct context.";
            return skiagm::DrawResult::kSkip;
        }

        auto starRect = SkRect::MakeWH(fStarSize, fStarSize);
        SkPath star7_winding = ToolUtils::make_star(starRect, 7);
        star7_winding.setFillType(SkPathFillType::kWinding);

        SkPath star7_evenOdd = star7_winding;
        star7_evenOdd.transform(SkMatrix::Translate(0, fStarSize));
        star7_evenOdd.setFillType(SkPathFillType::kEvenOdd);

        SkPath star5_winding = ToolUtils::make_star(starRect, 5);
        star5_winding.transform(SkMatrix::Translate(fStarSize, 0));
        star5_winding.setFillType(SkPathFillType::kWinding);

        SkPath star5_evenOdd = star5_winding;
        star5_evenOdd.transform(SkMatrix::Translate(0, fStarSize));
        star5_evenOdd.setFillType(SkPathFillType::kEvenOdd);

        SkPaint paint;
        paint.setColor(SK_ColorGREEN);
        paint.setAntiAlias(true);

        canvas->clear(SK_ColorWHITE);
        canvas->drawPath(star7_winding, paint);
        canvas->drawPath(star7_evenOdd, paint);
        canvas->drawPath(star5_winding, paint);
        canvas->drawPath(star5_evenOdd, paint);
        dContext->priv().flushSurface(sfc->asSurfaceProxy());

        return DrawResult::kOk;
    }

private:
    const bool fBig;
    const int fStarSize;
};

DEF_GM( return new PreserveFillRuleGM(true); )
DEF_GM( return new PreserveFillRuleGM(false); )

}  // namespace skiagm
