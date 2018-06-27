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

// Please see https://skia.org/dev/design/conical for how our shader works.
class TwoPointConicalEffect : public GrGradientEffect {
public:
    using Type = SkTwoPointConicalGradient::Type;
    class DegeneratedGLSLProcessor; // radial (center0 == center1) or strip (r0 == r1) case
    class FocalGLSLProcessor; // all other cases where we can derive a focal point

    struct Data {
        Type        fType;
        SkScalar    fRadius0;
        SkScalar    fDiffRadius;
        SkTwoPointConicalGradient::FocalData fFocalData;

        // Construct from the shader, and set the matrix accordingly
        Data(const SkTwoPointConicalGradient& shader, SkMatrix& matrix);

        bool operator== (const Data& d) const {
            if (fType != d.fType) {
                return false;
            }
            switch (fType) {
                case Type::kRadial:
                case Type::kStrip:
                    return fRadius0 == d.fRadius0 && fDiffRadius == d.fDiffRadius;
                case Type::kFocal:
                    return fFocalData.fR1 == d.fFocalData.fR1 &&
                            fFocalData.fFocalX == d.fFocalData.fFocalX &&
                            fFocalData.fIsSwapped == d.fFocalData.fIsSwapped;
            }
            SkDEBUGFAIL("This return should be unreachable; it's here just for compile warning");
            return false;
        }
    };

    static std::unique_ptr<GrFragmentProcessor> Make(const CreateArgs& args, const Data& data);

    SkScalar diffRadius() const {
        SkASSERT(!this->isFocal()); // fDiffRadius is uninitialized for focal cases
        return fData.fDiffRadius;
    }
    SkScalar r0() const {
        SkASSERT(!this->isFocal()); // fRadius0 is uninitialized for focal cases
        return fData.fRadius0;
    }

    SkScalar r1() const {
        SkASSERT(this->isFocal()); // fFocalData is uninitialized for non-focal cases
        return fData.fFocalData.fR1;
    }
    SkScalar focalX() const {
        SkASSERT(this->isFocal()); // fFocalData is uninitialized for non-focal cases
        return fData.fFocalData.fFocalX;
    }

    const char* name() const override { return "Two-Point Conical Gradient"; }

    // Whether the focal point (0, 0) is on the end circle with center (1, 0) and radius r1. If this
    // is true, it's as if an aircraft is flying at Mach 1 and all circles (soundwaves) will go
    // through the focal point (aircraft). In our previous implementations, this was known as the
    // edge case where the inside circle touches the outside circle (on the focal point). If we were
    // to solve for t bruteforcely using a quadratic equation, this case implies that the quadratic
    // equation degenerates to a linear equation.
    bool isFocalOnCircle() const { return this->isFocal() && fData.fFocalData.isFocalOnCircle(); }
    bool isSwapped() const { return this->isFocal() && fData.fFocalData.isSwapped(); }

    Type getType() const { return fData.fType; }
    bool isFocal() const { return fData.fType == Type::kFocal; }

    // Whether the t we solved is always valid (so we don't need to check r(t) > 0).
    bool isWellBehaved() const { return this->isFocal() && fData.fFocalData.isWellBehaved(); }

    // Whether r0 == 0 so it's focal without any transformation
    bool isNativelyFocal() const { return this->isFocal() && fData.fFocalData.isNativelyFocal(); }

    // Note that focalX = f = r0 / (r0 - r1), so 1 - focalX > 0 == r0 < r1
    bool isRadiusIncreasing() const {
        return this->isFocal() ? 1 - fData.fFocalData.fFocalX > 0 : this->diffRadius() > 0;
    }

protected:
    void onGetGLSLProcessorKey(const GrShaderCaps& c, GrProcessorKeyBuilder* b) const override {
        INHERITED::onGetGLSLProcessorKey(c, b);
        uint32_t key = 0;
        key |= static_cast<int>(fData.fType);
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
    if (type == static_cast<int>(TwoPointConicalEffect::Type::kRadial)) {
        center2 = center1;
        // Make sure that the radii are different
        if (SkScalarNearlyZero(radius1 - radius2)) {
            radius2 += .1f;
        }
    } else if (type == static_cast<int>(TwoPointConicalEffect::Type::kStrip)) {
        radius1 = SkTMax(radius1, .1f); // Make sure that the radius is non-zero
        radius2 = radius1;
        // Make sure that the centers are different
        if (SkScalarNearlyZero(SkPoint::Distance(center1, center2))) {
            center2.fX += .1f;
        }
    } else { // kFocal_Type
        // Make sure that the centers are different
        if (SkScalarNearlyZero(SkPoint::Distance(center1, center2))) {
            center2.fX += .1f;
        }

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

        // Make sure that the radii are different
        if (SkScalarNearlyZero(radius1 - radius2)) {
            radius2 += .1f;
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
        const TwoPointConicalEffect& effect = args.fFp.cast<TwoPointConicalEffect>();
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
        this->emitUniforms(uniformHandler, effect);
        fParamUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType,
                                               "Conical2FSParams");

        SkString p0; // r0 for radial case, r0^2 for strip case
        p0.appendf("%s", uniformHandler->getUniformVariable(fParamUni).getName().c_str());
        const char* tName = "t"; // the gradient

        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkString coords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);
        const char* p = coords2D.c_str();

        if (effect.getType() == Type::kRadial) {
            char sign = effect.diffRadius() < 0 ? '-' : '+';
            fragBuilder->codeAppendf("half %s = %clength(%s) - %s;", tName, sign, p, p0.c_str());
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
                        effect,
                        tName,
                        args.fOutputColor,
                        args.fInputColor,
                        args.fTexSamplers);

        if (effect.getType() != Type::kRadial) {
            fragBuilder->codeAppendf("}");
        }
    }

