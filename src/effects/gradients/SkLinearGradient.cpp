/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLinearGradient.h"

static inline int repeat_bits(int x, const int bits) {
    return x & ((1 << bits) - 1);
}

static inline int repeat_8bits(int x) {
    return x & 0xFF;
}

// Visual Studio 2010 (MSC_VER=1600) optimizes bit-shift code incorrectly.
// See http://code.google.com/p/skia/issues/detail?id=472
#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#pragma optimize("", off)
#endif

static inline int mirror_bits(int x, const int bits) {
    if (x & (1 << bits)) {
        x = ~x;
    }
    return x & ((1 << bits) - 1);
}

static inline int mirror_8bits(int x) {
    if (x & 256) {
        x = ~x;
    }
    return x & 255;
}

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#pragma optimize("", on)
#endif

static SkMatrix pts_to_unit_matrix(const SkPoint pts[2]) {
    SkVector    vec = pts[1] - pts[0];
    SkScalar    mag = vec.length();
    SkScalar    inv = mag ? SkScalarInvert(mag) : 0;

    vec.scale(inv);
    SkMatrix matrix;
    matrix.setSinCos(-vec.fY, vec.fX, pts[0].fX, pts[0].fY);
    matrix.postTranslate(-pts[0].fX, -pts[0].fY);
    matrix.postScale(inv, inv);
    return matrix;
}

///////////////////////////////////////////////////////////////////////////////

SkLinearGradient::SkLinearGradient(const SkPoint pts[2], const Descriptor& desc)
    : SkGradientShaderBase(desc, pts_to_unit_matrix(pts))
    , fStart(pts[0])
    , fEnd(pts[1]) {
}

SkFlattenable* SkLinearGradient::CreateProc(SkReadBuffer& buffer) {
    DescriptorScope desc;
    if (!desc.unflatten(buffer)) {
        return NULL;
    }
    SkPoint pts[2];
    pts[0] = buffer.readPoint();
    pts[1] = buffer.readPoint();
    return SkGradientShader::CreateLinear(pts, desc.fColors, desc.fPos, desc.fCount,
                                          desc.fTileMode, desc.fGradFlags, desc.fLocalMatrix);
}

void SkLinearGradient::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fStart);
    buffer.writePoint(fEnd);
}

size_t SkLinearGradient::contextSize() const {
    return sizeof(LinearGradientContext);
}

SkShader::Context* SkLinearGradient::onCreateContext(const ContextRec& rec, void* storage) const {
    return SkNEW_PLACEMENT_ARGS(storage, LinearGradientContext, (*this, rec));
}

SkLinearGradient::LinearGradientContext::LinearGradientContext(
        const SkLinearGradient& shader, const ContextRec& rec)
    : INHERITED(shader, rec)
{
    unsigned mask = SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask;
    if ((fDstToIndex.getType() & ~mask) == 0) {
        // when we dither, we are (usually) not const-in-Y
        if ((fFlags & SkShader::kHasSpan16_Flag) && !rec.fPaint->isDither()) {
            // only claim this if we do have a 16bit mode (i.e. none of our
            // colors have alpha), and if we are not dithering (which obviously
            // is not const in Y).
            fFlags |= SkShader::kConstInY16_Flag;
        }
    }
}

#define NO_CHECK_ITER               \
    do {                            \
    unsigned fi = SkGradFixedToFixed(fx) >> SkGradientShaderBase::kCache32Shift; \
    SkASSERT(fi <= 0xFF);           \
    fx += dx;                       \
    *dstC++ = cache[toggle + fi];   \
    toggle = next_dither_toggle(toggle); \
    } while (0)

