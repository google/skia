
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSweepGradient.h"

SkSweepGradient::SkSweepGradient(SkScalar cx, SkScalar cy,
                                 const Descriptor& desc)
    : SkGradientShaderBase(desc)
    , fCenter(SkPoint::Make(cx, cy))
{
    fPtsToUnit.setTranslate(-cx, -cy);

    // overwrite the tilemode to a canonical value (since sweep ignores it)
    fTileMode = SkShader::kClamp_TileMode;
}

SkShader::BitmapType SkSweepGradient::asABitmap(SkBitmap* bitmap,
    SkMatrix* matrix, SkShader::TileMode* xy) const {
    if (bitmap) {
        this->getGradientTableBitmap(bitmap);
    }
    if (matrix) {
        *matrix = fPtsToUnit;
    }
    if (xy) {
        xy[0] = fTileMode;
        xy[1] = kClamp_TileMode;
    }
    return kSweep_BitmapType;
}

SkShader::GradientType SkSweepGradient::asAGradient(GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fCenter;
    }
    return kSweep_GradientType;
}

SkSweepGradient::SkSweepGradient(SkFlattenableReadBuffer& buffer)
    : INHERITED(buffer),
      fCenter(buffer.readPoint()) {
}

void SkSweepGradient::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fCenter);
}

#ifndef SK_SCALAR_IS_FLOAT
#ifdef COMPUTE_SWEEP_TABLE
#define PI  3.14159265
static bool gSweepTableReady;
static uint8_t gSweepTable[65];

/*  Our table stores precomputed values for atan: [0...1] -> [0..PI/4]
    We scale the results to [0..32]
*/
static const uint8_t* build_sweep_table() {
    if (!gSweepTableReady) {
        const int N = 65;
        const double DENOM = N - 1;

        for (int i = 0; i < N; i++)
        {
            double arg = i / DENOM;
            double v = atan(arg);
            int iv = (int)round(v * DENOM * 2 / PI);
//            printf("[%d] atan(%g) = %g %d\n", i, arg, v, iv);
            printf("%d, ", iv);
            gSweepTable[i] = iv;
        }
        gSweepTableReady = true;
    }
    return gSweepTable;
}
#else
static const uint8_t gSweepTable[] = {
    0, 1, 1, 2, 3, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 9,
    10, 11, 11, 12, 12, 13, 13, 14, 15, 15, 16, 16, 17, 17, 18, 18,
    19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 25, 26,
    26, 27, 27, 27, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31, 31, 32,
    32
};
static const uint8_t* build_sweep_table() { return gSweepTable; }
#endif
#endif

// divide numer/denom, with a bias of 6bits. Assumes numer <= denom
// and denom != 0. Since our table is 6bits big (+1), this is a nice fit.
// Same as (but faster than) SkFixedDiv(numer, denom) >> 10

//unsigned div_64(int numer, int denom);
#ifndef SK_SCALAR_IS_FLOAT
static unsigned div_64(int numer, int denom) {
    SkASSERT(numer <= denom);
    SkASSERT(numer > 0);
    SkASSERT(denom > 0);

    int nbits = SkCLZ(numer);
    int dbits = SkCLZ(denom);
    int bits = 6 - nbits + dbits;
    SkASSERT(bits <= 6);

    if (bits < 0) {  // detect underflow
        return 0;
    }

    denom <<= dbits - 1;
    numer <<= nbits - 1;

    unsigned result = 0;

    // do the first one
    if ((numer -= denom) >= 0) {
        result = 1;
    } else {
        numer += denom;
    }

    // Now fall into our switch statement if there are more bits to compute
    if (bits > 0) {
        // make room for the rest of the answer bits
        result <<= bits;
        switch (bits) {
        case 6:
            if ((numer = (numer << 1) - denom) >= 0)
                result |= 32;
            else
                numer += denom;
        case 5:
            if ((numer = (numer << 1) - denom) >= 0)
                result |= 16;
            else
                numer += denom;
        case 4:
            if ((numer = (numer << 1) - denom) >= 0)
                result |= 8;
            else
                numer += denom;
        case 3:
            if ((numer = (numer << 1) - denom) >= 0)
                result |= 4;
            else
                numer += denom;
        case 2:
            if ((numer = (numer << 1) - denom) >= 0)
                result |= 2;
            else
                numer += denom;
        case 1:
        default:    // not strictly need, but makes GCC make better ARM code
            if ((numer = (numer << 1) - denom) >= 0)
                result |= 1;
            else
                numer += denom;
        }
    }
    return result;
}
#endif

