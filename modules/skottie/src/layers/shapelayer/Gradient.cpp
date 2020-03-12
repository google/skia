/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/layers/shapelayer/ShapeLayer.h"
#include "modules/sksg/include/SkSGGradient.h"
#include "modules/sksg/include/SkSGPaint.h"

namespace skottie {
namespace internal {

namespace  {

class GradientAdapter final : public AnimatablePropertyContainer {
public:
    static sk_sp<GradientAdapter> Make(const skjson::ObjectValue& jgrad,
                                       const AnimationBuilder& abuilder) {
        const skjson::ObjectValue* jstops = jgrad["g"];
        if (!jstops)
            return nullptr;

        const auto stopCount = ParseDefault<int>((*jstops)["p"], -1);
        if (stopCount < 0)
            return nullptr;

        const auto type = (ParseDefault<int>(jgrad["t"], 1) == 1) ? Type::kLinear
                                                                  : Type::kRadial;
        auto gradient_node = (type == Type::kLinear)
                ? sk_sp<sksg::Gradient>(sksg::LinearGradient::Make())
                : sk_sp<sksg::Gradient>(sksg::RadialGradient::Make());

        return sk_sp<GradientAdapter>(new GradientAdapter(std::move(gradient_node),
                                                          type,
                                                          SkToSizeT(stopCount),
                                                          jgrad, *jstops, abuilder));
    }

    const sk_sp<sksg::Gradient>& node() const { return fGradient; }

private:
    enum class Type { kLinear, kRadial };

    GradientAdapter(sk_sp<sksg::Gradient> gradient,
                    Type type,
                    size_t stop_count,
                    const skjson::ObjectValue& jgrad,
                    const skjson::ObjectValue& jstops,
                    const AnimationBuilder& abuilder)
        : fGradient(std::move(gradient))
        , fType(type)
        , fStopCount(stop_count) {
        this->bind(abuilder,  jgrad["s"], fStartPoint);
        this->bind(abuilder,  jgrad["e"], fEndPoint  );
        this->bind(abuilder, jstops["k"], fStops     );
    }

