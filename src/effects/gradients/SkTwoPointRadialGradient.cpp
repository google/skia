
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

 #include "SkTwoPointRadialGradient.h"

/* Two-point radial gradients are specified by two circles, each with a center
   point and radius.  The gradient can be considered to be a series of
   concentric circles, with the color interpolated from the start circle
   (at t=0) to the end circle (at t=1).

   For each point (x, y) in the span, we want to find the
   interpolated circle that intersects that point.  The center
   of the desired circle (Cx, Cy) falls at some distance t
   along the line segment between the start point (Sx, Sy) and
   end point (Ex, Ey):

      Cx = (1 - t) * Sx + t * Ex        (0 <= t <= 1)
      Cy = (1 - t) * Sy + t * Ey

   The radius of the desired circle (r) is also a linear interpolation t
   between the start and end radii (Sr and Er):

      r = (1 - t) * Sr + t * Er

   But

      (x - Cx)^2 + (y - Cy)^2 = r^2

   so

     (x - ((1 - t) * Sx + t * Ex))^2
   + (y - ((1 - t) * Sy + t * Ey))^2
   = ((1 - t) * Sr + t * Er)^2

   Solving for t yields

     [(Sx - Ex)^2 + (Sy - Ey)^2 - (Er - Sr)^2)] * t^2
   + [2 * (Sx - Ex)(x - Sx) + 2 * (Sy - Ey)(y - Sy) - 2 * (Er - Sr) * Sr] * t
   + [(x - Sx)^2 + (y - Sy)^2 - Sr^2] = 0

   To simplify, let Dx = Sx - Ex, Dy = Sy - Ey, Dr = Er - Sr, dx = x - Sx, dy = y - Sy

     [Dx^2 + Dy^2 - Dr^2)] * t^2
   + 2 * [Dx * dx + Dy * dy - Dr * Sr] * t
   + [dx^2 + dy^2 - Sr^2] = 0

   A quadratic in t.  The two roots of the quadratic reflect the two
   possible circles on which the point may fall.  Solving for t yields
   the gradient value to use.

   If a<0, the start circle is entirely contained in the
   end circle, and one of the roots will be <0 or >1 (off the line
   segment).  If a>0, the start circle falls at least partially
   outside the end circle (or vice versa), and the gradient
   defines a "tube" where a point may be on one circle (on the
   inside of the tube) or the other (outside of the tube).  We choose
   one arbitrarily.

   In order to keep the math to within the limits of fixed point,
   we divide the entire quadratic by Dr^2, and replace
   (x - Sx)/Dr with x' and (y - Sy)/Dr with y', giving

   [Dx^2 / Dr^2 + Dy^2 / Dr^2 - 1)] * t^2
   + 2 * [x' * Dx / Dr + y' * Dy / Dr - Sr / Dr] * t
   + [x'^2 + y'^2 - Sr^2/Dr^2] = 0

   (x' and y' are computed by appending the subtract and scale to the
   fDstToIndex matrix in the constructor).

   Since the 'A' component of the quadratic is independent of x' and y', it
   is precomputed in the constructor.  Since the 'B' component is linear in
   x' and y', if x and y are linear in the span, 'B' can be computed
   incrementally with a simple delta (db below).  If it is not (e.g.,
   a perspective projection), it must be computed in the loop.

*/

