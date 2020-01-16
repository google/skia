/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieAdapter_DEFINED
#define SkottieAdapter_DEFINED

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

class RRectAdapter final : public SkNVRefCnt<RRectAdapter> {
public:
    explicit RRectAdapter(sk_sp<sksg::RRect>);
    ~RRectAdapter();

    ADAPTER_PROPERTY(Position, SkPoint , SkPoint::Make(0, 0))
    ADAPTER_PROPERTY(Size    , SkSize  ,  SkSize::Make(0, 0))
    ADAPTER_PROPERTY(Radius  , SkSize  ,  SkSize::Make(0, 0))

private:
    void apply();

    sk_sp<sksg::RRect> fRRectNode;
};

class PolyStarAdapter final : public SkNVRefCnt<PolyStarAdapter> {
public:
    enum class Type {
        kStar, kPoly,
    };

    PolyStarAdapter(sk_sp<sksg::Path>, Type);
    ~PolyStarAdapter();

    ADAPTER_PROPERTY(Position      , SkPoint , SkPoint::Make(0, 0))
    ADAPTER_PROPERTY(PointCount    , SkScalar, 0)
    ADAPTER_PROPERTY(InnerRadius   , SkScalar, 0)
    ADAPTER_PROPERTY(OuterRadius   , SkScalar, 0)
    ADAPTER_PROPERTY(InnerRoundness, SkScalar, 0)
    ADAPTER_PROPERTY(OuterRoundness, SkScalar, 0)
    ADAPTER_PROPERTY(Rotation      , SkScalar, 0)

private:
    void apply();

    sk_sp<sksg::Path> fPathNode;
    Type              fType;
};

class RepeaterAdapter final : public SkNVRefCnt<RepeaterAdapter> {
public:
    enum class Composite { kAbove, kBelow };

    RepeaterAdapter(sk_sp<sksg::RenderNode>, Composite);
    ~RepeaterAdapter();

    // Repeater props
    ADAPTER_PROPERTY(Count       , SkScalar, 0)
    ADAPTER_PROPERTY(Offset      , SkScalar, 0)

    // Transform props
    ADAPTER_PROPERTY(AnchorPoint , SkPoint , SkPoint::Make(0, 0))
    ADAPTER_PROPERTY(Position    , SkPoint , SkPoint::Make(0, 0))
    ADAPTER_PROPERTY(Scale       , SkVector, SkPoint::Make(100, 100))
    ADAPTER_PROPERTY(Rotation    , SkScalar, 0)
    ADAPTER_PROPERTY(StartOpacity, SkScalar, 100)
    ADAPTER_PROPERTY(EndOpacity  , SkScalar, 100)

    const sk_sp<sksg::Group>& root() const { return fRoot; }

private:
    void apply();

    const sk_sp<sksg::RenderNode> fRepeaterNode;
    const Composite               fComposite;

    sk_sp<sksg::Group>            fRoot;
};

class GradientAdapter : public SkRefCnt {
public:
    ADAPTER_PROPERTY(StartPoint, SkPoint        , SkPoint::Make(0, 0)   )
    ADAPTER_PROPERTY(EndPoint  , SkPoint        , SkPoint::Make(0, 0)   )
    ADAPTER_PROPERTY(Stops     , VectorValue    , VectorValue()         )

protected:
    GradientAdapter(sk_sp<sksg::Gradient>, size_t colorStopCount);

    const SkPoint& startPoint() const { return fStartPoint; }
    const SkPoint& endPoint()   const { return fEndPoint;   }

    sk_sp<sksg::Gradient> fGradient;
    size_t                fColorStopCount;

    virtual void onApply() = 0;

private:
    void apply();
};

class LinearGradientAdapter final : public GradientAdapter {
public:
    LinearGradientAdapter(sk_sp<sksg::LinearGradient>, size_t stopCount);

private:
    void onApply() override;

    using INHERITED = GradientAdapter;
};

class RadialGradientAdapter final : public GradientAdapter {
public:
    RadialGradientAdapter(sk_sp<sksg::RadialGradient>, size_t stopCount);

private:
    void onApply() override;

    using INHERITED = GradientAdapter;
};

class TrimEffectAdapter final : public SkNVRefCnt<TrimEffectAdapter> {
public:
    explicit TrimEffectAdapter(sk_sp<sksg::TrimEffect>);
    ~TrimEffectAdapter();

    ADAPTER_PROPERTY(Start , SkScalar,   0)
    ADAPTER_PROPERTY(End   , SkScalar, 100)
    ADAPTER_PROPERTY(Offset, SkScalar,   0)

private:
    void apply();

    sk_sp<sksg::TrimEffect> fTrimEffect;
};

} // namespace skottie

#endif // SkottieAdapter_DEFINED
