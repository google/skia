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
        SkASSERT(this->children().size() == 1ul);
        const auto content_bounds = this->children()[0]->revalidate(ic, ctm);

        if (fCompletion >= 100) {
            return SkRect::MakeEmpty();
        }

        if (fCompletion <= 0) {
            fMaskSigma  = 0;
            fMaskFilter = nullptr;
        } else {
            static constexpr float kFeatherToSigma = 0.3f; // close enough to AE
            fMaskSigma = std::max(fFeather, 0.0f) * kFeatherToSigma;

            // The gradient is inverted between non-blurred and blurred (latter requires dstOut).
            const SkColor c0 = fMaskSigma > 0 ? 0xffffffff : 0x00000000,
                          c1 = 0xffffffff - c0;
            auto t = fCompletion * 0.01f;

            const SkColor grad_colors[] = { c0, c1 };
            const SkScalar   grad_pos[] = {  t,  t };

            SkMatrix lm;
            lm.setRotate(fStartAngle - 90 + t * this->wipeAlignment(),
                         fWipeCenter.x(), fWipeCenter.y());

            fMaskFilter = SkShaderMaskFilter::Make(
                            SkGradientShader::MakeSweep(fWipeCenter.x(), fWipeCenter.y(),
                                                        grad_colors, grad_pos,
                                                        SK_ARRAY_COUNT(grad_colors), 0, &lm));

            // Edge feather requires a real blur.
            if (fMaskSigma > 0) {
                fMaskFilter = SkMaskFilter::MakeCompose(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle,
                                                                               fMaskSigma),
                                                        std::move(fMaskFilter));
            }
        }

        return content_bounds;
    }

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
        if (fCompletion >= 100) {
            // Fully masked out.
            return;
        }

        if (!fMaskSigma) {
            // No mask filter, or a shader-only mask filter: we can draw the content directly.
            const auto local_ctx = ScopedRenderContext(canvas, ctx)
                                        .modulateMaskFilter(fMaskFilter, canvas->getTotalMatrix());
            this->children()[0]->render(canvas, local_ctx);
            return;
        }

        // Blurred mask filters require a separate layer.
        SkAutoCanvasRestore acr(canvas, false);
        canvas->saveLayer(this->bounds(), nullptr);

        this->children()[0]->render(canvas, ctx);

        // Outset the mask to clip-out any edge blur.
        const auto mask_bounds = this->bounds().makeOutset(fMaskSigma * 3, fMaskSigma * 3);

        SkPaint mask_paint;
        mask_paint.setBlendMode(SkBlendMode::kDstOut);
        mask_paint.setMaskFilter(fMaskFilter);
        canvas->drawRect(mask_bounds, mask_paint);
    }

private:
    float wipeAlignment() const {
        switch (SkScalarRoundToInt(fWipe)) {
        case 1: return    0.0f; // Clockwise
        case 2: return -360.0f; // Counterclockwise
        case 3: return -180.0f; // Both/center
        default: break;
        }
        return 0.0f;
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

    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kCompletion_Index),
        [wiper](const ScalarValue& c) {
            wiper->setCompletion(c);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kStartAngle_Index),
        [wiper](const ScalarValue& sa) {
            wiper->setStartAngle(sa);
        });
    fBuilder->bindProperty<VectorValue>(GetPropValue(jprops, kWipeCenter_Index),
        [wiper](const VectorValue& c) {
            wiper->setWipeCenter(ValueTraits<VectorValue>::As<SkPoint>(c));
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kWipe_Index),
        [wiper](const ScalarValue& w) {
            wiper->setWipe(w);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kFeather_Index),
        [wiper](const ScalarValue& f) {
            wiper->setFeather(f);
        });

    return wiper;
}

} // namespace internal
} // namespace skottie
