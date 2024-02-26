/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkTPin.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/layers/shapelayer/ShapeLayer.h"
#include "modules/sksg/include/SkSGNode.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "src/utils/SkJSON.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

struct SkPoint;

namespace sksg {
class InvalidationController;
}

namespace skottie {
namespace internal {

namespace  {

class RepeaterRenderNode final : public sksg::CustomRenderNode {
public:
    enum class CompositeMode { kBelow, kAbove };

    RepeaterRenderNode(std::vector<sk_sp<RenderNode>>&& children, CompositeMode mode)
        : INHERITED(std::move(children))
        , fMode(mode) {}

    SG_ATTRIBUTE(Count       , size_t, fCount       )
    SG_ATTRIBUTE(Offset      , float , fOffset      )
    SG_ATTRIBUTE(AnchorPoint , SkV2  , fAnchorPoint )
    SG_ATTRIBUTE(Position    , SkV2  , fPosition    )
    SG_ATTRIBUTE(Scale       , SkV2  , fScale       )
    SG_ATTRIBUTE(Rotation    , float , fRotation    )
    SG_ATTRIBUTE(StartOpacity, float , fStartOpacity)
    SG_ATTRIBUTE(EndOpacity  , float , fEndOpacity  )

private:
    const RenderNode* onNodeAt(const SkPoint&) const override { return nullptr; } // no hit-testing

    SkMatrix instanceTransform(size_t i) const {
        const auto t = fOffset + i;

        // Position, scale & rotation are "scaled" by index/offset.
        return SkMatrix::Translate(t * fPosition.x + fAnchorPoint.x,
                                   t * fPosition.y + fAnchorPoint.y)
             * SkMatrix::RotateDeg(t * fRotation)
             * SkMatrix::Scale(std::pow(fScale.x, t),
                               std::pow(fScale.y, t))
             * SkMatrix::Translate(-fAnchorPoint.x,
                                   -fAnchorPoint.y);
    }

    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        fChildrenBounds = SkRect::MakeEmpty();
        for (const auto& child : this->children()) {
            fChildrenBounds.join(child->revalidate(ic, ctm));
        }

        auto bounds = SkRect::MakeEmpty();
        for (size_t i = 0; i < fCount; ++i) {
            bounds.join(this->instanceTransform(i).mapRect(fChildrenBounds));
        }

        return bounds;
    }

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
        // To cover the full opacity range, the denominator below should be (fCount - 1).
        // Interstingly, that's not what AE does.  Off-by-one bug?
        const auto dOpacity = fCount > 1 ? (fEndOpacity - fStartOpacity) / fCount : 0.0f;

        for (size_t i = 0; i < fCount; ++i) {
            const auto render_index = fMode == CompositeMode::kAbove ? i : fCount - i - 1;
            const auto opacity      = fStartOpacity + dOpacity * render_index;

            if (opacity <= 0) {
                continue;
            }

            SkAutoCanvasRestore acr(canvas, true);
            canvas->concat(this->instanceTransform(render_index));

            const auto& children = this->children();
            const auto local_ctx = ScopedRenderContext(canvas, ctx)
                                        .modulateOpacity(opacity)
                                        .setIsolation(fChildrenBounds,
                                                      canvas->getTotalMatrix(),
                                                      children.size() > 1);
            for (const auto& child : children) {
                child->render(canvas, local_ctx);
            }
        }
    }

    const CompositeMode           fMode;

    SkRect fChildrenBounds = SkRect::MakeEmpty(); // cached

    size_t fCount          = 0;
    float  fOffset         = 0,
           fRotation       = 0,
           fStartOpacity   = 1,
           fEndOpacity     = 1;
    SkV2   fAnchorPoint    = {0,0},
           fPosition       = {0,0},
           fScale          = {1,1};

    using INHERITED = sksg::CustomRenderNode;
};

class RepeaterAdapter final : public DiscardableAdapterBase<RepeaterAdapter, RepeaterRenderNode> {
public:
    RepeaterAdapter(const skjson::ObjectValue& jrepeater,
                    const skjson::ObjectValue& jtransform,
                    const AnimationBuilder& abuilder,
                    std::vector<sk_sp<sksg::RenderNode>>&& draws)
        : INHERITED(sk_make_sp<RepeaterRenderNode>(std::move(draws),
                                                   (ParseDefault(jrepeater["m"], 1) == 1)
                                                       ? RepeaterRenderNode::CompositeMode::kBelow
                                                       : RepeaterRenderNode::CompositeMode::kAbove))
    {
        this->bind(abuilder, jrepeater["c"], fCount);
        this->bind(abuilder, jrepeater["o"], fOffset);

        this->bind(abuilder, jtransform["a" ], fAnchorPoint);
        this->bind(abuilder, jtransform["p" ], fPosition);
        this->bind(abuilder, jtransform["s" ], fScale);
        this->bind(abuilder, jtransform["r" ], fRotation);
        this->bind(abuilder, jtransform["so"], fStartOpacity);
        this->bind(abuilder, jtransform["eo"], fEndOpacity);
    }

private:
    void onSync() override {
        static constexpr SkScalar kMaxCount = 1024;
        this->node()->setCount(static_cast<size_t>(SkTPin(fCount, 0.0f, kMaxCount) + 0.5f));
        this->node()->setOffset(fOffset);
        this->node()->setAnchorPoint(fAnchorPoint);
        this->node()->setPosition(fPosition);
        this->node()->setScale(fScale * 0.01f);
        this->node()->setRotation(fRotation);
        this->node()->setStartOpacity(SkTPin(fStartOpacity * 0.01f, 0.0f, 1.0f));
        this->node()->setEndOpacity  (SkTPin(fEndOpacity   * 0.01f, 0.0f, 1.0f));
    }

    // Repeater props
    ScalarValue fCount  = 0,
                fOffset = 0;

    // Transform props
    Vec2Value   fAnchorPoint  = {   0,   0 },
                fPosition     = {   0,   0 },
                fScale        = { 100, 100 };
    ScalarValue fRotation     = 0,
                fStartOpacity = 100,
                fEndOpacity   = 100;

    using INHERITED = DiscardableAdapterBase<RepeaterAdapter, RepeaterRenderNode>;
};

} // namespace

std::vector<sk_sp<sksg::RenderNode>> ShapeBuilder::AttachRepeaterDrawEffect(
        const skjson::ObjectValue& jrepeater,
        const AnimationBuilder* abuilder,
        std::vector<sk_sp<sksg::RenderNode>>&& draws) {
    std::vector<sk_sp<sksg::RenderNode>> repeater_draws;

    if (const skjson::ObjectValue* jtransform = jrepeater["tr"]) {
        // input draws are in top->bottom order - reverse for paint order
        std::reverse(draws.begin(), draws.end());

        repeater_draws.reserve(1);
        repeater_draws.push_back(
                    abuilder->attachDiscardableAdapter<RepeaterAdapter>(jrepeater,
                                                                        *jtransform,
                                                                        *abuilder,
                                                                        std::move(draws)));
    } else {
        repeater_draws = std::move(draws);
    }

    return repeater_draws;
}

} // namespace internal
} // namespace skottie