namespace {

inline SkFixed two_point_radial(SkScalar b, SkScalar fx, SkScalar fy,
                                SkScalar sr2d2, SkScalar foura,
                                SkScalar oneOverTwoA, bool posRoot) {
    SkScalar c = SkScalarSquare(fx) + SkScalarSquare(fy) - sr2d2;
    if (0 == foura) {
        return SkScalarToFixed(SkScalarDiv(-c, b));
    }

    SkScalar discrim = SkScalarSquare(b) - SkScalarMul(foura, c);
    if (discrim < 0) {
        discrim = -discrim;
    }
    SkScalar rootDiscrim = SkScalarSqrt(discrim);
    SkScalar result;
    if (posRoot) {
        result = SkScalarMul(-b + rootDiscrim, oneOverTwoA);
    } else {
        result = SkScalarMul(-b - rootDiscrim, oneOverTwoA);
    }
    return SkScalarToFixed(result);
}

typedef void (* TwoPointRadialShadeProc)(SkScalar fx, SkScalar dx,
        SkScalar fy, SkScalar dy,
        SkScalar b, SkScalar db,
        SkScalar fSr2D2, SkScalar foura, SkScalar fOneOverTwoA, bool posRoot,
        SkPMColor* SK_RESTRICT dstC, const SkPMColor* SK_RESTRICT cache,
        int count);

void shadeSpan_twopoint_clamp(SkScalar fx, SkScalar dx,
        SkScalar fy, SkScalar dy,
        SkScalar b, SkScalar db,
        SkScalar fSr2D2, SkScalar foura, SkScalar fOneOverTwoA, bool posRoot,
        SkPMColor* SK_RESTRICT dstC, const SkPMColor* SK_RESTRICT cache,
        int count) {
    for (; count > 0; --count) {
        SkFixed t = two_point_radial(b, fx, fy, fSr2D2, foura,
                                     fOneOverTwoA, posRoot);
        SkFixed index = SkClampMax(t, 0xFFFF);
        SkASSERT(index <= 0xFFFF);
        *dstC++ = cache[index >> SkGradientShaderBase::kCache32Shift];
        fx += dx;
        fy += dy;
        b += db;
    }
}
void shadeSpan_twopoint_mirror(SkScalar fx, SkScalar dx,
        SkScalar fy, SkScalar dy,
        SkScalar b, SkScalar db,
        SkScalar fSr2D2, SkScalar foura, SkScalar fOneOverTwoA, bool posRoot,
        SkPMColor* SK_RESTRICT dstC, const SkPMColor* SK_RESTRICT cache,
        int count) {
    for (; count > 0; --count) {
        SkFixed t = two_point_radial(b, fx, fy, fSr2D2, foura,
                                     fOneOverTwoA, posRoot);
        SkFixed index = mirror_tileproc(t);
        SkASSERT(index <= 0xFFFF);
        *dstC++ = cache[index >> SkGradientShaderBase::kCache32Shift];
        fx += dx;
        fy += dy;
        b += db;
    }
}

void shadeSpan_twopoint_repeat(SkScalar fx, SkScalar dx,
        SkScalar fy, SkScalar dy,
        SkScalar b, SkScalar db,
        SkScalar fSr2D2, SkScalar foura, SkScalar fOneOverTwoA, bool posRoot,
        SkPMColor* SK_RESTRICT dstC, const SkPMColor* SK_RESTRICT cache,
        int count) {
    for (; count > 0; --count) {
        SkFixed t = two_point_radial(b, fx, fy, fSr2D2, foura,
                                     fOneOverTwoA, posRoot);
        SkFixed index = repeat_tileproc(t);
        SkASSERT(index <= 0xFFFF);
        *dstC++ = cache[index >> SkGradientShaderBase::kCache32Shift];
        fx += dx;
        fy += dy;
        b += db;
    }
}
}

/////////////////////////////////////////////////////////////////////

SkTwoPointRadialGradient::SkTwoPointRadialGradient(
    const SkPoint& start, SkScalar startRadius,
    const SkPoint& end, SkScalar endRadius,
    const Descriptor& desc)
    : SkGradientShaderBase(desc),
      fCenter1(start),
      fCenter2(end),
      fRadius1(startRadius),
      fRadius2(endRadius) {
    init();
}

