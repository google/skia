/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieComposition_DEFINED
#define SkottieComposition_DEFINED

#include "modules/skottie/src/SkottiePriv.h"

#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "modules/skottie/src/Layer.h"

namespace skottie {
namespace internal {

class CompositionBuilder final : SkNoncopyable {
public:
    CompositionBuilder(const AnimationBuilder&, const skjson::ObjectValue&);
    ~CompositionBuilder();

    sk_sp<sksg::RenderNode> build(const AnimationBuilder&);

    const LayerBuilder* layerBuilder(int layer_index) const;

    const sk_sp<sksg::Transform>& getCameraTransform() const { return fCameraTransform; }

    void pushMatte(sk_sp<sksg::RenderNode>);
    sk_sp<sksg::RenderNode> popMatte();

    bool hasMotionBlur(const skjson::ObjectValue&) const;

private:
    friend class LayerBuilder;

    SkSTArray<64, LayerBuilder> fLayerBuilders;
    SkTHashMap<int, size_t>     fLayerIndexMap; // maps layer "ind" to layer builder index.

    sk_sp<sksg::Transform>      fCameraTransform;
    int                         fCameraBuilderIndex = -1;
    bool                        fHas3D              = false;

    sk_sp<sksg::RenderNode>     fCurrentMatte;

    size_t                      fMotionBlurSamples  = 1;
    float                       fMotionBlurAngle    = 0,
                                fMotionBlurPhase    = 0;
};

} // namespace internal
} // namespace skottie

#endif // SkottieComposition_DEFINED
