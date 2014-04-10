/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTwoPointConicalGradient_gpu.h"
#include "GrTBackendEffectFactory.h"

#include "SkTwoPointConicalGradient.h"

#if SK_SUPPORT_GPU
// For brevity
typedef GrGLUniformManager::UniformHandle UniformHandle;

//////////////////////////////////////////////////////////////////////////////

static void set_matrix_default_conical(const SkTwoPointConicalGradient& shader,
                                       SkMatrix* invLMatrix) {
    // Inverse of the current local matrix is passed in then,
    // translate to center1, rotate so center2 is on x axis.
    const SkPoint& center1 = shader.getStartCenter();
    const SkPoint& center2 = shader.getEndCenter();

    invLMatrix->postTranslate(-center1.fX, -center1.fY);

    SkPoint diff = center2 - center1;
    SkScalar diffLen = diff.length();
    if (0 != diffLen) {
        SkScalar invDiffLen = SkScalarInvert(diffLen);
        SkMatrix rot;
        rot.setSinCos(-SkScalarMul(invDiffLen, diff.fY),
                       SkScalarMul(invDiffLen, diff.fX));
        invLMatrix->postConcat(rot);
    }
}

class GLDefault2PtConicalEffect;

class Default2PtConicalEffect : public GrGradientEffect {
public:

    static GrEffectRef* Create(GrContext* ctx,
                               const SkTwoPointConicalGradient& shader,
                               const SkMatrix& matrix,
                               SkShader::TileMode tm) {
        AutoEffectUnref effect(SkNEW_ARGS(Default2PtConicalEffect, (ctx, shader, matrix, tm)));
        return CreateEffectRef(effect);
    }

    virtual ~Default2PtConicalEffect() { }

    static const char* Name() { return "Two-Point Conical Gradient"; }
    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

    // The radial gradient parameters can collapse to a linear (instead of quadratic) equation.
    bool isDegenerate() const { return SkScalarAbs(fDiffRadius) == SkScalarAbs(fCenterX1); }
    SkScalar center() const { return fCenterX1; }
    SkScalar diffRadius() const { return fDiffRadius; }
    SkScalar radius() const { return fRadius0; }

    typedef GLDefault2PtConicalEffect GLEffect;

private:
    virtual bool onIsEqual(const GrEffect& sBase) const SK_OVERRIDE {
        const Default2PtConicalEffect& s = CastEffect<Default2PtConicalEffect>(sBase);
        return (INHERITED::onIsEqual(sBase) &&
                this->fCenterX1 == s.fCenterX1 &&
                this->fRadius0 == s.fRadius0 &&
                this->fDiffRadius == s.fDiffRadius);
    }

    Default2PtConicalEffect(GrContext* ctx,
                            const SkTwoPointConicalGradient& shader,
                            const SkMatrix& matrix,
                            SkShader::TileMode tm)
        : INHERITED(ctx, shader, matrix, tm),
        fCenterX1(shader.getCenterX1()),
        fRadius0(shader.getStartRadius()),
        fDiffRadius(shader.getDiffRadius()) {
        // We pass the linear part of the quadratic as a varying.
        //    float b = -2.0 * (fCenterX1 * x + fRadius0 * fDiffRadius * z)
        fBTransform = this->getCoordTransform();
        SkMatrix& bMatrix = *fBTransform.accessMatrix();
        SkScalar r0dr = SkScalarMul(fRadius0, fDiffRadius);
        bMatrix[SkMatrix::kMScaleX] = -2 * (SkScalarMul(fCenterX1, bMatrix[SkMatrix::kMScaleX]) +
                                            SkScalarMul(r0dr, bMatrix[SkMatrix::kMPersp0]));
        bMatrix[SkMatrix::kMSkewX] = -2 * (SkScalarMul(fCenterX1, bMatrix[SkMatrix::kMSkewX]) +
                                           SkScalarMul(r0dr, bMatrix[SkMatrix::kMPersp1]));
        bMatrix[SkMatrix::kMTransX] = -2 * (SkScalarMul(fCenterX1, bMatrix[SkMatrix::kMTransX]) +
                                            SkScalarMul(r0dr, bMatrix[SkMatrix::kMPersp2]));
        this->addCoordTransform(&fBTransform);
    }

