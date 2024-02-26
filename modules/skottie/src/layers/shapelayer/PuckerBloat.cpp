/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkPoint_impl.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/layers/shapelayer/ShapeLayer.h"
#include "modules/sksg/include/SkSGGeometryEffect.h"
#include "modules/sksg/include/SkSGGeometryNode.h"
#include "modules/sksg/include/SkSGNode.h"
#include "src/core/SkGeometry.h"
#include "src/utils/SkJSON.h"

#include <utility>
#include <vector>

namespace skottie::internal {

namespace  {

static SkPoint lerp(const SkPoint& p0, const SkPoint& p1, SkScalar t) {
    return p0 + (p1 - p0) * t;
}

// Operates on the cubic representation of a shape.  Pulls vertices towards the shape center,
// and cubic control points away from the center.  The general shape center is the vertex average.
class PuckerBloatEffect final : public sksg::GeometryEffect {
public:
    explicit PuckerBloatEffect(sk_sp<sksg::GeometryNode> geo) : INHERITED({std::move(geo)}) {}

    // Fraction of the transition to center. I.e.
    //
    //     0 -> no effect
    //     1 -> vertices collapsed to center
    //
    // Negative values are allowed (inverse direction), as are extranormal values.
    SG_ATTRIBUTE(Amount, float, fAmount)

private:
    SkPath onRevalidateEffect(const sk_sp<GeometryNode>& geo) override {
        struct CubicInfo {
            SkPoint ctrl0, ctrl1, pt; // corresponding to SkPath::cubicTo() params, respectively.
        };

        const auto input = geo->asPath();
        if (SkScalarNearlyZero(fAmount)) {
            return input;
        }

        const auto input_bounds = input.computeTightBounds();
        const SkPoint center{input_bounds.centerX(), input_bounds.centerY()};

        SkPath path;

        SkPoint contour_start = {0, 0};
        std::vector<CubicInfo> cubics;

        auto commit_contour = [&]() {
            path.moveTo(lerp(contour_start, center, fAmount));
            for (const auto& c : cubics) {
                path.cubicTo(lerp(c.ctrl0, center, -fAmount),
                             lerp(c.ctrl1, center, -fAmount),
                             lerp(c.pt   , center,  fAmount));
            }
            path.close();

            cubics.clear();
        };

        // Normalize all verbs to cubic representation.
        SkPoint pts[4];
        SkPath::Verb verb;
        SkPath::Iter iter(input, true);
        while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kMove_Verb:
                    commit_contour();
                    contour_start = pts[0];
                    break;
                case SkPath::kLine_Verb: {
                    // Empirically, straight lines are treated as cubics with control points
                    // located length/100 away from extremities.
                    static constexpr float kCtrlPosFraction = 1.f / 100;
                    const auto line_start = pts[0],
                               line_end   = pts[1];
                    cubics.push_back({
                            lerp(line_start, line_end,     kCtrlPosFraction),
                            lerp(line_start, line_end, 1 - kCtrlPosFraction),
                            line_end
                    });
                } break;
                case SkPath::kQuad_Verb:
                    SkConvertQuadToCubic(pts, pts);
                    cubics.push_back({pts[1], pts[2], pts[3]});
                    break;
                case SkPath::kConic_Verb: {
                    // We should only ever encounter conics from circles/ellipses.
                    SkASSERT(SkScalarNearlyEqual(iter.conicWeight(), SK_ScalarRoot2Over2));

                    // http://spencermortensen.com/articles/bezier-circle/
                    static constexpr float kCubicCircleCoeff = 1 - 0.551915024494f;

                    const auto conic_start = cubics.empty() ? contour_start
                                                            : cubics.back().pt,
                               conic_end   = pts[2];

                    cubics.push_back({
                        lerp(pts[1], conic_start, kCubicCircleCoeff),
                        lerp(pts[1], conic_end  , kCubicCircleCoeff),
                        conic_end
                    });
                } break;
                case SkPath::kCubic_Verb:
                    cubics.push_back({pts[1], pts[2], pts[3]});
                    break;
                case SkPath::kClose_Verb:
                    commit_contour();
                    break;
                default:
                    break;
            }
        }

        return path;
    }

    float fAmount = 0;

    using INHERITED = sksg::GeometryEffect;
};

class PuckerBloatAdapter final : public DiscardableAdapterBase<PuckerBloatAdapter,
                                                               PuckerBloatEffect> {
public:
    PuckerBloatAdapter(const skjson::ObjectValue& joffset,
                       const AnimationBuilder& abuilder,
                       sk_sp<sksg::GeometryNode> child)
        : INHERITED(sk_make_sp<PuckerBloatEffect>(std::move(child))) {
        this->bind(abuilder, joffset["a" ], fAmount);
    }

private:
    void onSync() override {
        // AE amount is percentage-based.
        this->node()->setAmount(fAmount / 100);
    }

    ScalarValue fAmount = 0;

    using INHERITED = DiscardableAdapterBase<PuckerBloatAdapter, PuckerBloatEffect>;
};

} // namespace

std::vector<sk_sp<sksg::GeometryNode>> ShapeBuilder::AttachPuckerBloatGeometryEffect(
        const skjson::ObjectValue& jround, const AnimationBuilder* abuilder,
        std::vector<sk_sp<sksg::GeometryNode>>&& geos) {
    std::vector<sk_sp<sksg::GeometryNode>> bloated;
    bloated.reserve(geos.size());

    for (auto& g : geos) {
        bloated.push_back(abuilder->attachDiscardableAdapter<PuckerBloatAdapter>
                                        (jround, *abuilder, std::move(g)));
    }

    return bloated;
}

} // namespace skottie::internal