    void onSetData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& p) override {
        INHERITED::onSetData(pdman, p);
        const TwoPointConicalEffect& effect = p.cast<TwoPointConicalEffect>();
        // kRadialType should imply |r1 - r0| = 1 (after our transformation)
        SkASSERT(effect.getType() == Type::kStrip ||
                 SkScalarNearlyZero(SkTAbs(effect.diffRadius()) - 1));
        pdman.set1f(fParamUni, effect.getType() == Type::kRadial ? effect.r0()
                                                                 : effect.r0() * effect.r0());
    }

    UniformHandle fParamUni;

private:
    typedef GrGradientEffect::GLSLProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////
// FocalGLSLProcessor
//////////////////////////////////////////////////////////////////////////////

// Please see https://skia.org/dev/design/conical for how our shader works.
class TwoPointConicalEffect::FocalGLSLProcessor : public GrGradientEffect::GLSLProcessor {
protected:
    void emitCode(EmitArgs& args) override {
        const TwoPointConicalEffect& effect = args.fFp.cast<TwoPointConicalEffect>();
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
        this->emitUniforms(uniformHandler, effect);
        fParamUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf2_GrSLType,
                                               "Conical2FSParams");

        SkString p0; // 1 / r1
        SkString p1; // f = focalX = r0 / (r0 - r1)
        p0.appendf("%s.x", uniformHandler->getUniformVariable(fParamUni).getName().c_str());
        p1.appendf("%s.y", uniformHandler->getUniformVariable(fParamUni).getName().c_str());
        const char* tName = "t"; // the gradient

        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkString coords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);
        const char* p = coords2D.c_str();

        if (effect.isFocalOnCircle()) {
            fragBuilder->codeAppendf("half x_t = dot(%s, %s) / %s.x;", p, p, p);
        } else if (effect.isWellBehaved()) {
            fragBuilder->codeAppendf("half x_t = length(%s) - %s.x * %s;", p, p, p0.c_str());
        } else {
            char sign = (effect.isSwapped() || !effect.isRadiusIncreasing()) ? '-' : ' ';
            fragBuilder->codeAppendf("half temp = %s.x * %s.x - %s.y * %s.y;", p, p, p, p);
            // Initialize x_t to illegal state
            fragBuilder->codeAppendf("half x_t = -1;");

            // Only do sqrt if temp >= 0; this is significantly slower than checking temp >= 0 in
            // the if statement that checks r(t) >= 0. But GPU may break if we sqrt a negative
            // float. (Although I havevn't observed that on any devices so far, and the old approach
            // also does sqrt negative value without a check.) If the performance is really
            // critical, maybe we should just compute the area where temp and x_t are always
            // valid and drop all these ifs.
            fragBuilder->codeAppendf("if (temp >= 0) {");
            fragBuilder->codeAppendf("x_t = (%csqrt(temp) - %s.x * %s);", sign, p, p0.c_str());
            fragBuilder->codeAppendf("}");
        }

        // empty sign is positive
        char sign = effect.isRadiusIncreasing() ? ' ' : '-';

        // "+ 0" is much faster than "+ p1" so we specialize the natively focal case where p1 = 0.
        fragBuilder->codeAppendf("half %s = %cx_t + %s;", tName, sign,
                effect.isNativelyFocal() ? "0" : p1.c_str());

        if (!effect.isWellBehaved()) {
            // output will default to transparent black (we simply won't write anything
            // else to it if invalid, instead of discarding or returning prematurely)
            fragBuilder->codeAppendf("%s = half4(0.0,0.0,0.0,0.0);", args.fOutputColor);
            fragBuilder->codeAppendf("if (x_t > 0.0) {");
        }

        if (effect.isSwapped()) {
            fragBuilder->codeAppendf("%s = 1 - %s;", tName, tName);
        }

        this->emitColor(fragBuilder,
                        uniformHandler,
                        args.fShaderCaps,
                        effect,
                        tName,
                        args.fOutputColor,
                        args.fInputColor,
                        args.fTexSamplers);
        if (!effect.isWellBehaved()) {
            fragBuilder->codeAppend("};");
        }
    }

    void onSetData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& p) override {
        INHERITED::onSetData(pdman, p);
        const TwoPointConicalEffect& effect = p.cast<TwoPointConicalEffect>();
        pdman.set2f(fParamUni, 1 / effect.r1(), effect.focalX());
    }

    UniformHandle fParamUni;

private:
    typedef GrGradientEffect::GLSLProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

GrGLSLFragmentProcessor* TwoPointConicalEffect::onCreateGLSLInstance() const {
    if (fData.fType == Type::kRadial || fData.fType == Type::kStrip) {
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
    fType = shader.getType();
    if (fType == Type::kRadial) {
        // Map center to (0, 0)
        matrix.postTranslate(-shader.getStartCenter().fX, -shader.getStartCenter().fY);

        // scale |fDiffRadius| to 1
        SkScalar dr = shader.getDiffRadius();
        matrix.postScale(1 / dr, 1 / dr);
        fRadius0 = shader.getStartRadius() / dr;
        fDiffRadius = dr < 0 ? -1 : 1;
    } else if (fType == Type::kStrip) {
        fRadius0 = shader.getStartRadius() / shader.getCenterX1();
        fDiffRadius = 0;
        matrix.postConcat(shader.getGradientMatrix());
    } else if (fType == Type::kFocal) {
        fFocalData = shader.getFocalData();
        matrix.postConcat(shader.getGradientMatrix());
    }
}

#endif