namespace {

typedef void (*LinearShadeProc)(TileProc proc, SkGradFixed dx, SkGradFixed fx,
                                SkPMColor* dstC, const SkPMColor* cache,
                                int toggle, int count);

// Linear interpolation (lerp) is unnecessary if there are no sharp
// discontinuities in the gradient - which must be true if there are
// only 2 colors - but it's cheap.
void shadeSpan_linear_vertical_lerp(TileProc proc, SkGradFixed dx, SkGradFixed fx,
                                    SkPMColor* SK_RESTRICT dstC,
                                    const SkPMColor* SK_RESTRICT cache,
                                    int toggle, int count) {
    // We're a vertical gradient, so no change in a span.
    // If colors change sharply across the gradient, dithering is
    // insufficient (it subsamples the color space) and we need to lerp.
    unsigned fullIndex = proc(SkGradFixedToFixed(fx));
    unsigned fi = fullIndex >> SkGradientShaderBase::kCache32Shift;
    unsigned remainder = fullIndex & ((1 << SkGradientShaderBase::kCache32Shift) - 1);

    int index0 = fi + toggle;
    int index1 = index0;
    if (fi < SkGradientShaderBase::kCache32Count - 1) {
        index1 += 1;
    }
    SkPMColor lerp = SkFastFourByteInterp(cache[index1], cache[index0], remainder);
    index0 ^= SkGradientShaderBase::kDitherStride32;
    index1 ^= SkGradientShaderBase::kDitherStride32;
    SkPMColor dlerp = SkFastFourByteInterp(cache[index1], cache[index0], remainder);
    sk_memset32_dither(dstC, lerp, dlerp, count);
}

void shadeSpan_linear_clamp(TileProc proc, SkGradFixed dx, SkGradFixed fx,
                            SkPMColor* SK_RESTRICT dstC,
                            const SkPMColor* SK_RESTRICT cache,
                            int toggle, int count) {
    SkClampRange range;
    range.init(fx, dx, count, 0, SkGradientShaderBase::kCache32Count - 1);
    range.validate(count);

    if ((count = range.fCount0) > 0) {
        sk_memset32_dither(dstC,
            cache[toggle + range.fV0],
            cache[next_dither_toggle(toggle) + range.fV0],
            count);
        dstC += count;
    }
    if ((count = range.fCount1) > 0) {
        int unroll = count >> 3;
        fx = range.fFx1;
        for (int i = 0; i < unroll; i++) {
            NO_CHECK_ITER;  NO_CHECK_ITER;
            NO_CHECK_ITER;  NO_CHECK_ITER;
            NO_CHECK_ITER;  NO_CHECK_ITER;
            NO_CHECK_ITER;  NO_CHECK_ITER;
        }
        if ((count &= 7) > 0) {
            do {
                NO_CHECK_ITER;
            } while (--count != 0);
        }
    }
    if ((count = range.fCount2) > 0) {
        sk_memset32_dither(dstC,
            cache[toggle + range.fV1],
            cache[next_dither_toggle(toggle) + range.fV1],
            count);
    }
}

void shadeSpan_linear_mirror(TileProc proc, SkGradFixed dx, SkGradFixed fx,
                             SkPMColor* SK_RESTRICT dstC,
                             const SkPMColor* SK_RESTRICT cache,
                             int toggle, int count) {
    do {
        unsigned fi = mirror_8bits(SkGradFixedToFixed(fx) >> 8);
        SkASSERT(fi <= 0xFF);
        fx += dx;
        *dstC++ = cache[toggle + fi];
        toggle = next_dither_toggle(toggle);
    } while (--count != 0);
}

void shadeSpan_linear_repeat(TileProc proc, SkGradFixed dx, SkGradFixed fx,
        SkPMColor* SK_RESTRICT dstC,
        const SkPMColor* SK_RESTRICT cache,
        int toggle, int count) {
    do {
        unsigned fi = repeat_8bits(SkGradFixedToFixed(fx) >> 8);
        SkASSERT(fi <= 0xFF);
        fx += dx;
        *dstC++ = cache[toggle + fi];
        toggle = next_dither_toggle(toggle);
    } while (--count != 0);
}

}

void SkLinearGradient::LinearGradientContext::shadeSpan(int x, int y, SkPMColor* SK_RESTRICT dstC,
                                                        int count) {
    SkASSERT(count > 0);

    const SkLinearGradient& linearGradient = static_cast<const SkLinearGradient&>(fShader);

    SkPoint             srcPt;
    SkMatrix::MapXYProc dstProc = fDstToIndexProc;
    TileProc            proc = linearGradient.fTileProc;
    const SkPMColor* SK_RESTRICT cache = fCache->getCache32();
    int                 toggle = init_dither_toggle(x, y);

    if (fDstToIndexClass != kPerspective_MatrixClass) {
        dstProc(fDstToIndex, SkIntToScalar(x) + SK_ScalarHalf,
                             SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
        SkGradFixed dx, fx = SkScalarToGradFixed(srcPt.fX);

        if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
            SkFixed dxStorage[1];
            (void)fDstToIndex.fixedStepInX(SkIntToScalar(y), dxStorage, NULL);
            // todo: do we need a real/high-precision value for dx here?
            dx = SkFixedToGradFixed(dxStorage[0]);
        } else {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
            dx = SkScalarToGradFixed(fDstToIndex.getScaleX());
        }

        LinearShadeProc shadeProc = shadeSpan_linear_repeat;
        if (0 == dx) {
            shadeProc = shadeSpan_linear_vertical_lerp;
        } else if (SkShader::kClamp_TileMode == linearGradient.fTileMode) {
            shadeProc = shadeSpan_linear_clamp;
        } else if (SkShader::kMirror_TileMode == linearGradient.fTileMode) {
            shadeProc = shadeSpan_linear_mirror;
        } else {
            SkASSERT(SkShader::kRepeat_TileMode == linearGradient.fTileMode);
        }
        (*shadeProc)(proc, dx, fx, dstC, cache, toggle, count);
    } else {
        SkScalar    dstX = SkIntToScalar(x);
        SkScalar    dstY = SkIntToScalar(y);
        do {
            dstProc(fDstToIndex, dstX, dstY, &srcPt);
            unsigned fi = proc(SkScalarToFixed(srcPt.fX));
            SkASSERT(fi <= 0xFFFF);
            *dstC++ = cache[toggle + (fi >> kCache32Shift)];
            toggle = next_dither_toggle(toggle);
            dstX += SK_Scalar1;
        } while (--count != 0);
    }
}

