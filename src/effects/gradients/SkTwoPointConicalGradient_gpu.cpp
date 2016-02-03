
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTwoPointConicalGradient_gpu.h"

#include "SkTwoPointConicalGradient.h"

#if SK_SUPPORT_GPU
#include "GrCoordTransform.h"
#include "GrInvariantOutput.h"
#include "GrPaint.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
// For brevity
typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

static const SkScalar kErrorTol = 0.00001f;
static const SkScalar kEdgeErrorTol = 5.f * kErrorTol;

/**
 * We have three general cases for 2pt conical gradients. First we always assume that
 * the start radius <= end radius. Our first case (kInside_) is when the start circle
 * is completely enclosed by the end circle. The second case (kOutside_) is the case
 * when the start circle is either completely outside the end circle or the circles
 * overlap. The final case (kEdge_) is when the start circle is inside the end one,
 * but the two are just barely touching at 1 point along their edges.
 */
enum ConicalType {
    kInside_ConicalType,
    kOutside_ConicalType,
    kEdge_ConicalType,
};

//////////////////////////////////////////////////////////////////////////////

static void set_matrix_edge_conical(const SkTwoPointConicalGradient& shader,
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

class Edge2PtConicalEffect : public GrGradientEffect {
public:

    static GrFragmentProcessor* Create(GrContext* ctx,
                                       const SkTwoPointConicalGradient& shader,
                                       const SkMatrix& matrix,
                                       SkShader::TileMode tm) {
        return new Edge2PtConicalEffect(ctx, shader, matrix, tm);
    }

    virtual ~Edge2PtConicalEffect() {}

    const char* name() const override {
        return "Two-Point Conical Gradient Edge Touching";
    }

    // The radial gradient parameters can collapse to a linear (instead of quadratic) equation.
    SkScalar center() const { return fCenterX1; }
    SkScalar diffRadius() const { return fDiffRadius; }
    SkScalar radius() const { return fRadius0; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor& sBase) const override {
        const Edge2PtConicalEffect& s = sBase.cast<Edge2PtConicalEffect>();
        return (INHERITED::onIsEqual(sBase) &&
                this->fCenterX1 == s.fCenterX1 &&
                this->fRadius0 == s.fRadius0 &&
                this->fDiffRadius == s.fDiffRadius);
    }

    Edge2PtConicalEffect(GrContext* ctx,
                         const SkTwoPointConicalGradient& shader,
                         const SkMatrix& matrix,
                         SkShader::TileMode tm)
        : INHERITED(ctx, shader, matrix, tm),
        fCenterX1(shader.getCenterX1()),
        fRadius0(shader.getStartRadius()),
        fDiffRadius(shader.getDiffRadius()){
        this->initClassID<Edge2PtConicalEffect>();
        // We should only be calling this shader if we are degenerate case with touching circles
        // When deciding if we are in edge case, we scaled by the end radius for cases when the
        // start radius was close to zero, otherwise we scaled by the start radius.  In addition
        // Our test for the edge case in set_matrix_circle_conical has a higher tolerance so we
        // need the sqrt value below
        SkASSERT(SkScalarAbs(SkScalarAbs(fDiffRadius) - fCenterX1) <
                 (fRadius0 < kErrorTol ? shader.getEndRadius() * kEdgeErrorTol :
                                         fRadius0 * sqrt(kEdgeErrorTol)));

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

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

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

class GLEdge2PtConicalEffect : public GrGLGradientEffect {
public:
    GLEdge2PtConicalEffect(const GrProcessor&);
    virtual ~GLEdge2PtConicalEffect() { }

    virtual void emitCode(EmitArgs&) override;

    static void GenKey(const GrProcessor&, const GrGLSLCaps& caps, GrProcessorKeyBuilder* b);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

    UniformHandle fParamUni;

    const char* fVSVaryingName;
    const char* fFSVaryingName;

    // @{
    /// Values last uploaded as uniforms

    SkScalar fCachedRadius;
    SkScalar fCachedDiffRadius;

    // @}

private:
    typedef GrGLGradientEffect INHERITED;

};

void Edge2PtConicalEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                 GrProcessorKeyBuilder* b) const {
    GLEdge2PtConicalEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* Edge2PtConicalEffect::onCreateGLSLInstance() const {
    return new GLEdge2PtConicalEffect(*this);
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(Edge2PtConicalEffect);

/*
 * All Two point conical gradient test create functions may occasionally create edge case shaders
 */
const GrFragmentProcessor* Edge2PtConicalEffect::TestCreate(GrProcessorTestData* d) {
    SkPoint center1 = {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()};
    SkScalar radius1 = d->fRandom->nextUScalar1();
    SkPoint center2;
    SkScalar radius2;
    do {
        center2.set(d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1());
        // If the circles are identical the factory will give us an empty shader.
        // This will happen if we pick identical centers
    } while (center1 == center2);

    // Below makes sure that circle one is contained within circle two
    // and both circles are touching on an edge
    SkPoint diff = center2 - center1;
    SkScalar diffLen = diff.length();
    radius2 = radius1 + diffLen;

    SkColor colors[kMaxRandomGradientColors];
    SkScalar stopsArray[kMaxRandomGradientColors];
    SkScalar* stops = stopsArray;
    SkShader::TileMode tm;
    int colorCount = RandomGradientParams(d->fRandom, colors, &stops, &tm);
    SkAutoTUnref<SkShader> shader(SkGradientShader::CreateTwoPointConical(center1, radius1,
                                                                          center2, radius2,
                                                                          colors, stops, colorCount,
                                                                          tm));
    const GrFragmentProcessor* fp = shader->asFragmentProcessor(d->fContext,
        GrTest::TestMatrix(d->fRandom), NULL, kNone_SkFilterQuality);
    GrAlwaysAssert(fp);
    return fp;
}

GLEdge2PtConicalEffect::GLEdge2PtConicalEffect(const GrProcessor&)
    : fVSVaryingName(nullptr)
    , fFSVaryingName(nullptr)
    , fCachedRadius(-SK_ScalarMax)
    , fCachedDiffRadius(-SK_ScalarMax) {}

void GLEdge2PtConicalEffect::emitCode(EmitArgs& args) {
    const Edge2PtConicalEffect& ge = args.fFp.cast<Edge2PtConicalEffect>();
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    this->emitUniforms(uniformHandler, ge);
    fParamUni = uniformHandler->addUniformArray(GrGLSLUniformHandler::kFragment_Visibility,
                                                kFloat_GrSLType, kDefault_GrSLPrecision,
                                                "Conical2FSParams", 3);

    SkString cName("c");
    SkString tName("t");
    SkString p0; // start radius
    SkString p1; // start radius squared
    SkString p2; // difference in radii (r1 - r0)

    uniformHandler->getUniformVariable(fParamUni).appendArrayAccess(0, &p0);
    uniformHandler->getUniformVariable(fParamUni).appendArrayAccess(1, &p1);
    uniformHandler->getUniformVariable(fParamUni).appendArrayAccess(2, &p2);

    // We interpolate the linear component in coords[1].
    SkASSERT(args.fCoords[0].getType() == args.fCoords[1].getType());
    const char* coords2D;
    SkString bVar;
    GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
    if (kVec3f_GrSLType == args.fCoords[0].getType()) {
        fragBuilder->codeAppendf("\tvec3 interpolants = vec3(%s.xy / %s.z, %s.x / %s.z);\n",
                               args.fCoords[0].c_str(), args.fCoords[0].c_str(),
                               args.fCoords[1].c_str(), args.fCoords[1].c_str());
        coords2D = "interpolants.xy";
        bVar = "interpolants.z";
    } else {
        coords2D = args.fCoords[0].c_str();
        bVar.printf("%s.x", args.fCoords[1].c_str());
    }

    // output will default to transparent black (we simply won't write anything
    // else to it if invalid, instead of discarding or returning prematurely)
    fragBuilder->codeAppendf("\t%s = vec4(0.0,0.0,0.0,0.0);\n", args.fOutputColor);

    // c = (x^2)+(y^2) - params[1]
    fragBuilder->codeAppendf("\tfloat %s = dot(%s, %s) - %s;\n",
                           cName.c_str(), coords2D, coords2D, p1.c_str());

    // linear case: t = -c/b
    fragBuilder->codeAppendf("\tfloat %s = -(%s / %s);\n", tName.c_str(),
                           cName.c_str(), bVar.c_str());

    // if r(t) > 0, then t will be the x coordinate
    fragBuilder->codeAppendf("\tif (%s * %s + %s > 0.0) {\n", tName.c_str(),
                           p2.c_str(), p0.c_str());
    fragBuilder->codeAppend("\t");
    this->emitColor(fragBuilder,
                    uniformHandler,
                    args.fGLSLCaps,
                    ge,
                    tName.c_str(),
                    args.fOutputColor,
                    args.fInputColor,
                    args.fSamplers);
    fragBuilder->codeAppend("\t}\n");
}

void GLEdge2PtConicalEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                       const GrProcessor& processor) {
    INHERITED::onSetData(pdman, processor);
    const Edge2PtConicalEffect& data = processor.cast<Edge2PtConicalEffect>();
    SkScalar radius0 = data.radius();
    SkScalar diffRadius = data.diffRadius();

    if (fCachedRadius != radius0 ||
        fCachedDiffRadius != diffRadius) {

        float values[3] = {
            SkScalarToFloat(radius0),
            SkScalarToFloat(SkScalarMul(radius0, radius0)),
            SkScalarToFloat(diffRadius)
        };

        pdman.set1fv(fParamUni, 3, values);
        fCachedRadius = radius0;
        fCachedDiffRadius = diffRadius;
    }
}

void GLEdge2PtConicalEffect::GenKey(const GrProcessor& processor,
                                    const GrGLSLCaps&, GrProcessorKeyBuilder* b) {
    b->add32(GenBaseGradientKey(processor));
}

//////////////////////////////////////////////////////////////////////////////
// Focal Conical Gradients
//////////////////////////////////////////////////////////////////////////////

static ConicalType set_matrix_focal_conical(const SkTwoPointConicalGradient& shader,
                                            SkMatrix* invLMatrix, SkScalar* focalX) {
    // Inverse of the current local matrix is passed in then,
    // translate, scale, and rotate such that endCircle is unit circle on x-axis,
    // and focal point is at the origin.
    ConicalType conicalType;
    const SkPoint& focal = shader.getStartCenter();
    const SkPoint& centerEnd = shader.getEndCenter();
    SkScalar radius = shader.getEndRadius();
    SkScalar invRadius = 1.f / radius;

    SkMatrix matrix;

    matrix.setTranslate(-centerEnd.fX, -centerEnd.fY);
    matrix.postScale(invRadius, invRadius);

    SkPoint focalTrans;
    matrix.mapPoints(&focalTrans, &focal, 1);
    *focalX = focalTrans.length();

    if (0.f != *focalX) {
        SkScalar invFocalX = SkScalarInvert(*focalX);
        SkMatrix rot;
        rot.setSinCos(-SkScalarMul(invFocalX, focalTrans.fY),
                      SkScalarMul(invFocalX, focalTrans.fX));
        matrix.postConcat(rot);
    }

    matrix.postTranslate(-(*focalX), 0.f);

    // If the focal point is touching the edge of the circle it will
    // cause a degenerate case that must be handled separately
    // kEdgeErrorTol = 5 * kErrorTol was picked after manual testing the
    // stability trade off versus the linear approx used in the Edge Shader
    if (SkScalarAbs(1.f - (*focalX)) < kEdgeErrorTol) {
        return kEdge_ConicalType;
    }

    // Scale factor 1 / (1 - focalX * focalX)
    SkScalar oneMinusF2 = 1.f - SkScalarMul(*focalX, *focalX);
    SkScalar s = SkScalarInvert(oneMinusF2);


    if (s >= 0.f) {
        conicalType = kInside_ConicalType;
        matrix.postScale(s, s * SkScalarSqrt(oneMinusF2));
    } else {
        conicalType = kOutside_ConicalType;
        matrix.postScale(s, s);
    }

    invLMatrix->postConcat(matrix);

    return conicalType;
}

//////////////////////////////////////////////////////////////////////////////

class FocalOutside2PtConicalEffect : public GrGradientEffect {
public:

    static GrFragmentProcessor* Create(GrContext* ctx,
                                       const SkTwoPointConicalGradient& shader,
                                       const SkMatrix& matrix,
                                       SkShader::TileMode tm,
                                       SkScalar focalX) {
        return new FocalOutside2PtConicalEffect(ctx, shader, matrix, tm, focalX);
    }

    virtual ~FocalOutside2PtConicalEffect() { }

    const char* name() const override {
        return "Two-Point Conical Gradient Focal Outside";
    }

    bool isFlipped() const { return fIsFlipped; }
    SkScalar focal() const { return fFocalX; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor& sBase) const override {
        const FocalOutside2PtConicalEffect& s = sBase.cast<FocalOutside2PtConicalEffect>();
        return (INHERITED::onIsEqual(sBase) &&
                this->fFocalX == s.fFocalX &&
                this->fIsFlipped == s.fIsFlipped);
    }

    FocalOutside2PtConicalEffect(GrContext* ctx,
                                 const SkTwoPointConicalGradient& shader,
                                 const SkMatrix& matrix,
                                 SkShader::TileMode tm,
                                 SkScalar focalX)
    : INHERITED(ctx, shader, matrix, tm)
    , fFocalX(focalX)
    , fIsFlipped(shader.isFlippedGrad()) {
        this->initClassID<FocalOutside2PtConicalEffect>();
    }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    SkScalar         fFocalX;
    bool             fIsFlipped;

    typedef GrGradientEffect INHERITED;
};

class GLFocalOutside2PtConicalEffect : public GrGLGradientEffect {
public:
    GLFocalOutside2PtConicalEffect(const GrProcessor&);
    virtual ~GLFocalOutside2PtConicalEffect() { }

    virtual void emitCode(EmitArgs&) override;

    static void GenKey(const GrProcessor&, const GrGLSLCaps& caps, GrProcessorKeyBuilder* b);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

    UniformHandle fParamUni;

    const char* fVSVaryingName;
    const char* fFSVaryingName;

    bool fIsFlipped;

    // @{
    /// Values last uploaded as uniforms

    SkScalar fCachedFocal;

    // @}

private:
    typedef GrGLGradientEffect INHERITED;

};

void FocalOutside2PtConicalEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                         GrProcessorKeyBuilder* b) const {
    GLFocalOutside2PtConicalEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* FocalOutside2PtConicalEffect::onCreateGLSLInstance() const {
    return new GLFocalOutside2PtConicalEffect(*this);
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(FocalOutside2PtConicalEffect);

/*
 * All Two point conical gradient test create functions may occasionally create edge case shaders
 */
const GrFragmentProcessor* FocalOutside2PtConicalEffect::TestCreate(GrProcessorTestData* d) {
    SkPoint center1 = {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()};
    SkScalar radius1 = 0.f;
    SkPoint center2;
    SkScalar radius2;
    do {
        center2.set(d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1());
        // Need to make sure the centers are not the same or else focal point will be inside
    } while (center1 == center2);
        SkPoint diff = center2 - center1;
        SkScalar diffLen = diff.length();
        // Below makes sure that the focal point is not contained within circle two
        radius2 = d->fRandom->nextRangeF(0.f, diffLen);

    SkColor colors[kMaxRandomGradientColors];
    SkScalar stopsArray[kMaxRandomGradientColors];
    SkScalar* stops = stopsArray;
    SkShader::TileMode tm;
    int colorCount = RandomGradientParams(d->fRandom, colors, &stops, &tm);
    SkAutoTUnref<SkShader> shader(SkGradientShader::CreateTwoPointConical(center1, radius1,
                                                                          center2, radius2,
                                                                          colors, stops, colorCount,
                                                                          tm));
    const GrFragmentProcessor* fp = shader->asFragmentProcessor(d->fContext,
        GrTest::TestMatrix(d->fRandom), NULL, kNone_SkFilterQuality);
    GrAlwaysAssert(fp);
    return fp;
}

GLFocalOutside2PtConicalEffect::GLFocalOutside2PtConicalEffect(const GrProcessor& processor)
    : fVSVaryingName(nullptr)
    , fFSVaryingName(nullptr)
    , fCachedFocal(SK_ScalarMax) {
    const FocalOutside2PtConicalEffect& data = processor.cast<FocalOutside2PtConicalEffect>();
    fIsFlipped = data.isFlipped();
}

void GLFocalOutside2PtConicalEffect::emitCode(EmitArgs& args) {
    const FocalOutside2PtConicalEffect& ge = args.fFp.cast<FocalOutside2PtConicalEffect>();
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    this->emitUniforms(uniformHandler, ge);
    fParamUni = uniformHandler->addUniformArray(GrGLSLUniformHandler::kFragment_Visibility,
                                                kFloat_GrSLType, kDefault_GrSLPrecision,
                                                "Conical2FSParams", 2);
    SkString tName("t");
    SkString p0; // focalX
    SkString p1; // 1 - focalX * focalX

    uniformHandler->getUniformVariable(fParamUni).appendArrayAccess(0, &p0);
    uniformHandler->getUniformVariable(fParamUni).appendArrayAccess(1, &p1);

    // if we have a vec3 from being in perspective, convert it to a vec2 first
    GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2DString = fragBuilder->ensureFSCoords2D(args.fCoords, 0);
    const char* coords2D = coords2DString.c_str();

    // t = p.x * focal.x +/- sqrt(p.x^2 + (1 - focal.x^2) * p.y^2)

    // output will default to transparent black (we simply won't write anything
    // else to it if invalid, instead of discarding or returning prematurely)
    fragBuilder->codeAppendf("\t%s = vec4(0.0,0.0,0.0,0.0);\n", args.fOutputColor);

    fragBuilder->codeAppendf("\tfloat xs = %s.x * %s.x;\n", coords2D, coords2D);
    fragBuilder->codeAppendf("\tfloat ys = %s.y * %s.y;\n", coords2D, coords2D);
    fragBuilder->codeAppendf("\tfloat d = xs + %s * ys;\n", p1.c_str());

    // Must check to see if we flipped the circle order (to make sure start radius < end radius)
    // If so we must also flip sign on sqrt
    if (!fIsFlipped) {
        fragBuilder->codeAppendf("\tfloat %s = %s.x * %s  + sqrt(d);\n", tName.c_str(),
                                 coords2D, p0.c_str());
    } else {
        fragBuilder->codeAppendf("\tfloat %s = %s.x * %s  - sqrt(d);\n", tName.c_str(),
                                 coords2D, p0.c_str());
    }

    fragBuilder->codeAppendf("\tif (%s >= 0.0 && d >= 0.0) {\n", tName.c_str());
    fragBuilder->codeAppend("\t\t");
    this->emitColor(fragBuilder,
                    uniformHandler,
                    args.fGLSLCaps,
                    ge,
                    tName.c_str(),
                    args.fOutputColor,
                    args.fInputColor,
                    args.fSamplers);
    fragBuilder->codeAppend("\t}\n");
}

void GLFocalOutside2PtConicalEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                               const GrProcessor& processor) {
    INHERITED::onSetData(pdman, processor);
    const FocalOutside2PtConicalEffect& data = processor.cast<FocalOutside2PtConicalEffect>();
    SkASSERT(data.isFlipped() == fIsFlipped);
    SkScalar focal = data.focal();

    if (fCachedFocal != focal) {
        SkScalar oneMinus2F = 1.f - SkScalarMul(focal, focal);

        float values[2] = {
            SkScalarToFloat(focal),
            SkScalarToFloat(oneMinus2F),
        };

        pdman.set1fv(fParamUni, 2, values);
        fCachedFocal = focal;
    }
}

void GLFocalOutside2PtConicalEffect::GenKey(const GrProcessor& processor,
                                            const GrGLSLCaps&, GrProcessorKeyBuilder* b) {
    uint32_t* key = b->add32n(2);
    key[0] = GenBaseGradientKey(processor);
    key[1] = processor.cast<FocalOutside2PtConicalEffect>().isFlipped();
}

//////////////////////////////////////////////////////////////////////////////

class GLFocalInside2PtConicalEffect;

class FocalInside2PtConicalEffect : public GrGradientEffect {
public:

    static GrFragmentProcessor* Create(GrContext* ctx,
                                       const SkTwoPointConicalGradient& shader,
                                       const SkMatrix& matrix,
                                       SkShader::TileMode tm,
                                       SkScalar focalX) {
        return new FocalInside2PtConicalEffect(ctx, shader, matrix, tm, focalX);
    }

    virtual ~FocalInside2PtConicalEffect() {}

    const char* name() const override {
        return "Two-Point Conical Gradient Focal Inside";
    }

    SkScalar focal() const { return fFocalX; }

    typedef GLFocalInside2PtConicalEffect GLSLProcessor;

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor& sBase) const override {
        const FocalInside2PtConicalEffect& s = sBase.cast<FocalInside2PtConicalEffect>();
        return (INHERITED::onIsEqual(sBase) &&
                this->fFocalX == s.fFocalX);
    }

    FocalInside2PtConicalEffect(GrContext* ctx,
                                const SkTwoPointConicalGradient& shader,
                                const SkMatrix& matrix,
                                SkShader::TileMode tm,
                                SkScalar focalX)
        : INHERITED(ctx, shader, matrix, tm), fFocalX(focalX) {
        this->initClassID<FocalInside2PtConicalEffect>();
    }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    SkScalar         fFocalX;

    typedef GrGradientEffect INHERITED;
};

class GLFocalInside2PtConicalEffect : public GrGLGradientEffect {
public:
    GLFocalInside2PtConicalEffect(const GrProcessor&);
    virtual ~GLFocalInside2PtConicalEffect() {}

    virtual void emitCode(EmitArgs&) override;

    static void GenKey(const GrProcessor&, const GrGLSLCaps& caps, GrProcessorKeyBuilder* b);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

    UniformHandle fFocalUni;

    const char* fVSVaryingName;
    const char* fFSVaryingName;

    // @{
    /// Values last uploaded as uniforms

    SkScalar fCachedFocal;

    // @}

private:
    typedef GrGLGradientEffect INHERITED;

};

void FocalInside2PtConicalEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                        GrProcessorKeyBuilder* b) const {
    GLFocalInside2PtConicalEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* FocalInside2PtConicalEffect::onCreateGLSLInstance() const {
    return new GLFocalInside2PtConicalEffect(*this);
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(FocalInside2PtConicalEffect);

/*
 * All Two point conical gradient test create functions may occasionally create edge case shaders
 */
const GrFragmentProcessor* FocalInside2PtConicalEffect::TestCreate(GrProcessorTestData* d) {
    SkPoint center1 = {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()};
    SkScalar radius1 = 0.f;
    SkPoint center2;
    SkScalar radius2;
    do {
        center2.set(d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1());
        // Below makes sure radius2 is larger enouch such that the focal point
        // is inside the end circle
        SkScalar increase = d->fRandom->nextUScalar1();
        SkPoint diff = center2 - center1;
        SkScalar diffLen = diff.length();
        radius2 = diffLen + increase;
        // If the circles are identical the factory will give us an empty shader.
    } while (radius1 == radius2 && center1 == center2);

    SkColor colors[kMaxRandomGradientColors];
    SkScalar stopsArray[kMaxRandomGradientColors];
    SkScalar* stops = stopsArray;
    SkShader::TileMode tm;
    int colorCount = RandomGradientParams(d->fRandom, colors, &stops, &tm);
    SkAutoTUnref<SkShader> shader(SkGradientShader::CreateTwoPointConical(center1, radius1,
                                                                          center2, radius2,
                                                                          colors, stops, colorCount,
                                                                          tm));
    const GrFragmentProcessor* fp = shader->asFragmentProcessor(d->fContext,
        GrTest::TestMatrix(d->fRandom), NULL, kNone_SkFilterQuality);
    GrAlwaysAssert(fp);
    return fp;
}

GLFocalInside2PtConicalEffect::GLFocalInside2PtConicalEffect(const GrProcessor&)
    : fVSVaryingName(nullptr)
    , fFSVaryingName(nullptr)
    , fCachedFocal(SK_ScalarMax) {}

void GLFocalInside2PtConicalEffect::emitCode(EmitArgs& args) {
    const FocalInside2PtConicalEffect& ge = args.fFp.cast<FocalInside2PtConicalEffect>();
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    this->emitUniforms(uniformHandler, ge);
    fFocalUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                           kFloat_GrSLType, kDefault_GrSLPrecision,
                                           "Conical2FSParams");
    SkString tName("t");

    // this is the distance along x-axis from the end center to focal point in
    // transformed coordinates
    GrGLSLShaderVar focal = uniformHandler->getUniformVariable(fFocalUni);

    // if we have a vec3 from being in perspective, convert it to a vec2 first
    GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2DString = fragBuilder->ensureFSCoords2D(args.fCoords, 0);
    const char* coords2D = coords2DString.c_str();

    // t = p.x * focalX + length(p)
    fragBuilder->codeAppendf("\tfloat %s = %s.x * %s  + length(%s);\n", tName.c_str(),
                             coords2D, focal.c_str(), coords2D);

    this->emitColor(fragBuilder,
                    uniformHandler,
                    args.fGLSLCaps,
                    ge,
                    tName.c_str(),
                    args.fOutputColor,
                    args.fInputColor,
                    args.fSamplers);
}

void GLFocalInside2PtConicalEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                              const GrProcessor& processor) {
    INHERITED::onSetData(pdman, processor);
    const FocalInside2PtConicalEffect& data = processor.cast<FocalInside2PtConicalEffect>();
    SkScalar focal = data.focal();

    if (fCachedFocal != focal) {
        pdman.set1f(fFocalUni, SkScalarToFloat(focal));
        fCachedFocal = focal;
    }
}

void GLFocalInside2PtConicalEffect::GenKey(const GrProcessor& processor,
                                           const GrGLSLCaps&, GrProcessorKeyBuilder* b) {
    b->add32(GenBaseGradientKey(processor));
}

//////////////////////////////////////////////////////////////////////////////
// Circle Conical Gradients
//////////////////////////////////////////////////////////////////////////////

struct CircleConicalInfo {
    SkPoint fCenterEnd;
    SkScalar fA;
    SkScalar fB;
    SkScalar fC;
};

// Returns focal distance along x-axis in transformed coords
static ConicalType set_matrix_circle_conical(const SkTwoPointConicalGradient& shader,
                                             SkMatrix* invLMatrix, CircleConicalInfo* info) {
    // Inverse of the current local matrix is passed in then,
    // translate and scale such that start circle is on the origin and has radius 1
    const SkPoint& centerStart = shader.getStartCenter();
    const SkPoint& centerEnd = shader.getEndCenter();
    SkScalar radiusStart = shader.getStartRadius();
    SkScalar radiusEnd = shader.getEndRadius();

    SkMatrix matrix;

    matrix.setTranslate(-centerStart.fX, -centerStart.fY);

    SkScalar invStartRad = 1.f / radiusStart;
    matrix.postScale(invStartRad, invStartRad);

    radiusEnd /= radiusStart;

    SkPoint centerEndTrans;
    matrix.mapPoints(&centerEndTrans, &centerEnd, 1);

    SkScalar A = centerEndTrans.fX * centerEndTrans.fX + centerEndTrans.fY * centerEndTrans.fY
                 - radiusEnd * radiusEnd + 2 * radiusEnd - 1;

    // Check to see if start circle is inside end circle with edges touching.
    // If touching we return that it is of kEdge_ConicalType, and leave the matrix setting
    // to the edge shader. kEdgeErrorTol = 5 * kErrorTol was picked after manual testing
    // so that C = 1 / A is stable, and the linear approximation used in the Edge shader is
    // still accurate.
    if (SkScalarAbs(A) < kEdgeErrorTol) {
        return kEdge_ConicalType;
    }

    SkScalar C = 1.f / A;
    SkScalar B = (radiusEnd - 1.f) * C;

    matrix.postScale(C, C);

    invLMatrix->postConcat(matrix);

    info->fCenterEnd = centerEndTrans;
    info->fA = A;
    info->fB = B;
    info->fC = C;

    // if A ends up being negative, the start circle is contained completely inside the end cirlce
    if (A < 0.f) {
        return kInside_ConicalType;
    }
    return kOutside_ConicalType;
}

class CircleInside2PtConicalEffect : public GrGradientEffect {
public:

    static GrFragmentProcessor* Create(GrContext* ctx,
                                       const SkTwoPointConicalGradient& shader,
                                       const SkMatrix& matrix,
                                       SkShader::TileMode tm,
                                       const CircleConicalInfo& info) {
        return new CircleInside2PtConicalEffect(ctx, shader, matrix, tm, info);
    }

    virtual ~CircleInside2PtConicalEffect() {}

    const char* name() const override { return "Two-Point Conical Gradient Inside"; }

    SkScalar centerX() const { return fInfo.fCenterEnd.fX; }
    SkScalar centerY() const { return fInfo.fCenterEnd.fY; }
    SkScalar A() const { return fInfo.fA; }
    SkScalar B() const { return fInfo.fB; }
    SkScalar C() const { return fInfo.fC; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    virtual void onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                       GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor& sBase) const override {
        const CircleInside2PtConicalEffect& s = sBase.cast<CircleInside2PtConicalEffect>();
        return (INHERITED::onIsEqual(sBase) &&
                this->fInfo.fCenterEnd == s.fInfo.fCenterEnd &&
                this->fInfo.fA == s.fInfo.fA &&
                this->fInfo.fB == s.fInfo.fB &&
                this->fInfo.fC == s.fInfo.fC);
    }

    CircleInside2PtConicalEffect(GrContext* ctx,
                                 const SkTwoPointConicalGradient& shader,
                                 const SkMatrix& matrix,
                                 SkShader::TileMode tm,
                                 const CircleConicalInfo& info)
        : INHERITED(ctx, shader, matrix, tm), fInfo(info) {
        this->initClassID<CircleInside2PtConicalEffect>();
    }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    const CircleConicalInfo fInfo;

    typedef GrGradientEffect INHERITED;
};

class GLCircleInside2PtConicalEffect : public GrGLGradientEffect {
public:
    GLCircleInside2PtConicalEffect(const GrProcessor&);
    virtual ~GLCircleInside2PtConicalEffect() {}

    virtual void emitCode(EmitArgs&) override;

    static void GenKey(const GrProcessor&, const GrGLSLCaps& caps, GrProcessorKeyBuilder* b);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

    UniformHandle fCenterUni;
    UniformHandle fParamUni;

    const char* fVSVaryingName;
    const char* fFSVaryingName;

    // @{
    /// Values last uploaded as uniforms

    SkScalar fCachedCenterX;
    SkScalar fCachedCenterY;
    SkScalar fCachedA;
    SkScalar fCachedB;
    SkScalar fCachedC;

    // @}

private:
    typedef GrGLGradientEffect INHERITED;

};

void CircleInside2PtConicalEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                         GrProcessorKeyBuilder* b) const {
    GLCircleInside2PtConicalEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* CircleInside2PtConicalEffect::onCreateGLSLInstance() const {
    return new GLCircleInside2PtConicalEffect(*this);
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(CircleInside2PtConicalEffect);

/*
 * All Two point conical gradient test create functions may occasionally create edge case shaders
 */
const GrFragmentProcessor* CircleInside2PtConicalEffect::TestCreate(GrProcessorTestData* d) {
    SkPoint center1 = {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()};
    SkScalar radius1 = d->fRandom->nextUScalar1() + 0.0001f; // make sure radius1 != 0
    SkPoint center2;
    SkScalar radius2;
    do {
        center2.set(d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1());
        // Below makes sure that circle one is contained within circle two
        SkScalar increase = d->fRandom->nextUScalar1();
        SkPoint diff = center2 - center1;
        SkScalar diffLen = diff.length();
        radius2 = radius1 + diffLen + increase;
        // If the circles are identical the factory will give us an empty shader.
    } while (radius1 == radius2 && center1 == center2);

    SkColor colors[kMaxRandomGradientColors];
    SkScalar stopsArray[kMaxRandomGradientColors];
    SkScalar* stops = stopsArray;
    SkShader::TileMode tm;
    int colorCount = RandomGradientParams(d->fRandom, colors, &stops, &tm);
    SkAutoTUnref<SkShader> shader(SkGradientShader::CreateTwoPointConical(center1, radius1,
                                                                          center2, radius2,
                                                                          colors, stops, colorCount,
                                                                          tm));
    const GrFragmentProcessor* fp = shader->asFragmentProcessor(d->fContext,
        GrTest::TestMatrix(d->fRandom), NULL, kNone_SkFilterQuality);
    GrAlwaysAssert(fp);
    return fp;
}

GLCircleInside2PtConicalEffect::GLCircleInside2PtConicalEffect(const GrProcessor& processor)
    : fVSVaryingName(nullptr)
    , fFSVaryingName(nullptr)
    , fCachedCenterX(SK_ScalarMax)
    , fCachedCenterY(SK_ScalarMax)
    , fCachedA(SK_ScalarMax)
    , fCachedB(SK_ScalarMax)
    , fCachedC(SK_ScalarMax) {}

void GLCircleInside2PtConicalEffect::emitCode(EmitArgs& args) {
    const CircleInside2PtConicalEffect& ge = args.fFp.cast<CircleInside2PtConicalEffect>();
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    this->emitUniforms(uniformHandler, ge);
    fCenterUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                            kVec2f_GrSLType, kDefault_GrSLPrecision,
                                            "Conical2FSCenter");
    fParamUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                           kVec3f_GrSLType, kDefault_GrSLPrecision,
                                           "Conical2FSParams");
    SkString tName("t");

    GrGLSLShaderVar center = uniformHandler->getUniformVariable(fCenterUni);
    // params.x = A
    // params.y = B
    // params.z = C
    GrGLSLShaderVar params = uniformHandler->getUniformVariable(fParamUni);

    // if we have a vec3 from being in perspective, convert it to a vec2 first
    GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2DString = fragBuilder->ensureFSCoords2D(args.fCoords, 0);
    const char* coords2D = coords2DString.c_str();

    // p = coords2D
    // e = center end
    // r = radius end
    // A = dot(e, e) - r^2 + 2 * r - 1
    // B = (r -1) / A
    // C = 1 / A
    // d = dot(e, p) + B
    // t = d +/- sqrt(d^2 - A * dot(p, p) + C)
    fragBuilder->codeAppendf("\tfloat pDotp = dot(%s,  %s);\n", coords2D, coords2D);
    fragBuilder->codeAppendf("\tfloat d = dot(%s,  %s) + %s.y;\n", coords2D, center.c_str(),
                             params.c_str());
    fragBuilder->codeAppendf("\tfloat %s = d + sqrt(d * d - %s.x * pDotp + %s.z);\n",
                             tName.c_str(), params.c_str(), params.c_str());

    this->emitColor(fragBuilder,
                    uniformHandler,
                    args.fGLSLCaps,
                    ge,
                    tName.c_str(),
                    args.fOutputColor,
                    args.fInputColor,
                    args.fSamplers);
}