SkShader::BitmapType SkTwoPointRadialGradient::asABitmap(
    SkBitmap* bitmap,
    SkMatrix* matrix,
    SkShader::TileMode* xy) const {
    if (bitmap) {
        this->getGradientTableBitmap(bitmap);
    }
    SkScalar diffL = 0; // just to avoid gcc warning
    if (matrix) {
        diffL = SkScalarSqrt(SkScalarSquare(fDiff.fX) +
                             SkScalarSquare(fDiff.fY));
    }
    if (matrix) {
        if (diffL) {
            SkScalar invDiffL = SkScalarInvert(diffL);
            matrix->setSinCos(-SkScalarMul(invDiffL, fDiff.fY),
                              SkScalarMul(invDiffL, fDiff.fX));
        } else {
            matrix->reset();
        }
        matrix->preConcat(fPtsToUnit);
    }
    if (xy) {
        xy[0] = fTileMode;
        xy[1] = kClamp_TileMode;
    }
    return kTwoPointRadial_BitmapType;
}

SkShader::GradientType SkTwoPointRadialGradient::asAGradient(
    SkShader::GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fCenter1;
        info->fPoint[1] = fCenter2;
        info->fRadius[0] = fRadius1;
        info->fRadius[1] = fRadius2;
    }
    return kRadial2_GradientType;
}

void SkTwoPointRadialGradient::shadeSpan(int x, int y, SkPMColor* dstCParam,
                                         int count) {
    SkASSERT(count > 0);

    SkPMColor* SK_RESTRICT dstC = dstCParam;

    // Zero difference between radii:  fill with transparent black.
    if (fDiffRadius == 0) {
      sk_bzero(dstC, count * sizeof(*dstC));
      return;
    }
    SkMatrix::MapXYProc dstProc = fDstToIndexProc;
    TileProc            proc = fTileProc;
    const SkPMColor* SK_RESTRICT cache = this->getCache32();

    SkScalar foura = fA * 4;
    bool posRoot = fDiffRadius < 0;
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
        SkScalar b = (SkScalarMul(fDiff.fX, fx) +
                     SkScalarMul(fDiff.fY, fy) - fStartRadius) * 2;
        SkScalar db = (SkScalarMul(fDiff.fX, dx) +
                      SkScalarMul(fDiff.fY, dy)) * 2;

        TwoPointRadialShadeProc shadeProc = shadeSpan_twopoint_repeat;
        if (SkShader::kClamp_TileMode == fTileMode) {
            shadeProc = shadeSpan_twopoint_clamp;
        } else if (SkShader::kMirror_TileMode == fTileMode) {
            shadeProc = shadeSpan_twopoint_mirror;
        } else {
            SkASSERT(SkShader::kRepeat_TileMode == fTileMode);
        }
        (*shadeProc)(fx, dx, fy, dy, b, db,
                     fSr2D2, foura, fOneOverTwoA, posRoot,
                     dstC, cache, count);
    } else {    // perspective case
        SkScalar dstX = SkIntToScalar(x);
        SkScalar dstY = SkIntToScalar(y);
        for (; count > 0; --count) {
            SkPoint             srcPt;
            dstProc(fDstToIndex, dstX, dstY, &srcPt);
            SkScalar fx = srcPt.fX;
            SkScalar fy = srcPt.fY;
            SkScalar b = (SkScalarMul(fDiff.fX, fx) +
                         SkScalarMul(fDiff.fY, fy) - fStartRadius) * 2;
            SkFixed t = two_point_radial(b, fx, fy, fSr2D2, foura,
                                         fOneOverTwoA, posRoot);
            SkFixed index = proc(t);
            SkASSERT(index <= 0xFFFF);
            *dstC++ = cache[index >> SkGradientShaderBase::kCache32Shift];
            dstX += SK_Scalar1;
        }
    }
}

bool SkTwoPointRadialGradient::setContext( const SkBitmap& device,
                                          const SkPaint& paint,
                                          const SkMatrix& matrix){
    // For now, we might have divided by zero, so detect that
    if (0 == fDiffRadius) {
        return false;
    }

    if (!this->INHERITED::setContext(device, paint, matrix)) {
        return false;
    }

    // we don't have a span16 proc
    fFlags &= ~kHasSpan16_Flag;
    return true;
}

