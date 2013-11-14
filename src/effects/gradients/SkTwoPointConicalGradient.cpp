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

typedef void (*TwoPointConicalProc)(TwoPtRadial* rec, SkPMColor* dstC,
                                    const SkPMColor* cache, int toggle, int count);

static void twopoint_clamp(TwoPtRadial* rec, SkPMColor* SK_RESTRICT dstC,
                           const SkPMColor* SK_RESTRICT cache, int toggle,
                           int count) {
    for (; count > 0; --count) {
        SkFixed t = rec->nextT();
        if (TwoPtRadial::DontDrawT(t)) {
            *dstC++ = 0;
        } else {
            SkFixed index = SkClampMax(t, 0xFFFF);
            SkASSERT(index <= 0xFFFF);
            *dstC++ = cache[toggle +
                            (index >> SkGradientShaderBase::kCache32Shift)];
        }
        toggle = next_dither_toggle(toggle);
    }
}

static void twopoint_repeat(TwoPtRadial* rec, SkPMColor* SK_RESTRICT dstC,
                            const SkPMColor* SK_RESTRICT cache, int toggle,
                            int count) {
    for (; count > 0; --count) {
        SkFixed t = rec->nextT();
        if (TwoPtRadial::DontDrawT(t)) {
            *dstC++ = 0;
        } else {
            SkFixed index = repeat_tileproc(t);
            SkASSERT(index <= 0xFFFF);
            *dstC++ = cache[toggle +
                            (index >> SkGradientShaderBase::kCache32Shift)];
        }
        toggle = next_dither_toggle(toggle);
    }
}

