/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTwoPointConicalGradient.h"

#if SK_SUPPORT_GPU
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "SkTwoPointConicalGradient_gpu.h"

// For brevity
typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

class TwoPointConicalEffect : public GrGradientEffect {
public:
    class DegeneratedGLSLProcessor; // radial (center0 == center1) or strip (r0 == r1) case
    class FocalGLSLProcessor; // all other cases where we can derive a focal point

    enum Type {
        kRadial_Type,
        kStrip_Type,
        kFocal_Type
    };

    struct Data {
        SkScalar    fRadius0;
        SkScalar    fDiffRadius;
        Type        fType;
        bool        fIsSwapped;

        // Construct from the shader, and set the matrix accordingly
        Data(const SkTwoPointConicalGradient& shader, SkMatrix& matrix);

        bool operator== (const Data& d) const {
            return fRadius0 == d.fRadius0 && fDiffRadius == d.fDiffRadius && fType == d.fType &&
                   fIsSwapped == d.fIsSwapped;
        }
    };

    static std::unique_ptr<GrFragmentProcessor> Make(const CreateArgs& args, const Data& data);

    SkScalar diffRadius() const { return fData.fDiffRadius; }
    SkScalar r0() const { return fData.fRadius0; }
    SkScalar r1() const { return fData.fRadius0 + fData.fDiffRadius; }

    const char* name() const override { return "Two-Point Conical Gradient"; }

    // Whether the focal point (0, 0) is on the end circle with center (1, 0) and radius r1. If this
    // is true, it's as if an aircraft is flying at Mach 1 and all circles (soundwaves) will go
    // through the focal point (aircraft). In our previous implementations, this was known as the
    // edge case where the inside circle touches the outside circle (on the focal point). If we were
    // to solve for t bruteforcely using a quadratic equation, this case implies that the quadratic
    // equation degenerates to a linear equation.
    bool isFocalOnCircle() const { return SkScalarNearlyZero(1 - this->r1()); }
    bool isSwapped() const { return fData.fIsSwapped; }

    Type getType() const { return fData.fType; }

    // Whether the t we solved is always valid (so we don't need to check r(t) > 0).
    bool isWellBehaved() const { return !this->isFocalOnCircle() && this->r1() > 1; }

    // Whether r0 == 0 so it's focal without any transformation
    bool isNativelyFocal() const { return SkScalarNearlyZero(fData.fRadius0); }

    bool isRadiusIncreasing() const { return fData.fDiffRadius > 0; }

protected:
    void onGetGLSLProcessorKey(const GrShaderCaps& c, GrProcessorKeyBuilder* b) const override {
        INHERITED::onGetGLSLProcessorKey(c, b);
        uint32_t key = 0;
        key |= fData.fType;
        SkASSERT(key < (1 << 2));
        key |= (this->isFocalOnCircle() << 2);
        key |= (this->isWellBehaved() << 3);
        key |= (this->isRadiusIncreasing() << 4);
        key |= (this->isNativelyFocal() << 5);
        key |= (this->isSwapped() << 6);
        SkASSERT(key < (1 << 7));
        b->add32(key);
    }


    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new TwoPointConicalEffect(*this));
    }

    bool onIsEqual(const GrFragmentProcessor& sBase) const override {
        const TwoPointConicalEffect& s = sBase.cast<TwoPointConicalEffect>();
        return (INHERITED::onIsEqual(sBase) && fData == s.fData);
    }

    explicit TwoPointConicalEffect(const CreateArgs& args, const Data data)
        : INHERITED(kTwoPointConicalEffect_ClassID, args,
            false /* opaque: draws transparent black outside of the cone. */)
        , fData(data) {}

    explicit TwoPointConicalEffect(const TwoPointConicalEffect& that)
            : INHERITED(that)
            , fData(that.fData) {}

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    Data fData;

    typedef GrGradientEffect INHERITED;
};

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(TwoPointConicalEffect);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> TwoPointConicalEffect::TestCreate(
        GrProcessorTestData* d) {
    SkPoint center1 = {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()};
    SkPoint center2 = {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()};
    SkScalar radius1 = d->fRandom->nextUScalar1();
    SkScalar radius2 = d->fRandom->nextUScalar1();

    constexpr int   kTestTypeMask           = (1 << 2) - 1,
                    kTestNativelyFocalBit   = (1 << 2),
                    kTestFocalOnCircleBit   = (1 << 3),
                    kTestSwappedBit         = (1 << 4);
                    // We won't treat isWellDefined and isRadiusIncreasing specially beacuse they
                    // should have high probability to be turned on and off as we're getting random
                    // radii and centers.

    int mask = d->fRandom->nextU();
    int type = mask & kTestTypeMask;
    if (type == TwoPointConicalEffect::kRadial_Type) {
        center2 = center1;
    } else if (type == TwoPointConicalEffect::kStrip_Type) {
        radius1 = SkTMax(radius1, .1f); // Make sure that the radius is non-zero
        radius2 = radius1;
    } else { // kFocal_Type
        if (kTestNativelyFocalBit & mask) {
            radius1 = 0;
        }
        if (kTestFocalOnCircleBit & mask) {
            radius2 = radius1 + SkPoint::Distance(center1, center2);
        }
        if (kTestSwappedBit & mask) {
            std::swap(radius1, radius2);
            radius2 = 0;
        }
    }

    if (SkScalarNearlyZero(radius1 - radius2) &&
            SkScalarNearlyZero(SkPoint::Distance(center1, center2))) {
        radius2 += .1f; // make sure that we're not degenerated
    }
    RandomGradientParams params(d->fRandom);
    auto shader = params.fUseColors4f ?
        SkGradientShader::MakeTwoPointConical(center1, radius1, center2, radius2,
                                              params.fColors4f, params.fColorSpace, params.fStops,
                                              params.fColorCount, params.fTileMode) :
        SkGradientShader::MakeTwoPointConical(center1, radius1, center2, radius2,
                                              params.fColors, params.fStops,
                                              params.fColorCount, params.fTileMode);
    GrTest::TestAsFPArgs asFPArgs(d);
    std::unique_ptr<GrFragmentProcessor> fp = as_SB(shader)->asFragmentProcessor(asFPArgs.args());
    GrAlwaysAssert(fp);
    return fp;
}
#endif