void GLCircleInside2PtConicalEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                               const GrProcessor& processor) {
    INHERITED::onSetData(pdman, processor);
    const CircleInside2PtConicalEffect& data = processor.cast<CircleInside2PtConicalEffect>();
    SkScalar centerX = data.centerX();
    SkScalar centerY = data.centerY();
    SkScalar A = data.A();
    SkScalar B = data.B();
    SkScalar C = data.C();

    if (fCachedCenterX != centerX || fCachedCenterY != centerY ||
        fCachedA != A || fCachedB != B || fCachedC != C) {

        pdman.set2f(fCenterUni, SkScalarToFloat(centerX), SkScalarToFloat(centerY));
        pdman.set3f(fParamUni, SkScalarToFloat(A), SkScalarToFloat(B), SkScalarToFloat(C));

        fCachedCenterX = centerX;
        fCachedCenterY = centerY;
        fCachedA = A;
        fCachedB = B;
        fCachedC = C;
    }
}

void GLCircleInside2PtConicalEffect::GenKey(const GrProcessor& processor,
                                            const GrGLSLCaps&, GrProcessorKeyBuilder* b) {
    b->add32(GenBaseGradientKey(processor));
}

//////////////////////////////////////////////////////////////////////////////

class CircleOutside2PtConicalEffect : public GrGradientEffect {
public:

    static GrFragmentProcessor* Create(GrContext* ctx,
                                       const SkTwoPointConicalGradient& shader,
                                       const SkMatrix& matrix,
                                       SkShader::TileMode tm,
                                       const CircleConicalInfo& info) {
        return new CircleOutside2PtConicalEffect(ctx, shader, matrix, tm, info);
    }

    virtual ~CircleOutside2PtConicalEffect() {}

    const char* name() const override { return "Two-Point Conical Gradient Outside"; }

    SkScalar centerX() const { return fInfo.fCenterEnd.fX; }
    SkScalar centerY() const { return fInfo.fCenterEnd.fY; }
    SkScalar A() const { return fInfo.fA; }
    SkScalar B() const { return fInfo.fB; }
    SkScalar C() const { return fInfo.fC; }
    SkScalar tLimit() const { return fTLimit; }
    bool isFlipped() const { return fIsFlipped; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor& sBase) const override {
        const CircleOutside2PtConicalEffect& s = sBase.cast<CircleOutside2PtConicalEffect>();
        return (INHERITED::onIsEqual(sBase) &&
                this->fInfo.fCenterEnd == s.fInfo.fCenterEnd &&
                this->fInfo.fA == s.fInfo.fA &&
                this->fInfo.fB == s.fInfo.fB &&
                this->fInfo.fC == s.fInfo.fC &&
                this->fTLimit == s.fTLimit &&
                this->fIsFlipped == s.fIsFlipped);
    }