    GR_DECLARE_EFFECT_TEST;

    // @{
    // Cache of values - these can change arbitrarily, EXCEPT
    // we shouldn't change between degenerate and non-degenerate?!

    GrCoordTransform fBTransform;
    SkScalar         fCenterX1;
    SkScalar         fRadius0;
    SkScalar         fDiffRadius;

    // @}

    typedef GrGradientEffect INHERITED;
};

class GLDefault2PtConicalEffect : public GrGLGradientEffect {
public:
    GLDefault2PtConicalEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&);
    virtual ~GLDefault2PtConicalEffect() { }

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          EffectKey,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;
    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

    static EffectKey GenKey(const GrDrawEffect&, const GrGLCaps& caps);

protected:
    UniformHandle fParamUni;

    const char* fVSVaryingName;
    const char* fFSVaryingName;

    bool fIsDegenerate;

    // @{
    /// Values last uploaded as uniforms

    SkScalar fCachedCenter;
    SkScalar fCachedRadius;
    SkScalar fCachedDiffRadius;

    // @}

private:
    typedef GrGLGradientEffect INHERITED;

};

const GrBackendEffectFactory& Default2PtConicalEffect::getFactory() const {
    return GrTBackendEffectFactory<Default2PtConicalEffect>::getInstance();
}

GR_DEFINE_EFFECT_TEST(Default2PtConicalEffect);

GrEffectRef* Default2PtConicalEffect::TestCreate(SkRandom* random,
                                            GrContext* context,
                                            const GrDrawTargetCaps&,
                                            GrTexture**) {
    SkPoint center1 = {random->nextUScalar1(), random->nextUScalar1()};
    SkScalar radius1 = random->nextUScalar1();
    SkPoint center2;
    SkScalar radius2;
    do {
        center2.set(random->nextUScalar1(), random->nextUScalar1());
        radius2 = random->nextUScalar1 ();
        // If the circles are identical the factory will give us an empty shader.
    } while (radius1 == radius2 && center1 == center2);

    SkColor colors[kMaxRandomGradientColors];
    SkScalar stopsArray[kMaxRandomGradientColors];
    SkScalar* stops = stopsArray;
    SkShader::TileMode tm;
    int colorCount = RandomGradientParams(random, colors, &stops, &tm);
    SkAutoTUnref<SkShader> shader(SkGradientShader::CreateTwoPointConical(center1, radius1,
                                                                          center2, radius2,
                                                                          colors, stops, colorCount,
                                                                          tm));
    SkPaint paint;
    return shader->asNewEffect(context, paint);
}


/////////////////////////////////////////////////////////////////////

GLDefault2PtConicalEffect::GLDefault2PtConicalEffect(const GrBackendEffectFactory& factory,
                                           const GrDrawEffect& drawEffect)
    : INHERITED(factory)
    , fVSVaryingName(NULL)
    , fFSVaryingName(NULL)
    , fCachedCenter(SK_ScalarMax)
    , fCachedRadius(-SK_ScalarMax)
    , fCachedDiffRadius(-SK_ScalarMax) {

    const Default2PtConicalEffect& data = drawEffect.castEffect<Default2PtConicalEffect>();
    fIsDegenerate = data.isDegenerate();
}

