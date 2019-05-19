/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/SkottieAdapter.h"

#include "include/core/SkFont.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkMatrix44.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/effects/SkTableColorFilter.h"
#include "include/private/SkTo.h"
#include "include/utils/Sk3D.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "modules/sksg/include/SkSGDraw.h"
#include "modules/sksg/include/SkSGGradient.h"
#include "modules/sksg/include/SkSGGroup.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGPath.h"
#include "modules/sksg/include/SkSGRect.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "modules/sksg/include/SkSGTransform.h"
#include "modules/sksg/include/SkSGTrimEffect.h"

#include <cmath>
#include <utility>

namespace skottie {

RRectAdapter::RRectAdapter(sk_sp<sksg::RRect> wrapped_node)
    : fRRectNode(std::move(wrapped_node)) {}

RRectAdapter::~RRectAdapter() = default;

void RRectAdapter::apply() {
    // BM "position" == "center position"
    auto rr = SkRRect::MakeRectXY(SkRect::MakeXYWH(fPosition.x() - fSize.width() / 2,
                                                   fPosition.y() - fSize.height() / 2,
                                                   fSize.width(), fSize.height()),
                                  fRadius.width(),
                                  fRadius.height());
   fRRectNode->setRRect(rr);
}

TransformAdapter2D::TransformAdapter2D(sk_sp<sksg::Matrix<SkMatrix>> matrix)
    : fMatrixNode(std::move(matrix)) {}

TransformAdapter2D::~TransformAdapter2D() = default;

SkMatrix TransformAdapter2D::totalMatrix() const {
    SkMatrix t = SkMatrix::MakeTrans(-fAnchorPoint.x(), -fAnchorPoint.y());

    t.postScale(fScale.x() / 100, fScale.y() / 100); // 100% based
    t.postRotate(fRotation);
    t.postTranslate(fPosition.x(), fPosition.y());
    // TODO: skew

    return t;
}

void TransformAdapter2D::apply() {
    fMatrixNode->setMatrix(this->totalMatrix());
}

TransformAdapter3D::Vec3::Vec3(const VectorValue& v) {
    fX = v.size() > 0 ? v[0] : 0;
    fY = v.size() > 1 ? v[1] : 0;
    fZ = v.size() > 2 ? v[2] : 0;
}

TransformAdapter3D::TransformAdapter3D()
    : fMatrixNode(sksg::Matrix<SkMatrix44>::Make(SkMatrix::I())) {}

TransformAdapter3D::~TransformAdapter3D() = default;

sk_sp<sksg::Transform> TransformAdapter3D::refTransform() const {
    return fMatrixNode;
}

SkMatrix44 TransformAdapter3D::totalMatrix() const {
    SkMatrix44 t;

    t.setTranslate(-fAnchorPoint.fX, -fAnchorPoint.fY, -fAnchorPoint.fZ);
    t.postScale(fScale.fX / 100, fScale.fY / 100, fScale.fZ / 100);

    SkMatrix44 r;
    r.setRotateDegreesAbout(0, 0, 1, fRotation.fZ);
    t.postConcat(r);
    r.setRotateDegreesAbout(0, 1, 0, fRotation.fY);
    t.postConcat(r);
    r.setRotateDegreesAbout(1, 0, 0, fRotation.fX);
    t.postConcat(r);

    t.postTranslate(fPosition.fX, fPosition.fY, fPosition.fZ);

    return t;
}

void TransformAdapter3D::apply() {
    fMatrixNode->setMatrix(this->totalMatrix());
}

CameraAdapter:: CameraAdapter(const SkSize& viewport_size)
    : fViewportSize(viewport_size) {}

CameraAdapter::~CameraAdapter() = default;

SkMatrix44 CameraAdapter::totalMatrix() const {
    // Camera parameters:
    //
    //   * location          -> position attribute
    //   * point of interest -> anchor point attribute
    //   * orientation       -> rotation attribute
    //
    SkPoint3 pos = { this->getPosition().fX,
                     this->getPosition().fY,
                    -this->getPosition().fZ },
             poi = { this->getAnchorPoint().fX,
                     this->getAnchorPoint().fY,
                    -this->getAnchorPoint().fZ },
              up = { 0, 1, 0 };

    // Initial camera vector.
    SkMatrix44 cam_t;
    Sk3LookAt(&cam_t, pos, poi, up);

    // Rotation origin is camera position.
    {
        SkMatrix44 rot;
        rot.setRotateDegreesAbout(1, 0, 0,  this->getRotation().fX);
        cam_t.postConcat(rot);
        rot.setRotateDegreesAbout(0, 1, 0,  this->getRotation().fY);
        cam_t.postConcat(rot);
        rot.setRotateDegreesAbout(0, 0, 1, -this->getRotation().fZ);
        cam_t.postConcat(rot);
    }

    // Flip world Z, as it is opposite of what Sk3D expects.
    cam_t.preScale(1, 1, -1);

    // View parameters:
    //
    //   * size     -> composition size (TODO: AE seems to base it on width only?)
    //   * distance -> "zoom" camera attribute
    //
    const auto view_size     = SkTMax(fViewportSize.width(), fViewportSize.height()),
               view_distance = this->getZoom(),
               view_angle    = std::atan(view_size * 0.5f / view_distance);

    SkMatrix44 persp_t;
    Sk3Perspective(&persp_t, 0, view_distance, 2 * view_angle);
    persp_t.postScale(view_size * 0.5f, view_size * 0.5f, 1);

    SkMatrix44 t;
    t.setTranslate(fViewportSize.width() * 0.5f, fViewportSize.height() * 0.5f, 0);
    t.preConcat(persp_t);
    t.preConcat(cam_t);

    return t;
}

RepeaterAdapter::RepeaterAdapter(sk_sp<sksg::RenderNode> repeater_node, Composite composite)
    : fRepeaterNode(repeater_node)
    , fComposite(composite)
    , fRoot(sksg::Group::Make()) {}

RepeaterAdapter::~RepeaterAdapter() = default;

void RepeaterAdapter::apply() {
    static constexpr SkScalar kMaxCount = 512;
    const auto count = static_cast<size_t>(SkTPin(fCount, 0.0f, kMaxCount) + 0.5f);

    const auto& compute_transform = [this] (size_t index) {
        const auto t = fOffset + index;

        // Position, scale & rotation are "scaled" by index/offset.
        SkMatrix m = SkMatrix::MakeTrans(-fAnchorPoint.x(),
                                         -fAnchorPoint.y());
        m.postScale(std::pow(fScale.x() * .01f, fOffset),
                    std::pow(fScale.y() * .01f, fOffset));
        m.postRotate(t * fRotation);
        m.postTranslate(t * fPosition.x() + fAnchorPoint.x(),
                        t * fPosition.y() + fAnchorPoint.y());

        return m;
    };

    // TODO: start/end opacity support.

    // TODO: we can avoid rebuilding all the fragments in most cases.
    fRoot->clear();
    for (size_t i = 0; i < count; ++i) {
        const auto insert_index = (fComposite == Composite::kAbove) ? i : count - i - 1;
        fRoot->addChild(sksg::TransformEffect::Make(fRepeaterNode,
                                                    compute_transform(insert_index)));
    }
}

PolyStarAdapter::PolyStarAdapter(sk_sp<sksg::Path> wrapped_node, Type t)
    : fPathNode(std::move(wrapped_node))
    , fType(t) {}

PolyStarAdapter::~PolyStarAdapter() = default;

void PolyStarAdapter::apply() {
    static constexpr int kMaxPointCount = 100000;
    const auto count = SkToUInt(SkTPin(SkScalarRoundToInt(fPointCount), 0, kMaxPointCount));
    const auto arc   = sk_ieee_float_divide(SK_ScalarPI * 2, count);

    const auto pt_on_circle = [](const SkPoint& c, SkScalar r, SkScalar a) {
        return SkPoint::Make(c.x() + r * std::cos(a),
                             c.y() + r * std::sin(a));
    };

    // TODO: inner/outer "roundness"?

    SkPath poly;

    auto angle = SkDegreesToRadians(fRotation - 90);
    poly.moveTo(pt_on_circle(fPosition, fOuterRadius, angle));
    poly.incReserve(fType == Type::kStar ? count * 2 : count);

    for (unsigned i = 0; i < count; ++i) {
        if (fType == Type::kStar) {
            poly.lineTo(pt_on_circle(fPosition, fInnerRadius, angle + arc * 0.5f));
        }
        angle += arc;
        poly.lineTo(pt_on_circle(fPosition, fOuterRadius, angle));
    }

    poly.close();
    fPathNode->setPath(poly);
}

GradientAdapter::GradientAdapter(sk_sp<sksg::Gradient> grad, size_t stopCount)
    : fGradient(std::move(grad))
    , fStopCount(stopCount) {}

void GradientAdapter::apply() {
    this->onApply();

    // |fColorStops| holds |fStopCount| x [ pos, r, g, g ] + ? x [ pos, alpha ]

    if (fColorStops.size() < fStopCount * 4 || ((fColorStops.size() - fStopCount * 4) % 2)) {
        // apply() may get called before the stops are set, so only log when we have some stops.
        if (!fColorStops.empty()) {
            SkDebugf("!! Invalid gradient stop array size: %zu\n", fColorStops.size());
        }
        return;
    }

    std::vector<sksg::Gradient::ColorStop> stops;

    // TODO: merge/lerp opacity stops
    const auto csEnd = fColorStops.cbegin() + fStopCount * 4;
    for (auto cs = fColorStops.cbegin(); cs != csEnd; cs += 4) {
        const auto pos = cs[0];
        const VectorValue rgb({ cs[1], cs[2], cs[3] });

        stops.push_back({ pos, ValueTraits<VectorValue>::As<SkColor>(rgb) });
    }

    fGradient->setColorStops(std::move(stops));
}

LinearGradientAdapter::LinearGradientAdapter(sk_sp<sksg::LinearGradient> grad, size_t stopCount)
    : INHERITED(std::move(grad), stopCount) {}

void LinearGradientAdapter::onApply() {
    auto* grad = static_cast<sksg::LinearGradient*>(fGradient.get());
    grad->setStartPoint(this->startPoint());
    grad->setEndPoint(this->endPoint());
}

RadialGradientAdapter::RadialGradientAdapter(sk_sp<sksg::RadialGradient> grad, size_t stopCount)
    : INHERITED(std::move(grad), stopCount) {}

void RadialGradientAdapter::onApply() {
    auto* grad = static_cast<sksg::RadialGradient*>(fGradient.get());
    grad->setStartCenter(this->startPoint());
    grad->setEndCenter(this->startPoint());
    grad->setStartRadius(0);
    grad->setEndRadius(SkPoint::Distance(this->startPoint(), this->endPoint()));
}

GradientRampEffectAdapter::GradientRampEffectAdapter(sk_sp<sksg::RenderNode> child)
    : fRoot(sksg::ShaderEffect::Make(std::move(child))) {}

GradientRampEffectAdapter::~GradientRampEffectAdapter() = default;

void GradientRampEffectAdapter::apply() {
    // This adapter manages a SG fragment with the following structure:
    //
    // - ShaderEffect [fRoot]
    //     \  GradientShader [fGradient]
    //     \  child/wrapped fragment
    //
    // The gradient shader is updated based on the (animatable) intance type (linear/radial).

    auto update_gradient = [this] (InstanceType new_type) {
        if (new_type != fInstanceType) {
            fGradient = new_type == InstanceType::kLinear
                    ? sk_sp<sksg::Gradient>(sksg::LinearGradient::Make())
                    : sk_sp<sksg::Gradient>(sksg::RadialGradient::Make());

            fRoot->setShader(fGradient);
            fInstanceType = new_type;
        }

        fGradient->setColorStops({ {0, fStartColor}, {1, fEndColor} });
    };

    static constexpr int kLinearShapeValue = 1;
    const auto instance_type = (SkScalarRoundToInt(fShape) == kLinearShapeValue)
            ? InstanceType::kLinear
            : InstanceType::kRadial;

    // Sync the gradient shader instance if needed.
    update_gradient(instance_type);

    // Sync instance-dependent gradient params.
    if (instance_type == InstanceType::kLinear) {
        auto* lg = static_cast<sksg::LinearGradient*>(fGradient.get());
        lg->setStartPoint(fStartPoint);
        lg->setEndPoint(fEndPoint);
    } else {
        SkASSERT(instance_type == InstanceType::kRadial);

        auto* rg = static_cast<sksg::RadialGradient*>(fGradient.get());
        rg->setStartCenter(fStartPoint);
        rg->setEndCenter(fStartPoint);
        rg->setEndRadius(SkPoint::Distance(fStartPoint, fEndPoint));
    }

    // TODO: blend, scatter
}

TrimEffectAdapter::TrimEffectAdapter(sk_sp<sksg::TrimEffect> trimEffect)
    : fTrimEffect(std::move(trimEffect)) {
    SkASSERT(fTrimEffect);
}

TrimEffectAdapter::~TrimEffectAdapter() = default;

void TrimEffectAdapter::apply() {
    // BM semantics: start/end are percentages, offset is "degrees" (?!).
    const auto  start = fStart  / 100,
                  end = fEnd    / 100,
               offset = fOffset / 360;

    auto startT = SkTMin(start, end) + offset,
          stopT = SkTMax(start, end) + offset;
    auto   mode = SkTrimPathEffect::Mode::kNormal;

    if (stopT - startT < 1) {
        startT -= SkScalarFloorToScalar(startT);
        stopT  -= SkScalarFloorToScalar(stopT);

        if (startT > stopT) {
            using std::swap;
            swap(startT, stopT);
            mode = SkTrimPathEffect::Mode::kInverted;
        }
    } else {
        startT = 0;
        stopT  = 1;
    }

    fTrimEffect->setStart(startT);
    fTrimEffect->setStop(stopT);
    fTrimEffect->setMode(mode);
}

DropShadowEffectAdapter::DropShadowEffectAdapter(sk_sp<sksg::DropShadowImageFilter> dropShadow)
    : fDropShadow(std::move(dropShadow)) {
    SkASSERT(fDropShadow);
}

DropShadowEffectAdapter::~DropShadowEffectAdapter() = default;

void DropShadowEffectAdapter::apply() {
    // fColor -> RGB, fOpacity -> A
    fDropShadow->setColor(SkColorSetA(fColor, SkTPin(SkScalarRoundToInt(fOpacity), 0, 255)));

    // The offset is specified in terms of a bearing angle + distance.
    SkScalar rad = SkDegreesToRadians(90 - fDirection);
    fDropShadow->setOffset(SkVector::Make( fDistance * SkScalarCos(rad),
                                          -fDistance * SkScalarSin(rad)));

    // Close enough to AE.
    static constexpr SkScalar kSoftnessToSigmaFactor = 0.3f;
    const auto sigma = fSoftness * kSoftnessToSigmaFactor;
    fDropShadow->setSigma(SkVector::Make(sigma, sigma));

    fDropShadow->setMode(fShadowOnly ? sksg::DropShadowImageFilter::Mode::kShadowOnly
                                     : sksg::DropShadowImageFilter::Mode::kShadowAndForeground);
}

GaussianBlurEffectAdapter::GaussianBlurEffectAdapter(sk_sp<sksg::BlurImageFilter> blur)
    : fBlur(std::move(blur)) {
    SkASSERT(fBlur);
}

GaussianBlurEffectAdapter::~GaussianBlurEffectAdapter() = default;

void GaussianBlurEffectAdapter::apply() {
    static constexpr SkVector kDimensionsMap[] = {
        { 1, 1 }, // 1 -> horizontal and vertical
        { 1, 0 }, // 2 -> horizontal
        { 0, 1 }, // 3 -> vertical
    };

    const auto dim_index = SkTPin<size_t>(static_cast<size_t>(fDimensions),
                                          1, SK_ARRAY_COUNT(kDimensionsMap)) - 1;

    // Close enough to AE.
    static constexpr SkScalar kBlurrinessToSigmaFactor = 0.3f;
    const auto sigma = fBlurriness * kBlurrinessToSigmaFactor;

    fBlur->setSigma({ sigma * kDimensionsMap[dim_index].x(),
                      sigma * kDimensionsMap[dim_index].y() });

    static constexpr SkBlurImageFilter::TileMode kRepeatEdgeMap[] = {
        SkBlurImageFilter::kClampToBlack_TileMode, // 0 -> repeat edge pixels: off
        SkBlurImageFilter::       kClamp_TileMode, // 1 -> repeat edge pixels: on
    };

    const auto repeat_index = SkTPin<size_t>(static_cast<size_t>(fRepeatEdge),
                                             0, SK_ARRAY_COUNT(kRepeatEdgeMap) - 1);
    fBlur->setTileMode(kRepeatEdgeMap[repeat_index]);
}


// Levels color correction effect.
//
// Maps the selected channels from [inBlack...inWhite] to [outBlack, outWhite],
// based on a gamma exponent.
//
// For [i0..i1] -> [o0..o1]:
//
//   c' = o0 + (o1 - o0) * ((c - i0) / (i1 - i0)) ^ G
//
// The output is optionally clipped to the output range.
//
// In/out intervals are clampped to [0..1].  Inversion is allowed.
LevelsEffectAdapter::LevelsEffectAdapter(sk_sp<sksg::RenderNode> child)
    : fEffect(sksg::ExternalColorFilter::Make(std::move(child))) {
    SkASSERT(fEffect);
}

LevelsEffectAdapter::~LevelsEffectAdapter() = default;

void LevelsEffectAdapter::apply() {
    enum LottieChannel {
        kRGB_Channel = 1,
          kR_Channel = 2,
          kG_Channel = 3,
          kB_Channel = 4,
          kA_Channel = 5,
    };

    const auto channel = SkScalarTruncToInt(fChannel);
    if (channel < kRGB_Channel || channel > kA_Channel) {
        fEffect->setColorFilter(nullptr);
        return;
    }

    auto in_0 = SkTPin(fInBlack,  0.0f, 1.0f),
         in_1 = SkTPin(fInWhite,  0.0f, 1.0f),
        out_0 = SkTPin(fOutBlack, 0.0f, 1.0f),
        out_1 = SkTPin(fOutWhite, 0.0f, 1.0f),
            g = 1 / SkTMax(fGamma, 0.0f);

    float clip[] = {0, 1};
    const auto kLottieDoClip = 1;
    if (SkScalarTruncToInt(fClipBlack) == kLottieDoClip) {
        const auto idx = fOutBlack <= fOutWhite ? 0 : 1;
        clip[idx] = out_0;
    }
    if (SkScalarTruncToInt(fClipWhite) == kLottieDoClip) {
        const auto idx = fOutBlack <= fOutWhite ? 1 : 0;
        clip[idx] = out_1;
    }
    SkASSERT(clip[0] <= clip[1]);

    auto dIn  =  in_1 -  in_0,
         dOut = out_1 - out_0;

    if (SkScalarNearlyZero(dIn)) {
        // Degenerate dIn == 0 makes the arithmetic below explode.
        //
        // We could specialize the builder to deal with that case, or we could just
        // nudge by epsilon to make it all work.  The latter approach is simpler
        // and doesn't have any noticeable downsides.
        //
        // Also nudge in_0 towards 0.5, in case it was sqashed against an extremity.
        // This allows for some abrupt transition when the output interval is not
        // collapsed, and produces results closer to AE.
        static constexpr auto kEpsilon = 2 * SK_ScalarNearlyZero;
        dIn  += std::copysign(kEpsilon, dIn);
        in_0 += std::copysign(kEpsilon, .5f - in_0);
        SkASSERT(!SkScalarNearlyZero(dIn));
    }

    uint8_t lut[256];

    auto t =      -in_0 / dIn,
        dT = 1 / 255.0f / dIn;

    // TODO: is linear gamma common-enough to warrant a fast path?
    for (size_t i = 0; i < 256; ++i) {
        const auto out = out_0 + dOut * std::pow(std::max(t, 0.0f), g);
        SkASSERT(!SkScalarIsNaN(out));

        lut[i] = static_cast<uint8_t>(std::round(SkTPin(out, clip[0], clip[1]) * 255));

        t += dT;
    }

    fEffect->setColorFilter(SkTableColorFilter::MakeARGB(
        channel == kA_Channel                            ? lut : nullptr,
        channel == kR_Channel || channel == kRGB_Channel ? lut : nullptr,
        channel == kG_Channel || channel == kRGB_Channel ? lut : nullptr,
        channel == kB_Channel || channel == kRGB_Channel ? lut : nullptr
    ));
}

} // namespace skottie