//////////////////////////////////////////////////////////////////////////////
// DegeneratedGLSLProcessor
//////////////////////////////////////////////////////////////////////////////

class TwoPointConicalEffect::DegeneratedGLSLProcessor : public GrGradientEffect::GLSLProcessor {
protected:
    void emitCode(EmitArgs& args) override {
        const TwoPointConicalEffect& ge = args.fFp.cast<TwoPointConicalEffect>();
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
        this->emitUniforms(uniformHandler, ge);
        fParamUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType,
                                               "Conical2FSParams");

        SkString p0; // r0 for radial case, r0^2 for strip case
        p0.appendf("%s", uniformHandler->getUniformVariable(fParamUni).getName().c_str());
        const char* tName = "t"; // the gradient

        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkString coords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);
        const char* p = coords2D.c_str();

        if (ge.getType() == kRadial_Type) {
            fragBuilder->codeAppendf("half %s = length(%s) - %s;", tName, p, p0.c_str());
        } else {
            // output will default to transparent black (we simply won't write anything
            // else to it if invalid, instead of discarding or returning prematurely)
            fragBuilder->codeAppendf("%s = half4(0.0,0.0,0.0,0.0);", args.fOutputColor);
            fragBuilder->codeAppendf("half temp = %s - %s.y * %s.y;", p0.c_str(), p, p);
            fragBuilder->codeAppendf("if (temp >= 0) {");
            fragBuilder->codeAppendf("half %s = %s.x + sqrt(temp);", tName, p);
        }
        this->emitColor(fragBuilder,
                        uniformHandler,
                        args.fShaderCaps,
                        ge,
                        tName,
                        args.fOutputColor,
                        args.fInputColor,
                        args.fTexSamplers);

        if (ge.getType() != kRadial_Type) {
            fragBuilder->codeAppendf("}");
        }
    }

    void onSetData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& p) override {
        INHERITED::onSetData(pdman, p);
        const TwoPointConicalEffect& data = p.cast<TwoPointConicalEffect>();
        // kRadialType should imply r1 - r0 = 1 (after our transformation) so r0 = r0 / (r1 - r0)
        SkASSERT(data.getType() == kStrip_Type || SkScalarNearlyZero(data.r1() - data.r0() - 1));
        pdman.set1f(fParamUni, data.getType() == kRadial_Type ? data.r0() : data.r0() * data.r0());
    }

    UniformHandle fParamUni;

private:
    typedef GrGradientEffect::GLSLProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////