    void onSync() override {
        const auto s_point = SkPoint{fStartPoint.x, fStartPoint.y},
                   e_point = SkPoint{  fEndPoint.x,   fEndPoint.y};

        switch (fType) {
        case Type::kLinear: {
            auto* grad = static_cast<sksg::LinearGradient*>(fGradient.get());
            grad->setStartPoint(s_point);
            grad->setEndPoint(e_point);

            break;
        }
        case Type::kRadial: {
            auto* grad = static_cast<sksg::RadialGradient*>(fGradient.get());
            grad->setStartCenter(s_point);
            grad->setEndCenter(s_point);
            grad->setStartRadius(0);
            grad->setEndRadius(SkPoint::Distance(s_point, e_point));

            break;
        }
        }

        // Gradient color stops are specified as a consolidated float vector holding:
        //
        //   a) an (optional) array of color/RGB stop records (t, r, g, b)
        //
        // followed by
        //
        //   b) an (optional) array of opacity/alpha stop records (t, a)
        //
        struct   ColorRec { float t, r, g, b; };
        struct OpacityRec { float t, a;       };

        // The number of color records is explicit (fColorStopCount),
        // while the number of opacity stops is implicit (based on the size of fStops).
        //
        // |fStops| holds ColorRec x |fColorStopCount| + OpacityRec x N
        const auto c_count = fStopCount,
                   c_size  = c_count * 4,
                   o_count = (fStops.size() - c_size) / 2;
        if (fStops.size() < c_size || fStops.size() != (c_count * 4 + o_count * 2)) {
            // apply() may get called before the stops are set, so only log when we have some stops.
            if (!fStops.empty()) {
                SkDebugf("!! Invalid gradient stop array size: %zu\n", fStops.size());
            }
            return;
        }

        const auto* c_rec = c_count > 0
                ? reinterpret_cast<const ColorRec*>(fStops.data())
                : nullptr;
        const auto* o_rec = o_count > 0
                ? reinterpret_cast<const OpacityRec*>(fStops.data() + c_size)
                : nullptr;
        const auto* c_end = c_rec + c_count;
        const auto* o_end = o_rec + o_count;

        sksg::Gradient::ColorStop current_stop = {
            0.0f, {
                c_rec ? c_rec->r : 0,
                c_rec ? c_rec->g : 0,
                c_rec ? c_rec->b : 0,
                o_rec ? o_rec->a : 1,
        }};

        std::vector<sksg::Gradient::ColorStop> stops;
        stops.reserve(c_count);

        // Merge-sort the color and opacity stops, LERP-ing intermediate channel values as needed.
        while (c_rec || o_rec) {
            // After exhausting one of color recs / opacity recs, continue propagating the last
            // computed values (as if they were specified at the current position).
            const auto& cs = c_rec
                    ? *c_rec
                    : ColorRec{ o_rec->t,
                                current_stop.fColor.fR,
                                current_stop.fColor.fG,
                                current_stop.fColor.fB };
            const auto& os = o_rec
                    ? *o_rec
                    : OpacityRec{ c_rec->t, current_stop.fColor.fA };

            // Compute component lerp coefficients based on the relative position of the stops
            // being considered. The idea is to select the smaller-pos stop, use its own properties
            // as specified (lerp with t == 1), and lerp (with t < 1) the properties from the
            // larger-pos stop against the previously computed gradient stop values.
            const auto     c_pos = std::max(cs.t, current_stop.fPosition),
                           o_pos = std::max(os.t, current_stop.fPosition),
                       c_pos_rel = c_pos - current_stop.fPosition,
                       o_pos_rel = o_pos - current_stop.fPosition,
                             t_c = SkTPin(sk_ieee_float_divide(o_pos_rel, c_pos_rel), 0.0f, 1.0f),
                             t_o = SkTPin(sk_ieee_float_divide(c_pos_rel, o_pos_rel), 0.0f, 1.0f);

            auto lerp = [](float a, float b, float t) { return a + t * (b - a); };

            current_stop = {
                    std::min(c_pos, o_pos),
                    {
                        lerp(current_stop.fColor.fR, cs.r, t_c ),
                        lerp(current_stop.fColor.fG, cs.g, t_c ),
                        lerp(current_stop.fColor.fB, cs.b, t_c ),
                        lerp(current_stop.fColor.fA, os.a, t_o)
                    }
            };
            stops.push_back(current_stop);

            // Consume one of, or both (for coincident positions) color/opacity stops.
            if (c_pos <= o_pos) {
                c_rec = next_rec<ColorRec>(c_rec, c_end);
            }
            if (o_pos <= c_pos) {
                o_rec = next_rec<OpacityRec>(o_rec, o_end);
            }
        }

        stops.shrink_to_fit();
        fGradient->setColorStops(std::move(stops));
    }

private:
    template <typename T>
    const T* next_rec(const T* rec, const T* end_rec) const {
        if (!rec) return nullptr;

        SkASSERT(rec < end_rec);
        rec++;

        return rec < end_rec ? rec : nullptr;
    }

    const sk_sp<sksg::Gradient> fGradient;
    const Type                  fType;
    const size_t                fStopCount;

    VectorValue  fStops;
    Vec2Value    fStartPoint = {0,0},
                 fEndPoint   = {0,0};
};

} // namespace

sk_sp<sksg::PaintNode> ShapeBuilder::AttachGradientFill(const skjson::ObjectValue& jgrad,
                                                        const AnimationBuilder* abuilder) {
    auto adapter = GradientAdapter::Make(jgrad, *abuilder);

    return adapter
            ? AttachFill(jgrad, abuilder, sksg::ShaderPaint::Make(adapter->node()), adapter)
            : nullptr;
}

sk_sp<sksg::PaintNode> ShapeBuilder::AttachGradientStroke(const skjson::ObjectValue& jgrad,
                                                          const AnimationBuilder* abuilder) {
    auto adapter = GradientAdapter::Make(jgrad, *abuilder);

    return adapter
            ? AttachStroke(jgrad, abuilder, sksg::ShaderPaint::Make(adapter->node()), adapter)
            : nullptr;
}

} // namespace internal
} // namespace skottie