#ifdef SK_DEVELOPER
void SkTwoPointRadialGradient::toString(SkString* str) const {
    str->append("SkTwoPointRadialGradient: (");

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

SkTwoPointRadialGradient::SkTwoPointRadialGradient(
    SkFlattenableReadBuffer& buffer)
    : INHERITED(buffer),
      fCenter1(buffer.readPoint()),
      fCenter2(buffer.readPoint()),
      fRadius1(buffer.readScalar()),
      fRadius2(buffer.readScalar()) {
    init();
};

void SkTwoPointRadialGradient::flatten(
    SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fCenter1);
    buffer.writePoint(fCenter2);
    buffer.writeScalar(fRadius1);
    buffer.writeScalar(fRadius2);
}

void SkTwoPointRadialGradient::init() {
    fDiff = fCenter1 - fCenter2;
    fDiffRadius = fRadius2 - fRadius1;
    // hack to avoid zero-divide for now
    SkScalar inv = fDiffRadius ? SkScalarInvert(fDiffRadius) : 0;
    fDiff.fX = SkScalarMul(fDiff.fX, inv);
    fDiff.fY = SkScalarMul(fDiff.fY, inv);
    fStartRadius = SkScalarMul(fRadius1, inv);
    fSr2D2 = SkScalarSquare(fStartRadius);
    fA = SkScalarSquare(fDiff.fX) + SkScalarSquare(fDiff.fY) - SK_Scalar1;
    fOneOverTwoA = fA ? SkScalarInvert(fA * 2) : 0;

    fPtsToUnit.setTranslate(-fCenter1.fX, -fCenter1.fY);
    fPtsToUnit.postScale(inv, inv);
}

/////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrTBackendEffectFactory.h"

// For brevity
typedef GrGLUniformManager::UniformHandle UniformHandle;

class GrGLRadial2Gradient : public GrGLGradientEffect {

public:

    GrGLRadial2Gradient(const GrBackendEffectFactory& factory, const GrDrawEffect&);
    virtual ~GrGLRadial2Gradient() { }

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
    bool     fCachedPosRoot;

    // @}

private:

    typedef GrGLGradientEffect INHERITED;

};

/////////////////////////////////////////////////////////////////////

class GrRadial2Gradient : public GrGradientEffect {
public:
    static GrEffectRef* Create(GrContext* ctx,
                               const SkTwoPointRadialGradient& shader,
                               const SkMatrix& matrix,
                               SkShader::TileMode tm) {
        AutoEffectUnref effect(SkNEW_ARGS(GrRadial2Gradient, (ctx, shader, matrix, tm)));
        return CreateEffectRef(effect);
    }

    virtual ~GrRadial2Gradient() { }

    static const char* Name() { return "Two-Point Radial Gradient"; }
    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<GrRadial2Gradient>::getInstance();
    }

    // The radial gradient parameters can collapse to a linear (instead of quadratic) equation.
    bool isDegenerate() const { return SK_Scalar1 == fCenterX1; }
    SkScalar center() const { return fCenterX1; }
    SkScalar radius() const { return fRadius0; }
    bool isPosRoot() const { return SkToBool(fPosRoot); }

    typedef GrGLRadial2Gradient GLEffect;

