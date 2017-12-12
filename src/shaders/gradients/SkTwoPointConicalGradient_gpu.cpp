/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTwoPointConicalGradient.h"

#if SK_SUPPORT_GPU
#include "GrCoordTransform.h"
#include "GrPaint.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "SkTwoPointConicalGradient_gpu.h"

#include <cmath>

// For brevity
typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

static const SkScalar kErrorTol = 0.00001f;

class TwoPointConicalEffect : public GrGradientEffect {
public:
    class RadialGLSLProcessor;
    class GeneralGLSLProcessor;
    class QuadraticGLSLProcessor;

    enum Type {
        kRadial_Type = 0,
        kInsideFocal_Type = 1,
        kOutsideFocal_Type = 2,
        kGeneral_Type = 3,
        kUnknown_Type = 4
    };

    static std::unique_ptr<GrFragmentProcessor> Make(const CreateArgs& args);

    // The radial gradient parameters can collapse to a linear (instead of quadratic) equation.
    SkScalar diffRadius() const { return fDiffRadius; }
    SkScalar radius() const { return fRadius0; }
    SkScalar a() const { return 1 - fDiffRadius * fDiffRadius; }

    const char* name() const override { return "Two-Point Conical Gradient"; }

    bool isLinear() const { return SkScalarNearlyZero(1 - fRadius0 - fDiffRadius); }
    bool isSwapped() const { return fIsSwapped; }

    Type getType() const {
        return fType;
    }

    // Whether the t we solved is always valid (so we don't need to check r(t) > 0).
    bool isWellBehaved() const {
        if (fType == kInsideFocal_Type || fType == kOutsideFocal_Type) {
            return !this->isLinear() && SkScalarAbs(fRadius0 + fDiffRadius) > 1;
        }
        return !this->isLinear() && SkScalarAbs(fDiffRadius) >= 1;
    }

