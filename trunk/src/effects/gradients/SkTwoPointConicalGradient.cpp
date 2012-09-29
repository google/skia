
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTwoPointConicalGradient.h"

static int valid_divide(float numer, float denom, float* ratio) {
    SkASSERT(ratio);
    if (0 == denom) {
        return 0;
    }
    *ratio = numer / denom;
    return 1;
}

// Return the number of distinct real roots, and write them into roots[] in
// ascending order
static int find_quad_roots(float A, float B, float C, float roots[2]) {
    SkASSERT(roots);

    if (A == 0) {
        return valid_divide(-C, B, roots);
    }

    float R = B*B - 4*A*C;
    if (R < 0) {
        return 0;
    }
    R = sk_float_sqrt(R);

#if 1
    float Q = B;
    if (Q < 0) {
        Q -= R;
    } else {
        Q += R;
    }
#else
    // on 10.6 this was much slower than the above branch :(
    float Q = B + copysignf(R, B);
#endif
    Q *= -0.5f;
    if (0 == Q) {
        roots[0] = 0;
        return 1;
    }

    float r0 = Q / A;
    float r1 = C / Q;
    roots[0] = r0 < r1 ? r0 : r1;
    roots[1] = r0 > r1 ? r0 : r1;
    return 2;
}

static float lerp(float x, float dx, float t) {
    return x + t * dx;
}

static float sqr(float x) { return x * x; }

void TwoPtRadial::init(const SkPoint& center0, SkScalar rad0,
                       const SkPoint& center1, SkScalar rad1) {
    fCenterX = SkScalarToFloat(center0.fX);
    fCenterY = SkScalarToFloat(center0.fY);
    fDCenterX = SkScalarToFloat(center1.fX) - fCenterX;
    fDCenterY = SkScalarToFloat(center1.fY) - fCenterY;
    fRadius = SkScalarToFloat(rad0);
    fDRadius = SkScalarToFloat(rad1) - fRadius;

    fA = sqr(fDCenterX) + sqr(fDCenterY) - sqr(fDRadius);
    fRadius2 = sqr(fRadius);
    fRDR = fRadius * fDRadius;
}

void TwoPtRadial::setup(SkScalar fx, SkScalar fy, SkScalar dfx, SkScalar dfy) {
    fRelX = SkScalarToFloat(fx) - fCenterX;
    fRelY = SkScalarToFloat(fy) - fCenterY;
    fIncX = SkScalarToFloat(dfx);
    fIncY = SkScalarToFloat(dfy);
    fB = -2 * (fDCenterX * fRelX + fDCenterY * fRelY + fRDR);
    fDB = -2 * (fDCenterX * fIncX + fDCenterY * fIncY);
}

SkFixed TwoPtRadial::nextT() {
    float roots[2];

    float C = sqr(fRelX) + sqr(fRelY) - fRadius2;
    int countRoots = find_quad_roots(fA, fB, C, roots);

    fRelX += fIncX;
    fRelY += fIncY;
    fB += fDB;

    if (0 == countRoots) {
        return kDontDrawT;
    }

    // Prefer the bigger t value if both give a radius(t) > 0
    // find_quad_roots returns the values sorted, so we start with the last
    float t = roots[countRoots - 1];
    float r = lerp(fRadius, fDRadius, t);
    if (r <= 0) {
        t = roots[0];   // might be the same as roots[countRoots-1]
        r = lerp(fRadius, fDRadius, t);
        if (r <= 0) {
            return kDontDrawT;
        }
    }
    return SkFloatToFixed(t);
}

typedef void (*TwoPointRadialProc)(TwoPtRadial* rec, SkPMColor* dstC,
                                   const SkPMColor* cache, int count);

static void twopoint_clamp(TwoPtRadial* rec, SkPMColor* SK_RESTRICT dstC,
                           const SkPMColor* SK_RESTRICT cache, int count) {
    for (; count > 0; --count) {
        SkFixed t = rec->nextT();
        if (TwoPtRadial::DontDrawT(t)) {
            *dstC++ = 0;
        } else {
            SkFixed index = SkClampMax(t, 0xFFFF);
            SkASSERT(index <= 0xFFFF);
            *dstC++ = cache[index >> SkGradientShaderBase::kCache32Shift];
        }
    }
}

