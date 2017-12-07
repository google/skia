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
static const SkScalar kEdgeErrorTol = 5.f * kErrorTol;

// The case where two circles share the same center and the gradient is radial.
class TwoPointConicalEffectRadial : public GrGradientEffect {
public:
    class RadialProcessor;

    static std::unique_ptr<GrFragmentProcessor> Make(const CreateArgs& args);

protected:
    bool onIsEqual(const GrFragmentProcessor& sBase) const override {
        const TwoPointConicalEffectRadial& s = sBase.cast<TwoPointConicalEffectRadial>();
        return (INHERITED::onIsEqual(sBase) && fDiffRadius == s.fDiffRadius
                && fRadius0 == s.fRadius0);
    }

    explicit TwoPointConicalEffectRadial(const CreateArgs& args)
            : INHERITED(kTwoPointConicalEffectRadial_ClassID, args
                , false /* opaque: draws transparent black outside of the cone. */) {
        const SkTwoPointConicalGradient& shader =
                *static_cast<const SkTwoPointConicalGradient*>(args.fShader);
        fRadius0 = shader.getStartRadius();
        fDiffRadius = shader.getDiffRadius();
    }

    const char* name() const override {
        return "Two-Point Conical Gradient Radial";
    }

    SkScalar radius() const { return fRadius0; }
    SkScalar diffRadius() const { return fDiffRadius; }

protected:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new TwoPointConicalEffectRadial(*this));
    }

private:
    SkScalar         fRadius0;
    SkScalar         fDiffRadius;

    typedef GrGradientEffect INHERITED;
};

std::unique_ptr<GrFragmentProcessor> TwoPointConicalEffectRadial::Make(const CreateArgs& args) {
    const SkTwoPointConicalGradient& shader =
        *static_cast<const SkTwoPointConicalGradient*>(args.fShader);

    std::unique_ptr<TwoPointConicalEffectRadial> effect;
    effect.reset(new TwoPointConicalEffectRadial(args));

    return GrGradientEffect::AdjustFP(std::move(effect), args);
}

class TwoPointConicalEffectRadial::RadialProcessor : public GrGradientEffect::GLSLProcessor {
protected:
    void emitCode(EmitArgs& args) override {
        const TwoPointConicalEffectRadial& ge = args.fFp.cast<TwoPointConicalEffectRadial>();
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
        const TwoPointConicalEffectRadial& data = p.cast<TwoPointConicalEffectRadial>();
        pdman.set2f(fParamUni, data.radius(), data.diffRadius());
    }

    UniformHandle fParamUni;

private:
    typedef GrGradientEffect::GLSLProcessor INHERITED;
};

GrGLSLFragmentProcessor* TwoPointConicalEffectRadial::onCreateGLSLInstance() const {
    return new TwoPointConicalEffectRadial::RadialProcessor;
}

//////////////////////////////////////////////////////////////////////////////

// The general case where two circles do not share the same center.
class TwoPointConicalEffectGeneral : public GrGradientEffect {
public:
    class BaseProcessor;

    static std::unique_ptr<GrFragmentProcessor> Make(const CreateArgs& args);

    // The radial gradient parameters can collapse to a linear (instead of quadratic) equation.
    SkScalar diffRadius() const { return fDiffRadius; }
    SkScalar radius() const { return fRadius0; }
    SkScalar a() const { return 1 - fDiffRadius * fDiffRadius; }

protected:
    bool onIsEqual(const GrFragmentProcessor& sBase) const override {
        const TwoPointConicalEffectGeneral& s = sBase.cast<TwoPointConicalEffectGeneral>();
        return (INHERITED::onIsEqual(sBase) && fDiffRadius == s.fDiffRadius
            && fRadius0 == s.fRadius0);
    }

