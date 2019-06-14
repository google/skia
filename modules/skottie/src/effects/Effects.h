/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieEffects_DEFINED
#define SkottieEffects_DEFINED

#include "modules/skottie/src/SkottiePriv.h"

namespace skottie {
namespace internal {

class AnimationBuilder;

// TODO: relocate SkottieLayerEffect builder logic here.
class EffectBuilder final : public SkNoncopyable {
public:
    static const skjson::Value& GetPropValue(const skjson::ArrayValue& jprops, size_t prop_index);
};

sk_sp<sksg::RenderNode> AttachTransformEffect(const skjson::ArrayValue&,
                                              const AnimationBuilder*,
                                              AnimatorScope*,
                                              sk_sp<sksg::RenderNode>);


} // namespace internal
} // namespace skottie

#endif // SkottieEffects_DEFINED