SkShader::BitmapType SkLinearGradient::asABitmap(SkBitmap* bitmap,
                                                SkMatrix* matrix,
                                                TileMode xy[]) const {
    if (bitmap) {
        this->getGradientTableBitmap(bitmap);
    }
    if (matrix) {
        matrix->preConcat(fPtsToUnit);
    }
    if (xy) {
        xy[0] = fTileMode;
        xy[1] = kClamp_TileMode;
    }
    return kLinear_BitmapType;
}

SkShader::GradientType SkLinearGradient::asAGradient(GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fStart;
        info->fPoint[1] = fEnd;
    }
    return kLinear_GradientType;
}

static void dither_memset16(uint16_t dst[], uint16_t value, uint16_t other,
                            int count) {
    if (reinterpret_cast<uintptr_t>(dst) & 2) {
        *dst++ = value;
        count -= 1;
        SkTSwap(value, other);
    }

    sk_memset32((uint32_t*)dst, (value << 16) | other, count >> 1);

    if (count & 1) {
        dst[count - 1] = value;
    }
}

#define NO_CHECK_ITER_16                \
    do {                                \
    unsigned fi = SkGradFixedToFixed(fx) >> SkGradientShaderBase::kCache16Shift;  \
    SkASSERT(fi < SkGradientShaderBase::kCache16Count);       \
    fx += dx;                           \
    *dstC++ = cache[toggle + fi];       \
    toggle = next_dither_toggle16(toggle);            \
    } while (0)

namespace {

typedef void (*LinearShade16Proc)(TileProc proc, SkGradFixed dx, SkGradFixed fx,
                                  uint16_t* dstC, const uint16_t* cache,
                                  int toggle, int count);

void shadeSpan16_linear_vertical(TileProc proc, SkGradFixed dx, SkGradFixed fx,
                                 uint16_t* SK_RESTRICT dstC,
                                 const uint16_t* SK_RESTRICT cache,
                                 int toggle, int count) {
    // we're a vertical gradient, so no change in a span
    unsigned fi = proc(SkGradFixedToFixed(fx)) >> SkGradientShaderBase::kCache16Shift;
    SkASSERT(fi < SkGradientShaderBase::kCache16Count);
    dither_memset16(dstC, cache[toggle + fi],
        cache[next_dither_toggle16(toggle) + fi], count);
}

void shadeSpan16_linear_clamp(TileProc proc, SkGradFixed dx, SkGradFixed fx,
                              uint16_t* SK_RESTRICT dstC,
                              const uint16_t* SK_RESTRICT cache,
                              int toggle, int count) {
    SkClampRange range;
    range.init(fx, dx, count, 0, SkGradientShaderBase::kCache32Count - 1);
    range.validate(count);

    if ((count = range.fCount0) > 0) {
        dither_memset16(dstC,
            cache[toggle + range.fV0],
            cache[next_dither_toggle16(toggle) + range.fV0],
            count);
        dstC += count;
    }
    if ((count = range.fCount1) > 0) {
        int unroll = count >> 3;
        fx = range.fFx1;
        for (int i = 0; i < unroll; i++) {
            NO_CHECK_ITER_16;  NO_CHECK_ITER_16;
            NO_CHECK_ITER_16;  NO_CHECK_ITER_16;
            NO_CHECK_ITER_16;  NO_CHECK_ITER_16;
            NO_CHECK_ITER_16;  NO_CHECK_ITER_16;
        }
        if ((count &= 7) > 0) {
            do {
                NO_CHECK_ITER_16;
            } while (--count != 0);
        }
    }
    if ((count = range.fCount2) > 0) {
        dither_memset16(dstC,
            cache[toggle + range.fV1],
            cache[next_dither_toggle16(toggle) + range.fV1],
            count);
    }
}

void shadeSpan16_linear_mirror(TileProc proc, SkGradFixed dx, SkGradFixed fx,
                               uint16_t* SK_RESTRICT dstC,
                               const uint16_t* SK_RESTRICT cache,
                               int toggle, int count) {
    do {
        unsigned fi = mirror_bits(SkGradFixedToFixed(fx) >> SkGradientShaderBase::kCache16Shift,
                                        SkGradientShaderBase::kCache16Bits);
        SkASSERT(fi < SkGradientShaderBase::kCache16Count);
        fx += dx;
        *dstC++ = cache[toggle + fi];
        toggle = next_dither_toggle16(toggle);
    } while (--count != 0);
}

void shadeSpan16_linear_repeat(TileProc proc, SkGradFixed dx, SkGradFixed fx,
                               uint16_t* SK_RESTRICT dstC,
                               const uint16_t* SK_RESTRICT cache,
                               int toggle, int count) {
    do {
        unsigned fi = repeat_bits(SkGradFixedToFixed(fx) >> SkGradientShaderBase::kCache16Shift,
                                  SkGradientShaderBase::kCache16Bits);
        SkASSERT(fi < SkGradientShaderBase::kCache16Count);
        fx += dx;
        *dstC++ = cache[toggle + fi];
        toggle = next_dither_toggle16(toggle);
    } while (--count != 0);
}
}

