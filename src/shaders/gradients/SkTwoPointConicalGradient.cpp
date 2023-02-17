/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFloatingPoint.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkLocalMatrixShader.h"
#include "src/shaders/gradients/SkGradientShaderBase.h"

#include <utility>

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

// Please see https://skia.org/dev/design/conical for how our shader works.

class SkTwoPointConicalGradient final : public SkGradientShaderBase {
public:
    // See https://skia.org/dev/design/conical for what focal data means and how our shader works.
    // We make it public so the GPU shader can also use it.
    struct FocalData {
        SkScalar    fR1;        // r1 after mapping focal point to (0, 0)
        SkScalar    fFocalX;    // f
        bool        fIsSwapped; // whether we swapped r0, r1

        // The input r0, r1 are the radii when we map centers to {(0, 0), (1, 0)}.
        // We'll post concat matrix with our transformation matrix that maps focal point to (0, 0).
        // Returns true if the set succeeded
        bool set(SkScalar r0, SkScalar r1, SkMatrix* matrix);

        // Whether the focal point (0, 0) is on the end circle with center (1, 0) and radius r1. If
        // this is true, it's as if an aircraft is flying at Mach 1 and all circles (soundwaves)
        // will go through the focal point (aircraft). In our previous implementations, this was
        // known as the edge case where the inside circle touches the outside circle (on the focal
        // point). If we were to solve for t bruteforcely using a quadratic equation, this case
        // implies that the quadratic equation degenerates to a linear equation.
        bool isFocalOnCircle() const { return SkScalarNearlyZero(1 - fR1); }

        bool isSwapped() const { return fIsSwapped; }
        bool isWellBehaved() const { return !this->isFocalOnCircle() && fR1 > 1; }
        bool isNativelyFocal() const { return SkScalarNearlyZero(fFocalX); }
    };

    enum class Type {
        kRadial,
        kStrip,
        kFocal
    };

    static sk_sp<SkShader> Create(const SkPoint& start, SkScalar startRadius,
                                  const SkPoint& end,   SkScalar endRadius,
                                  const Descriptor&, const SkMatrix* localMatrix);

    GradientType asGradient(GradientInfo* info, SkMatrix* localMatrix) const override;
#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&,
                                                             const MatrixRec&) const override;
#endif
#ifdef SK_GRAPHITE_ENABLED
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif
    bool isOpaque() const override;

    SkScalar getCenterX1() const { return SkPoint::Distance(fCenter1, fCenter2); }
    SkScalar getStartRadius() const { return fRadius1; }
    SkScalar getDiffRadius() const { return fRadius2 - fRadius1; }
    const SkPoint& getStartCenter() const { return fCenter1; }
    const SkPoint& getEndCenter() const { return fCenter2; }
    SkScalar getEndRadius() const { return fRadius2; }

    Type getType() const { return fType; }
    const FocalData& getFocalData() const { return fFocalData; }

    SkTwoPointConicalGradient(const SkPoint& c0, SkScalar r0,
                              const SkPoint& c1, SkScalar r1,
                              const Descriptor&, Type, const SkMatrix&, const FocalData&);

protected:
    void flatten(SkWriteBuffer& buffer) const override;

    void appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* tPipeline,
                              SkRasterPipeline* postPipeline) const override;

    skvm::F32 transformT(skvm::Builder*, skvm::Uniforms*,
                         skvm::Coord coord, skvm::I32* mask) const final;

private:
    friend void ::SkRegisterTwoPointConicalGradientShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkTwoPointConicalGradient)

    SkPoint  fCenter1;
    SkPoint  fCenter2;
    SkScalar fRadius1;
    SkScalar fRadius2;
    Type     fType;

    FocalData fFocalData;
};