    // Whether we should use the first root of the quadratic equation
    bool isUsingFirstRoot() const {
        // For focal types, this means whether dr > 0
        if (fType == kInsideFocal_Type || fType == kOutsideFocal_Type) {
            return fDiffRadius > 0;
        }

        // See SkTwoPointConicalGradient.cpp for the discussion about isFlipped and isFirst
        bool isFlipped = this->isWellBehaved() && fDiffRadius < 0;
        return (this->a() > 0) ^ isFlipped;
    }

protected:
    uint32_t customGLSLProcessorKey() const override {
        uint32_t key = 0;
        SkASSERT(fType < 4);
        key |= fType;
        key |= (this->isLinear() << 2);
        key |= (this->isWellBehaved() << 3);
        key |= (this->isUsingFirstRoot() << 4);
        key |= (SkScalarNearlyZero(this->radius()) << 5);
        key |= (this->isSwapped() << 6);
        return key;
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new TwoPointConicalEffect(*this));
    }

    bool onIsEqual(const GrFragmentProcessor& sBase) const override {
        const TwoPointConicalEffect& s = sBase.cast<TwoPointConicalEffect>();
        return (INHERITED::onIsEqual(sBase) && fDiffRadius == s.fDiffRadius
            && fRadius0 == s.fRadius0 && fType == s.fType && fIsSwapped == s.fIsSwapped);
    }

    explicit TwoPointConicalEffect(const CreateArgs& args)
        : INHERITED(kTwoPointConicalEffect_ClassID, args,
            false /* opaque: draws transparent black outside of the cone. */) {
        const SkTwoPointConicalGradient& shader =
            *static_cast<const SkTwoPointConicalGradient*>(args.fShader);

        SkScalar centerX1 = shader.getCenterX1();
        fType = kUnknown_Type;
        fIsSwapped = false;
        if (SkScalarNearlyZero(centerX1)) {
            fType = kRadial_Type;
        }

        fRadius0 = shader.getStartRadius();
        fDiffRadius = shader.getDiffRadius();

        if (fType != kRadial_Type) {
            // Normalize radius as centers are mapped to (0, 0), (1, 0)
            fRadius0 /= centerX1;
            fDiffRadius /= centerX1;

            bool isFocal = !SkScalarNearlyZero(fDiffRadius);

            if (isFocal) {
                if (SkScalarNearlyZero(fRadius0 + fDiffRadius)) {
                    // swap r0, r1
                    fIsSwapped = true;
                    fRadius0 = 0;
                    fDiffRadius = -fDiffRadius;
                }
                SkScalar focalX = fRadius0 / fDiffRadius;
                fRadius0 /= SkScalarAbs(focalX + 1); // normalization to put focal point at (0, 0)
                fDiffRadius /= SkScalarAbs(focalX + 1);
                fType = fRadius0 + fDiffRadius >= 1 ? kInsideFocal_Type : kOutsideFocal_Type;
            } else {
                fType = kGeneral_Type;
                // We pass the linear coefficient of the quadratic equation as a varying.
                //    float b = -2.0 * (x + fRadius0 * fDiffRadius * z)
                fBTransform = this->getCoordTransform();
                SkMatrix& bMatrix = *fBTransform.accessMatrix();
                SkScalar r0dr = fRadius0 * fDiffRadius;
                bMatrix[SkMatrix::kMScaleX] = -2 * (bMatrix[SkMatrix::kMScaleX] +
                    r0dr * bMatrix[SkMatrix::kMPersp0]);
                bMatrix[SkMatrix::kMSkewX] = -2 * (bMatrix[SkMatrix::kMSkewX] +
                    r0dr * bMatrix[SkMatrix::kMPersp1]);
                bMatrix[SkMatrix::kMTransX] = -2 * (bMatrix[SkMatrix::kMTransX] +
                    r0dr * bMatrix[SkMatrix::kMPersp2]);
                this->addCoordTransform(&fBTransform);
            }
        }
    }

    explicit TwoPointConicalEffect(const TwoPointConicalEffect& that)
            : INHERITED(that)
            , fBTransform(that.fBTransform)
            , fRadius0(that.fRadius0)
            , fDiffRadius(that.fDiffRadius)
            , fType(that.fType)
            , fIsSwapped(that.fIsSwapped) {
        if (fType == kGeneral_Type) {
            this->addCoordTransform(&fBTransform);
        }
    }


    // @{
    // Cache of values - these can change arbitrarily, EXCEPT
    // we shouldn't change between degenerate and non-degenerate?!

    GrCoordTransform    fBTransform;
    SkScalar            fRadius0;
    SkScalar            fDiffRadius;
    Type                fType;
    bool                fIsSwapped;

    // @}

    typedef GrGradientEffect INHERITED;
};

//////////////////////////////////////////////////////////////////////////////
// RadialGLSLProcessor
//////////////////////////////////////////////////////////////////////////////

class TwoPointConicalEffect::RadialGLSLProcessor : public GrGradientEffect::GLSLProcessor {
protected:
    void emitCode(EmitArgs& args) override {
        const TwoPointConicalEffect& ge = args.fFp.cast<TwoPointConicalEffect>();
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
        this->emitUniforms(uniformHandler, ge);
        fParamUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf2_GrSLType,
                                               "Conical2FSParams");

        SkString r0; // start radius
        SkString dr; // difference in radii (r1 - r0)
        r0.appendf("%s.x", uniformHandler->getUniformVariable(fParamUni).getName().c_str());
        dr.appendf("%s.y", uniformHandler->getUniformVariable(fParamUni).getName().c_str());
        SkString tName("t"); // the solution to the quadratic equation

        const char* coords2D;
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        if (kHalf3_GrSLType == args.fTransformedCoords[0].getType()) {
            fragBuilder->codeAppendf("\thalf3 interpolants = %s.xy / %s.z;\n",
                args.fTransformedCoords[0].c_str(),
                args.fTransformedCoords[0].c_str());
            coords2D = "interpolants";
        } else {
            coords2D = args.fTransformedCoords[0].c_str();
        }

        fragBuilder->codeAppendf("\thalf %s = (sqrt(dot(%s, %s)) - %s) / %s;",
                tName.c_str(), coords2D, coords2D, r0.c_str(), dr.c_str());

