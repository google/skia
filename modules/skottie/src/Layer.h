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

struct AnimationBuilder::AttachLayerContext {
    AttachLayerContext(const skjson::ArrayValue&, AnimatorScope*);
    ~AttachLayerContext();

    const skjson::ArrayValue&               fLayerList;
    AnimatorScope*                          fScope;
    SkTHashMap<int, sk_sp<sksg::Transform>> fLayerMatrixMap;
    sk_sp<sksg::RenderNode>                 fCurrentMatte;
    sk_sp<sksg::Transform>                  fCameraTransform;

    enum class TransformType { kLayer, kCamera };

    sk_sp<sksg::Transform> attachLayerTransform(const skjson::ObjectValue& jlayer,
                                                const AnimationBuilder* abuilder,
                                                TransformType type = TransformType::kLayer);

private:
    sk_sp<sksg::Transform> attachParentLayerTransform(const skjson::ObjectValue& jlayer,
                                                      const AnimationBuilder* abuilder,
                                                      int layer_index);

    sk_sp<sksg::Transform> attachTransformNode(const skjson::ObjectValue& jlayer,
                                               const AnimationBuilder* abuilder,
                                               sk_sp<sksg::Transform> parent_transform,
                                               TransformType type) const;

    sk_sp<sksg::Transform> attachLayerTransformImpl(const skjson::ObjectValue& jlayer,
                                                    const AnimationBuilder* abuilder,
                                                    TransformType type, int layer_index);
};

} // namespace internal
} // namespace skottie

#endif // SkottieLayer_DEFINED