static void twopoint_repeat(TwoPtRadial* rec, SkPMColor* SK_RESTRICT dstC,
                            const SkPMColor* SK_RESTRICT cache, int count) {
    for (; count > 0; --count) {
        SkFixed t = rec->nextT();
        if (TwoPtRadial::DontDrawT(t)) {
            *dstC++ = 0;
        } else {
            SkFixed index = repeat_tileproc(t);
            SkASSERT(index <= 0xFFFF);
            *dstC++ = cache[index >> SkGradientShaderBase::kCache32Shift];
        }
    }
}

static void twopoint_mirror(TwoPtRadial* rec, SkPMColor* SK_RESTRICT dstC,
                            const SkPMColor* SK_RESTRICT cache, int count) {
    for (; count > 0; --count) {
        SkFixed t = rec->nextT();
        if (TwoPtRadial::DontDrawT(t)) {
            *dstC++ = 0;
        } else {
            SkFixed index = mirror_tileproc(t);
            SkASSERT(index <= 0xFFFF);
            *dstC++ = cache[index >> SkGradientShaderBase::kCache32Shift];
        }
    }
}

void SkTwoPointConicalGradient::init() {
    fRec.init(fCenter1, fRadius1, fCenter2, fRadius2);
    fPtsToUnit.reset();
}

/////////////////////////////////////////////////////////////////////

SkTwoPointConicalGradient::SkTwoPointConicalGradient(
    const SkPoint& start, SkScalar startRadius,
    const SkPoint& end, SkScalar endRadius,
    const SkColor colors[], const SkScalar pos[],
    int colorCount, SkShader::TileMode mode,
    SkUnitMapper* mapper)
    : SkGradientShaderBase(colors, pos, colorCount, mode, mapper),
    fCenter1(start),
    fCenter2(end),
    fRadius1(startRadius),
    fRadius2(endRadius) {
    // this is degenerate, and should be caught by our caller
    SkASSERT(fCenter1 != fCenter2 || fRadius1 != fRadius2);
    this->init();
}

void SkTwoPointConicalGradient::shadeSpan(int x, int y, SkPMColor* dstCParam,
                                          int count) {
    SkASSERT(count > 0);

    SkPMColor* SK_RESTRICT dstC = dstCParam;

    SkMatrix::MapXYProc dstProc = fDstToIndexProc;

    const SkPMColor* SK_RESTRICT cache = this->getCache32();

    TwoPointRadialProc shadeProc = twopoint_repeat;
    if (SkShader::kClamp_TileMode == fTileMode) {
        shadeProc = twopoint_clamp;
    } else if (SkShader::kMirror_TileMode == fTileMode) {
        shadeProc = twopoint_mirror;
    } else {
        SkASSERT(SkShader::kRepeat_TileMode == fTileMode);
    }

    if (fDstToIndexClass != kPerspective_MatrixClass) {
        SkPoint srcPt;
        dstProc(fDstToIndex, SkIntToScalar(x) + SK_ScalarHalf,
                SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
        SkScalar dx, fx = srcPt.fX;
        SkScalar dy, fy = srcPt.fY;

        if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
            SkFixed fixedX, fixedY;
            (void)fDstToIndex.fixedStepInX(SkIntToScalar(y), &fixedX, &fixedY);
            dx = SkFixedToScalar(fixedX);
            dy = SkFixedToScalar(fixedY);
        } else {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
            dx = fDstToIndex.getScaleX();
            dy = fDstToIndex.getSkewY();
        }

        fRec.setup(fx, fy, dx, dy);
        (*shadeProc)(&fRec, dstC, cache, count);
    } else {    // perspective case
        SkScalar dstX = SkIntToScalar(x);
        SkScalar dstY = SkIntToScalar(y);
        for (; count > 0; --count) {
            SkPoint srcPt;
            dstProc(fDstToIndex, dstX, dstY, &srcPt);
            dstX += SK_Scalar1;

            fRec.setup(srcPt.fX, srcPt.fY, 0, 0);
            (*shadeProc)(&fRec, dstC, cache, 1);
        }
    }
}

