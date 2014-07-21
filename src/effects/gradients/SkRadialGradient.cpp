
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRadialGradient.h"
#include "SkRadialGradient_Table.h"

#define kSQRT_TABLE_BITS    11
#define kSQRT_TABLE_SIZE    (1 << kSQRT_TABLE_BITS)

#if 0

#include <stdio.h>

void SkRadialGradient_BuildTable() {
    // build it 0..127 x 0..127, so we use 2^15 - 1 in the numerator for our "fixed" table

    FILE* file = ::fopen("SkRadialGradient_Table.h", "w");
    SkASSERT(file);
    ::fprintf(file, "static const uint8_t gSqrt8Table[] = {\n");

    for (int i = 0; i < kSQRT_TABLE_SIZE; i++) {
        if ((i & 15) == 0) {
            ::fprintf(file, "\t");
        }

        uint8_t value = SkToU8(SkFixedSqrt(i * SK_Fixed1 / kSQRT_TABLE_SIZE) >> 8);

        ::fprintf(file, "0x%02X", value);
        if (i < kSQRT_TABLE_SIZE-1) {
            ::fprintf(file, ", ");
        }
        if ((i & 15) == 15) {
            ::fprintf(file, "\n");
        }
    }
    ::fprintf(file, "};\n");
    ::fclose(file);
}

#endif

namespace {

// GCC doesn't like using static functions as template arguments.  So force these to be non-static.
inline SkFixed mirror_tileproc_nonstatic(SkFixed x) {
    return mirror_tileproc(x);
}

inline SkFixed repeat_tileproc_nonstatic(SkFixed x) {
    return repeat_tileproc(x);
}

void rad_to_unit_matrix(const SkPoint& center, SkScalar radius,
                               SkMatrix* matrix) {
    SkScalar    inv = SkScalarInvert(radius);

    matrix->setTranslate(-center.fX, -center.fY);
    matrix->postScale(inv, inv);
}

typedef void (* RadialShade16Proc)(SkScalar sfx, SkScalar sdx,
        SkScalar sfy, SkScalar sdy,
        uint16_t* dstC, const uint16_t* cache,
        int toggle, int count);

void shadeSpan16_radial_clamp(SkScalar sfx, SkScalar sdx,
        SkScalar sfy, SkScalar sdy,
        uint16_t* SK_RESTRICT dstC, const uint16_t* SK_RESTRICT cache,
        int toggle, int count) {
    const uint8_t* SK_RESTRICT sqrt_table = gSqrt8Table;

    /* knock these down so we can pin against +- 0x7FFF, which is an
       immediate load, rather than 0xFFFF which is slower. This is a
       compromise, since it reduces our precision, but that appears
       to be visually OK. If we decide this is OK for all of our cases,
       we could (it seems) put this scale-down into fDstToIndex,
       to avoid having to do these extra shifts each time.
    */
    SkFixed fx = SkScalarToFixed(sfx) >> 1;
    SkFixed dx = SkScalarToFixed(sdx) >> 1;
    SkFixed fy = SkScalarToFixed(sfy) >> 1;
    SkFixed dy = SkScalarToFixed(sdy) >> 1;
    // might perform this check for the other modes,
    // but the win will be a smaller % of the total
    if (dy == 0) {
        fy = SkPin32(fy, -0xFFFF >> 1, 0xFFFF >> 1);
        fy *= fy;
        do {
            unsigned xx = SkPin32(fx, -0xFFFF >> 1, 0xFFFF >> 1);
            unsigned fi = (xx * xx + fy) >> (14 + 16 - kSQRT_TABLE_BITS);
            fi = SkFastMin32(fi, 0xFFFF >> (16 - kSQRT_TABLE_BITS));
            fx += dx;
            *dstC++ = cache[toggle +
                            (sqrt_table[fi] >> SkGradientShaderBase::kSqrt16Shift)];
            toggle = next_dither_toggle16(toggle);
        } while (--count != 0);
    } else {
        do {
            unsigned xx = SkPin32(fx, -0xFFFF >> 1, 0xFFFF >> 1);
            unsigned fi = SkPin32(fy, -0xFFFF >> 1, 0xFFFF >> 1);
            fi = (xx * xx + fi * fi) >> (14 + 16 - kSQRT_TABLE_BITS);
            fi = SkFastMin32(fi, 0xFFFF >> (16 - kSQRT_TABLE_BITS));
            fx += dx;
            fy += dy;
            *dstC++ = cache[toggle +
                            (sqrt_table[fi] >> SkGradientShaderBase::kSqrt16Shift)];
            toggle = next_dither_toggle16(toggle);
        } while (--count != 0);
    }
}

template <SkFixed (*TileProc)(SkFixed)>
void shadeSpan16_radial(SkScalar fx, SkScalar dx, SkScalar fy, SkScalar dy,
                        uint16_t* SK_RESTRICT dstC, const uint16_t* SK_RESTRICT cache,
                        int toggle, int count) {
    do {
        const SkFixed dist = SkFloatToFixed(sk_float_sqrt(fx*fx + fy*fy));
        const unsigned fi = TileProc(dist);
        SkASSERT(fi <= 0xFFFF);
        *dstC++ = cache[toggle + (fi >> SkGradientShaderBase::kCache16Shift)];
        toggle = next_dither_toggle16(toggle);
        fx += dx;
        fy += dy;
    } while (--count != 0);
}

void shadeSpan16_radial_mirror(SkScalar fx, SkScalar dx, SkScalar fy, SkScalar dy,
                               uint16_t* SK_RESTRICT dstC, const uint16_t* SK_RESTRICT cache,
                               int toggle, int count) {
    shadeSpan16_radial<mirror_tileproc_nonstatic>(fx, dx, fy, dy, dstC, cache, toggle, count);
}

void shadeSpan16_radial_repeat(SkScalar fx, SkScalar dx, SkScalar fy, SkScalar dy,
                               uint16_t* SK_RESTRICT dstC, const uint16_t* SK_RESTRICT cache,
                               int toggle, int count) {
    shadeSpan16_radial<repeat_tileproc_nonstatic>(fx, dx, fy, dy, dstC, cache, toggle, count);
}

}  // namespace