bool SkTwoPointConicalGradient::FocalData::set(SkScalar r0, SkScalar r1, SkMatrix* matrix) {
    fIsSwapped = false;
    fFocalX = sk_ieee_float_divide(r0, (r0 - r1));
    if (SkScalarNearlyZero(fFocalX - 1)) {
        // swap r0, r1
        matrix->postTranslate(-1, 0);
        matrix->postScale(-1, 1);
        std::swap(r0, r1);
        fFocalX = 0; // because r0 is now 0
        fIsSwapped = true;
    }

    // Map {focal point, (1, 0)} to {(0, 0), (1, 0)}
    const SkPoint from[2]   = { {fFocalX, 0}, {1, 0} };
    const SkPoint to[2]     = { {0, 0}, {1, 0} };
    SkMatrix focalMatrix;
    if (!focalMatrix.setPolyToPoly(from, to, 2)) {
        return false;
    }
    matrix->postConcat(focalMatrix);
    fR1 = r1 / SkScalarAbs(1 - fFocalX); // focalMatrix has a scale of 1/(1-f)

    // The following transformations are just to accelerate the shader computation by saving
    // some arithmatic operations.
    if (this->isFocalOnCircle()) {
        matrix->postScale(0.5, 0.5);
    } else {
        matrix->postScale(fR1 / (fR1 * fR1 - 1), 1 / sqrt(SkScalarAbs(fR1 * fR1 - 1)));
    }
    matrix->postScale(SkScalarAbs(1 - fFocalX), SkScalarAbs(1 - fFocalX)); // scale |1 - f|
    return true;
}

sk_sp<SkShader> SkTwoPointConicalGradient::Create(const SkPoint& c0, SkScalar r0,
                                                  const SkPoint& c1, SkScalar r1,
                                                  const Descriptor& desc,
                                                  const SkMatrix* localMatrix) {
    SkMatrix gradientMatrix;
    Type     gradientType;

    if (SkScalarNearlyZero((c0 - c1).length())) {
        if (SkScalarNearlyZero(std::max(r0, r1)) || SkScalarNearlyEqual(r0, r1)) {
            // Degenerate case; avoid dividing by zero. Should have been caught by caller but
            // just in case, recheck here.
            return nullptr;
        }
        // Concentric case: we can pretend we're radial (with a tiny twist).
        const SkScalar scale = sk_ieee_float_divide(1, std::max(r0, r1));
        gradientMatrix = SkMatrix::Translate(-c1.x(), -c1.y());
        gradientMatrix.postScale(scale, scale);

        gradientType = Type::kRadial;
    } else {
        const SkPoint centers[2] = { c0    , c1     };
        const SkPoint unitvec[2] = { {0, 0}, {1, 0} };

        if (!gradientMatrix.setPolyToPoly(centers, unitvec, 2)) {
            // Degenerate case.
            return nullptr;
        }

        gradientType = SkScalarNearlyZero(r1 - r0) ? Type::kStrip : Type::kFocal;
    }

    FocalData focalData;
    if (gradientType == Type::kFocal) {
        const auto dCenter = (c0 - c1).length();
        if (!focalData.set(r0 / dCenter, r1 / dCenter, &gradientMatrix)) {
            return nullptr;
        }
    }
    return SkLocalMatrixShader::MakeWrapped<SkTwoPointConicalGradient>(localMatrix,
                                                                       c0, r0,
                                                                       c1, r1,
                                                                       desc,
                                                                       gradientType,
                                                                       gradientMatrix,
                                                                       focalData);
}

SkTwoPointConicalGradient::SkTwoPointConicalGradient(
        const SkPoint& start, SkScalar startRadius,
        const SkPoint& end, SkScalar endRadius,
        const Descriptor& desc, Type type, const SkMatrix& gradientMatrix, const FocalData& data)
    : SkGradientShaderBase(desc, gradientMatrix)
    , fCenter1(start)
    , fCenter2(end)
    , fRadius1(startRadius)
    , fRadius2(endRadius)
    , fType(type)
{
    // this is degenerate, and should be caught by our caller
    SkASSERT(fCenter1 != fCenter2 || fRadius1 != fRadius2);
    if (type == Type::kFocal) {
        fFocalData = data;
    }
}

