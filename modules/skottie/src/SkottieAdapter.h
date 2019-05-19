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

class TransformAdapter2D final : public SkNVRefCnt<TransformAdapter2D> {
public:
    explicit TransformAdapter2D(sk_sp<sksg::Matrix<SkMatrix>>);
    ~TransformAdapter2D();

    ADAPTER_PROPERTY(AnchorPoint, SkPoint , SkPoint::Make(0, 0))
    ADAPTER_PROPERTY(Position   , SkPoint , SkPoint::Make(0, 0))
    ADAPTER_PROPERTY(Scale      , SkVector, SkPoint::Make(100, 100))
    ADAPTER_PROPERTY(Rotation   , SkScalar, 0)
    ADAPTER_PROPERTY(Skew       , SkScalar, 0)
    ADAPTER_PROPERTY(SkewAxis   , SkScalar, 0)

    SkMatrix totalMatrix() const;

private:
    void apply();

    sk_sp<sksg::Matrix<SkMatrix>> fMatrixNode;
};

class TransformAdapter3D : public SkRefCnt {
public:
    TransformAdapter3D();
    ~TransformAdapter3D() override;

    struct Vec3 {
        float fX, fY, fZ;

        explicit Vec3(const VectorValue&);

        bool operator==(const Vec3& other) const {
            return fX == other.fX && fY == other.fY && fZ == other.fZ;
        }
        bool operator!=(const Vec3& other) const { return !(*this == other); }
    };

    ADAPTER_PROPERTY(AnchorPoint, Vec3, Vec3({  0,   0,   0}))
    ADAPTER_PROPERTY(Position   , Vec3, Vec3({  0,   0,   0}))
    ADAPTER_PROPERTY(Rotation   , Vec3, Vec3({  0,   0,   0}))
    ADAPTER_PROPERTY(Scale      , Vec3, Vec3({100, 100, 100}))

    sk_sp<sksg::Transform> refTransform() const;

protected:
    void apply();

private:
    virtual SkMatrix44 totalMatrix() const;

    sk_sp<sksg::Matrix<SkMatrix44>> fMatrixNode;

    using INHERITED = SkRefCnt;
};

class CameraAdapter final : public TransformAdapter3D {
public:
    explicit CameraAdapter(const SkSize& viewport_size);
    ~CameraAdapter() override;

    ADAPTER_PROPERTY(Zoom, SkScalar, 0)

private:
    SkMatrix44 totalMatrix() const override;

    const SkSize fViewportSize;

    using INHERITED = TransformAdapter3D;
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
    ADAPTER_PROPERTY(ColorStops, VectorValue    , VectorValue()         )

protected:
    GradientAdapter(sk_sp<sksg::Gradient>, size_t stopCount);

    const SkPoint& startPoint() const { return fStartPoint; }
    const SkPoint& endPoint()   const { return fEndPoint;   }

    sk_sp<sksg::Gradient> fGradient;
    size_t                fStopCount;

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

class GradientRampEffectAdapter final : public SkNVRefCnt<GradientRampEffectAdapter> {
public:
    explicit GradientRampEffectAdapter(sk_sp<sksg::RenderNode> child);
    ~GradientRampEffectAdapter();

    ADAPTER_PROPERTY(StartPoint, SkPoint , SkPoint::Make(0, 0))
    ADAPTER_PROPERTY(EndPoint  , SkPoint , SkPoint::Make(0, 0))
    ADAPTER_PROPERTY(StartColor, SkColor ,       SK_ColorBLACK)
    ADAPTER_PROPERTY(EndColor  , SkColor ,       SK_ColorBLACK)
    ADAPTER_PROPERTY(Blend     , SkScalar,                   0)
    ADAPTER_PROPERTY(Scatter   , SkScalar,                   0)

    // Really an enum: 1 -> linear, 7 -> radial (?!)
    ADAPTER_PROPERTY(Shape     , SkScalar,                   0)

    const sk_sp<sksg::ShaderEffect>& root() const { return fRoot; }

private:
    enum class InstanceType {
        kNone,
        kLinear,
        kRadial,
    };

    void apply();

    sk_sp<sksg::ShaderEffect> fRoot;
    sk_sp<sksg::Gradient>     fGradient;
    InstanceType              fInstanceType = InstanceType::kNone;
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

class DropShadowEffectAdapter final : public SkNVRefCnt<DropShadowEffectAdapter> {
public:
    explicit DropShadowEffectAdapter(sk_sp<sksg::DropShadowImageFilter>);
    ~DropShadowEffectAdapter();

    ADAPTER_PROPERTY(Color     , SkColor , SK_ColorBLACK)
    ADAPTER_PROPERTY(Opacity   , SkScalar,           255)
    ADAPTER_PROPERTY(Direction , SkScalar,             0)
    ADAPTER_PROPERTY(Distance  , SkScalar,             0)
    ADAPTER_PROPERTY(Softness  , SkScalar,             0)
    ADAPTER_PROPERTY(ShadowOnly, bool    ,         false)

private:
    void apply();

    const sk_sp<sksg::DropShadowImageFilter> fDropShadow;
};

class GaussianBlurEffectAdapter final : public SkNVRefCnt<GaussianBlurEffectAdapter> {
public:
    explicit GaussianBlurEffectAdapter(sk_sp<sksg::BlurImageFilter>);
    ~GaussianBlurEffectAdapter();

    // AE/BM model properties.  These are all animatable/interpolatable.

    // Controls the blur sigma.
    ADAPTER_PROPERTY(Blurriness, SkScalar, 0)

    // Enum selecting the blur dimensionality:
    //
    //   1 -> horizontal & vertical
    //   2 -> horizontal
    //   3 -> vertical
    //
    ADAPTER_PROPERTY(Dimensions, SkScalar, 1)

    // Enum selecting edge behavior:
    //
    //   0 -> clamp
    //   1 -> repeat
    //
    ADAPTER_PROPERTY(RepeatEdge, SkScalar, 0)

private:
    void apply();

    const sk_sp<sksg::BlurImageFilter> fBlur;
};

class LevelsEffectAdapter final : public SkNVRefCnt<LevelsEffectAdapter> {
public:
    explicit LevelsEffectAdapter(sk_sp<sksg::RenderNode> child);
    ~LevelsEffectAdapter();

    // 1: RGB, 2: R, 3: G, 4: B, 5: A
    ADAPTER_PROPERTY(  Channel, SkScalar, 1)
    ADAPTER_PROPERTY(  InBlack, SkScalar, 0)
    ADAPTER_PROPERTY(  InWhite, SkScalar, 1)
    ADAPTER_PROPERTY( OutBlack, SkScalar, 0)
    ADAPTER_PROPERTY( OutWhite, SkScalar, 1)
    ADAPTER_PROPERTY(    Gamma, SkScalar, 1)
    // 1: clip, 2,3: don't clip
    ADAPTER_PROPERTY(ClipBlack, SkScalar, 1)
    ADAPTER_PROPERTY(ClipWhite, SkScalar, 1)

    const sk_sp<sksg::ExternalColorFilter>& root() const { return fEffect; }

private:
    void apply();

    sk_sp<sksg::ExternalColorFilter> fEffect;
};

} // namespace skottie

#endif // SkottieAdapter_DEFINED