/////////////////////////////////////////////////////////////////////

SkRadialGradient::SkRadialGradient(const SkPoint& center, SkScalar radius,
                                   const Descriptor& desc, const SkMatrix* localMatrix)
    : SkGradientShaderBase(desc, localMatrix),
      fCenter(center),
      fRadius(radius)
{
    // make sure our table is insync with our current #define for kSQRT_TABLE_SIZE
    SkASSERT(sizeof(gSqrt8Table) == kSQRT_TABLE_SIZE);

    rad_to_unit_matrix(center, radius, &fPtsToUnit);
}

size_t SkRadialGradient::contextSize() const {
    return sizeof(RadialGradientContext);
}

SkShader::Context* SkRadialGradient::onCreateContext(const ContextRec& rec, void* storage) const {
    return SkNEW_PLACEMENT_ARGS(storage, RadialGradientContext, (*this, rec));
}

SkRadialGradient::RadialGradientContext::RadialGradientContext(
        const SkRadialGradient& shader, const ContextRec& rec)
    : INHERITED(shader, rec) {}

void SkRadialGradient::RadialGradientContext::shadeSpan16(int x, int y, uint16_t* dstCParam,
                                                          int count) {
    SkASSERT(count > 0);

    const SkRadialGradient& radialGradient = static_cast<const SkRadialGradient&>(fShader);

    uint16_t* SK_RESTRICT dstC = dstCParam;

    SkPoint             srcPt;
    SkMatrix::MapXYProc dstProc = fDstToIndexProc;
    TileProc            proc = radialGradient.fTileProc;
    const uint16_t* SK_RESTRICT cache = fCache->getCache16();
    int                 toggle = init_dither_toggle16(x, y);

    if (fDstToIndexClass != kPerspective_MatrixClass) {
        dstProc(fDstToIndex, SkIntToScalar(x) + SK_ScalarHalf,
                             SkIntToScalar(y) + SK_ScalarHalf, &srcPt);

        SkScalar sdx = fDstToIndex.getScaleX();
        SkScalar sdy = fDstToIndex.getSkewY();

        if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
            SkFixed storage[2];
            (void)fDstToIndex.fixedStepInX(SkIntToScalar(y),
                                           &storage[0], &storage[1]);
            sdx = SkFixedToScalar(storage[0]);
            sdy = SkFixedToScalar(storage[1]);
        } else {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
        }

        RadialShade16Proc shadeProc = shadeSpan16_radial_repeat;
        if (SkShader::kClamp_TileMode == radialGradient.fTileMode) {
            shadeProc = shadeSpan16_radial_clamp;
        } else if (SkShader::kMirror_TileMode == radialGradient.fTileMode) {
            shadeProc = shadeSpan16_radial_mirror;
        } else {
            SkASSERT(SkShader::kRepeat_TileMode == radialGradient.fTileMode);
        }
        (*shadeProc)(srcPt.fX, sdx, srcPt.fY, sdy, dstC,
                     cache, toggle, count);
    } else {    // perspective case
        SkScalar dstX = SkIntToScalar(x);
        SkScalar dstY = SkIntToScalar(y);
        do {
            dstProc(fDstToIndex, dstX, dstY, &srcPt);
            unsigned fi = proc(SkScalarToFixed(srcPt.length()));
            SkASSERT(fi <= 0xFFFF);

            int index = fi >> (16 - kCache16Bits);
            *dstC++ = cache[toggle + index];
            toggle = next_dither_toggle16(toggle);

            dstX += SK_Scalar1;
        } while (--count != 0);
    }
}

