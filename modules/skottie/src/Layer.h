/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieLayer_DEFINED
#define SkottieLayer_DEFINED

#include <cstddef>
#include <cstdint>
#include "include/core/SkRefCnt.h"
#include "modules/skottie/src/SkottiePriv.h"

struct SkSize;

namespace skjson {
class ObjectValue;
}
namespace sksg {
class RenderNode;
class Transform;
}  // namespace sksg

namespace skottie {
namespace internal {

class CompositionBuilder;

class LayerBuilder final {
public:
    LayerBuilder(const skjson::ObjectValue& jlayer, const SkSize& comp_size);
    LayerBuilder(const LayerBuilder&) = default;
    ~LayerBuilder();

    int index() const { return fIndex; }

    bool isCamera() const;

    // Attaches the local and ancestor transform chain for the layer "native" type.
    sk_sp<sksg::Transform> buildTransform(const AnimationBuilder&, CompositionBuilder*);

    // Attaches the actual layer content and finalizes its render tree.  Called once per layer.
    sk_sp<sksg::RenderNode> buildRenderTree(const AnimationBuilder&, CompositionBuilder*,
                                            const LayerBuilder* prev_layer);

    const sk_sp<sksg::RenderNode>& contentTree() const { return fContentTree; }

    const SkSize& size() const { return fInfo.fSize; }

private:
    enum TransformType : uint8_t {
        k2D = 0,
        k3D = 1,
    };

    enum Flags {
        // k2DTransformValid = 0x01,  // reserved for cache tracking
        // k3DTransformValie = 0x02,  // reserved for cache tracking
        kIs3D                = 0x04,  // 3D layer ("ddd": 1) or camera layer
    };

    bool is3D() const { return fFlags & Flags::kIs3D; }

    bool hasMotionBlur(const CompositionBuilder*) const;

    // Attaches (if needed) and caches the transform chain for a given layer,
    // as either a 2D or 3D chain type.
    // Called transitively (and possibly repeatedly) to resolve layer parenting.
    sk_sp<sksg::Transform> getTransform(const AnimationBuilder&, CompositionBuilder*,
                                        TransformType);

    sk_sp<sksg::Transform> getParentTransform(const AnimationBuilder&, CompositionBuilder*,
                                              TransformType);

    sk_sp<sksg::Transform> doAttachTransform(const AnimationBuilder&, CompositionBuilder*,
                                             TransformType);

    const skjson::ObjectValue& fJlayer;
    const int                  fIndex;
    const int                  fParentIndex;
    const int                  fType;
    const bool                 fAutoOrient;

    AnimationBuilder::LayerInfo fInfo;
    sk_sp<sksg::Transform>     fLayerTransform;             // this layer's transform node.
    sk_sp<sksg::Transform>     fTransformCache[2];          // cached 2D/3D chain for the local node
    sk_sp<sksg::RenderNode>    fContentTree;                // render tree for layer content,
                                                            // excluding mask/matte and blending

    AnimatorScope              fLayerScope;                 // layer-scoped animators
    size_t                     fTransformAnimatorCount = 0; // transform-related animator count
    uint32_t                   fFlags                  = 0;
};

} // namespace internal
} // namespace skottie

#endif // SkottieLayer_DEFINED