bool SkTwoPointConicalGradient::isOpaque() const {
    // Because areas outside the cone are left untouched, we cannot treat the
    // shader as opaque even if the gradient itself is opaque.
    // TODO(junov): Compute whether the cone fills the plane crbug.com/222380
    return false;
}

// Returns the original non-sorted version of the gradient
SkShaderBase::GradientType SkTwoPointConicalGradient::asGradient(GradientInfo* info,
                                                                 SkMatrix* localMatrix) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fCenter1;
        info->fPoint[1] = fCenter2;
        info->fRadius[0] = fRadius1;
        info->fRadius[1] = fRadius2;
    }
    if (localMatrix) {
        *localMatrix = SkMatrix::I();
    }
    return GradientType::kConical;
}

sk_sp<SkFlattenable> SkTwoPointConicalGradient::CreateProc(SkReadBuffer& buffer) {
    DescriptorScope desc;
    SkMatrix legacyLocalMatrix;
    if (!desc.unflatten(buffer, &legacyLocalMatrix)) {
        return nullptr;
    }
    SkPoint c1 = buffer.readPoint();
    SkPoint c2 = buffer.readPoint();
    SkScalar r1 = buffer.readScalar();
    SkScalar r2 = buffer.readScalar();

    if (!buffer.isValid()) {
        return nullptr;
    }
    return SkGradientShader::MakeTwoPointConical(c1, r1,
                                                 c2, r2,
                                                 desc.fColors,
                                                 std::move(desc.fColorSpace),
                                                 desc.fPositions,
                                                 desc.fColorCount,
                                                 desc.fTileMode,
                                                 desc.fInterpolation,
                                                 &legacyLocalMatrix);
}

void SkTwoPointConicalGradient::flatten(SkWriteBuffer& buffer) const {
    this->SkGradientShaderBase::flatten(buffer);
    buffer.writePoint(fCenter1);
    buffer.writePoint(fCenter2);
    buffer.writeScalar(fRadius1);
    buffer.writeScalar(fRadius2);
}

void SkTwoPointConicalGradient::appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* p,
                                                     SkRasterPipeline* postPipeline) const {
    const auto dRadius = fRadius2 - fRadius1;

    if (fType == Type::kRadial) {
        p->append(SkRasterPipelineOp::xy_to_radius);

        // Tiny twist: radial computes a t for [0, r2], but we want a t for [r1, r2].
        auto scale =  std::max(fRadius1, fRadius2) / dRadius;
        auto bias  = -fRadius1 / dRadius;

        p->append_matrix(alloc, SkMatrix::Translate(bias, 0) * SkMatrix::Scale(scale, 1));
        return;
    }

    if (fType == Type::kStrip) {
        auto* ctx = alloc->make<SkRasterPipeline_2PtConicalCtx>();
        SkScalar scaledR0 = fRadius1 / this->getCenterX1();
        ctx->fP0 = scaledR0 * scaledR0;
        p->append(SkRasterPipelineOp::xy_to_2pt_conical_strip, ctx);
        p->append(SkRasterPipelineOp::mask_2pt_conical_nan, ctx);
        postPipeline->append(SkRasterPipelineOp::apply_vector_mask, &ctx->fMask);
        return;
    }

    auto* ctx = alloc->make<SkRasterPipeline_2PtConicalCtx>();
    ctx->fP0 = 1/fFocalData.fR1;
    ctx->fP1 = fFocalData.fFocalX;

    if (fFocalData.isFocalOnCircle()) {
        p->append(SkRasterPipelineOp::xy_to_2pt_conical_focal_on_circle);
    } else if (fFocalData.isWellBehaved()) {
        p->append(SkRasterPipelineOp::xy_to_2pt_conical_well_behaved, ctx);
    } else if (fFocalData.isSwapped() || 1 - fFocalData.fFocalX < 0) {
        p->append(SkRasterPipelineOp::xy_to_2pt_conical_smaller, ctx);
    } else {
        p->append(SkRasterPipelineOp::xy_to_2pt_conical_greater, ctx);
    }

    if (!fFocalData.isWellBehaved()) {
        p->append(SkRasterPipelineOp::mask_2pt_conical_degenerates, ctx);
    }
    if (1 - fFocalData.fFocalX < 0) {
        p->append(SkRasterPipelineOp::negate_x);
    }
    if (!fFocalData.isNativelyFocal()) {
        p->append(SkRasterPipelineOp::alter_2pt_conical_compensate_focal, ctx);
    }
    if (fFocalData.isSwapped()) {
        p->append(SkRasterPipelineOp::alter_2pt_conical_unswap);
    }
    if (!fFocalData.isWellBehaved()) {
        postPipeline->append(SkRasterPipelineOp::apply_vector_mask, &ctx->fMask);
    }
}

