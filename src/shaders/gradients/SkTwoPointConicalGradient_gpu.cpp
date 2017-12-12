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

class TwoPointConicalEffect : public GrGradientEffect {
public:
    class DegeneratedGLSLProcessor;
    class FocalGLSLProcessor;

    enum Type {
        kRadial_Type = 0,
        kStrip_Type = 1,
        kFocal_Type = 2,
        kUnknown_Type = 3
    };

    static std::unique_ptr<GrFragmentProcessor> Make(const CreateArgs& args);

    // The radial gradient parameters can collapse to a linear (instead of quadratic) equation.
    SkScalar diffRadius() const { return fDiffRadius; }
    SkScalar r0() const { return fRadius0; }
    SkScalar r1() const { return fRadius0 + fDiffRadius; }

    const char* name() const override { return "Two-Point Conical Gradient"; }

    bool isLinear() const { return SkScalarNearlyZero(1 - this->r1()); }
    bool isSwapped() const { return fIsSwapped; }

    Type getType() const {
        return fType;
    }

    // Whether the t we solved is always valid (so we don't need to check r(t) > 0).
    bool isWellBehaved() const {
        return !this->isLinear() && this->r1() > 1;
    }

    // Whether we should use the first root of the quadratic equation
    bool isRadiusIncreasing() const {
        return fDiffRadius > 0;
    }

    bool isNativeFocal() const {
        return SkScalarNearlyZero(fRadius0);
    }

protected:
    uint32_t customGLSLProcessorKey() const override {
        uint32_t key = 0;
        SkASSERT(fType < 4);
        key |= fType;
        key |= (this->isLinear() << 2);
        key |= (this->isWellBehaved() << 3);
        key |= (this->isRadiusIncreasing() << 4);
        key |= (this->isNativeFocal() << 5);
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
                if (SkScalarNearlyZero(this->r1())) {
                    // swap r0, r1
                    fIsSwapped = true;
                    fRadius0 = 0;
                    fDiffRadius = -fDiffRadius;
                }
                SkScalar focalX = fRadius0 / fDiffRadius;
                fRadius0 /= SkScalarAbs(focalX + 1); // normalization to put focal point at (0, 0)
                fDiffRadius /= SkScalarAbs(focalX + 1);
                fType = kFocal_Type;
            } else {
                SkASSERT(SkScalarNearlyZero(fDiffRadius));
                fType = kStrip_Type;
            }
        }
    }

    explicit TwoPointConicalEffect(const TwoPointConicalEffect& that)
            : INHERITED(that)
            , fRadius0(that.fRadius0)
            , fDiffRadius(that.fDiffRadius)
            , fType(that.fType)
            , fIsSwapped(that.fIsSwapped) {}


    // @{
    // Cache of values - these can change arbitrarily, EXCEPT
    // we shouldn't change between degenerate and non-degenerate?!

    SkScalar            fRadius0;
    SkScalar            fDiffRadius;
    Type                fType;
    bool                fIsSwapped;

    // @}

    typedef GrGradientEffect INHERITED;
};

//////////////////////////////////////////////////////////////////////////////
// DegeneratedGLSLProcessor
//////////////////////////////////////////////////////////////////////////////

class TwoPointConicalEffect::DegeneratedGLSLProcessor : public GrGradientEffect::GLSLProcessor {
protected:
    void emitCode(EmitArgs& args) override {
        const TwoPointConicalEffect& ge = args.fFp.cast<TwoPointConicalEffect>();
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
        this->emitUniforms(uniformHandler, ge);
        fParamUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf2_GrSLType,
                                               "Conical2FSParams");

        SkString p0; // r0 / (r1 - r0) for radial case, r0^2 for strip case
        p0.appendf("%s.x", uniformHandler->getUniformVariable(fParamUni).getName().c_str());
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