        this->emitColor(fragBuilder,
                        uniformHandler,
                        args.fShaderCaps,
                        ge,
                        tName.c_str(),
                        args.fOutputColor,
                        args.fInputColor,
                        args.fTexSamplers);
    }

    void onSetData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& p) override {
        INHERITED::onSetData(pdman, p);
        const TwoPointConicalEffect& data = p.cast<TwoPointConicalEffect>();
        pdman.set2f(fParamUni, data.radius(), data.diffRadius());
    }

    UniformHandle fParamUni;

private:
    typedef GrGradientEffect::GLSLProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////
// QuadraticGLSLProcessor
//////////////////////////////////////////////////////////////////////////////

class TwoPointConicalEffect::QuadraticGLSLProcessor : public GrGradientEffect::GLSLProcessor {
protected:
    void emitCode(EmitArgs& args) override {
        const TwoPointConicalEffect& ge = args.fFp.cast<TwoPointConicalEffect>();
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
        this->emitUniforms(uniformHandler, ge);
        fParamUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf2_GrSLType,
                                               "Conical2FSParams");

        SkString p0; // r1^2 for ouside case, r1 for inside case, r1 + 1 for linear case
        SkString p1; // r0 / (r1 - r0)
        p0.appendf("%s.x", uniformHandler->getUniformVariable(fParamUni).getName().c_str());
        p1.appendf("%s.y", uniformHandler->getUniformVariable(fParamUni).getName().c_str());
        SkString tName("t"); // the solution to the quadratic equation

        const char* coords2D;
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        if (kHalf3_GrSLType == args.fTransformedCoords[0].getType()) {
            fragBuilder->codeAppendf("\thalf3 interpolants = %s.xy / %s.z;\n",
                args.fTransformedCoords[0].c_str(),
                args.fTransformedCoords[0].c_str());
            coords2D = "interpolants";
        } else {
            coords2D = args.fTransformedCoords[0].c_str();
        }

        if (ge.isLinear()) {
            // Note that r1 = 1 so r1 + 1 = 2
            fragBuilder->codeAppendf("\thalf %s_prime = dot(%s, %s) / %s.x;",
                    tName.c_str(), coords2D, coords2D, coords2D);
        } else if (ge.getType() == kInsideFocal_Type) {
            // empty sign is positive
            char sign = ge.isUsingFirstRoot() ? ' ' : '-';
            fragBuilder->codeAppendf("\thalf %s_prime = %clength(%s) - %s.x / %s;",
                    tName.c_str(), sign, coords2D, coords2D, p0.c_str());
        } else {
            char sign = ge.isSwapped() ? '-' : ' ';
            fragBuilder->codeAppendf("\thalf temp = %s * %s.x * %s.x - %s.y * %s.y;",
                    p0.c_str(), coords2D, coords2D, coords2D, coords2D);
            fragBuilder->codeAppendf("\thalf %s_prime = (%csqrt(temp) - %s.x);",
                    tName.c_str(), sign, coords2D);
        }
        fragBuilder->codeAppendf("\thalf %s = %s_prime - %s;", tName.c_str(), tName.c_str(),
                SkScalarNearlyZero(ge.radius()) ? "0" : p1.c_str());

        if (ge.isSwapped()) {
            fragBuilder->codeAppendf("\t%s = 1 - %s;", tName.c_str(), tName.c_str());
        }

        if (!ge.isWellBehaved()) {
            // output will default to transparent black (we simply won't write anything
            // else to it if invalid, instead of discarding or returning prematurely)
            fragBuilder->codeAppendf("\t%s = half4(0.0,0.0,0.0,0.0);\n", args.fOutputColor);
            // r(t) must be nonnegative
            char direction = ge.isUsingFirstRoot() ? '>' : '<';
            fragBuilder->codeAppendf("\tif (%s_prime %c= 0.0) {\n", tName.c_str(), direction);
            fragBuilder->codeAppend("\t\t");
        }
        this->emitColor(fragBuilder,
                        uniformHandler,
                        args.fShaderCaps,
                        ge,
                        tName.c_str(),
                        args.fOutputColor,
                        args.fInputColor,
                        args.fTexSamplers);
        if (!ge.isWellBehaved()) {
            fragBuilder->codeAppend("\t};");
        }
    }

    void onSetData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& p) override {
        INHERITED::onSetData(pdman, p);
        const TwoPointConicalEffect& data = p.cast<TwoPointConicalEffect>();
        SkScalar r0 = data.radius();
        SkScalar r1 = data.radius() + data.diffRadius();
        SkScalar p0 = data.getType() == kInsideFocal_Type ? r1 : r1 * r1;
        pdman.set2f(fParamUni, p0, r0 / (r1 - r0));
    }

    UniformHandle fParamUni;