skvm::F32 SkTwoPointConicalGradient::transformT(skvm::Builder* p, skvm::Uniforms* uniforms,
                                                skvm::Coord coord, skvm::I32* mask) const {
    auto mag = [](skvm::F32 x, skvm::F32 y) { return sqrt(x*x + y*y); };

    // See https://skia.org/dev/design/conical, and appendStages() above.
    // There's a lot going on here, and I'm not really sure what's independent
    // or disjoint, what can be reordered, simplified, etc.  Tweak carefully.

    const skvm::F32 x = coord.x,
                    y = coord.y;
    if (fType == Type::kRadial) {
        float denom = 1.0f / (fRadius2 - fRadius1),
              scale = std::max(fRadius1, fRadius2) * denom,
               bias =                  -fRadius1 * denom;
        return mag(x,y) * p->uniformF(uniforms->pushF(scale))
                        + p->uniformF(uniforms->pushF(bias ));
    }

    if (fType == Type::kStrip) {
        float r = fRadius1 / this->getCenterX1();
        skvm::F32 t = x + sqrt(p->uniformF(uniforms->pushF(r*r)) - y*y);

        *mask = (t == t);   // t != NaN
        return t;
    }

    const skvm::F32 invR1 = p->uniformF(uniforms->pushF(1 / fFocalData.fR1));

    skvm::F32 t;
    if (fFocalData.isFocalOnCircle()) {
        t = (y/x) * y + x;       // (x^2 + y^2) / x  ~~>  x + y^2/x  ~~>  y/x * y + x
    } else if (fFocalData.isWellBehaved()) {
        t = mag(x,y) - x*invR1;
    } else {
        skvm::F32 k = sqrt(x*x - y*y);
        if (fFocalData.isSwapped() || 1 - fFocalData.fFocalX < 0) {
            k = -k;
        }
        t = k - x*invR1;
    }

    if (!fFocalData.isWellBehaved()) {
        // TODO: not sure why we consider t == 0 degenerate
        *mask = (t > 0.0f);  // and implicitly, t != NaN
    }

    const skvm::F32 focalX = p->uniformF(uniforms->pushF(fFocalData.fFocalX));
    if (1 - fFocalData.fFocalX < 0)    { t = -t; }
    if (!fFocalData.isNativelyFocal()) { t += focalX; }
    if ( fFocalData.isSwapped())       { t = 1.0f - t; }
    return t;
}

/////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#include "src/gpu/ganesh/gradients/GrGradientShader.h"