        if (ge.getType() == kRadial_Type) {
            fragBuilder->codeAppendf("\thalf %s = length(%s) - %s;",
                    tName.c_str(), coords2D, p0.c_str());
        } else {
            // output will default to transparent black (we simply won't write anything
            // else to it if invalid, instead of discarding or returning prematurely)
            fragBuilder->codeAppendf("\t%s = half4(0.0,0.0,0.0,0.0);\n", args.fOutputColor);

            fragBuilder->codeAppendf("\thalf temp = %s - %s.y * %s.y;",
                    p0.c_str(), coords2D, coords2D);
            fragBuilder->codeAppendf("\tif (temp >= 0) {");
            fragBuilder->codeAppendf("\t\thalf %s = %s.x + sqrt(temp);", tName.c_str(), coords2D);
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

        if (ge.getType() != kRadial_Type) {
            fragBuilder->codeAppendf("\t}");
        }
    }

    void onSetData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& p) override {
        INHERITED::onSetData(pdman, p);
        const TwoPointConicalEffect& data = p.cast<TwoPointConicalEffect>();
        SkScalar r0 = data.r0();
        SkScalar r1 = data.r1();
        SkScalar p0 = data.getType() == kRadial_Type ? r0 / (r1 - r0) : r0 * r0;
        pdman.set2f(fParamUni, p0, data.diffRadius());
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
            fragBuilder->codeAppendf("\thalf %s_prime = dot(%s, %s) / %s.x;",
                    tName.c_str(), coords2D, coords2D, coords2D);
        } else if (ge.isWellBehaved()) {
            // empty sign is positive
            char sign = ge.isRadiusIncreasing() ? ' ' : '-';
            fragBuilder->codeAppendf("\thalf %s_prime = %clength(%s) - %s.x * %s;",
                    tName.c_str(), sign, coords2D, coords2D, p0.c_str());
        } else {
            char sign = ge.isSwapped() ? '-' : ' ';
            fragBuilder->codeAppendf("\thalf temp = %s.x * %s.x - %s.y * %s.y;",
                    coords2D, coords2D, coords2D, coords2D);
            fragBuilder->codeAppendf("\thalf %s_prime = (%csqrt(temp) - %s.x * %s);",
                    tName.c_str(), sign, coords2D, p0.c_str());
        }
        fragBuilder->codeAppendf("\thalf %s = %s_prime - %s;", tName.c_str(), tName.c_str(),
                ge.isNativeFocal() ? "0" : p1.c_str());

        if (ge.isSwapped()) {
            fragBuilder->codeAppendf("\t%s = 1 - %s;", tName.c_str(), tName.c_str());
        }

        if (!ge.isWellBehaved()) {
            // output will default to transparent black (we simply won't write anything
            // else to it if invalid, instead of discarding or returning prematurely)
            fragBuilder->codeAppendf("\t%s = half4(0.0,0.0,0.0,0.0);\n", args.fOutputColor);
            // r(t) must be nonnegative
            char direction = ge.isRadiusIncreasing() ? '>' : '<';
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
    if (fType == kRadial_Type || fType == kStrip_Type) {
        return new DegeneratedGLSLProcessor;
    }
    return new FocalGLSLProcessor;
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
        SkScalar dr = shader.getDiffRadius();
        matrix.postTranslate(-shader.getStartCenter().fX, -shader.getStartCenter().fY);
        matrix.postScale(1 / dr, 1 / dr);
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

        if (!SkScalarNearlyZero(shader.getDiffRadius())) {
            SkScalar normalizedDR = shader.getDiffRadius() / shader.getCenterX1();
            SkScalar normalizedR0 = shader.getStartRadius() / shader.getCenterX1();
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

            bool isLinear = SkScalarNearlyZero(1 - r1);
            if (isLinear) {
                // Note that r1 = 1 so r1 + 1 = 2 and 0.5 = 1 / (r1 + 1)
                scaleX = 0.5;
                scaleY = 0.5;
            } else {
                scaleX = r1 / (r1 * r1 - 1);
                scaleY = sqrt(SkScalarAbs(r1 * r1 - 1)) / (r1 * r1 - 1);
            }
            SkScalar scaleT = r1 / (r1 - r0);
            matrix.postScale(scaleX * scaleT, scaleY * scaleT);
        }
    }

    return TwoPointConicalEffect::Make(newArgs);
}

#endif
