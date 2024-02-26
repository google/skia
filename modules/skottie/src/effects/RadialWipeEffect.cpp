/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkPoint_impl.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/effects/Effects.h"
#include "modules/sksg/include/SkSGNode.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

class SkMatrix;

namespace skjson {
class ArrayValue;
}
namespace sksg {
class InvalidationController;
}

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
            fMaskShader = nullptr;
        } else {
            fMaskSigma = std::max(fFeather, 0.0f) * kBlurSizeToSigma;

            const auto t = fCompletion * 0.01f;

            // Note: this could be simplified as a one-hard-stop gradient + local matrix
            // (to apply rotation).  Alas, local matrices are no longer supported in SkSG.
            SkColor c0 = 0x00000000,
                    c1 = 0xffffffff;
            auto sanitize_angle = [](float a) {
                a = std::fmod(a, 360);
                if (a < 0) {
                    a += 360;
                }
                return a;
            };

            auto a0 = sanitize_angle(fStartAngle - 90 + t * this->wipeAlignment()),
                 a1 = sanitize_angle(a0 + t * 360);
            if (a0 > a1) {
                std::swap(a0, a1);
                std::swap(c0, c1);
            }

            const SkColor grad_colors[] = { c1, c0, c0, c1 };
            const SkScalar   grad_pos[] = {  0,  0,  1,  1 };

            fMaskShader = SkGradientShader::MakeSweep(fWipeCenter.x(), fWipeCenter.y(),
                                                      grad_colors, grad_pos,
                                                      std::size(grad_colors),
                                                      SkTileMode::kClamp,
                                                      a0, a1, 0, nullptr);

            // Edge feather requires a real blur.
            if (fMaskSigma > 0) {
                // TODO: this feature is disabled ATM.
            }
        }

        return content_bounds;
    }

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
        if (fCompletion >= 100) {
            // Fully masked out.
            return;
        }

        const auto local_ctx = ScopedRenderContext(canvas, ctx)
                                    .modulateMaskShader(fMaskShader, canvas->getTotalMatrix());
        this->children()[0]->render(canvas, local_ctx);
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
    sk_sp<SkShader> fMaskShader;
    float           fMaskSigma; // edge feather/blur

    using INHERITED = sksg::CustomRenderNode;
};

class RadialWipeAdapter final : public DiscardableAdapterBase<RadialWipeAdapter, RWipeRenderNode> {
public:
    RadialWipeAdapter(const skjson::ArrayValue& jprops,
                      sk_sp<sksg::RenderNode> layer,
                      const AnimationBuilder& abuilder)
        : INHERITED(sk_make_sp<RWipeRenderNode>(std::move(layer))) {

        enum : size_t {
            kCompletion_Index = 0,
            kStartAngle_Index = 1,
            kWipeCenter_Index = 2,
                  kWipe_Index = 3,
               kFeather_Index = 4,
        };

        EffectBinder(jprops, abuilder, this)
            .bind(kCompletion_Index, fCompletion)
            .bind(kStartAngle_Index, fStartAngle)
            .bind(kWipeCenter_Index, fWipeCenter)
            .bind(      kWipe_Index, fWipe      )
            .bind(   kFeather_Index, fFeather   );
    }

private:
    void onSync() override {
        const auto& wiper = this->node();

        wiper->setCompletion(fCompletion);
        wiper->setStartAngle(fStartAngle);
        wiper->setWipeCenter({fWipeCenter.x, fWipeCenter.y});
        wiper->setWipe(fWipe);
        wiper->setFeather(fFeather);
    }

    Vec2Value   fWipeCenter = {0,0};
    ScalarValue fCompletion = 0,
                fStartAngle = 0,
                fWipe       = 0,
                fFeather    = 0;

    using INHERITED = DiscardableAdapterBase<RadialWipeAdapter, RWipeRenderNode>;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachRadialWipeEffect(const skjson::ArrayValue& jprops,
                                                              sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<RadialWipeAdapter>(jprops,
                                                                 std::move(layer),
                                                                 *fBuilder);
}

} // namespace internal
} // namespace skottie