static bool fixed_nearly_zero(SkFixed x) {
    return SkAbs32(x) < (SK_Fixed1 >> 12);
}

void SkLinearGradient::LinearGradientContext::shadeSpan16(int x, int y,
                                                          uint16_t* SK_RESTRICT dstC, int count) {
    SkASSERT(count > 0);

    const SkLinearGradient& linearGradient = static_cast<const SkLinearGradient&>(fShader);

    SkPoint             srcPt;
    SkMatrix::MapXYProc dstProc = fDstToIndexProc;
    TileProc            proc = linearGradient.fTileProc;
    const uint16_t* SK_RESTRICT cache = fCache->getCache16();
    int                 toggle = init_dither_toggle16(x, y);

    if (fDstToIndexClass != kPerspective_MatrixClass) {
        dstProc(fDstToIndex, SkIntToScalar(x) + SK_ScalarHalf,
                             SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
        SkGradFixed dx, fx = SkScalarToGradFixed(srcPt.fX);

        if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
            SkFixed dxStorage[1];
            (void)fDstToIndex.fixedStepInX(SkIntToScalar(y), dxStorage, NULL);
            // todo: do we need a real/high-precision value for dx here?
            dx = SkFixedToGradFixed(dxStorage[0]);
        } else {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
            dx = SkScalarToGradFixed(fDstToIndex.getScaleX());
        }

        LinearShade16Proc shadeProc = shadeSpan16_linear_repeat;
        if (fixed_nearly_zero(SkGradFixedToFixed(dx))) {
            shadeProc = shadeSpan16_linear_vertical;
        } else if (SkShader::kClamp_TileMode == linearGradient.fTileMode) {
            shadeProc = shadeSpan16_linear_clamp;
        } else if (SkShader::kMirror_TileMode == linearGradient.fTileMode) {
            shadeProc = shadeSpan16_linear_mirror;
        } else {
            SkASSERT(SkShader::kRepeat_TileMode == linearGradient.fTileMode);
        }
        (*shadeProc)(proc, dx, fx, dstC, cache, toggle, count);
    } else {
        SkScalar    dstX = SkIntToScalar(x);
        SkScalar    dstY = SkIntToScalar(y);
        do {
            dstProc(fDstToIndex, dstX, dstY, &srcPt);
            unsigned fi = proc(SkScalarToFixed(srcPt.fX));
            SkASSERT(fi <= 0xFFFF);

            int index = fi >> kCache16Shift;
            *dstC++ = cache[toggle + index];
            toggle = next_dither_toggle16(toggle);

            dstX += SK_Scalar1;
        } while (--count != 0);
    }
}

#if SK_SUPPORT_GPU

#include "gl/builders/GrGLProgramBuilder.h"
#include "SkGr.h"