    explicit TwoPointConicalEffectGeneral(const CreateArgs& args, GrProcessor::ClassID classID)
        : INHERITED(classID, args,
            false /* opaque: draws transparent black outside of the cone. */) {
        const SkTwoPointConicalGradient& shader =
            *static_cast<const SkTwoPointConicalGradient*>(args.fShader);

        SkScalar centerX1 = shader.getCenterX1();

        // Normalize radius as centers are mapped to (0, 0), (1, 0)
        fRadius0 = shader.getStartRadius() / centerX1;
        fDiffRadius = shader.getDiffRadius() / centerX1;

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

    explicit TwoPointConicalEffectGeneral(const TwoPointConicalEffectGeneral& that)
        : INHERITED(that)
        , fBTransform(that.fBTransform)
        , fRadius0(that.fRadius0)
        , fDiffRadius(that.fDiffRadius) {
        this->addCoordTransform(&fBTransform);
    }


    // @{
    // Cache of values - these can change arbitrarily, EXCEPT
    // we shouldn't change between degenerate and non-degenerate?!

    GrCoordTransform fBTransform;
    SkScalar         fRadius0;
    SkScalar         fDiffRadius;

    // @}

    typedef GrGradientEffect INHERITED;
};

class TwoPointConicalEffectGeneral::BaseProcessor : public GrGradientEffect::GLSLProcessor {
protected:
    void emitCode(EmitArgs& args) override;

    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

    virtual void solveTR(GrGLSLFragmentBuilder* fragBuilder, const char* a, const char* b,
                         const char* c, const char* r0, const char* dr,
                         const char* t, const char* r) = 0;

    UniformHandle fParamUni;

private:
    typedef GrGradientEffect::GLSLProcessor INHERITED;
};

void TwoPointConicalEffectGeneral::BaseProcessor::onSetData(
    const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& processor) {
    INHERITED::onSetData(pdman, processor);
    const TwoPointConicalEffectGeneral& data = processor.cast<TwoPointConicalEffectGeneral>();
    pdman.set3f(fParamUni, data.a(), data.radius(), data.diffRadius());
}

void TwoPointConicalEffectGeneral::BaseProcessor::emitCode(EmitArgs& args) {
    const TwoPointConicalEffectGeneral& ge = args.fFp.cast<TwoPointConicalEffectGeneral>();
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    this->emitUniforms(uniformHandler, ge);
    fParamUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf3_GrSLType,
        "Conical2FSParams");

    SkString cName("c"); // constant coefficient of the quadratic equation
    SkString tName("t"); // the solution to the quadratic equation
    SkString rName("r"); // the radius when x = t
    SkString a;  // dr^2 - 1
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

    // c = (x^2)+(y^2) - params[1]
    fragBuilder->codeAppendf("\thalf %s = dot(%s, %s) - %s * %s;\n",
        cName.c_str(), coords2D, coords2D, r0.c_str(), r0.c_str());

    this->solveTR(fragBuilder, a.c_str(), bVar.c_str(), cName.c_str(), r0.c_str(), dr.c_str(),
                  tName.c_str(), rName.c_str());

    // if r(t) > 0, then t will be the x coordinate
    fragBuilder->codeAppendf("\tif (%s > 0.0) {\n", rName.c_str());
    fragBuilder->codeAppend("\t");
    this->emitColor(fragBuilder,
                    uniformHandler,
                    args.fShaderCaps,
                    ge,
                    tName.c_str(),
                    args.fOutputColor,
                    args.fInputColor,
                    args.fTexSamplers);
    fragBuilder->codeAppend("\t}\n");
}

// The case where the quadratic equation degenerates to a linear equation. That is,
// the inner circle touches outter circle on the edge.
class TwoPointConicalEffectLinear : public TwoPointConicalEffectGeneral {
public:
    explicit TwoPointConicalEffectLinear(const CreateArgs& args)
        : TwoPointConicalEffectGeneral(args, kTwoPointConicalEffectLinear_ClassID) {}

    const char* name() const override {
        return "Two-Point Conical Gradient Linear";
    }