void GLDefault2PtConicalEffect::emitCode(GrGLShaderBuilder* builder,
                                    const GrDrawEffect&,
                                    EffectKey key,
                                    const char* outputColor,
                                    const char* inputColor,
                                    const TransformedCoordsArray& coords,
                                    const TextureSamplerArray& samplers) {
    this->emitUniforms(builder, key);
    fParamUni = builder->addUniformArray(GrGLShaderBuilder::kFragment_Visibility,
                                         kFloat_GrSLType, "Conical2FSParams", 6);

    SkString cName("c");
    SkString ac4Name("ac4");
    SkString dName("d");
    SkString qName("q");
    SkString r0Name("r0");
    SkString r1Name("r1");
    SkString tName("t");
    SkString p0; // 4a
    SkString p1; // 1/a
    SkString p2; // distance between centers
    SkString p3; // start radius
    SkString p4; // start radius squared
    SkString p5; // difference in radii (r1 - r0)

    builder->getUniformVariable(fParamUni).appendArrayAccess(0, &p0);
    builder->getUniformVariable(fParamUni).appendArrayAccess(1, &p1);
    builder->getUniformVariable(fParamUni).appendArrayAccess(2, &p2);
    builder->getUniformVariable(fParamUni).appendArrayAccess(3, &p3);
    builder->getUniformVariable(fParamUni).appendArrayAccess(4, &p4);
    builder->getUniformVariable(fParamUni).appendArrayAccess(5, &p5);

    // We interpolate the linear component in coords[1].
    SkASSERT(coords[0].type() == coords[1].type());
    const char* coords2D;
    SkString bVar;
    if (kVec3f_GrSLType == coords[0].type()) {
        builder->fsCodeAppendf("\tvec3 interpolants = vec3(%s.xy, %s.x) / %s.z;\n",
                               coords[0].c_str(), coords[1].c_str(), coords[0].c_str());
        coords2D = "interpolants.xy";
        bVar = "interpolants.z";
    } else {
        coords2D = coords[0].c_str();
        bVar.printf("%s.x", coords[1].c_str());
    }

    // output will default to transparent black (we simply won't write anything
    // else to it if invalid, instead of discarding or returning prematurely)
    builder->fsCodeAppendf("\t%s = vec4(0.0,0.0,0.0,0.0);\n", outputColor);

    // c = (x^2)+(y^2) - params[4]
    builder->fsCodeAppendf("\tfloat %s = dot(%s, %s) - %s;\n",
                           cName.c_str(), coords2D, coords2D, p4.c_str());

    // Non-degenerate case (quadratic)
    if (!fIsDegenerate) {

        // ac4 = params[0] * c
        builder->fsCodeAppendf("\tfloat %s = %s * %s;\n", ac4Name.c_str(), p0.c_str(),
                               cName.c_str());

        // d = b^2 - ac4
        builder->fsCodeAppendf("\tfloat %s = %s * %s - %s;\n", dName.c_str(),
                               bVar.c_str(), bVar.c_str(), ac4Name.c_str());

        // only proceed if discriminant is >= 0
        builder->fsCodeAppendf("\tif (%s >= 0.0) {\n", dName.c_str());

        // intermediate value we'll use to compute the roots
        // q = -0.5 * (b +/- sqrt(d))
        builder->fsCodeAppendf("\t\tfloat %s = -0.5 * (%s + (%s < 0.0 ? -1.0 : 1.0)"
                               " * sqrt(%s));\n", qName.c_str(), bVar.c_str(),
                               bVar.c_str(), dName.c_str());

        // compute both roots
        // r0 = q * params[1]
        builder->fsCodeAppendf("\t\tfloat %s = %s * %s;\n", r0Name.c_str(),
                               qName.c_str(), p1.c_str());
        // r1 = c / q
        builder->fsCodeAppendf("\t\tfloat %s = %s / %s;\n", r1Name.c_str(),
                               cName.c_str(), qName.c_str());

        // Note: If there are two roots that both generate radius(t) > 0, the
        // Canvas spec says to choose the larger t.

        // so we'll look at the larger one first:
        builder->fsCodeAppendf("\t\tfloat %s = max(%s, %s);\n", tName.c_str(),
                               r0Name.c_str(), r1Name.c_str());

        // if r(t) > 0, then we're done; t will be our x coordinate
        builder->fsCodeAppendf("\t\tif (%s * %s + %s > 0.0) {\n", tName.c_str(),
                               p5.c_str(), p3.c_str());

        builder->fsCodeAppend("\t\t");
        this->emitColor(builder, tName.c_str(), key, outputColor, inputColor, samplers);

        // otherwise, if r(t) for the larger root was <= 0, try the other root
        builder->fsCodeAppend("\t\t} else {\n");
        builder->fsCodeAppendf("\t\t\t%s = min(%s, %s);\n", tName.c_str(),
                               r0Name.c_str(), r1Name.c_str());

        // if r(t) > 0 for the smaller root, then t will be our x coordinate
        builder->fsCodeAppendf("\t\t\tif (%s * %s + %s > 0.0) {\n",
                               tName.c_str(), p5.c_str(), p3.c_str());

        builder->fsCodeAppend("\t\t\t");
        this->emitColor(builder, tName.c_str(), key, outputColor, inputColor, samplers);

        // end if (r(t) > 0) for smaller root
        builder->fsCodeAppend("\t\t\t}\n");
        // end if (r(t) > 0), else, for larger root
        builder->fsCodeAppend("\t\t}\n");
        // end if (discriminant >= 0)
        builder->fsCodeAppend("\t}\n");
    } else {

        // linear case: t = -c/b
        builder->fsCodeAppendf("\tfloat %s = -(%s / %s);\n", tName.c_str(),
                               cName.c_str(), bVar.c_str());

        // if r(t) > 0, then t will be the x coordinate
        builder->fsCodeAppendf("\tif (%s * %s + %s > 0.0) {\n", tName.c_str(),
                               p5.c_str(), p3.c_str());
        builder->fsCodeAppend("\t");
        this->emitColor(builder, tName.c_str(), key, outputColor, inputColor, samplers);
        builder->fsCodeAppend("\t}\n");
    }
}