    CircleOutside2PtConicalEffect(GrContext* ctx,
                                  const SkTwoPointConicalGradient& shader,
                                  const SkMatrix& matrix,
                                  SkShader::TileMode tm,
                                  const CircleConicalInfo& info)
        : INHERITED(ctx, shader, matrix, tm), fInfo(info) {
        this->initClassID<CircleOutside2PtConicalEffect>();
        if (shader.getStartRadius() != shader.getEndRadius()) {
            fTLimit = shader.getStartRadius() / (shader.getStartRadius() - shader.getEndRadius());
        } else {
            fTLimit = SK_ScalarMin;
        }

        fIsFlipped = shader.isFlippedGrad();
    }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    const CircleConicalInfo fInfo;
    SkScalar fTLimit;
    bool fIsFlipped;

    typedef GrGradientEffect INHERITED;
};

class GLCircleOutside2PtConicalEffect : public GrGLGradientEffect {
public:
    GLCircleOutside2PtConicalEffect(const GrProcessor&);
    virtual ~GLCircleOutside2PtConicalEffect() {}

    virtual void emitCode(EmitArgs&) override;

    static void GenKey(const GrProcessor&, const GrGLSLCaps& caps, GrProcessorKeyBuilder* b);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

    UniformHandle fCenterUni;
    UniformHandle fParamUni;