private:
    typedef GrGradientEffect::GLSLProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////
// GeneralGLSLProcessor
//////////////////////////////////////////////////////////////////////////////

class TwoPointConicalEffect::GeneralGLSLProcessor : public GrGradientEffect::GLSLProcessor {
protected:
    void emitCode(EmitArgs& args) override;

    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

    UniformHandle fParamUni;

private:
    typedef GrGradientEffect::GLSLProcessor INHERITED;
};

void TwoPointConicalEffect::GeneralGLSLProcessor::onSetData(
    const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& processor) {
    INHERITED::onSetData(pdman, processor);
    const TwoPointConicalEffect& data = processor.cast<TwoPointConicalEffect>();
    pdman.set3f(fParamUni, data.a(), data.radius(), data.diffRadius());
}

void TwoPointConicalEffect::GeneralGLSLProcessor::emitCode(EmitArgs& args) {
    const TwoPointConicalEffect& ge = args.fFp.cast<TwoPointConicalEffect>();
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    this->emitUniforms(uniformHandler, ge);
    fParamUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf3_GrSLType,
        "Conical2FSParams");

    SkString cName("c"); // constant coefficient of the quadratic equation: x^2 + y^2 - r0^2
    SkString tName("t"); // the solution to the quadratic equation
    SkString a;  // quadratic coefficient of the quadratic equation: 1 - dr^2
    SkString r0; // start radius
    SkString dr; // difference in radii (r1 - r0)

    a.appendf("%s.x", uniformHandler->getUniformVariable(fParamUni).getName().c_str());
    r0.appendf("%s.y", uniformHandler->getUniformVariable(fParamUni).getName().c_str());
    dr.appendf("%s.z", uniformHandler->getUniformVariable(fParamUni).getName().c_str());

    // We interpolate the linear component in coords[1].
    SkASSERT(args.fTransformedCoords[0].getType() == args.fTransformedCoords[1].getType());
    const char* coords2D;
    SkString bVar;
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    if (kHalf3_GrSLType == args.fTransformedCoords[0].getType()) {
        fragBuilder->codeAppendf("\thalf3 interpolants = half3(%s.xy / %s.z, %s.x / %s.z);\n",
            args.fTransformedCoords[0].c_str(),
            args.fTransformedCoords[0].c_str(),
            args.fTransformedCoords[1].c_str(),
            args.fTransformedCoords[1].c_str());
        coords2D = "interpolants.xy";
        bVar = "interpolants.z";
    } else {
        coords2D = args.fTransformedCoords[0].c_str();
        bVar.printf("%s.x", args.fTransformedCoords[1].c_str());
    }

    // output will default to transparent black (we simply won't write anything
    // else to it if invalid, instead of discarding or returning prematurely)
    fragBuilder->codeAppendf("\t%s = half4(0.0,0.0,0.0,0.0);\n", args.fOutputColor);

    // c = x^2 + y^2 - r0^2
    fragBuilder->codeAppendf("\thalf %s = dot(%s, %s) - %s * %s;\n",
        cName.c_str(), coords2D, coords2D, r0.c_str(), r0.c_str());

    const char* ac  = a.c_str();
    const char* bc  = bVar.c_str();
    const char* cc  = cName.c_str();
    const char* r0c = r0.c_str();
    const char* drc = dr.c_str();
    const char* tc  = tName.c_str();

    fragBuilder->codeAppendf("\thalf disc = %s * %s - 4 * %s * %s;\n", bc, bc, ac, cc);
    fragBuilder->codeAppendf("\thalf sqrt_disc = sqrt(disc);\n");
    if (ge.isUsingFirstRoot()) {
        fragBuilder->codeAppendf("\thalf %s = (-%s + sqrt_disc) / (2 * %s);\n", tc, bc, ac);
    } else {
        fragBuilder->codeAppendf("\thalf %s = (-%s - sqrt_disc) / (2 * %s);\n", tc, bc, ac);
    }

    if (!ge.isWellBehaved()) {
        // if r(t) > 0, then t will be the x coordinate
        fragBuilder->codeAppendf("\tif (%s * %s + %s > 0.0) {\n", tc, drc, r0c);
        fragBuilder->codeAppend("\t");
    }

    this->emitColor(fragBuilder,
                    uniformHandler,
                    args.fShaderCaps,
                    ge,
                    tName.c_str(),
                    args.fOutputColor,
                    args.fInputColor,
                    args.fTexSamplers);

    if (!ge.isWellBehaved()) {
        fragBuilder->codeAppend("\t}\n");
    }
}