SkShader::BitmapType SkRadialGradient::asABitmap(SkBitmap* bitmap,
    SkMatrix* matrix, SkShader::TileMode* xy) const {
    if (bitmap) {
        this->getGradientTableBitmap(bitmap);
    }
    if (matrix) {
        matrix->setScale(SkIntToScalar(kCache32Count),
                         SkIntToScalar(kCache32Count));
        matrix->preConcat(fPtsToUnit);
    }
    if (xy) {
        xy[0] = fTileMode;
        xy[1] = kClamp_TileMode;
    }
    return kRadial_BitmapType;
}

SkShader::GradientType SkRadialGradient::asAGradient(GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fCenter;
        info->fRadius[0] = fRadius;
    }
    return kRadial_GradientType;
}

SkRadialGradient::SkRadialGradient(SkReadBuffer& buffer)
    : INHERITED(buffer),
      fCenter(buffer.readPoint()),
      fRadius(buffer.readScalar()) {
}

void SkRadialGradient::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fCenter);
    buffer.writeScalar(fRadius);
}

namespace {

inline bool radial_completely_pinned(int fx, int dx, int fy, int dy) {
    // fast, overly-conservative test: checks unit square instead
    // of unit circle
    bool xClamped = (fx >= SK_FixedHalf && dx >= 0) ||
                    (fx <= -SK_FixedHalf && dx <= 0);
    bool yClamped = (fy >= SK_FixedHalf && dy >= 0) ||
                    (fy <= -SK_FixedHalf && dy <= 0);

    return xClamped || yClamped;
}

// Return true if (fx * fy) is always inside the unit circle
// SkPin32 is expensive, but so are all the SkFixedMul in this test,
// so it shouldn't be run if count is small.
inline bool no_need_for_radial_pin(int fx, int dx,
                                          int fy, int dy, int count) {
    SkASSERT(count > 0);
    if (SkAbs32(fx) > 0x7FFF || SkAbs32(fy) > 0x7FFF) {
        return false;
    }
    if (fx*fx + fy*fy > 0x7FFF*0x7FFF) {
        return false;
    }
    fx += (count - 1) * dx;
    fy += (count - 1) * dy;
    if (SkAbs32(fx) > 0x7FFF || SkAbs32(fy) > 0x7FFF) {
        return false;
    }
    return fx*fx + fy*fy <= 0x7FFF*0x7FFF;
}

#define UNPINNED_RADIAL_STEP \
    fi = (fx * fx + fy * fy) >> (14 + 16 - kSQRT_TABLE_BITS); \
    *dstC++ = cache[toggle + \
                    (sqrt_table[fi] >> SkGradientShaderBase::kSqrt32Shift)]; \
    toggle = next_dither_toggle(toggle); \
    fx += dx; \
    fy += dy;

typedef void (* RadialShadeProc)(SkScalar sfx, SkScalar sdx,
        SkScalar sfy, SkScalar sdy,
        SkPMColor* dstC, const SkPMColor* cache,
        int count, int toggle);

// On Linux, this is faster with SkPMColor[] params than SkPMColor* SK_RESTRICT
void shadeSpan_radial_clamp(SkScalar sfx, SkScalar sdx,
        SkScalar sfy, SkScalar sdy,
        SkPMColor* SK_RESTRICT dstC, const SkPMColor* SK_RESTRICT cache,
        int count, int toggle) {
    // Floating point seems to be slower than fixed point,
    // even when we have float hardware.
    const uint8_t* SK_RESTRICT sqrt_table = gSqrt8Table;
    SkFixed fx = SkScalarToFixed(sfx) >> 1;
    SkFixed dx = SkScalarToFixed(sdx) >> 1;
    SkFixed fy = SkScalarToFixed(sfy) >> 1;
    SkFixed dy = SkScalarToFixed(sdy) >> 1;
    if ((count > 4) && radial_completely_pinned(fx, dx, fy, dy)) {
        unsigned fi = SkGradientShaderBase::kCache32Count - 1;
        sk_memset32_dither(dstC,
            cache[toggle + fi],
            cache[next_dither_toggle(toggle) + fi],
            count);
    } else if ((count > 4) &&
               no_need_for_radial_pin(fx, dx, fy, dy, count)) {
        unsigned fi;
        // 4x unroll appears to be no faster than 2x unroll on Linux
        while (count > 1) {
            UNPINNED_RADIAL_STEP;
            UNPINNED_RADIAL_STEP;
            count -= 2;
        }
        if (count) {
            UNPINNED_RADIAL_STEP;
        }
    } else  {
        // Specializing for dy == 0 gains us 25% on Skia benchmarks
        if (dy == 0) {
            unsigned yy = SkPin32(fy, -0xFFFF >> 1, 0xFFFF >> 1);
            yy *= yy;
            do {
                unsigned xx = SkPin32(fx, -0xFFFF >> 1, 0xFFFF >> 1);
                unsigned fi = (xx * xx + yy) >> (14 + 16 - kSQRT_TABLE_BITS);
                fi = SkFastMin32(fi, 0xFFFF >> (16 - kSQRT_TABLE_BITS));
                *dstC++ = cache[toggle + (sqrt_table[fi] >>
                    SkGradientShaderBase::kSqrt32Shift)];
                toggle = next_dither_toggle(toggle);
                fx += dx;
            } while (--count != 0);
        } else {
            do {
                unsigned xx = SkPin32(fx, -0xFFFF >> 1, 0xFFFF >> 1);
                unsigned fi = SkPin32(fy, -0xFFFF >> 1, 0xFFFF >> 1);
                fi = (xx * xx + fi * fi) >> (14 + 16 - kSQRT_TABLE_BITS);
                fi = SkFastMin32(fi, 0xFFFF >> (16 - kSQRT_TABLE_BITS));
                *dstC++ = cache[toggle + (sqrt_table[fi] >>
                    SkGradientShaderBase::kSqrt32Shift)];
                toggle = next_dither_toggle(toggle);
                fx += dx;
                fy += dy;
            } while (--count != 0);
        }
    }
}

// Unrolling this loop doesn't seem to help (when float); we're stalling to
// get the results of the sqrt (?), and don't have enough extra registers to
// have many in flight.
template <SkFixed (*TileProc)(SkFixed)>
void shadeSpan_radial(SkScalar fx, SkScalar dx, SkScalar fy, SkScalar dy,
                      SkPMColor* SK_RESTRICT dstC, const SkPMColor* SK_RESTRICT cache,
                      int count, int toggle) {
    do {
        const SkFixed dist = SkFloatToFixed(sk_float_sqrt(fx*fx + fy*fy));
        const unsigned fi = TileProc(dist);
        SkASSERT(fi <= 0xFFFF);
        *dstC++ = cache[toggle + (fi >> SkGradientShaderBase::kCache32Shift)];
        toggle = next_dither_toggle(toggle);
        fx += dx;
        fy += dy;
    } while (--count != 0);
}

void shadeSpan_radial_mirror(SkScalar fx, SkScalar dx, SkScalar fy, SkScalar dy,
                             SkPMColor* SK_RESTRICT dstC, const SkPMColor* SK_RESTRICT cache,
                             int count, int toggle) {
    shadeSpan_radial<mirror_tileproc_nonstatic>(fx, dx, fy, dy, dstC, cache, count, toggle);
}

void shadeSpan_radial_repeat(SkScalar fx, SkScalar dx, SkScalar fy, SkScalar dy,
                             SkPMColor* SK_RESTRICT dstC, const SkPMColor* SK_RESTRICT cache,
                             int count, int toggle) {
    shadeSpan_radial<repeat_tileproc_nonstatic>(fx, dx, fy, dy, dstC, cache, count, toggle);
}

}  // namespace