std::unique_ptr<GrFragmentProcessor>
SkTwoPointConicalGradient::asFragmentProcessor(const GrFPArgs& args, const MatrixRec& mRec) const {
    // The 2 point conical gradient can reject a pixel so it does change opacity even if the input
    // was opaque. Thus, all of these layout FPs disable that optimization.
    std::unique_ptr<GrFragmentProcessor> fp;
    SkTLazy<SkMatrix> matrix;
    switch (this->getType()) {
        case SkTwoPointConicalGradient::Type::kStrip: {
            static const SkRuntimeEffect* kEffect =
                SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                        "uniform half r0_2;"
                        "half4 main(float2 p) {"
                            "half v = 1;" // validation flag,set to negative to discard fragment later
                            "float t = r0_2 - p.y * p.y;"
                            "if (t >= 0) {"
                                "t = p.x + sqrt(t);"
                            "} else {"
                                "v = -1;"
                            "}"
                            "return half4(half(t), v, 0, 0);"
                        "}"
                    );
            float r0 = this->getStartRadius() / this->getCenterX1();
            fp = GrSkSLFP::Make(kEffect, "TwoPointConicalStripLayout", /*inputFP=*/nullptr,
                                GrSkSLFP::OptFlags::kNone,
                                "r0_2", r0 * r0);
        } break;

        case SkTwoPointConicalGradient::Type::kRadial: {
            static const SkRuntimeEffect* kEffect =
                SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                        "uniform half r0;"
                        "uniform half lengthScale;"
                        "half4 main(float2 p) {"
                            "half v = 1;" // validation flag,set to negative to discard fragment later
                            "float t = length(p) * lengthScale - r0;"
                            "return half4(half(t), v, 0, 0);"
                        "}"
                    );
            float dr = this->getDiffRadius();
            float r0 = this->getStartRadius() / dr;
            bool isRadiusIncreasing = dr >= 0;
            fp = GrSkSLFP::Make(kEffect, "TwoPointConicalRadialLayout", /*inputFP=*/nullptr,
                                GrSkSLFP::OptFlags::kNone,
                                "r0", r0,
                                "lengthScale", isRadiusIncreasing ? 1.0f : -1.0f);

            // GPU radial matrix is different from the original matrix, since we map the diff radius
            // to have |dr| = 1, so manually compute the final gradient matrix here.

            // Map center to (0, 0)
            matrix.set(SkMatrix::Translate(-this->getStartCenter().fX,
                                           -this->getStartCenter().fY));
            // scale |diffRadius| to 1
            matrix->postScale(1 / dr, 1 / dr);
        } break;

        case SkTwoPointConicalGradient::Type::kFocal: {
            static const SkRuntimeEffect* kEffect =
                SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                        // Optimization flags, all specialized:
                        "uniform int isRadiusIncreasing;"
                        "uniform int isFocalOnCircle;"
                        "uniform int isWellBehaved;"
                        "uniform int isSwapped;"
                        "uniform int isNativelyFocal;"

                        "uniform half invR1;"  // 1/r1
                        "uniform half fx;"     // focalX = r0/(r0-r1)

                        "half4 main(float2 p) {"
                            "float t = -1;"
                            "half v = 1;" // validation flag,set to negative to discard fragment later

                            "float x_t = -1;"
                            "if (bool(isFocalOnCircle)) {"
                                "x_t = dot(p, p) / p.x;"
                            "} else if (bool(isWellBehaved)) {"
                                "x_t = length(p) - p.x * invR1;"
                            "} else {"
                                "float temp = p.x * p.x - p.y * p.y;"

                                // Only do sqrt if temp >= 0; this is significantly slower than
                                // checking temp >= 0 in the if statement that checks r(t) >= 0.
                                // But GPU may break if we sqrt a negative float. (Although I
                                // haven't observed that on any devices so far, and the old
                                // approach also does sqrt negative value without a check.) If
                                // the performance is really critical, maybe we should just
                                // compute the area where temp and x_t are always valid and drop
                                // all these ifs.
                                "if (temp >= 0) {"
                                    "if (bool(isSwapped) || !bool(isRadiusIncreasing)) {"
                                        "x_t = -sqrt(temp) - p.x * invR1;"
                                    "} else {"
                                        "x_t = sqrt(temp) - p.x * invR1;"
                                    "}"
                                "}"
                            "}"

                            // The final calculation of t from x_t has lots of static
                            // optimizations but only do them when x_t is positive (which
                            // can be assumed true if isWellBehaved is true)
                            "if (!bool(isWellBehaved)) {"
                                // This will still calculate t even though it will be ignored
                                // later in the pipeline to avoid a branch
                                "if (x_t <= 0.0) {"
                                    "v = -1;"
                                "}"
                            "}"
                            "if (bool(isRadiusIncreasing)) {"
                                "if (bool(isNativelyFocal)) {"
                                    "t = x_t;"
                                "} else {"
                                    "t = x_t + fx;"
                                "}"
                            "} else {"
                                "if (bool(isNativelyFocal)) {"
                                    "t = -x_t;"
                                "} else {"
                                    "t = -x_t + fx;"
                                "}"
                            "}"

                            "if (bool(isSwapped)) {"
                                "t = 1 - t;"
                            "}"

                            "return half4(half(t), v, 0, 0);"
                        "}"
                    );

            const SkTwoPointConicalGradient::FocalData& focalData = this->getFocalData();
            bool isRadiusIncreasing = (1 - focalData.fFocalX) > 0,
                    isFocalOnCircle    = focalData.isFocalOnCircle(),
                    isWellBehaved      = focalData.isWellBehaved(),
                    isSwapped          = focalData.isSwapped(),
                    isNativelyFocal    = focalData.isNativelyFocal();

            fp = GrSkSLFP::Make(kEffect, "TwoPointConicalFocalLayout", /*inputFP=*/nullptr,
                                GrSkSLFP::OptFlags::kNone,
                                "isRadiusIncreasing", GrSkSLFP::Specialize<int>(isRadiusIncreasing),
                                "isFocalOnCircle",    GrSkSLFP::Specialize<int>(isFocalOnCircle),
                                "isWellBehaved",      GrSkSLFP::Specialize<int>(isWellBehaved),
                                "isSwapped",          GrSkSLFP::Specialize<int>(isSwapped),
                                "isNativelyFocal",    GrSkSLFP::Specialize<int>(isNativelyFocal),
                                "invR1", 1.0f / focalData.fR1,
                                "fx", focalData.fFocalX);
        } break;
    }
    return GrGradientShader::MakeGradientFP(*this,
                                            args,
                                            mRec,
                                            std::move(fp),
                                            matrix.getMaybeNull());
}