// Given x,y in the first quadrant, return 0..63 for the angle [0..90]
#ifndef SK_SCALAR_IS_FLOAT
static unsigned atan_0_90(SkFixed y, SkFixed x) {
#ifdef SK_DEBUG
    {
        static bool gOnce;
        if (!gOnce) {
            gOnce = true;
            SkASSERT(div_64(55, 55) == 64);
            SkASSERT(div_64(128, 256) == 32);
            SkASSERT(div_64(2326528, 4685824) == 31);
            SkASSERT(div_64(753664, 5210112) == 9);
            SkASSERT(div_64(229376, 4882432) == 3);
            SkASSERT(div_64(2, 64) == 2);
            SkASSERT(div_64(1, 64) == 1);
            // test that we handle underflow correctly
            SkASSERT(div_64(12345, 0x54321234) == 0);
        }
    }
#endif

    SkASSERT(y > 0 && x > 0);
    const uint8_t* table = build_sweep_table();

    unsigned result;
    bool swap = (x < y);
    if (swap) {
        // first part of the atan(v) = PI/2 - atan(1/v) identity
        // since our div_64 and table want v <= 1, where v = y/x
        SkTSwap<SkFixed>(x, y);
    }

    result = div_64(y, x);

#ifdef SK_DEBUG
    {
        unsigned result2 = SkDivBits(y, x, 6);
        SkASSERT(result2 == result ||
                 (result == 1 && result2 == 0));
    }
#endif

    SkASSERT(result < SK_ARRAY_COUNT(gSweepTable));
    result = table[result];

    if (swap) {
        // complete the atan(v) = PI/2 - atan(1/v) identity
        result = 64 - result;
        // pin to 63
        result -= result >> 6;
    }

    SkASSERT(result <= 63);
    return result;
}
#endif

//  returns angle in a circle [0..2PI) -> [0..255]
#ifdef SK_SCALAR_IS_FLOAT
static unsigned SkATan2_255(float y, float x) {
    //    static const float g255Over2PI = 255 / (2 * SK_ScalarPI);
    static const float g255Over2PI = 40.584510488433314f;

    float result = sk_float_atan2(y, x);
    if (result < 0) {
        result += 2 * SK_ScalarPI;
    }
    SkASSERT(result >= 0);
    // since our value is always >= 0, we can cast to int, which is faster than
    // calling floorf()
    int ir = (int)(result * g255Over2PI);
    SkASSERT(ir >= 0 && ir <= 255);
    return ir;
}
#else
static unsigned SkATan2_255(SkFixed y, SkFixed x) {
    if (x == 0) {
        if (y == 0) {
            return 0;
        }
        return y < 0 ? 192 : 64;
    }
    if (y == 0) {
        return x < 0 ? 128 : 0;
    }

    /*  Find the right quadrant for x,y
        Since atan_0_90 only handles the first quadrant, we rotate x,y
        appropriately before calling it, and then add the right amount
        to account for the real quadrant.
        quadrant 0 : add 0                  | x > 0 && y > 0
        quadrant 1 : add 64 (90 degrees)    | x < 0 && y > 0
        quadrant 2 : add 128 (180 degrees)  | x < 0 && y < 0
        quadrant 3 : add 192 (270 degrees)  | x > 0 && y < 0

        map x<0 to (1 << 6)
        map y<0 to (3 << 6)
        add = map_x ^ map_y
    */
    int xsign = x >> 31;
    int ysign = y >> 31;
    int add = ((-xsign) ^ (ysign & 3)) << 6;

#ifdef SK_DEBUG
    if (0 == add)
        SkASSERT(x > 0 && y > 0);
    else if (64 == add)
        SkASSERT(x < 0 && y > 0);
    else if (128 == add)
        SkASSERT(x < 0 && y < 0);
    else if (192 == add)
        SkASSERT(x > 0 && y < 0);
    else
        SkDEBUGFAIL("bad value for add");
#endif

    /*  This ^ trick makes x, y positive, and the swap<> handles quadrants
        where we need to rotate x,y by 90 or -90
    */
    x = (x ^ xsign) - xsign;
    y = (y ^ ysign) - ysign;
    if (add & 64) {             // quads 1 or 3 need to swap x,y
        SkTSwap<SkFixed>(x, y);
    }

    unsigned result = add + atan_0_90(y, x);
    SkASSERT(result < 256);
    return result;
}
#endif

