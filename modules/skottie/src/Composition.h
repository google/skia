/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieComposition_DEFINED
#define SkottieComposition_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/private/base/SkNoncopyable.h"
#include "modules/skottie/src/Layer.h"
#include "src/core/SkTHash.h"

#include <cstddef>
#include <vector>

namespace skjson {
class ObjectValue;
}
namespace sksg {
class RenderNode;
class Transform;
}  // namespace sksg

namespace skottie {
namespace internal {
class AnimationBuilder;

class CompositionBuilder final : SkNoncopyable {
public:
    CompositionBuilder(const AnimationBuilder&, const SkSize&, const skjson::ObjectValue&);
    ~CompositionBuilder();

    sk_sp<sksg::RenderNode> build(const AnimationBuilder&);

    LayerBuilder* layerBuilder(int layer_index);

private:
    const sk_sp<sksg::Transform>& getCameraTransform() const { return fCameraTransform; }

    friend class LayerBuilder;

    const SkSize              fSize;

    std::vector<LayerBuilder> fLayerBuilders;
    skia_private::THashMap<int, size_t>   fLayerIndexMap; // Maps layer "ind" to layer builder index.

    sk_sp<sksg::Transform>    fCameraTransform;

    size_t                    fMotionBlurSamples = 1;
    float                     fMotionBlurAngle   = 0,
                              fMotionBlurPhase   = 0;
};

} // namespace internal
} // namespace skottie

#endif // SkottieComposition_DEFINED