void GLDefault2PtConicalEffect::setData(const GrGLUniformManager& uman,
                                   const GrDrawEffect& drawEffect) {
    INHERITED::setData(uman, drawEffect);
    const Default2PtConicalEffect& data = drawEffect.castEffect<Default2PtConicalEffect>();
    SkASSERT(data.isDegenerate() == fIsDegenerate);
    SkScalar centerX1 = data.center();
    SkScalar radius0 = data.radius();
    SkScalar diffRadius = data.diffRadius();

    if (fCachedCenter != centerX1 ||
        fCachedRadius != radius0 ||
        fCachedDiffRadius != diffRadius) {

        SkScalar a = SkScalarMul(centerX1, centerX1) - diffRadius * diffRadius;

        // When we're in the degenerate (linear) case, the second
        // value will be INF but the program doesn't read it. (We
        // use the same 6 uniforms even though we don't need them
        // all in the linear case just to keep the code complexity
        // down).
        float values[6] = {
            SkScalarToFloat(a * 4),
            1.f / (SkScalarToFloat(a)),
            SkScalarToFloat(centerX1),
            SkScalarToFloat(radius0),
            SkScalarToFloat(SkScalarMul(radius0, radius0)),
            SkScalarToFloat(diffRadius)
        };

        uman.set1fv(fParamUni, 6, values);
        fCachedCenter = centerX1;
        fCachedRadius = radius0;
        fCachedDiffRadius = diffRadius;
    }
}

GrGLEffect::EffectKey GLDefault2PtConicalEffect::GenKey(const GrDrawEffect& drawEffect,
                                                   const GrGLCaps&) {
    enum {
        kIsDegenerate = 1 << kBaseKeyBitCnt,
    };

    EffectKey key = GenBaseGradientKey(drawEffect);
    if (drawEffect.castEffect<Default2PtConicalEffect>().isDegenerate()) {
        key |= kIsDegenerate;
    }
    return key;
}

//////////////////////////////////////////////////////////////////////////////

GrEffectRef* Gr2PtConicalGradientEffect::Create(GrContext* ctx,
                                                const SkTwoPointConicalGradient& shader,
                                                SkShader::TileMode tm) {

    SkMatrix matrix;
    if (!shader.getLocalMatrix().invert(&matrix)) {
        return NULL;
    }

    set_matrix_default_conical(shader, &matrix);
    return Default2PtConicalEffect::Create(ctx, shader, matrix, tm);
}

#endif