static void twopoint_mirror(TwoPtRadial* rec, SkPMColor* SK_RESTRICT dstC,
                            const SkPMColor* SK_RESTRICT cache, int toggle,
                            int count) {
    for (; count > 0; --count) {
        SkFixed t = rec->nextT();
        if (TwoPtRadial::DontDrawT(t)) {
            *dstC++ = 0;
        } else {
            SkFixed index = mirror_tileproc(t);
            SkASSERT(index <= 0xFFFF);
            *dstC++ = cache[toggle +
                            (index >> SkGradientShaderBase::kCache32Shift)];
        }
        toggle = next_dither_toggle(toggle);
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
        const Descriptor& desc)
    : SkGradientShaderBase(desc),
    fCenter1(start),
    fCenter2(end),
    fRadius1(startRadius),
    fRadius2(endRadius) {
    // this is degenerate, and should be caught by our caller
    SkASSERT(fCenter1 != fCenter2 || fRadius1 != fRadius2);
    this->init();
}

bool SkTwoPointConicalGradient::isOpaque() const {
    // Because areas outside the cone are left untouched, we cannot treat the
    // shader as opaque even if the gradient itself is opaque.
    // TODO(junov): Compute whether the cone fills the plane crbug.com/222380
    return false;
}

void SkTwoPointConicalGradient::shadeSpan(int x, int y, SkPMColor* dstCParam,
                                          int count) {
    int toggle = init_dither_toggle(x, y);

    SkASSERT(count > 0);

    SkPMColor* SK_RESTRICT dstC = dstCParam;

    SkMatrix::MapXYProc dstProc = fDstToIndexProc;

    const SkPMColor* SK_RESTRICT cache = this->getCache32();

    TwoPointConicalProc shadeProc = twopoint_repeat;
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
        (*shadeProc)(&fRec, dstC, cache, toggle, count);
    } else {    // perspective case
        SkScalar dstX = SkIntToScalar(x) + SK_ScalarHalf;
        SkScalar dstY = SkIntToScalar(y) + SK_ScalarHalf;
        for (; count > 0; --count) {
            SkPoint srcPt;
            dstProc(fDstToIndex, dstX, dstY, &srcPt);
            fRec.setup(srcPt.fX, srcPt.fY, 0, 0);
            (*shadeProc)(&fRec, dstC, cache, toggle, 1);

            dstX += SK_Scalar1;
            toggle = next_dither_toggle(toggle);
            dstC += 1;
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

#include "GrTBackendEffectFactory.h"

// For brevity
typedef GrGLUniformManager::UniformHandle UniformHandle;

class GrGLConical2Gradient : public GrGLGradientEffect {
public:

    GrGLConical2Gradient(const GrBackendEffectFactory& factory, const GrDrawEffect&);
    virtual ~GrGLConical2Gradient() { }

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

/////////////////////////////////////////////////////////////////////

class GrConical2Gradient : public GrGradientEffect {
public:

    static GrEffectRef* Create(GrContext* ctx,
                               const SkTwoPointConicalGradient& shader,
                               const SkMatrix& matrix,
                               SkShader::TileMode tm) {
        AutoEffectUnref effect(SkNEW_ARGS(GrConical2Gradient, (ctx, shader, matrix, tm)));
        return CreateEffectRef(effect);
    }

    virtual ~GrConical2Gradient() { }

    static const char* Name() { return "Two-Point Conical Gradient"; }
    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<GrConical2Gradient>::getInstance();
    }

    // The radial gradient parameters can collapse to a linear (instead of quadratic) equation.
    bool isDegenerate() const { return SkScalarAbs(fDiffRadius) == SkScalarAbs(fCenterX1); }
    SkScalar center() const { return fCenterX1; }
    SkScalar diffRadius() const { return fDiffRadius; }
    SkScalar radius() const { return fRadius0; }

    typedef GrGLConical2Gradient GLEffect;

private:
    virtual bool onIsEqual(const GrEffect& sBase) const SK_OVERRIDE {
        const GrConical2Gradient& s = CastEffect<GrConical2Gradient>(sBase);
        return (INHERITED::onIsEqual(sBase) &&
                this->fCenterX1 == s.fCenterX1 &&
                this->fRadius0 == s.fRadius0 &&
                this->fDiffRadius == s.fDiffRadius);
    }

    GrConical2Gradient(GrContext* ctx,
                       const SkTwoPointConicalGradient& shader,
                       const SkMatrix& matrix,
                       SkShader::TileMode tm)
        : INHERITED(ctx, shader, matrix, tm)
        , fCenterX1(shader.getCenterX1())
        , fRadius0(shader.getStartRadius())
        , fDiffRadius(shader.getDiffRadius()) {
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

GR_DEFINE_EFFECT_TEST(GrConical2Gradient);

GrEffectRef* GrConical2Gradient::TestCreate(SkRandom* random,
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

GrGLConical2Gradient::GrGLConical2Gradient(const GrBackendEffectFactory& factory,
                                           const GrDrawEffect& drawEffect)
    : INHERITED(factory)
    , fVSVaryingName(NULL)
    , fFSVaryingName(NULL)
    , fCachedCenter(SK_ScalarMax)
    , fCachedRadius(-SK_ScalarMax)
    , fCachedDiffRadius(-SK_ScalarMax) {

    const GrConical2Gradient& data = drawEffect.castEffect<GrConical2Gradient>();
    fIsDegenerate = data.isDegenerate();
}

void GrGLConical2Gradient::emitCode(GrGLShaderBuilder* builder,
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

void GrGLConical2Gradient::setData(const GrGLUniformManager& uman,
                                   const GrDrawEffect& drawEffect) {
    INHERITED::setData(uman, drawEffect);
    const GrConical2Gradient& data = drawEffect.castEffect<GrConical2Gradient>();
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

GrGLEffect::EffectKey GrGLConical2Gradient::GenKey(const GrDrawEffect& drawEffect,
                                                   const GrGLCaps&) {
    enum {
        kIsDegenerate = 1 << kBaseKeyBitCnt,
    };

    EffectKey key = GenBaseGradientKey(drawEffect);
    if (drawEffect.castEffect<GrConical2Gradient>().isDegenerate()) {
        key |= kIsDegenerate;
    }
    return key;
}

/////////////////////////////////////////////////////////////////////

GrEffectRef* SkTwoPointConicalGradient::asNewEffect(GrContext* context, const SkPaint&) const {
    SkASSERT(NULL != context);
    SkASSERT(fPtsToUnit.isIdentity());
    // invert the localM, translate to center1, rotate so center2 is on x axis.
    SkMatrix matrix;
    if (!this->getLocalMatrix().invert(&matrix)) {
        return NULL;
    }
    matrix.postTranslate(-fCenter1.fX, -fCenter1.fY);

    SkPoint diff = fCenter2 - fCenter1;
    SkScalar diffLen = diff.length();
    if (0 != diffLen) {
        SkScalar invDiffLen = SkScalarInvert(diffLen);
        SkMatrix rot;
        rot.setSinCos(-SkScalarMul(invDiffLen, diff.fY),
                       SkScalarMul(invDiffLen, diff.fX));
        matrix.postConcat(rot);
    }

    return GrConical2Gradient::Create(context, *this, matrix, fTileMode);
}

#else

GrEffectRef* SkTwoPointConicalGradient::asNewEffect(GrContext*, const SkPaint&) const {
    SkDEBUGFAIL("Should not call in GPU-less build");
    return NULL;
}

#endif

#ifdef SK_DEVELOPER
void SkTwoPointConicalGradient::toString(SkString* str) const {
    str->append("SkTwoPointConicalGradient: (");

    str->append("center1: (");
    str->appendScalar(fCenter1.fX);
    str->append(", ");
    str->appendScalar(fCenter1.fY);
    str->append(") radius1: ");
    str->appendScalar(fRadius1);
    str->append(" ");

    str->append("center2: (");
    str->appendScalar(fCenter2.fX);
    str->append(", ");
    str->appendScalar(fCenter2.fY);
    str->append(") radius2: ");
    str->appendScalar(fRadius2);
    str->append(" ");

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