bool SkTwoPointConicalGradient::setContext(const SkBitmap& device,
                                           const SkPaint& paint,
                                           const SkMatrix& matrix) {
    if (!this->INHERITED::setContext(device, paint, matrix)) {
        return false;
    }

    // we don't have a span16 proc
    fFlags &= ~kHasSpan16_Flag;

    // in general, we might discard based on computed-radius, so clear
    // this flag (todo: sometimes we can detect that we never discard...)
    fFlags &= ~kOpaqueAlpha_Flag;

    return true;
}

SkShader::BitmapType SkTwoPointConicalGradient::asABitmap(
    SkBitmap* bitmap, SkMatrix* matrix, SkShader::TileMode* xy) const {
    SkPoint diff = fCenter2 - fCenter1;
    SkScalar diffLen = 0;

    if (bitmap) {
        this->getGradientTableBitmap(bitmap);
    }
    if (matrix) {
        diffLen = diff.length();
    }
    if (matrix) {
        if (diffLen) {
            SkScalar invDiffLen = SkScalarInvert(diffLen);
            // rotate to align circle centers with the x-axis
            matrix->setSinCos(-SkScalarMul(invDiffLen, diff.fY),
                              SkScalarMul(invDiffLen, diff.fX));
        } else {
            matrix->reset();
        }
        matrix->preTranslate(-fCenter1.fX, -fCenter1.fY);
    }
    if (xy) {
        xy[0] = fTileMode;
        xy[1] = kClamp_TileMode;
    }
    return kTwoPointConical_BitmapType;
}

SkShader::GradientType SkTwoPointConicalGradient::asAGradient(
    GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fCenter1;
        info->fPoint[1] = fCenter2;
        info->fRadius[0] = fRadius1;
        info->fRadius[1] = fRadius2;
    }
    return kConical_GradientType;
}

SkTwoPointConicalGradient::SkTwoPointConicalGradient(
    SkFlattenableReadBuffer& buffer)
    : INHERITED(buffer),
    fCenter1(buffer.readPoint()),
    fCenter2(buffer.readPoint()),
    fRadius1(buffer.readScalar()),
    fRadius2(buffer.readScalar()) {
    this->init();
};

void SkTwoPointConicalGradient::flatten(
    SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fCenter1);
    buffer.writePoint(fCenter2);
    buffer.writeScalar(fRadius1);
    buffer.writeScalar(fRadius2);
}

/////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

// For brevity
typedef GrGLUniformManager::UniformHandle UniformHandle;
static const UniformHandle kInvalidUniformHandle = GrGLUniformManager::kInvalidUniformHandle;

class GrGLConical2Gradient : public GrGLGradientStage {
public:

    GrGLConical2Gradient(const GrProgramStageFactory& factory,
                         const GrCustomStage&);
    virtual ~GrGLConical2Gradient() { }

    virtual void setupVariables(GrGLShaderBuilder* builder) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* builder,
                        const char* vertexCoords) SK_OVERRIDE;
    virtual void emitFS(GrGLShaderBuilder* builder,
                        const char* outputColor,
                        const char* inputColor,
                        const TextureSamplerArray&) SK_OVERRIDE;
    virtual void setData(const GrGLUniformManager&,
                         const GrCustomStage&,
                         const GrRenderTarget*,
                         int stageNum) SK_OVERRIDE;

    static StageKey GenKey(const GrCustomStage& s, const GrGLCaps& caps);

protected:

    UniformHandle           fVSParamUni;
    UniformHandle           fFSParamUni;

    const char* fVSVaryingName;
    const char* fFSVaryingName;

    bool fIsDegenerate;

    // @{
    /// Values last uploaded as uniforms

    GrScalar fCachedCenter;
    GrScalar fCachedRadius;
    GrScalar fCachedDiffRadius;

    // @}

private:

    typedef GrGLGradientStage INHERITED;

};

/////////////////////////////////////////////////////////////////////

class GrConical2Gradient : public GrGradientEffect {
public:

    GrConical2Gradient(GrContext* ctx, const SkTwoPointConicalGradient& shader,
                       GrSamplerState* sampler)
        : INHERITED(ctx, shader, sampler)
        , fCenterX1(shader.getCenterX1())
        , fRadius0(shader.getStartRadius())
        , fDiffRadius(shader.getDiffRadius()) { }

    virtual ~GrConical2Gradient() { }