#endif

#ifdef SK_GRAPHITE_ENABLED
void SkTwoPointConicalGradient::addToKey(const skgpu::graphite::KeyContext& keyContext,
                                         skgpu::graphite::PaintParamsKeyBuilder* builder,
                                         skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    SkColor4fXformer xformedColors(this, keyContext.dstColorInfo().colorSpace());
    const SkPMColor4f* colors = xformedColors.fColors.begin();

    GradientShaderBlocks::GradientData data(GradientType::kConical,
                                            fCenter1, fCenter2,
                                            fRadius1, fRadius2,
                                            0.0f, 0.0f,
                                            fTileMode,
                                            fColorCount,
                                            colors,
                                            fPositions,
                                            fInterpolation);

    MakeInterpolatedToDst(keyContext, builder, gatherer,
                          data, fInterpolation,
                          xformedColors.fIntermediateColorSpace.get());
}
#endif

// assumes colors is SkColor4f* and pos is SkScalar*
#define EXPAND_1_COLOR(count)                \
     SkColor4f tmp[2];                       \
     do {                                    \
         if (1 == count) {                   \
             tmp[0] = tmp[1] = colors[0];    \
             colors = tmp;                   \
             pos = nullptr;                  \
             count = 2;                      \
         }                                   \
     } while (0)