// FocalGLSLProcessor
//////////////////////////////////////////////////////////////////////////////

class TwoPointConicalEffect::FocalGLSLProcessor : public GrGradientEffect::GLSLProcessor {
protected:
    void emitCode(EmitArgs& args) override {
        const TwoPointConicalEffect& ge = args.fFp.cast<TwoPointConicalEffect>();
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
        this->emitUniforms(uniformHandler, ge);
        fParamUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf2_GrSLType,
                                               "Conical2FSParams");

        SkString p0; // 1 / r1
        SkString p1; // r0 / (r1 - r0)
        p0.appendf("%s.x", uniformHandler->getUniformVariable(fParamUni).getName().c_str());
        p1.appendf("%s.y", uniformHandler->getUniformVariable(fParamUni).getName().c_str());
        const char* tName = "t"; // the gradient

        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkString coords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);
        const char* p = coords2D.c_str();

        if (ge.isFocalOnCircle()) {
            fragBuilder->codeAppendf("half %s_prime = dot(%s, %s) / %s.x;", tName, p, p, p);
        } else if (ge.isWellBehaved()) {
            // empty sign is positive
            char sign = ge.isRadiusIncreasing() ? ' ' : '-';
            fragBuilder->codeAppendf("half %s_prime = %clength(%s) - %s.x * %s;",
                    tName, sign, p, p, p0.c_str());
        } else {
            char sign = ge.isSwapped() ? '-' : ' ';
            fragBuilder->codeAppendf("half temp = %s.x * %s.x - %s.y * %s.y;", p, p, p, p);
            // Initialize t_prime to illegal state (where r(t) < 0)
            fragBuilder->codeAppendf("half %s_prime = %s;",
                    tName, ge.isRadiusIncreasing() ? "-1" : "1");

            // Only do sqrt if temp >= 0; this is significantly slower than checking temp >= 0 in
            // the if statement that checks r(t) >= 0. But GPU may break if we sqrt a negative
            // float. (Although I havevn't observed that on any devices so far, and the old approach
            // also does sqrt negative value without a check.) If the performance is really
            // critical, maybe we should just compute the area where temp and t_prime are always
            // valid and drop all these ifs.
            fragBuilder->codeAppendf("if (temp >= 0) {");
            fragBuilder->codeAppendf("%s_prime = (%csqrt(temp) - %s.x * %s);",
                    tName, sign, p, p0.c_str());
            fragBuilder->codeAppendf("}");
        }
        // "- 0" is much faster than "- p1" so we specialize the natively focal case where p1 = 0.
        fragBuilder->codeAppendf("half %s = %s_prime - %s;", tName, tName,
                ge.isNativelyFocal() ? "0" : p1.c_str());

        if (ge.isSwapped()) {
            fragBuilder->codeAppendf("%s = 1 - %s;", tName, tName);
        }

        if (!ge.isWellBehaved()) {
            // output will default to transparent black (we simply won't write anything
            // else to it if invalid, instead of discarding or returning prematurely)
            fragBuilder->codeAppendf("%s = half4(0.0,0.0,0.0,0.0);", args.fOutputColor);

            // r(t) must be nonnegative; we need to swap direction if r0 > r1 because we did a final
            // scale of r1 / (r1 - r0) that's negative if r0 > r1.
            char direction = ge.isRadiusIncreasing() ? '>' : '<';
            fragBuilder->codeAppendf("if (%s_prime %c= 0.0) {", tName, direction);
        }
        this->emitColor(fragBuilder,
                        uniformHandler,
                        args.fShaderCaps,
                        ge,
                        tName,
                        args.fOutputColor,
                        args.fInputColor,
                        args.fTexSamplers);
        if (!ge.isWellBehaved()) {
            fragBuilder->codeAppend("};");
        }
    }

    void onSetData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& p) override {
        INHERITED::onSetData(pdman, p);
        const TwoPointConicalEffect& data = p.cast<TwoPointConicalEffect>();
        SkScalar r0 = data.r0();
        SkScalar r1 = data.r1();
        pdman.set2f(fParamUni, 1 / r1, r0 / (r1 - r0));
    }

    UniformHandle fParamUni;

private:
    typedef GrGradientEffect::GLSLProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

GrGLSLFragmentProcessor* TwoPointConicalEffect::onCreateGLSLInstance() const {
    if (fData.fType == kRadial_Type || fData.fType == kStrip_Type) {
        return new DegeneratedGLSLProcessor;
    }
    return new FocalGLSLProcessor;
}

