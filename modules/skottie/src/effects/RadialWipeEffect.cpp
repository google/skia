/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/core/SkCanvas.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkShaderMaskFilter.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "src/utils/SkJSON.h"

#include <cmath>

namespace skottie {
namespace internal {

namespace  {

class RWipeRenderNode final : public sksg::CustomRenderNode {
public:
    explicit RWipeRenderNode(sk_sp<sksg::RenderNode> layer)
        : INHERITED({std::move(layer)}) {}

    SG_ATTRIBUTE(Completion, float  , fCompletion)
    SG_ATTRIBUTE(StartAngle, float  , fStartAngle)
    SG_ATTRIBUTE(WipeCenter, SkPoint, fWipeCenter)
    SG_ATTRIBUTE(Wipe      , float  , fWipe      )
    SG_ATTRIBUTE(Feather   , float  , fFeather   )

protected:
    const RenderNode* onNodeAt(const SkPoint&) const override { return nullptr; } // no hit-testing

    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        if (fCompletion >= 100) {
            return SkRect::MakeEmpty();
        }

        static constexpr float kFeatherToSigma = 0.28f; // close enough to AE
        fMaskSigma = std::max(fFeather, 0.0f) * kFeatherToSigma;

        fMaskFilter = this->makeMaskFilter();

        SkASSERT(this->children().size() == 1ul);
        return this->children()[0]->revalidate(ic, ctm);
    }

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
        if (fCompletion >= 100) {
            // Fully masked out.
            return;
        }

        if (fCompletion <= 0) {
            // No mask.
            this->children()[0]->render(canvas, ctx);
            return;
        }

        SkAutoCanvasRestore acr(canvas, false);

        canvas->saveLayer(this->bounds(), nullptr);

        // First, draw the content.
        SkASSERT(this->children().size() == 1ul);
        this->children()[0]->render(canvas, ctx);

        // It's important for the mask rect to fully cover our layer
        // (lest it leaves bleeding artifacts around edges).
        //
        // Furthermore, feather requires a large enough outset to clip-out the edge blur.
        const auto mask_outset = std::max(fMaskSigma * 3,
                                          1 / std::min(canvas->getTotalMatrix().getScaleX(),
                                                       canvas->getTotalMatrix().getScaleY()));
        const auto mask_bounds = this->bounds().makeOutset(mask_outset, mask_outset);

        // Second, draw the mask, atomically.
        SkPaint mask_paint;
        mask_paint.setBlendMode(SkBlendMode::kDstOut);
        mask_paint.setMaskFilter(fMaskFilter);
        canvas->drawRect(mask_bounds, mask_paint);
    }

private:
    float wipeAlignment() const {
        switch (SkScalarRoundToInt(fWipe)) {
        case 1: return  0.0f; // Clockwise
        case 2: return -1.0f; // Counterclockwise
        case 3: return -0.5f; // Both/center
        default: break;
        }
        return 0.0f;
    }

    sk_sp<SkMaskFilter> makeMaskFilter() const {
        if (fCompletion <= 0) {
            return nullptr;
        }

        const auto fract = [](float v) { return v - std::floor(v); };

        auto t = fCompletion * 0.01f,
            p0 = fract((fStartAngle - 90.0f) * (1 / 360.0f) + t * this->wipeAlignment()),
            p1 = fract(p0 + t);

        SkColor c0 = 0x00000000,
                c1 = 0xffffffff;

        if (p0 > p1) {
            std::swap(p0, p1);
            std::swap(c0, c1);
        }

        const SkColor colors[] = { c0, c1, c1, c0 };
        const SkScalar   pos[] = { p0, p0, p1, p1 };

        auto mf = SkShaderMaskFilter::Make(
                    SkGradientShader::MakeSweep(fWipeCenter.x(), fWipeCenter.y(),
                                                colors, pos, SK_ARRAY_COUNT(colors)));

        if (fMaskSigma > 0) {
            mf = SkMaskFilter::MakeCompose(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, fMaskSigma),
                                           std::move(mf));
        }

        return mf;
    }

    SkPoint fWipeCenter = { 0, 0 };
    float   fCompletion = 0,
            fStartAngle = 0,
            fWipe       = 0,
            fFeather    = 0;

    // Cached during revalidation.
    sk_sp<SkMaskFilter> fMaskFilter;
    float               fMaskSigma; // edge feather/blur

    using INHERITED = sksg::CustomRenderNode;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachRadialWipeEffect(const skjson::ArrayValue& jprops,
                                                              sk_sp<sksg::RenderNode> layer) const {
    enum : size_t {
        kCompletion_Index = 0,
        kStartAngle_Index = 1,
        kWipeCenter_Index = 2,
        kWipe_Index       = 3,
        kFeather_Index    = 4,
    };

    auto wiper = sk_make_sp<RWipeRenderNode>(std::move(layer));

    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kCompletion_Index), fScope,
        [wiper](const ScalarValue& c) {
            wiper->setCompletion(c);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kStartAngle_Index), fScope,
        [wiper](const ScalarValue& sa) {
            wiper->setStartAngle(sa);
        });
    fBuilder->bindProperty<VectorValue>(GetPropValue(jprops, kWipeCenter_Index), fScope,
        [wiper](const VectorValue& c) {
            wiper->setWipeCenter(ValueTraits<VectorValue>::As<SkPoint>(c));
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kWipe_Index), fScope,
        [wiper](const ScalarValue& w) {
            wiper->setWipe(w);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kFeather_Index), fScope,
        [wiper](const ScalarValue& f) {
            wiper->setFeather(f);
        });

    return std::move(wiper);
}

} // namespace internal
} // namespace skottie
