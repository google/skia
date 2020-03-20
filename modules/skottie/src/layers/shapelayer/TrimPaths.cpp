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
#include "modules/sksg/include/SkSGMerge.h"
#include "modules/sksg/include/SkSGTrimEffect.h"

#include <vector>

namespace skottie {
namespace internal {

namespace  {

class TrimEffectAdapter final : public DiscardableAdapterBase<TrimEffectAdapter, sksg::TrimEffect> {
public:
    TrimEffectAdapter(const skjson::ObjectValue& jtrim,
                      const AnimationBuilder& abuilder,
                      sk_sp<sksg::GeometryNode> child)
        : INHERITED(sksg::TrimEffect::Make(std::move(child))) {
        this->bind(abuilder, jtrim["s"], &fStart);
        this->bind(abuilder, jtrim["e"], &fEnd);
        this->bind(abuilder, jtrim["o"], &fOffset);
    }

private:
    void onSync() override {
        // BM semantics: start/end are percentages, offset is "degrees" (?!).
        const auto  start = fStart  / 100,
                      end = fEnd    / 100,
                   offset = fOffset / 360;

        auto startT = std::min(start, end) + offset,
              stopT = std::max(start, end) + offset;
        auto   mode = SkTrimPathEffect::Mode::kNormal;

        if (stopT - startT < 1) {
            startT -= SkScalarFloorToScalar(startT);
            stopT  -= SkScalarFloorToScalar(stopT);

            if (startT > stopT) {
                using std::swap;
                swap(startT, stopT);
                mode = SkTrimPathEffect::Mode::kInverted;
            }
        } else {
            startT = 0;
            stopT  = 1;
        }

        this->node()->setStart(startT);
        this->node()->setStop(stopT);
        this->node()->setMode(mode);
    }

    ScalarValue fStart  =   0,
                fEnd    = 100,
                fOffset =   0;

    using INHERITED = DiscardableAdapterBase<TrimEffectAdapter, sksg::TrimEffect>;
};

} // namespace

std::vector<sk_sp<sksg::GeometryNode>> ShapeBuilder::AttachTrimGeometryEffect(
        const skjson::ObjectValue& jtrim,
        const AnimationBuilder* abuilder,
        std::vector<sk_sp<sksg::GeometryNode>>&& geos) {

    enum class Mode {
        kParallel, // "m": 1 (Trim Multiple Shapes: Simultaneously)
        kSerial,   // "m": 2 (Trim Multiple Shapes: Individually)
    } gModes[] = { Mode::kParallel, Mode::kSerial};

    const auto mode = gModes[std::min<size_t>(ParseDefault<size_t>(jtrim["m"], 1) - 1,
                                            SK_ARRAY_COUNT(gModes) - 1)];

    std::vector<sk_sp<sksg::GeometryNode>> inputs;
    if (mode == Mode::kSerial) {
        inputs.push_back(ShapeBuilder::MergeGeometry(std::move(geos), sksg::Merge::Mode::kMerge));
    } else {
        inputs = std::move(geos);
    }

    std::vector<sk_sp<sksg::GeometryNode>> trimmed;
    trimmed.reserve(inputs.size());

    for (const auto& i : inputs) {
        trimmed.push_back(
            abuilder->attachDiscardableAdapter<TrimEffectAdapter>(jtrim, *abuilder, i));
    }

    return trimmed;
}

} // namespace internal
} // namespace skottie