private:
    virtual bool onIsEqual(const GrEffect& sBase) const SK_OVERRIDE {
        const GrRadial2Gradient& s = CastEffect<GrRadial2Gradient>(sBase);
        return (INHERITED::onIsEqual(sBase) &&
                this->fCenterX1 == s.fCenterX1 &&
                this->fRadius0 == s.fRadius0 &&
                this->fPosRoot == s.fPosRoot);
    }

    GrRadial2Gradient(GrContext* ctx,
                      const SkTwoPointRadialGradient& shader,
                      const SkMatrix& matrix,
                      SkShader::TileMode tm)
        : INHERITED(ctx, shader, matrix, tm)
        , fCenterX1(shader.getCenterX1())
        , fRadius0(shader.getStartRadius())
        , fPosRoot(shader.getDiffRadius() < 0) {
        // We pass the linear part of the quadratic as a varying.
        //    float b = 2.0 * (fCenterX1 * x - fRadius0 * z)
        fBTransform = this->getCoordTransform();
        SkMatrix& bMatrix = *fBTransform.accessMatrix();
        bMatrix[SkMatrix::kMScaleX] = 2 * (SkScalarMul(fCenterX1, bMatrix[SkMatrix::kMScaleX]) -
                                           SkScalarMul(fRadius0, bMatrix[SkMatrix::kMPersp0]));
        bMatrix[SkMatrix::kMSkewX] = 2 * (SkScalarMul(fCenterX1, bMatrix[SkMatrix::kMSkewX]) -
                                          SkScalarMul(fRadius0, bMatrix[SkMatrix::kMPersp1]));
        bMatrix[SkMatrix::kMTransX] = 2 * (SkScalarMul(fCenterX1, bMatrix[SkMatrix::kMTransX]) -
                                           SkScalarMul(fRadius0, bMatrix[SkMatrix::kMPersp2]));
        this->addCoordTransform(&fBTransform);
    }

    GR_DECLARE_EFFECT_TEST;

    // @{
    // Cache of values - these can change arbitrarily, EXCEPT
    // we shouldn't change between degenerate and non-degenerate?!

    GrCoordTransform fBTransform;
    SkScalar         fCenterX1;
    SkScalar         fRadius0;
    SkBool8          fPosRoot;

    // @}

    typedef GrGradientEffect INHERITED;
};

/////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrRadial2Gradient);

GrEffectRef* GrRadial2Gradient::TestCreate(SkRandom* random,
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
        // There is a bug in two point radial gradients with identical radii
    } while (radius1 == radius2);

    SkColor colors[kMaxRandomGradientColors];
    SkScalar stopsArray[kMaxRandomGradientColors];
    SkScalar* stops = stopsArray;
    SkShader::TileMode tm;
    int colorCount = RandomGradientParams(random, colors, &stops, &tm);
    SkAutoTUnref<SkShader> shader(SkGradientShader::CreateTwoPointRadial(center1, radius1,
                                                                         center2, radius2,
                                                                         colors, stops, colorCount,
                                                                         tm));
    SkPaint paint;
    return shader->asNewEffect(context, paint);
}

/////////////////////////////////////////////////////////////////////

GrGLRadial2Gradient::GrGLRadial2Gradient(const GrBackendEffectFactory& factory,
                                         const GrDrawEffect& drawEffect)
    : INHERITED(factory)
    , fVSVaryingName(NULL)
    , fFSVaryingName(NULL)
    , fCachedCenter(SK_ScalarMax)
    , fCachedRadius(-SK_ScalarMax)
    , fCachedPosRoot(0) {

    const GrRadial2Gradient& data = drawEffect.castEffect<GrRadial2Gradient>();
    fIsDegenerate = data.isDegenerate();
}

void GrGLRadial2Gradient::emitCode(GrGLShaderBuilder* builder,
                                   const GrDrawEffect& drawEffect,
                                   EffectKey key,
                                   const char* outputColor,
                                   const char* inputColor,
                                   const TransformedCoordsArray& coords,
                                   const TextureSamplerArray& samplers) {

    this->emitUniforms(builder, key);
    fParamUni = builder->addUniformArray(GrGLShaderBuilder::kFragment_Visibility,
                                         kFloat_GrSLType, "Radial2FSParams", 6);

    SkString cName("c");
    SkString ac4Name("ac4");
    SkString rootName("root");
    SkString t;
    SkString p0;
    SkString p1;
    SkString p2;
    SkString p3;
    SkString p4;
    SkString p5;
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

    // c = (x^2)+(y^2) - params[4]
    builder->fsCodeAppendf("\tfloat %s = dot(%s, %s) - %s;\n",
                           cName.c_str(), coords2D, coords2D, p4.c_str());

    // If we aren't degenerate, emit some extra code, and accept a slightly
    // more complex coord.
    if (!fIsDegenerate) {

        // ac4 = 4.0 * params[0] * c
        builder->fsCodeAppendf("\tfloat %s = %s * 4.0 * %s;\n",
                               ac4Name.c_str(), p0.c_str(),
                               cName.c_str());

        // root = sqrt(b^2-4ac)
        // (abs to avoid exception due to fp precision)
        builder->fsCodeAppendf("\tfloat %s = sqrt(abs(%s*%s - %s));\n",
                               rootName.c_str(), bVar.c_str(), bVar.c_str(),
                               ac4Name.c_str());

        // t is: (-b + params[5] * sqrt(b^2-4ac)) * params[1]
        t.printf("(-%s + %s * %s) * %s", bVar.c_str(), p5.c_str(),
                 rootName.c_str(), p1.c_str());
    } else {
        // t is: -c/b
        t.printf("-%s / %s", cName.c_str(), bVar.c_str());
    }

    this->emitColor(builder, t.c_str(), key, outputColor, inputColor, samplers);
}