void SkSweepGradient::shadeSpan(int x, int y, SkPMColor* SK_RESTRICT dstC,
                               int count) {
    SkMatrix::MapXYProc proc = fDstToIndexProc;
    const SkMatrix&     matrix = fDstToIndex;
    const SkPMColor* SK_RESTRICT cache = this->getCache32();
    int                 toggle = init_dither_toggle(x, y);
    SkPoint             srcPt;

    if (fDstToIndexClass != kPerspective_MatrixClass) {
        proc(matrix, SkIntToScalar(x) + SK_ScalarHalf,
                     SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
        SkScalar dx, fx = srcPt.fX;
        SkScalar dy, fy = srcPt.fY;

        if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
            SkFixed storage[2];
            (void)matrix.fixedStepInX(SkIntToScalar(y) + SK_ScalarHalf,
                                      &storage[0], &storage[1]);
            dx = SkFixedToScalar(storage[0]);
            dy = SkFixedToScalar(storage[1]);
        } else {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
            dx = matrix.getScaleX();
            dy = matrix.getSkewY();
        }

        for (; count > 0; --count) {
            *dstC++ = cache[toggle + SkATan2_255(fy, fx)];
            fx += dx;
            fy += dy;
            toggle = next_dither_toggle(toggle);
        }
    } else {  // perspective case
        for (int stop = x + count; x < stop; x++) {
            proc(matrix, SkIntToScalar(x) + SK_ScalarHalf,
                         SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
            *dstC++ = cache[toggle + SkATan2_255(srcPt.fY, srcPt.fX)];
            toggle = next_dither_toggle(toggle);
        }
    }
}

void SkSweepGradient::shadeSpan16(int x, int y, uint16_t* SK_RESTRICT dstC,
                                 int count) {
    SkMatrix::MapXYProc proc = fDstToIndexProc;
    const SkMatrix&     matrix = fDstToIndex;
    const uint16_t* SK_RESTRICT cache = this->getCache16();
    int                 toggle = init_dither_toggle16(x, y);
    SkPoint             srcPt;

    if (fDstToIndexClass != kPerspective_MatrixClass) {
        proc(matrix, SkIntToScalar(x) + SK_ScalarHalf,
                     SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
        SkScalar dx, fx = srcPt.fX;
        SkScalar dy, fy = srcPt.fY;

        if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
            SkFixed storage[2];
            (void)matrix.fixedStepInX(SkIntToScalar(y) + SK_ScalarHalf,
                                      &storage[0], &storage[1]);
            dx = SkFixedToScalar(storage[0]);
            dy = SkFixedToScalar(storage[1]);
        } else {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
            dx = matrix.getScaleX();
            dy = matrix.getSkewY();
        }

        for (; count > 0; --count) {
            int index = SkATan2_255(fy, fx) >> (8 - kCache16Bits);
            *dstC++ = cache[toggle + index];
            toggle = next_dither_toggle16(toggle);
            fx += dx;
            fy += dy;
        }
    } else {  // perspective case
        for (int stop = x + count; x < stop; x++) {
            proc(matrix, SkIntToScalar(x) + SK_ScalarHalf,
                         SkIntToScalar(y) + SK_ScalarHalf, &srcPt);

            int index = SkATan2_255(srcPt.fY, srcPt.fX);
            index >>= (8 - kCache16Bits);
            *dstC++ = cache[toggle + index];
            toggle = next_dither_toggle16(toggle);
        }
    }
}

/////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrTBackendEffectFactory.h"

class GrGLSweepGradient : public GrGLGradientEffect {
public:

    GrGLSweepGradient(const GrBackendEffectFactory& factory,
                      const GrDrawEffect&) : INHERITED (factory) { }
    virtual ~GrGLSweepGradient() { }

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          EffectKey,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static EffectKey GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
        return GenBaseGradientKey(drawEffect);
    }

private:

    typedef GrGLGradientEffect INHERITED;

};