    const char* fVSVaryingName;
    const char* fFSVaryingName;

    bool fIsFlipped;

    // @{
    /// Values last uploaded as uniforms

    SkScalar fCachedCenterX;
    SkScalar fCachedCenterY;
    SkScalar fCachedA;
    SkScalar fCachedB;
    SkScalar fCachedC;
    SkScalar fCachedTLimit;

    // @}

private:
    typedef GrGLGradientEffect INHERITED;

};

void CircleOutside2PtConicalEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                          GrProcessorKeyBuilder* b) const {
    GLCircleOutside2PtConicalEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* CircleOutside2PtConicalEffect::onCreateGLSLInstance() const {
    return new GLCircleOutside2PtConicalEffect(*this);
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(CircleOutside2PtConicalEffect);

/*
 * All Two point conical gradient test create functions may occasionally create edge case shaders
 */
const GrFragmentProcessor* CircleOutside2PtConicalEffect::TestCreate(GrProcessorTestData* d) {
    SkPoint center1 = {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()};
    SkScalar radius1 = d->fRandom->nextUScalar1() + 0.0001f; // make sure radius1 != 0
    SkPoint center2;
    SkScalar radius2;
    SkScalar diffLen;
    do {
        center2.set(d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1());
        // If the circles share a center than we can't be in the outside case
    } while (center1 == center2);
    SkPoint diff = center2 - center1;
    diffLen = diff.length();
    // Below makes sure that circle one is not contained within circle two
    // and have radius2 >= radius to match sorting on cpu side
    radius2 = radius1 + d->fRandom->nextRangeF(0.f, diffLen);

    SkColor colors[kMaxRandomGradientColors];
    SkScalar stopsArray[kMaxRandomGradientColors];
    SkScalar* stops = stopsArray;
    SkShader::TileMode tm;
    int colorCount = RandomGradientParams(d->fRandom, colors, &stops, &tm);
    SkAutoTUnref<SkShader> shader(SkGradientShader::CreateTwoPointConical(center1, radius1,
                                                                          center2, radius2,
                                                                          colors, stops, colorCount,
                                                                          tm));
    const GrFragmentProcessor* fp = shader->asFragmentProcessor(
        d->fContext,GrTest::TestMatrix(d->fRandom), NULL, kNone_SkFilterQuality);
    GrAlwaysAssert(fp);
    return fp;
}

GLCircleOutside2PtConicalEffect::GLCircleOutside2PtConicalEffect(const GrProcessor& processor)
    : fVSVaryingName(nullptr)
    , fFSVaryingName(nullptr)
    , fCachedCenterX(SK_ScalarMax)
    , fCachedCenterY(SK_ScalarMax)
    , fCachedA(SK_ScalarMax)
    , fCachedB(SK_ScalarMax)
    , fCachedC(SK_ScalarMax)
    , fCachedTLimit(SK_ScalarMax) {
    const CircleOutside2PtConicalEffect& data = processor.cast<CircleOutside2PtConicalEffect>();
    fIsFlipped = data.isFlipped();
    }

void GLCircleOutside2PtConicalEffect::emitCode(EmitArgs& args) {
    const CircleOutside2PtConicalEffect& ge = args.fFp.cast<CircleOutside2PtConicalEffect>();
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    this->emitUniforms(uniformHandler, ge);
    fCenterUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                            kVec2f_GrSLType, kDefault_GrSLPrecision,
                                            "Conical2FSCenter");
    fParamUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                           kVec4f_GrSLType, kDefault_GrSLPrecision,
                                           "Conical2FSParams");
    SkString tName("t");

    GrGLSLShaderVar center = uniformHandler->getUniformVariable(fCenterUni);
    // params.x = A
    // params.y = B
    // params.z = C
    GrGLSLShaderVar params = uniformHandler->getUniformVariable(fParamUni);

    // if we have a vec3 from being in perspective, convert it to a vec2 first
    GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2DString = fragBuilder->ensureFSCoords2D(args.fCoords, 0);
    const char* coords2D = coords2DString.c_str();

    // output will default to transparent black (we simply won't write anything
    // else to it if invalid, instead of discarding or returning prematurely)
    fragBuilder->codeAppendf("\t%s = vec4(0.0,0.0,0.0,0.0);\n", args.fOutputColor);

    // p = coords2D
    // e = center end
    // r = radius end
    // A = dot(e, e) - r^2 + 2 * r - 1
    // B = (r -1) / A
    // C = 1 / A
    // d = dot(e, p) + B
    // t = d +/- sqrt(d^2 - A * dot(p, p) + C)

    fragBuilder->codeAppendf("\tfloat pDotp = dot(%s,  %s);\n", coords2D, coords2D);
    fragBuilder->codeAppendf("\tfloat d = dot(%s,  %s) + %s.y;\n", coords2D, center.c_str(),
                             params.c_str());
    fragBuilder->codeAppendf("\tfloat deter = d * d - %s.x * pDotp + %s.z;\n", params.c_str(),
                             params.c_str());

    // Must check to see if we flipped the circle order (to make sure start radius < end radius)
    // If so we must also flip sign on sqrt
    if (!fIsFlipped) {
        fragBuilder->codeAppendf("\tfloat %s = d + sqrt(deter);\n", tName.c_str());
    } else {
        fragBuilder->codeAppendf("\tfloat %s = d - sqrt(deter);\n", tName.c_str());
    }

    fragBuilder->codeAppendf("\tif (%s >= %s.w && deter >= 0.0) {\n",
                             tName.c_str(), params.c_str());
    fragBuilder->codeAppend("\t\t");
    this->emitColor(fragBuilder,
                    uniformHandler,
                    args.fGLSLCaps,
                    ge,
                    tName.c_str(),
                    args.fOutputColor,
                    args.fInputColor,
                    args.fSamplers);
    fragBuilder->codeAppend("\t}\n");
}