sk_sp<SkShader> SkGradientShader::MakeTwoPointConical(const SkPoint& start,
                                                      SkScalar startRadius,
                                                      const SkPoint& end,
                                                      SkScalar endRadius,
                                                      const SkColor4f colors[],
                                                      sk_sp<SkColorSpace> colorSpace,
                                                      const SkScalar pos[],
                                                      int colorCount,
                                                      SkTileMode mode,
                                                      const Interpolation& interpolation,
                                                      const SkMatrix* localMatrix) {
    if (startRadius < 0 || endRadius < 0) {
        return nullptr;
    }
    if (!SkGradientShaderBase::ValidGradient(colors, colorCount, mode, interpolation)) {
        return nullptr;
    }
    if (SkScalarNearlyZero((start - end).length(), SkGradientShaderBase::kDegenerateThreshold)) {
        // If the center positions are the same, then the gradient is the radial variant of a 2 pt
        // conical gradient, an actual radial gradient (startRadius == 0), or it is fully degenerate
        // (startRadius == endRadius).
        if (SkScalarNearlyEqual(startRadius, endRadius,
                                SkGradientShaderBase::kDegenerateThreshold)) {
            // Degenerate case, where the interpolation region area approaches zero. The proper
            // behavior depends on the tile mode, which is consistent with the default degenerate
            // gradient behavior, except when mode = clamp and the radii > 0.
            if (mode == SkTileMode::kClamp &&
                endRadius > SkGradientShaderBase::kDegenerateThreshold) {
                // The interpolation region becomes an infinitely thin ring at the radius, so the
                // final gradient will be the first color repeated from p=0 to 1, and then a hard
                // stop switching to the last color at p=1.
                static constexpr SkScalar circlePos[3] = {0, 1, 1};
                SkColor4f reColors[3] = {colors[0], colors[0], colors[colorCount - 1]};
                return MakeRadial(start, endRadius, reColors, std::move(colorSpace),
                                  circlePos, 3, mode, interpolation, localMatrix);
            } else {
                // Otherwise use the default degenerate case
                return SkGradientShaderBase::MakeDegenerateGradient(colors, pos, colorCount,
                                                                    std::move(colorSpace), mode);
            }
        } else if (SkScalarNearlyZero(startRadius, SkGradientShaderBase::kDegenerateThreshold)) {
            // We can treat this gradient as radial, which is faster. If we got here, we know
            // that endRadius is not equal to 0, so this produces a meaningful gradient
            return MakeRadial(start, endRadius, colors, std::move(colorSpace), pos, colorCount,
                              mode, interpolation, localMatrix);
        }
        // Else it's the 2pt conical radial variant with no degenerate radii, so fall through to the
        // regular 2pt constructor.
    }

    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }
    EXPAND_1_COLOR(colorCount);

    SkGradientShaderBase::ColorStopOptimizer opt(colors, pos, colorCount, mode);

    SkGradientShaderBase::Descriptor desc(opt.fColors, std::move(colorSpace), opt.fPos,
                                          opt.fCount, mode, interpolation);
    return SkTwoPointConicalGradient::Create(start, startRadius, end, endRadius, desc, localMatrix);
}

#undef EXPAND_1_COLOR

sk_sp<SkShader> SkGradientShader::MakeTwoPointConical(const SkPoint& start,
                                                      SkScalar startRadius,
                                                      const SkPoint& end,
                                                      SkScalar endRadius,
                                                      const SkColor colors[],
                                                      const SkScalar pos[],
                                                      int colorCount,
                                                      SkTileMode mode,
                                                      uint32_t flags,
                                                      const SkMatrix* localMatrix) {
    SkColorConverter converter(colors, colorCount);
    return MakeTwoPointConical(start, startRadius, end, endRadius, converter.fColors4f.begin(),
                               nullptr, pos, colorCount, mode, flags, localMatrix);
}

void SkRegisterTwoPointConicalGradientShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkTwoPointConicalGradient);
}