/////////////////////////////////////////////////////////////////////

class GrSweepGradient : public GrGradientEffect {
public:
    static GrEffectRef* Create(GrContext* ctx,
                               const SkSweepGradient& shader,
                               const SkMatrix& matrix) {
        AutoEffectUnref effect(SkNEW_ARGS(GrSweepGradient, (ctx, shader, matrix)));
        return CreateEffectRef(effect);
    }
    virtual ~GrSweepGradient() { }

    static const char* Name() { return "Sweep Gradient"; }
    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<GrSweepGradient>::getInstance();
    }

    typedef GrGLSweepGradient GLEffect;

private:
    GrSweepGradient(GrContext* ctx,
                    const SkSweepGradient& shader,
                    const SkMatrix& matrix)
    : INHERITED(ctx, shader, matrix, SkShader::kClamp_TileMode) { }
    GR_DECLARE_EFFECT_TEST;

    typedef GrGradientEffect INHERITED;
};

/////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrSweepGradient);

GrEffectRef* GrSweepGradient::TestCreate(SkRandom* random,
                                         GrContext* context,
                                         const GrDrawTargetCaps&,
                                         GrTexture**) {
    SkPoint center = {random->nextUScalar1(), random->nextUScalar1()};

    SkColor colors[kMaxRandomGradientColors];
    SkScalar stopsArray[kMaxRandomGradientColors];
    SkScalar* stops = stopsArray;
    SkShader::TileMode tmIgnored;
    int colorCount = RandomGradientParams(random, colors, &stops, &tmIgnored);
    SkAutoTUnref<SkShader> shader(SkGradientShader::CreateSweep(center.fX, center.fY,
                                                                colors, stops, colorCount));
    SkPaint paint;
    return shader->asNewEffect(context, paint);
}

/////////////////////////////////////////////////////////////////////

void GrGLSweepGradient::emitCode(GrGLShaderBuilder* builder,
                                 const GrDrawEffect&,
                                 EffectKey key,
                                 const char* outputColor,
                                 const char* inputColor,
                                 const TransformedCoordsArray& coords,
                                 const TextureSamplerArray& samplers) {
    this->emitUniforms(builder, key);
    SkString coords2D = builder->ensureFSCoords2D(coords, 0);
    SkString t;
    t.printf("atan(- %s.y, - %s.x) * 0.1591549430918 + 0.5", coords2D.c_str(), coords2D.c_str());
    this->emitColor(builder, t.c_str(), key,
                          outputColor, inputColor, samplers);
}

/////////////////////////////////////////////////////////////////////

GrEffectRef* SkSweepGradient::asNewEffect(GrContext* context, const SkPaint&) const {
    SkMatrix matrix;
    if (!this->getLocalMatrix().invert(&matrix)) {
        return NULL;
    }
    matrix.postConcat(fPtsToUnit);
    return GrSweepGradient::Create(context, *this, matrix);
}

#else

GrEffectRef* SkSweepGradient::asNewEffect(GrContext*, const SkPaint&) const {
    SkDEBUGFAIL("Should not call in GPU-less build");
    return NULL;
}

#endif

#ifdef SK_DEVELOPER
void SkSweepGradient::toString(SkString* str) const {
    str->append("SkSweepGradient: (");

    str->append("center: (");
    str->appendScalar(fCenter.fX);
    str->append(", ");
    str->appendScalar(fCenter.fY);
    str->append(") ");

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
