/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkPath.h"
#include "src/core/SkCanvasPriv.h"
#include "tools/ToolUtils.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#endif

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
 * This test originally ensured that the ccpr path cache preserved fill rules properly. CCPR is gone
 * now, but we decided to keep the test.
 */
class PreserveFillRuleGM : public GM {
public:
    PreserveFillRuleGM(bool big) : fBig(big) , fStarSize((big) ? 200 : 20) {}

private:
    SkString getName() const override {
        SkString name("preservefillrule");
        name += (fBig) ? "_big" : "_little";
        return name;
    }
    SkISize getISize() override { return SkISize::Make(fStarSize * 2, fStarSize * 2); }

#if defined(SK_GANESH)
    void modifyGrContextOptions(GrContextOptions* ctxOptions) override {
        ctxOptions->fAllowPathMaskCaching = true;
    }
#endif

    void onDraw(SkCanvas* canvas) override {
        auto starRect = SkRect::MakeWH(fStarSize, fStarSize);
        SkPath star7_winding = ToolUtils::make_star(starRect, 7);
        star7_winding.setFillType(SkPathFillType::kWinding);

        SkPath star7_evenOdd = star7_winding.makeTransform(SkMatrix::Translate(0, fStarSize))
                                            .makeFillType(SkPathFillType::kEvenOdd);

        SkPath star5_winding = ToolUtils::make_star(starRect, 5)
                               .makeTransform(SkMatrix::Translate(fStarSize, 0))
                               .makeFillType(SkPathFillType::kWinding);

        SkPath star5_evenOdd = star5_winding.makeTransform(SkMatrix::Translate(0, fStarSize))
                                            .makeFillType(SkPathFillType::kEvenOdd);

        SkPaint paint;
        paint.setColor(SK_ColorGREEN);
        paint.setAntiAlias(true);

        canvas->clear(SK_ColorWHITE);
        canvas->drawPath(star7_winding, paint);
        canvas->drawPath(star7_evenOdd, paint);
        canvas->drawPath(star5_winding, paint);
        canvas->drawPath(star5_evenOdd, paint);

#if defined(SK_GANESH)
        if (auto dContext = GrAsDirectContext(canvas->recordingContext())) {
            dContext->flush();
        }
#endif
    }

private:
    const bool fBig;
    const int fStarSize;
};

DEF_GM( return new PreserveFillRuleGM(true); )
DEF_GM( return new PreserveFillRuleGM(false); )

}  // namespace skiagm