    class LinearProcessor : public TwoPointConicalEffectGeneral::BaseProcessor {
    protected:
        void solveTR(GrGLSLFragmentBuilder* fragBuilder, const char* a, const char* b,
                     const char* c, const char* r0, const char* dr,
                     const char* t, const char* r) override {
            fragBuilder->codeAppendf("\thalf %s = -(%s / %s);\n", t, c, b);
            fragBuilder->codeAppendf("\thalf %s = %s + %s * %s;\n", r, r0, t, dr);
        }
    };

protected:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        return new LinearProcessor;
    }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new TwoPointConicalEffectLinear(*this));
    }
};

// The most general case with a quadratic equation to solve.
class TwoPointConicalEffectQuadratic : public TwoPointConicalEffectGeneral {
public:
    explicit TwoPointConicalEffectQuadratic(const CreateArgs& args)
            : TwoPointConicalEffectGeneral(args, kTwoPointConicalEffectQuadratic_ClassID) {}

    const char* name() const override {
        return "Two-Point Conical Gradient Quadratic";
    }

    class QuadraticProcessor : public TwoPointConicalEffectGeneral::BaseProcessor {
    protected:
        void solveTR(GrGLSLFragmentBuilder* fragBuilder, const char* a, const char* b,
                     const char* c, const char* r0, const char* dr,
                     const char* t, const char* r) override {
            fragBuilder->codeAppendf("\thalf disc = %s * %s - 4 * %s * %s;\n", b, b, a, c);
            fragBuilder->codeAppendf("\thalf sqrt_disc = sqrt(disc);\n");
            fragBuilder->codeAppendf("\thalf %s = (-%s + sqrt_disc) / (2 * %s);\n", t, b, a);
            fragBuilder->codeAppendf("\thalf %s = %s + %s * %s;\n", r, r0, t, dr);
            fragBuilder->codeAppendf("\thalf %s_2 = (-%s - sqrt_disc) / (2 * %s);\n", t, b, a);
            fragBuilder->codeAppendf("\thalf %s_2 = %s + %s_2 * %s;\n", r, r0, t, dr);
            fragBuilder->codeAppendf("\tif (%s < 0 || (%s_2 > 0 && %s_2 > %s)) {\n", r, r, t, t);
            fragBuilder->codeAppendf("\t\t%s = %s_2;\n", t, t);
            fragBuilder->codeAppendf("\t\t%s = %s_2;\n", r, r);
            fragBuilder->codeAppendf("\t}\n");
        }
    };

protected:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        return new QuadraticProcessor;
    }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new TwoPointConicalEffectQuadratic(*this));
    }
};

std::unique_ptr<GrFragmentProcessor> TwoPointConicalEffectGeneral::Make(const CreateArgs& args) {
    const SkTwoPointConicalGradient& shader =
        *static_cast<const SkTwoPointConicalGradient*>(args.fShader);

    SkScalar centerX1 = shader.getCenterX1();

    // Normalize radius as centers are mapped to (0, 0), (1, 0)
    // SkScalar radius0 = shader.getStartRadius() / centerX1;
    SkScalar diffRadius = shader.getDiffRadius() / centerX1;

    std::unique_ptr<TwoPointConicalEffectGeneral> effect;

    if (SkScalarAbs(1 - diffRadius * diffRadius) < kEdgeErrorTol) {
        effect.reset(new TwoPointConicalEffectLinear(args));
    }
    else {
        effect.reset(new TwoPointConicalEffectQuadratic(args));
    }

    return GrGradientEffect::AdjustFP(std::move(effect), args);
}

//////////////////////////////////////////////////////////////////////////////

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
        return TwoPointConicalEffectRadial::Make(newArgs);
    }

    // Map centers to (0, 0), (1, 0)
    const SkPoint centers[2] = { shader.getStartCenter(), shader.getEndCenter() };
    const SkPoint unitvec[2] = { { 0, 0 },{ 1, 0 } };
    SkMatrix gradientMatrix;
    if (!gradientMatrix.setPolyToPoly(centers, unitvec, 2)) {
        // Degenerate case.
        return nullptr;
    }

    matrix.postConcat(gradientMatrix);

    return TwoPointConicalEffectGeneral::Make(newArgs);
}

#endif