    static const char* Name() { return "Two-Point Conical Gradient"; }
    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE {
        return GrTProgramStageFactory<GrConical2Gradient>::getInstance();
    }
    virtual bool isEqual(const GrCustomStage& sBase) const SK_OVERRIDE {
        const GrConical2Gradient& s = static_cast<const GrConical2Gradient&>(sBase);
        return (INHERITED::isEqual(sBase) &&
                this->fCenterX1 == s.fCenterX1 &&
                this->fRadius0 == s.fRadius0 &&
                this->fDiffRadius == s.fDiffRadius);
    }

    // The radial gradient parameters can collapse to a linear (instead of quadratic) equation.
    bool isDegenerate() const { return SkScalarAbs(fDiffRadius) == SkScalarAbs(fCenterX1); }
    GrScalar center() const { return fCenterX1; }
    GrScalar diffRadius() const { return fDiffRadius; }
    GrScalar radius() const { return fRadius0; }

    typedef GrGLConical2Gradient GLProgramStage;

private:
    GR_DECLARE_CUSTOM_STAGE_TEST;

    // @{
    // Cache of values - these can change arbitrarily, EXCEPT
    // we shouldn't change between degenerate and non-degenerate?!

    GrScalar fCenterX1;
    GrScalar fRadius0;
    GrScalar fDiffRadius;

    // @}

    typedef GrGradientEffect INHERITED;
};

GR_DEFINE_CUSTOM_STAGE_TEST(GrConical2Gradient);

