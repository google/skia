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

namespace skottie {
namespace internal {

sk_sp<sksg::Merge> ShapeBuilder::MergeGeometry(std::vector<sk_sp<sksg::GeometryNode>>&& geos,
                                               sksg::Merge::Mode mode) {
    std::vector<sksg::Merge::Rec> merge_recs;
    merge_recs.reserve(geos.size());

    for (auto& geo : geos) {
        merge_recs.push_back(
            {std::move(geo), merge_recs.empty() ? sksg::Merge::Mode::kMerge : mode});
    }

    return sksg::Merge::Make(std::move(merge_recs));
}

std::vector<sk_sp<sksg::GeometryNode>> ShapeBuilder::AttachMergeGeometryEffect(
        const skjson::ObjectValue& jmerge, const AnimationBuilder*,
        std::vector<sk_sp<sksg::GeometryNode>>&& geos) {
    static constexpr sksg::Merge::Mode gModes[] = {
        sksg::Merge::Mode::kMerge,      // "mm": 1
        sksg::Merge::Mode::kUnion,      // "mm": 2
        sksg::Merge::Mode::kDifference, // "mm": 3
        sksg::Merge::Mode::kIntersect,  // "mm": 4
        sksg::Merge::Mode::kXOR      ,  // "mm": 5
    };

    const auto mode = gModes[std::min<size_t>(ParseDefault<size_t>(jmerge["mm"], 1) - 1,
                                            SK_ARRAY_COUNT(gModes) - 1)];

    std::vector<sk_sp<sksg::GeometryNode>> merged;
    merged.push_back(ShapeBuilder::MergeGeometry(std::move(geos), mode));

    return merged;
}

} // namespace internal
} // namespace skottie