//////////////////////////////////////////////////////////////////////////////

GrGLSLFragmentProcessor* TwoPointConicalEffect::onCreateGLSLInstance() const {
    if (fType == kRadial_Type) {
        return new RadialGLSLProcessor;
    } else if (fType == kInsideFocal_Type || fType == kOutsideFocal_Type) {
        return new QuadraticGLSLProcessor;
    }
    return new GeneralGLSLProcessor;
}

std::unique_ptr<GrFragmentProcessor> TwoPointConicalEffect::Make(
        const GrGradientEffect::CreateArgs& args) {
    return GrGradientEffect::AdjustFP(
            std::unique_ptr<TwoPointConicalEffect>(new TwoPointConicalEffect(args)),
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

    if (SkScalarNearlyZero(shader.getCenterX1())) {
        matrix.postTranslate(-shader.getStartCenter().fX, -shader.getStartCenter().fY);
    } else {
        // Map centers to (0, 0), (1, 0)
        const SkPoint centers[2] = { shader.getStartCenter(), shader.getEndCenter() };
        const SkPoint unitvec[2] = { { 0, 0 },{ 1, 0 } };
        SkMatrix gradientMatrix;
        if (!gradientMatrix.setPolyToPoly(centers, unitvec, 2)) {
            // Degenerate case.
            return nullptr;
        }
        matrix.postConcat(gradientMatrix);

        SkScalar normalizedDR = shader.getDiffRadius() / shader.getCenterX1();
        SkScalar normalizedR0 = shader.getStartRadius() / shader.getCenterX1();
        if (!SkScalarNearlyZero(shader.getDiffRadius())) {
            if (SkScalarNearlyZero(shader.getEndRadius())) {
                // swap r0, r1
                normalizedR0 = 0;
                normalizedDR = -normalizedDR;
                matrix.postTranslate(-1, 0);
                matrix.postScale(-1, 1);
            }
            SkScalar focalX = normalizedR0 / normalizedDR;
            const SkPoint from[2]   = { {-focalX, 0}, {1, 0} };
            const SkPoint to[2]     = { {0, 0}, {1, 0} };
            SkMatrix focalMatrix;
            focalMatrix.setPolyToPoly(from, to, 2);
            matrix.postConcat(focalMatrix);

            SkScalar r0 = normalizedR0 / SkScalarAbs(focalX + 1);
            SkScalar r1 = (normalizedR0 + normalizedDR) / SkScalarAbs(focalX + 1);

            SkScalar scaleX = 1;
            SkScalar scaleY = 1;

            bool isLinear = SkScalarAbs(1 - normalizedDR * normalizedDR) < kErrorTol;
            if (isLinear) {
                // 0.5 = 1 / (r1 + 1)
                scaleX = 0.5;
                scaleY = 0.5;
            } else if (r1 > 1) {
                scaleX = r1 / (r1 * r1 - 1);
                scaleY = sqrt(r1 * r1 - 1) / (r1 * r1 - 1);
            } else {
                scaleX = 1 / (r1 * r1 - 1);
                scaleY = sqrt(1 - r1 * r1) / (r1 * r1 - 1);
            }
            SkScalar scaleT = r1 / (r1 - r0);
            matrix.postScale(scaleX * scaleT, scaleY * scaleT);
        }
    }

    return TwoPointConicalEffect::Make(newArgs);
}

#endif
