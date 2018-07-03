/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieAdapter_DEFINED
#define SkottieAdapter_DEFINED

#include "SkMatrix.h"
#include "SkPoint.h"
#include "SkRefCnt.h"
#include "SkSize.h"

#include <vector>

namespace sksg {

class Gradient;
class Group;
class LinearGradient;
class Matrix;
class Path;
class RadialGradient;
class RenderNode;
class RRect;
class TrimEffect;
class Transform;

};

namespace skottie {

#define ADAPTER_PROPERTY_FUNC(p_name, p_type, p_default, func) \
    void set##p_name(const p_type& p) {                        \
        if (p == f##p_name) return;                            \
        f##p_name = p;                                         \
        this->func();                                          \
    }                                                          \
  private:                                                     \
    p_type f##p_name = p_default;                              \
  public:

#define ADAPTER_PROPERTY(p_name, p_type, p_default) \
    ADAPTER_PROPERTY_FUNC(p_name, p_type, p_default, apply)

class RRectAdapter final : public SkRefCnt {
public:
    explicit RRectAdapter(sk_sp<sksg::RRect>);

    ADAPTER_PROPERTY(Position, SkPoint , SkPoint::Make(0, 0))
    ADAPTER_PROPERTY(Size    , SkSize  ,  SkSize::Make(0, 0))
    ADAPTER_PROPERTY(Radius  , SkSize  ,  SkSize::Make(0, 0))

private:
    void apply();

    sk_sp<sksg::RRect> fRRectNode;

    using INHERITED = SkRefCnt;
};

class PolyStarAdapter final : public SkRefCnt {
public:
    enum class Type {
        kStar, kPoly,
    };

    PolyStarAdapter(sk_sp<sksg::Path>, Type);

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

    using INHERITED = SkRefCnt;
};

class TransformAdapter : public SkRefCnt {
public:
    ADAPTER_PROPERTY(AnchorPoint, SkPoint , SkPoint::Make(0, 0))
    ADAPTER_PROPERTY(Position   , SkPoint , SkPoint::Make(0, 0))
    ADAPTER_PROPERTY(Scale      , SkVector, SkPoint::Make(100, 100))
    ADAPTER_PROPERTY(Rotation   , SkScalar, 0)
    ADAPTER_PROPERTY(Skew       , SkScalar, 0)
    ADAPTER_PROPERTY(SkewAxis   , SkScalar, 0)

protected:
    virtual void onApplyTransform(const SkMatrix&) = 0;

private:
    void apply();

    using INHERITED = SkRefCnt;
};

class MatrixAdapter final : public TransformAdapter {
public:
    explicit MatrixAdapter(sk_sp<sksg::Matrix>);

protected:
    void onApplyTransform(const SkMatrix&) override;

private:
    sk_sp<sksg::Matrix> fMatrixNode;

    using INHERITED = TransformAdapter;
};

class GradientAdapter : public SkRefCnt {
public:
    ADAPTER_PROPERTY(StartPoint, SkPoint              , SkPoint::Make(0, 0)    )
    ADAPTER_PROPERTY(EndPoint  , SkPoint              , SkPoint::Make(0, 0)    )
    ADAPTER_PROPERTY(ColorStops, std::vector<SkScalar>, std::vector<SkScalar>())

protected:
    GradientAdapter(sk_sp<sksg::Gradient>, size_t stopCount);

    const SkPoint& startPoint() const { return fStartPoint; }
    const SkPoint& endPoint()   const { return fEndPoint;   }

    sk_sp<sksg::Gradient> fGradient;
    size_t                fStopCount;

    virtual void onApply() = 0;

private:
    void apply();

    using INHERITED = SkRefCnt;
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

class TrimEffectAdapter final : public SkRefCnt {
public:
    explicit TrimEffectAdapter(sk_sp<sksg::TrimEffect>);

    ADAPTER_PROPERTY(Start , SkScalar,   0)
    ADAPTER_PROPERTY(End   , SkScalar, 100)
    ADAPTER_PROPERTY(Offset, SkScalar,   0)

private:
    void apply();

    sk_sp<sksg::TrimEffect> fTrimEffect;

    using INHERITED = SkRefCnt;
};

class RepeaterAdapter final : public TransformAdapter {
public:
    explicit RepeaterAdapter(sk_sp<sksg::RenderNode>);

    ADAPTER_PROPERTY_FUNC(CopyCount, SkScalar,             0, applyRepeater)
    ADAPTER_PROPERTY_FUNC(Offset   , SkScalar,             0, applyRepeater)
    ADAPTER_PROPERTY_FUNC(Transform, SkMatrix, SkMatrix::I(), applyRepeater)

    const sk_sp<sksg::Group>& root() const { return fInstanceGroup; }

protected:
    void onApplyTransform(const SkMatrix&) override;

private:
    void applyRepeater();

    const sk_sp<sksg::RenderNode>       fRepeatedNode;
    const sk_sp<sksg::Group>            fInstanceGroup;

    std::vector<sk_sp<sksg::Transform>> fInstances;


    using INHERITED = TransformAdapter;
};

#undef ADAPTER_PROPERTY_FUNC
#undef ADAPTER_PROPERTY

} // namespace skottie

#endif // SkottieAdapter_DEFINED