/////////////////////////////////////////////////////////////////////

class GrGLLinearGradient : public GrGLGradientEffect {
public:

    GrGLLinearGradient(const GrProcessor&) {}

    virtual ~GrGLLinearGradient() { }

    virtual void emitCode(EmitArgs&) override;

    static void GenKey(const GrProcessor& processor, const GrGLSLCaps&, GrProcessorKeyBuilder* b) {
        b->add32(GenBaseGradientKey(processor));
    }

private:

    typedef GrGLGradientEffect INHERITED;
};

/////////////////////////////////////////////////////////////////////

class GrLinearGradient : public GrGradientEffect {
public:

    static GrFragmentProcessor* Create(GrContext* ctx,
                                       GrProcessorDataManager* procDataManager,
                                       const SkLinearGradient& shader,
                                       const SkMatrix& matrix,
                                       SkShader::TileMode tm) {
        return SkNEW_ARGS(GrLinearGradient, (ctx, procDataManager, shader, matrix, tm));
    }

    virtual ~GrLinearGradient() { }

    const char* name() const override { return "Linear Gradient"; }

    GrGLFragmentProcessor* createGLInstance() const override {
        return SkNEW_ARGS(GrGLLinearGradient, (*this));
    }

private:
    GrLinearGradient(GrContext* ctx,
                     GrProcessorDataManager* procDataManager,
                     const SkLinearGradient& shader,
                     const SkMatrix& matrix,
                     SkShader::TileMode tm)
        : INHERITED(ctx, procDataManager, shader, matrix, tm) {
        this->initClassID<GrLinearGradient>();
    }

    virtual void onGetGLProcessorKey(const GrGLSLCaps& caps,
                                     GrProcessorKeyBuilder* b) const override {
        GrGLLinearGradient::GenKey(*this, caps, b);
    }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrGradientEffect INHERITED;
};

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrLinearGradient);

GrFragmentProcessor* GrLinearGradient::TestCreate(GrProcessorTestData* d) {
    SkPoint points[] = {{d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()},
                        {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()}};

    SkColor colors[kMaxRandomGradientColors];
    SkScalar stopsArray[kMaxRandomGradientColors];
    SkScalar* stops = stopsArray;
    SkShader::TileMode tm;
    int colorCount = RandomGradientParams(d->fRandom, colors, &stops, &tm);
    SkAutoTUnref<SkShader> shader(SkGradientShader::CreateLinear(points,
                                                                 colors, stops, colorCount,
                                                                 tm));
    SkPaint paint;
    GrColor paintColor;
    GrFragmentProcessor* fp;
    SkAssertResult(shader->asFragmentProcessor(d->fContext, paint,
                                               GrTest::TestMatrix(d->fRandom), NULL,
                                               &paintColor, d->fProcDataManager, &fp));
    return fp;
}

/////////////////////////////////////////////////////////////////////

void GrGLLinearGradient::emitCode(EmitArgs& args) {
    const GrLinearGradient& ge = args.fFp.cast<GrLinearGradient>();
    this->emitUniforms(args.fBuilder, ge);
    SkString t = args.fBuilder->getFragmentShaderBuilder()->ensureFSCoords2D(args.fCoords, 0);
    t.append(".x");
    this->emitColor(args.fBuilder, ge, t.c_str(), args.fOutputColor, args.fInputColor,
                    args.fSamplers);
}

/////////////////////////////////////////////////////////////////////

bool SkLinearGradient::asFragmentProcessor(GrContext* context, const SkPaint& paint,
                                           const SkMatrix& viewm, const SkMatrix* localMatrix,
                                           GrColor* paintColor,
                                           GrProcessorDataManager* procDataManager,
                                           GrFragmentProcessor** fp)  const {
    SkASSERT(context);

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
    *fp = GrLinearGradient::Create(context, procDataManager, *this, matrix, fTileMode);

    return true;
}

#else

bool SkLinearGradient::asFragmentProcessor(GrContext*, const SkPaint&, const SkMatrix&,
                                           const SkMatrix*, GrColor*, GrProcessorDataManager*,
                                           GrFragmentProcessor**)  const {
    SkDEBUGFAIL("Should not call in GPU-less build");
    return false;
}

#endif

#ifndef SK_IGNORE_TO_STRING
void SkLinearGradient::toString(SkString* str) const {
    str->append("SkLinearGradient (");

    str->appendf("start: (%f, %f)", fStart.fX, fStart.fY);
    str->appendf(" end: (%f, %f) ", fEnd.fX, fEnd.fY);

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