void SkRadialGradient::RadialGradientContext::shadeSpan(int x, int y,
                                                        SkPMColor* SK_RESTRICT dstC, int count) {
    SkASSERT(count > 0);

    const SkRadialGradient& radialGradient = static_cast<const SkRadialGradient&>(fShader);

    SkPoint             srcPt;
    SkMatrix::MapXYProc dstProc = fDstToIndexProc;
    TileProc            proc = radialGradient.fTileProc;
    const SkPMColor* SK_RESTRICT cache = fCache->getCache32();
    int toggle = init_dither_toggle(x, y);

    if (fDstToIndexClass != kPerspective_MatrixClass) {
        dstProc(fDstToIndex, SkIntToScalar(x) + SK_ScalarHalf,
                             SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
        SkScalar sdx = fDstToIndex.getScaleX();
        SkScalar sdy = fDstToIndex.getSkewY();

        if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
            SkFixed storage[2];
            (void)fDstToIndex.fixedStepInX(SkIntToScalar(y),
                                           &storage[0], &storage[1]);
            sdx = SkFixedToScalar(storage[0]);
            sdy = SkFixedToScalar(storage[1]);
        } else {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
        }

        RadialShadeProc shadeProc = shadeSpan_radial_repeat;
        if (SkShader::kClamp_TileMode == radialGradient.fTileMode) {
            shadeProc = shadeSpan_radial_clamp;
        } else if (SkShader::kMirror_TileMode == radialGradient.fTileMode) {
            shadeProc = shadeSpan_radial_mirror;
        } else {
            SkASSERT(SkShader::kRepeat_TileMode == radialGradient.fTileMode);
        }
        (*shadeProc)(srcPt.fX, sdx, srcPt.fY, sdy, dstC, cache, count, toggle);
    } else {    // perspective case
        SkScalar dstX = SkIntToScalar(x);
        SkScalar dstY = SkIntToScalar(y);
        do {
            dstProc(fDstToIndex, dstX, dstY, &srcPt);
            unsigned fi = proc(SkScalarToFixed(srcPt.length()));
            SkASSERT(fi <= 0xFFFF);
            *dstC++ = cache[fi >> SkGradientShaderBase::kCache32Shift];
            dstX += SK_Scalar1;
        } while (--count != 0);
    }
}

