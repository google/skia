/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieAdapter__DEFINED
#define SkottieAdapter__DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGScene.h"

namespace sksg {

class BlurImageFilter;
class Color;
class Draw;
class DropShadowImageFilter;
class ExternalColorFilter;
class Gradient;
class Group;
class LinearGradient;
template <typename>
class Matrix;
class Path;
class RadialGradient;
class RenderNode;
class RRect;
class ShaderEffect;
class Transform;
class TransformEffect;
class TrimEffect;

};

namespace skjson {
    class ObjectValue;
}

namespace skottie {

#define ADAPTER_PROPERTY(p_name, p_type, p_default) \
    const p_type& get##p_name() const {             \
        return f##p_name;                           \
    }                                               \
    void set##p_name(const p_type& p) {             \
        if (p == f##p_name) return;                 \
        f##p_name = p;                              \
        this->apply();                              \
    }                                               \
  private:                                          \
    p_type f##p_name = p_default;                   \
  public:

} // namespace skottie

#endif // SkottieAdapter__DEFINED