GrCustomStage* GrConical2Gradient::TestCreate(SkRandom* random,
                                              GrContext* context,
                                              GrTexture**) {
    SkPoint center1 = {random->nextUScalar1(), random->nextUScalar1()};
    SkScalar radius1 = random->nextUScalar1();
    SkPoint center2;
    SkScalar radius2;
    do {
        center1.set(random->nextUScalar1(), random->nextUScalar1());
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
    GrSamplerState sampler;
    GrCustomStage* stage = shader->asNewCustomStage(context, &sampler);
    GrAssert(NULL != stage);
    return stage;
}


/////////////////////////////////////////////////////////////////////

GrGLConical2Gradient::GrGLConical2Gradient(
        const GrProgramStageFactory& factory,
        const GrCustomStage& baseData)
    : INHERITED(factory)
    , fVSParamUni(kInvalidUniformHandle)
    , fFSParamUni(kInvalidUniformHandle)
    , fVSVaryingName(NULL)
    , fFSVaryingName(NULL)
    , fCachedCenter(GR_ScalarMax)
    , fCachedRadius(-GR_ScalarMax)
    , fCachedDiffRadius(-GR_ScalarMax) {

    const GrConical2Gradient& data =
        static_cast<const GrConical2Gradient&>(baseData);
    fIsDegenerate = data.isDegenerate();
}

void GrGLConical2Gradient::setupVariables(GrGLShaderBuilder* builder) {
    INHERITED::setupVariables(builder);
    // 2 copies of uniform array, 1 for each of vertex & fragment shader,
    // to work around Xoom bug. Doesn't seem to cause performance decrease
    // in test apps, but need to keep an eye on it.
    fVSParamUni = builder->addUniformArray(GrGLShaderBuilder::kVertex_ShaderType,
                                           kFloat_GrSLType, "Conical2VSParams", 6);
    fFSParamUni = builder->addUniformArray(GrGLShaderBuilder::kFragment_ShaderType,
                                           kFloat_GrSLType, "Conical2FSParams", 6);

    // For radial gradients without perspective we can pass the linear
    // part of the quadratic as a varying.
    if (!builder->defaultTextureMatrixIsPerspective()) {
        builder->addVarying(kFloat_GrSLType, "Conical2BCoeff",
                            &fVSVaryingName, &fFSVaryingName);
    }
}

void GrGLConical2Gradient::emitVS(GrGLShaderBuilder* builder,
                                  const char* vertexCoords) {
    SkString* code = &builder->fVSCode;
    SkString p2; // distance between centers
    SkString p3; // start radius
    SkString p5; // difference in radii (r1 - r0)
    builder->getUniformVariable(fVSParamUni).appendArrayAccess(2, &p2);
    builder->getUniformVariable(fVSParamUni).appendArrayAccess(3, &p3);
    builder->getUniformVariable(fVSParamUni).appendArrayAccess(5, &p5);

    // For radial gradients without perspective we can pass the linear
    // part of the quadratic as a varying.
    if (!builder->defaultTextureMatrixIsPerspective()) {
        // r2Var = -2 * (r2Parm[2] * varCoord.x - r2Param[3] * r2Param[5])
        code->appendf("\t%s = -2.0 * (%s * %s.x + %s * %s);\n",
                      fVSVaryingName, p2.c_str(),
                      vertexCoords, p3.c_str(), p5.c_str());
    }
}

void GrGLConical2Gradient::emitFS(GrGLShaderBuilder* builder,
                                  const char* outputColor,
                                  const char* inputColor,
                                  const TextureSamplerArray& samplers) {
    SkString* code = &builder->fFSCode;

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

    builder->getUniformVariable(fFSParamUni).appendArrayAccess(0, &p0);
    builder->getUniformVariable(fFSParamUni).appendArrayAccess(1, &p1);
    builder->getUniformVariable(fFSParamUni).appendArrayAccess(2, &p2);
    builder->getUniformVariable(fFSParamUni).appendArrayAccess(3, &p3);
    builder->getUniformVariable(fFSParamUni).appendArrayAccess(4, &p4);
    builder->getUniformVariable(fFSParamUni).appendArrayAccess(5, &p5);

    // If we we're able to interpolate the linear component,
    // bVar is the varying; otherwise compute it
    SkString bVar;
    if (!builder->defaultTextureMatrixIsPerspective()) {
        bVar = fFSVaryingName;
    } else {
        bVar = "b";
        code->appendf("\tfloat %s = -2.0 * (%s * %s.x + %s * %s);\n",
                      bVar.c_str(), p2.c_str(), builder->defaultTexCoordsName(),
                      p3.c_str(), p5.c_str());
    }

    // output will default to transparent black (we simply won't write anything
    // else to it if invalid, instead of discarding or returning prematurely)
    code->appendf("\t%s = vec4(0.0,0.0,0.0,0.0);\n", outputColor);

    // c = (x^2)+(y^2) - params[4]
    code->appendf("\tfloat %s = dot(%s, %s) - %s;\n", cName.c_str(),
                  builder->defaultTexCoordsName(), builder->defaultTexCoordsName(),
                  p4.c_str());

    // Non-degenerate case (quadratic)
    if (!fIsDegenerate) {

        // ac4 = params[0] * c
        code->appendf("\tfloat %s = %s * %s;\n", ac4Name.c_str(), p0.c_str(),
                      cName.c_str());

        // d = b^2 - ac4
        code->appendf("\tfloat %s = %s * %s - %s;\n", dName.c_str(),
                      bVar.c_str(), bVar.c_str(), ac4Name.c_str());

        // only proceed if discriminant is >= 0
        code->appendf("\tif (%s >= 0.0) {\n", dName.c_str());

        // intermediate value we'll use to compute the roots
        // q = -0.5 * (b +/- sqrt(d))
        code->appendf("\t\tfloat %s = -0.5 * (%s + (%s < 0.0 ? -1.0 : 1.0)"
                      " * sqrt(%s));\n", qName.c_str(), bVar.c_str(),
                      bVar.c_str(), dName.c_str());

        // compute both roots
        // r0 = q * params[1]
        code->appendf("\t\tfloat %s = %s * %s;\n", r0Name.c_str(),
                      qName.c_str(), p1.c_str());
        // r1 = c / q
        code->appendf("\t\tfloat %s = %s / %s;\n", r1Name.c_str(),
                      cName.c_str(), qName.c_str());

        // Note: If there are two roots that both generate radius(t) > 0, the
        // Canvas spec says to choose the larger t.

        // so we'll look at the larger one first:
        code->appendf("\t\tfloat %s = max(%s, %s);\n", tName.c_str(),
                      r0Name.c_str(), r1Name.c_str());

        // if r(t) > 0, then we're done; t will be our x coordinate
        code->appendf("\t\tif (%s * %s + %s > 0.0) {\n", tName.c_str(),
                      p5.c_str(), p3.c_str());

        code->appendf("\t\t");
        this->emitColorLookup(builder, tName.c_str(), outputColor, inputColor, samplers[0]);

        // otherwise, if r(t) for the larger root was <= 0, try the other root
        code->appendf("\t\t} else {\n");
        code->appendf("\t\t\t%s = min(%s, %s);\n", tName.c_str(),
                      r0Name.c_str(), r1Name.c_str());

        // if r(t) > 0 for the smaller root, then t will be our x coordinate
        code->appendf("\t\t\tif (%s * %s + %s > 0.0) {\n",
                      tName.c_str(), p5.c_str(), p3.c_str());

        code->appendf("\t\t\t");
        this->emitColorLookup(builder, tName.c_str(), outputColor, inputColor, samplers[0]);

        // end if (r(t) > 0) for smaller root
        code->appendf("\t\t\t}\n");
        // end if (r(t) > 0), else, for larger root
        code->appendf("\t\t}\n");
        // end if (discriminant >= 0)
        code->appendf("\t}\n");
    } else {

        // linear case: t = -c/b
        code->appendf("\tfloat %s = -(%s / %s);\n", tName.c_str(),
                      cName.c_str(), bVar.c_str());

        // if r(t) > 0, then t will be the x coordinate
        code->appendf("\tif (%s * %s + %s > 0.0) {\n", tName.c_str(),
                      p5.c_str(), p3.c_str());
        code->appendf("\t");
        this->emitColorLookup(builder, tName.c_str(), outputColor, inputColor, samplers[0]);
        code->appendf("\t}\n");
    }
}

void GrGLConical2Gradient::setData(const GrGLUniformManager& uman,
                                   const GrCustomStage& baseData,
                                   const GrRenderTarget* target,
                                   int stageNum) {
    INHERITED::setData(uman, baseData, target, stageNum);
    const GrConical2Gradient& data =
        static_cast<const GrConical2Gradient&>(baseData);
    GrAssert(data.isDegenerate() == fIsDegenerate);
    GrScalar centerX1 = data.center();
    GrScalar radius0 = data.radius();
    GrScalar diffRadius = data.diffRadius();

    if (fCachedCenter != centerX1 ||
        fCachedRadius != radius0 ||
        fCachedDiffRadius != diffRadius) {

        GrScalar a = GrMul(centerX1, centerX1) - diffRadius * diffRadius;

        // When we're in the degenerate (linear) case, the second
        // value will be INF but the program doesn't read it. (We
        // use the same 6 uniforms even though we don't need them
        // all in the linear case just to keep the code complexity
        // down).
        float values[6] = {
            GrScalarToFloat(a * 4),
            1.f / (GrScalarToFloat(a)),
            GrScalarToFloat(centerX1),
            GrScalarToFloat(radius0),
            GrScalarToFloat(SkScalarMul(radius0, radius0)),
            GrScalarToFloat(diffRadius)
        };

        uman.set1fv(fVSParamUni, 0, 6, values);
        uman.set1fv(fFSParamUni, 0, 6, values);
        fCachedCenter = centerX1;
        fCachedRadius = radius0;
        fCachedDiffRadius = diffRadius;
    }
}

GrCustomStage::StageKey GrGLConical2Gradient::GenKey(const GrCustomStage& s, const GrGLCaps& caps) {
    return (static_cast<const GrConical2Gradient&>(s).isDegenerate());
}

/////////////////////////////////////////////////////////////////////

GrCustomStage* SkTwoPointConicalGradient::asNewCustomStage(
    GrContext* context, GrSamplerState* sampler) const {
    SkASSERT(NULL != context && NULL != sampler);
    SkPoint diff = fCenter2 - fCenter1;
    SkScalar diffLen = diff.length();
    if (0 != diffLen) {
        SkScalar invDiffLen = SkScalarInvert(diffLen);
        sampler->matrix()->setSinCos(-SkScalarMul(invDiffLen, diff.fY),
                          SkScalarMul(invDiffLen, diff.fX));
    } else {
        sampler->matrix()->reset();
    }
    sampler->matrix()->preTranslate(-fCenter1.fX, -fCenter1.fY);
    sampler->textureParams()->setTileModeX(fTileMode);
    sampler->textureParams()->setTileModeY(kClamp_TileMode);
    sampler->textureParams()->setBilerp(true);
    return SkNEW_ARGS(GrConical2Gradient, (context, *this, sampler));
}

#else

GrCustomStage* SkTwoPointConicalGradient::asNewCustomStage(
    GrContext* context, GrSamplerState* sampler) const {
    SkDEBUGFAIL("Should not call in GPU-less build");
    return NULL;
}

#endif