/////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrTBackendEffectFactory.h"
#include "gl/GrGLShaderBuilder.h"
#include "SkGr.h"

class GrGLRadialGradient : public GrGLGradientEffect {
public:

    GrGLRadialGradient(const GrBackendEffectFactory& factory,
                       const GrDrawEffect&) : INHERITED (factory) { }
    virtual ~GrGLRadialGradient() { }

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          const GrEffectKey&,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static void GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&, GrEffectKeyBuilder* b) {
        b->add32(GenBaseGradientKey(drawEffect));
    }

private:

    typedef GrGLGradientEffect INHERITED;

};

/////////////////////////////////////////////////////////////////////

class GrRadialGradient : public GrGradientEffect {
public:
    static GrEffect* Create(GrContext* ctx,
                            const SkRadialGradient& shader,
                            const SkMatrix& matrix,
                            SkShader::TileMode tm) {
        return SkNEW_ARGS(GrRadialGradient, (ctx, shader, matrix, tm));
    }

    virtual ~GrRadialGradient() { }

    static const char* Name() { return "Radial Gradient"; }
    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<GrRadialGradient>::getInstance();
    }

    typedef GrGLRadialGradient GLEffect;

private:
    GrRadialGradient(GrContext* ctx,
                     const SkRadialGradient& shader,
                     const SkMatrix& matrix,
                     SkShader::TileMode tm)
        : INHERITED(ctx, shader, matrix, tm) {
    }

    GR_DECLARE_EFFECT_TEST;

    typedef GrGradientEffect INHERITED;
};