void GrGLRadial2Gradient::setData(const GrGLUniformManager& uman,
                                  const GrDrawEffect& drawEffect) {
    INHERITED::setData(uman, drawEffect);
    const GrRadial2Gradient& data = drawEffect.castEffect<GrRadial2Gradient>();
    SkASSERT(data.isDegenerate() == fIsDegenerate);
    SkScalar centerX1 = data.center();
    SkScalar radius0 = data.radius();
    if (fCachedCenter != centerX1 ||
        fCachedRadius != radius0 ||
        fCachedPosRoot != data.isPosRoot()) {

        SkScalar a = SkScalarMul(centerX1, centerX1) - SK_Scalar1;

        // When we're in the degenerate (linear) case, the second
        // value will be INF but the program doesn't read it. (We
        // use the same 6 uniforms even though we don't need them
        // all in the linear case just to keep the code complexity
        // down).
        float values[6] = {
            SkScalarToFloat(a),
            1 / (2.f * SkScalarToFloat(a)),
            SkScalarToFloat(centerX1),
            SkScalarToFloat(radius0),
            SkScalarToFloat(SkScalarMul(radius0, radius0)),
            data.isPosRoot() ? 1.f : -1.f
        };

        uman.set1fv(fParamUni, 6, values);
        fCachedCenter = centerX1;
        fCachedRadius = radius0;
        fCachedPosRoot = data.isPosRoot();
    }
}

GrGLEffect::EffectKey GrGLRadial2Gradient::GenKey(const GrDrawEffect& drawEffect,
                                                  const GrGLCaps&) {
    enum {
        kIsDegenerate = 1 << kBaseKeyBitCnt,
    };

    EffectKey key = GenBaseGradientKey(drawEffect);
    if (drawEffect.castEffect<GrRadial2Gradient>().isDegenerate()) {
        key |= kIsDegenerate;
    }
    return key;
}

/////////////////////////////////////////////////////////////////////

GrEffectRef* SkTwoPointRadialGradient::asNewEffect(GrContext* context, const SkPaint&) const {
    SkASSERT(NULL != context);
    // invert the localM, translate to center1 (fPtsToUni), rotate so center2 is on x axis.
    SkMatrix matrix;
    if (!this->getLocalMatrix().invert(&matrix)) {
        return NULL;
    }
    matrix.postConcat(fPtsToUnit);

    SkScalar diffLen = fDiff.length();
    if (0 != diffLen) {
        SkScalar invDiffLen = SkScalarInvert(diffLen);
        SkMatrix rot;
        rot.setSinCos(-SkScalarMul(invDiffLen, fDiff.fY),
                       SkScalarMul(invDiffLen, fDiff.fX));
        matrix.postConcat(rot);
    }

    return GrRadial2Gradient::Create(context, *this, matrix, fTileMode);
}

#else

GrEffectRef* SkTwoPointRadialGradient::asNewEffect(GrContext*, const SkPaint&) const {
    SkDEBUGFAIL("Should not call in GPU-less build");
    return NULL;
}

#endif