void GLCircleOutside2PtConicalEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                                const GrProcessor& processor) {
    INHERITED::onSetData(pdman, processor);
    const CircleOutside2PtConicalEffect& data = processor.cast<CircleOutside2PtConicalEffect>();
    SkASSERT(data.isFlipped() == fIsFlipped);
    SkScalar centerX = data.centerX();
    SkScalar centerY = data.centerY();
    SkScalar A = data.A();
    SkScalar B = data.B();
    SkScalar C = data.C();
    SkScalar tLimit = data.tLimit();

    if (fCachedCenterX != centerX || fCachedCenterY != centerY ||
        fCachedA != A || fCachedB != B || fCachedC != C || fCachedTLimit != tLimit) {

        pdman.set2f(fCenterUni, SkScalarToFloat(centerX), SkScalarToFloat(centerY));
        pdman.set4f(fParamUni, SkScalarToFloat(A), SkScalarToFloat(B), SkScalarToFloat(C),
                   SkScalarToFloat(tLimit));

        fCachedCenterX = centerX;
        fCachedCenterY = centerY;
        fCachedA = A;
        fCachedB = B;
        fCachedC = C;
        fCachedTLimit = tLimit;
    }
}

void GLCircleOutside2PtConicalEffect::GenKey(const GrProcessor& processor,
                                             const GrGLSLCaps&, GrProcessorKeyBuilder* b) {
    uint32_t* key = b->add32n(2);
    key[0] = GenBaseGradientKey(processor);
    key[1] = processor.cast<CircleOutside2PtConicalEffect>().isFlipped();
}