/////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrRadialGradient);

GrEffect* GrRadialGradient::TestCreate(SkRandom* random,
                                       GrContext* context,
                                       const GrDrawTargetCaps&,
                                       GrTexture**) {
    SkPoint center = {random->nextUScalar1(), random->nextUScalar1()};
    SkScalar radius = random->nextUScalar1();

    SkColor colors[kMaxRandomGradientColors];
    SkScalar stopsArray[kMaxRandomGradientColors];
    SkScalar* stops = stopsArray;
    SkShader::TileMode tm;
    int colorCount = RandomGradientParams(random, colors, &stops, &tm);
    SkAutoTUnref<SkShader> shader(SkGradientShader::CreateRadial(center, radius,
                                                                 colors, stops, colorCount,
                                                                 tm));
    SkPaint paint;
    GrColor paintColor;
    GrEffect* effect;
    SkAssertResult(shader->asNewEffect(context, paint, NULL, &paintColor, &effect));
    return effect;
}

/////////////////////////////////////////////////////////////////////

void GrGLRadialGradient::emitCode(GrGLShaderBuilder* builder,
                                  const GrDrawEffect&,
                                  const GrEffectKey& key,
                                  const char* outputColor,
                                  const char* inputColor,
                                  const TransformedCoordsArray& coords,
                                  const TextureSamplerArray& samplers) {
    uint32_t baseKey = key.get32(0);
    this->emitUniforms(builder, baseKey);
    SkString t("length(");
    t.append(builder->ensureFSCoords2D(coords, 0));
    t.append(")");
    this->emitColor(builder, t.c_str(), baseKey, outputColor, inputColor, samplers);
}

/////////////////////////////////////////////////////////////////////

bool SkRadialGradient::asNewEffect(GrContext* context, const SkPaint& paint,
                                   const SkMatrix* localMatrix, GrColor* paintColor,
                                   GrEffect** effect) const {
    SkASSERT(NULL != context);
    
    SkMatrix matrix;
    if (!this->getLocalMatrix().invert(&matrix)) {
        return false;
    }
    if (localMatrix) {
        SkMatrix inv;
        if (!localMatrix->invert(&inv)) {
            return false;
        }
        matrix.postConcat(inv);
    }
    matrix.postConcat(fPtsToUnit);
    
    *paintColor = SkColor2GrColorJustAlpha(paint.getColor());
    *effect = GrRadialGradient::Create(context, *this, matrix, fTileMode);
    
    return true;
}

#else

bool SkRadialGradient::asNewEffect(GrContext* context, const SkPaint& paint,
                                   const SkMatrix* localMatrix, GrColor* paintColor,
                                   GrEffect** effect) const {
    SkDEBUGFAIL("Should not call in GPU-less build");
    return false;
}

#endif

#ifndef SK_IGNORE_TO_STRING
void SkRadialGradient::toString(SkString* str) const {
    str->append("SkRadialGradient: (");

    str->append("center: (");
    str->appendScalar(fCenter.fX);
    str->append(", ");
    str->appendScalar(fCenter.fY);
    str->append(") radius: ");
    str->appendScalar(fRadius);
    str->append(" ");

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
