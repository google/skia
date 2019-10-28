/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieLayer_DEFINED
#define SkottieLayer_DEFINED

#include "modules/skottie/src/SkottiePriv.h"

namespace skottie {
namespace internal {

class CompositionBuilder;

class LayerBuilder final {
public:
    explicit LayerBuilder(const skjson::ObjectValue& jlayer);
    ~LayerBuilder();

    int index() const { return fIndex; }
    int  type() const { return  fType; }

    bool isCamera() const;
    bool     is3D() const { return fFlags & Flags::kIs3D; }

    void buildTransform(const AnimationBuilder& abuilder, CompositionBuilder* cbuilder);

    sk_sp<sksg::RenderNode> build(const AnimationBuilder&, CompositionBuilder* cbuilder) const;

    enum TransformType : uint8_t {
        k2D = 0,
        k3D = 1,
    };

    sk_sp<sksg::Transform> getTransform(const AnimationBuilder&, CompositionBuilder*,
                                        TransformType) const;

private:
    sk_sp<sksg::Transform> getParentTransform(const AnimationBuilder&, CompositionBuilder*,
                                              TransformType) const;

    sk_sp<sksg::Transform> doAttachTransform(const AnimationBuilder&, CompositionBuilder*,
                                             TransformType) const;

    const skjson::ObjectValue& fJlayer;
    const int                  fIndex;
    const int                  fParentIndex;
    const int                  fType;

    sk_sp<sksg::Transform>         fLayerTransform;    // this layer's transform node.
    mutable sk_sp<sksg::Transform> fTransformCache[2]; // cached 2D/3D chain for local node

    mutable AnimatorScope          fLayerScope;
    mutable size_t                 fTransformAnimatorCount = 0;

    enum Flags {
        // k2DTransformValid = 0x01,  // reserved for cache tracking
        // k3DTransformValie = 0x02,  // reserved for cache tracking
        kIs3D                = 0x04,  // 3D layer ("ddd": 1) or camera layer
    };

    mutable uint32_t fFlags = 0;
};

} // namespace internal
} // namespace skottie

#endif // SkottieLayer_DEFINED