//////////////////////////////////////////////////////////////////////////////

GrFragmentProcessor* Gr2PtConicalGradientEffect::Create(GrContext* ctx,
                                                        const SkTwoPointConicalGradient& shader,
                                                        SkShader::TileMode tm,
                                                        const SkMatrix* localMatrix) {
    SkMatrix matrix;
    if (!shader.getLocalMatrix().invert(&matrix)) {
        return nullptr;
    }
    if (localMatrix) {
        SkMatrix inv;
        if (!localMatrix->invert(&inv)) {
            return nullptr;
        }
        matrix.postConcat(inv);
    }

    if (shader.getStartRadius() < kErrorTol) {
        SkScalar focalX;
        ConicalType type = set_matrix_focal_conical(shader, &matrix, &focalX);
        if (type == kInside_ConicalType) {
            return FocalInside2PtConicalEffect::Create(ctx, shader, matrix, tm, focalX);
        } else if(type == kEdge_ConicalType) {
            set_matrix_edge_conical(shader, &matrix);
            return Edge2PtConicalEffect::Create(ctx, shader, matrix, tm);
        } else {
            return FocalOutside2PtConicalEffect::Create(ctx, shader, matrix, tm, focalX);
        }
    }

    CircleConicalInfo info;
    ConicalType type = set_matrix_circle_conical(shader, &matrix, &info);

    if (type == kInside_ConicalType) {
        return CircleInside2PtConicalEffect::Create(ctx, shader, matrix, tm, info);
    } else if (type == kEdge_ConicalType) {
        set_matrix_edge_conical(shader, &matrix);
        return Edge2PtConicalEffect::Create(ctx, shader, matrix, tm);
    } else {
        return CircleOutside2PtConicalEffect::Create(ctx, shader, matrix, tm, info);
    }
}

#endif