std::unique_ptr<GrFragmentProcessor> TwoPointConicalEffect::Make(
        const GrGradientEffect::CreateArgs& args, const Data& data) {
    return GrGradientEffect::AdjustFP(
            std::unique_ptr<TwoPointConicalEffect>(new TwoPointConicalEffect(args, data)),
            args);
}

std::unique_ptr<GrFragmentProcessor> Gr2PtConicalGradientEffect::Make(
        const GrGradientEffect::CreateArgs& args) {
    const SkTwoPointConicalGradient& shader =
        *static_cast<const SkTwoPointConicalGradient*>(args.fShader);

    SkMatrix matrix;
    if (!shader.getLocalMatrix().invert(&matrix)) {
        return nullptr;
    }
    if (args.fMatrix) {
        SkMatrix inv;
        if (!args.fMatrix->invert(&inv)) {
            return nullptr;
        }
        matrix.postConcat(inv);
    }

    GrGradientEffect::CreateArgs newArgs(args.fContext, args.fShader, &matrix, args.fWrapMode,
        args.fDstColorSpace);
    // Data and matrix has to be prepared before constructing TwoPointConicalEffect so its parent
    // class can have the right matrix to work with during construction.
    TwoPointConicalEffect::Data data(shader, matrix);
    return TwoPointConicalEffect::Make(newArgs, data);
}

TwoPointConicalEffect::Data::Data(const SkTwoPointConicalGradient& shader, SkMatrix& matrix) {
    fIsSwapped = false;
    if (SkScalarNearlyZero(shader.getCenterX1())) {
        fType = kRadial_Type;
        SkScalar dr = shader.getDiffRadius();
        // Map center to (0, 0) and scale dr to 1
        matrix.postTranslate(-shader.getStartCenter().fX, -shader.getStartCenter().fY);
        matrix.postScale(1 / dr, 1 / dr);
        fRadius0 = shader.getStartRadius() / dr;
        fDiffRadius = 1;
    } else {
        // Map centers to (0, 0), (1, 0)
        const SkPoint centers[2] = { shader.getStartCenter(), shader.getEndCenter() };
        const SkPoint unitvec[2] = { { 0, 0 },{ 1, 0 } };
        SkMatrix gradientMatrix;
        // The radial case is already handled so this must succeed
        SkAssertResult(gradientMatrix.setPolyToPoly(centers, unitvec, 2));
        matrix.postConcat(gradientMatrix);
        fRadius0 = shader.getStartRadius() / shader.getCenterX1();
        fDiffRadius = shader.getDiffRadius() / shader.getCenterX1();

        if (SkScalarNearlyZero(shader.getDiffRadius())) {
            fType = kStrip_Type;
        } else { // focal case
            fType = kFocal_Type;
            if (SkScalarNearlyZero(shader.getEndRadius())) {
                // swap r0, r1
                matrix.postTranslate(-1, 0);
                matrix.postScale(-1, 1);
                fRadius0 = 0;
                fDiffRadius = -fDiffRadius;
                fIsSwapped = true;
            }

            // Map {focal point, (1, 0)} to {(0, 0), (1, 0)}
            SkScalar focalX = - fRadius0 / fDiffRadius;
            const SkPoint from[2]   = { {focalX, 0}, {1, 0} };
            const SkPoint to[2]     = { {0, 0}, {1, 0} };
            SkMatrix focalMatrix;
            focalMatrix.setPolyToPoly(from, to, 2);
            matrix.postConcat(focalMatrix);
            fRadius0 /= SkScalarAbs(1 - focalX);
            fDiffRadius /= SkScalarAbs(1 - focalX);

            SkScalar r0 = fRadius0;
            SkScalar r1 = fRadius0 + fDiffRadius;
            // The following transformations are not reflected on data; they're just to accelerate
            // the shader computation by saving some arithmatic operations.
            bool isFocalOnCircle = SkScalarNearlyZero(1 - r1);
            if (isFocalOnCircle) {
                matrix.postScale(0.5, 0.5); // r1 = 1 so r1 + 1 = 2 and 0.5 = 1 / (r1 + 1)
            } else {
                matrix.postScale(r1 / (r1 * r1 - 1), 1 / sqrt(SkScalarAbs(r1 * r1 - 1)));
            }
            matrix.postScale(r1 / (r1 - r0), r1 / (r1 - r0));
        }
    }
}

#endif
