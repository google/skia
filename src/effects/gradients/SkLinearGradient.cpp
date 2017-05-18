/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sk4fLinearGradient.h"
#include "SkColorSpaceXformer.h"
#include "SkLinearGradient.h"
#include "SkRefCnt.h"

// define to test the 4f gradient path
// #define FORCE_4F_CONTEXT

static const float kInv255Float = 1.0f / 255;

static inline int repeat_8bits(int x) {
    return x & 0xFF;
}

static inline int mirror_8bits(int x) {
    if (x & 256) {
        x = ~x;
    }
    return x & 255;
}

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

static bool use_4f_context(const SkShader::ContextRec& rec, uint32_t flags) {
#ifdef FORCE_4F_CONTEXT
    return true;
#else
    return rec.fPreferredDstType == SkShader::ContextRec::kPM4f_DstType
        || SkToBool(flags & SkLinearGradient::kForce4fContext_PrivateFlag);
#endif
}

///////////////////////////////////////////////////////////////////////////////

SkLinearGradient::SkLinearGradient(const SkPoint pts[2], const Descriptor& desc)
    : SkGradientShaderBase(desc, pts_to_unit_matrix(pts))
    , fStart(pts[0])
    , fEnd(pts[1]) {
}

sk_sp<SkFlattenable> SkLinearGradient::CreateProc(SkReadBuffer& buffer) {
    DescriptorScope desc;
    if (!desc.unflatten(buffer)) {
        return nullptr;
    }
    SkPoint pts[2];
    pts[0] = buffer.readPoint();
    pts[1] = buffer.readPoint();
    return SkGradientShader::MakeLinear(pts, desc.fColors, std::move(desc.fColorSpace), desc.fPos,
                                        desc.fCount, desc.fTileMode, desc.fGradFlags,
                                        desc.fLocalMatrix);
}

void SkLinearGradient::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fStart);
    buffer.writePoint(fEnd);
}

SkShader::Context* SkLinearGradient::onMakeContext(
    const ContextRec& rec, SkArenaAlloc* alloc) const
{
    return use_4f_context(rec, fGradFlags)
           ? CheckedMakeContext<LinearGradient4fContext>(alloc, *this, rec)
           : CheckedMakeContext<  LinearGradientContext>(alloc, *this, rec);
}

bool SkLinearGradient::adjustMatrixAndAppendStages(SkArenaAlloc* alloc,
                                                   SkMatrix* matrix,
                                                   SkRasterPipeline* p) const {
    *matrix = SkMatrix::Concat(fPtsToUnit, *matrix);
    // If the gradient is less than a quarter of a pixel, this falls into the
    // subpixel gradient code handled on a different path.
    SkVector dx = matrix->mapVector(1, 0);
    if (dx.fX >= 4) {
        return false;
    }
    return true;
}

sk_sp<SkShader> SkLinearGradient::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    SkPoint pts[2] = { fStart, fEnd };
    SkSTArray<8, SkColor> xformedColors(fColorCount);
    xformer->apply(xformedColors.begin(), fOrigColors, fColorCount);
    return SkGradientShader::MakeLinear(pts, xformedColors.begin(), fOrigPos, fColorCount,
                                        fTileMode, fGradFlags, &this->getLocalMatrix());
}

// This swizzles SkColor into the same component order as SkPMColor, but does not actually
// "pre" multiply the color components.
//
// This allows us to map directly to Sk4f, and eventually scale down to bytes to output a
// SkPMColor from the floats, without having to swizzle each time.
//
static uint32_t SkSwizzle_Color_to_PMColor(SkColor c) {
    return SkPackARGB32NoCheck(SkColorGetA(c), SkColorGetR(c), SkColorGetG(c), SkColorGetB(c));
}

SkLinearGradient::LinearGradientContext::LinearGradientContext(
        const SkLinearGradient& shader, const ContextRec& ctx)
    : INHERITED(shader, ctx)
{
    // setup for Sk4f
    const int count = shader.fColorCount;
    SkASSERT(count > 1);

    fRecs.setCount(count);
    Rec* rec = fRecs.begin();
    if (shader.fOrigPos) {
        rec[0].fPos = 0;
        SkDEBUGCODE(rec[0].fPosScale = SK_FloatNaN;)   // should never get used
        for (int i = 1; i < count; ++i) {
            rec[i].fPos = SkTPin(shader.fOrigPos[i], rec[i - 1].fPos, 1.0f);
            float diff = rec[i].fPos - rec[i - 1].fPos;
            if (diff > 0) {
                rec[i].fPosScale = 1.0f / diff;
            } else {
                rec[i].fPosScale = 0;
            }
        }
    } else {
        // no pos specified, so we compute evenly spaced values
        const float scale = float(count - 1);
        const float invScale = 1.0f / scale;
        for (int i = 0; i < count; ++i) {
            rec[i].fPos = i * invScale;
            rec[i].fPosScale = scale;
        }
    }
    rec[count - 1].fPos = 1;    // overwrite the last value just to be sure we end at 1.0

    fApplyAlphaAfterInterp = true;
    if ((shader.getGradFlags() & SkGradientShader::kInterpolateColorsInPremul_Flag) ||
        shader.colorsAreOpaque())
    {
        fApplyAlphaAfterInterp = false;
    }

    if (fApplyAlphaAfterInterp) {
        // Our fColor values are in PMColor order, but are still unpremultiplied, allowing us to
        // interpolate in unpremultiplied space first, and then scale by alpha right before we
        // convert to SkPMColor bytes.
        const float paintAlpha = ctx.fPaint->getAlpha() * kInv255Float;
        const Sk4f scale(1, 1, 1, paintAlpha);
        for (int i = 0; i < count; ++i) {
            uint32_t c = SkSwizzle_Color_to_PMColor(shader.fOrigColors[i]);
            rec[i].fColor = SkNx_cast<float>(Sk4b::Load(&c)) * scale;
            if (i > 0) {
                SkASSERT(rec[i - 1].fPos <= rec[i].fPos);
            }
        }
    } else {
        // Our fColor values are premultiplied, so converting to SkPMColor is just a matter
        // of converting the floats down to bytes.
        unsigned alphaScale = ctx.fPaint->getAlpha() + (ctx.fPaint->getAlpha() >> 7);
        for (int i = 0; i < count; ++i) {
            SkPMColor pmc = SkPreMultiplyColor(shader.fOrigColors[i]);
            pmc = SkAlphaMulQ(pmc, alphaScale);
            rec[i].fColor = SkNx_cast<float>(Sk4b::Load(&pmc));
            if (i > 0) {
                SkASSERT(rec[i - 1].fPos <= rec[i].fPos);
            }
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

    if (SkShader::kClamp_TileMode == linearGradient.fTileMode &&
        kLinear_MatrixClass == fDstToIndexClass)
    {
        this->shade4_clamp(x, y, dstC, count);
        return;
    }

    SkPoint             srcPt;
    SkMatrix::MapXYProc dstProc = fDstToIndexProc;
    TileProc            proc = linearGradient.fTileProc;
    const SkPMColor* SK_RESTRICT cache = fCache->getCache32();
    int                 toggle = init_dither_toggle(x, y);

    if (fDstToIndexClass != kPerspective_MatrixClass) {
        dstProc(fDstToIndex, SkIntToScalar(x) + SK_ScalarHalf,
                             SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
        SkGradFixed dx, fx = SkScalarPinToGradFixed(srcPt.fX);

        if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
            const auto step = fDstToIndex.fixedStepInX(SkIntToScalar(y));
            // todo: do we need a real/high-precision value for dx here?
            dx = SkScalarPinToGradFixed(step.fX);
        } else {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
            dx = SkScalarPinToGradFixed(fDstToIndex.getScaleX());
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

SkShader::GradientType SkLinearGradient::asAGradient(GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fStart;
        info->fPoint[1] = fEnd;
    }
    return kLinear_GradientType;
}

#if SK_SUPPORT_GPU

#include "GrColorSpaceXform.h"
#include "GrShaderCaps.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "SkGr.h"

/////////////////////////////////////////////////////////////////////

class GrLinearGradient : public GrGradientEffect {
public:
    class GLSLLinearProcessor;

    static sk_sp<GrFragmentProcessor> Make(const CreateArgs& args) {
        return sk_sp<GrFragmentProcessor>(new GrLinearGradient(args));
    }

    ~GrLinearGradient() override {}

    const char* name() const override { return "Linear Gradient"; }

private:
    GrLinearGradient(const CreateArgs& args) : INHERITED(args, args.fShader->colorsAreOpaque()) {
        this->initClassID<GrLinearGradient>();
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    virtual void onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                       GrProcessorKeyBuilder* b) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrGradientEffect INHERITED;
};

/////////////////////////////////////////////////////////////////////

class GrLinearGradient::GLSLLinearProcessor : public GrGradientEffect::GLSLProcessor {
public:
    GLSLLinearProcessor(const GrProcessor&) {}

    ~GLSLLinearProcessor() override {}

    virtual void emitCode(EmitArgs&) override;

    static void GenKey(const GrProcessor& processor, const GrShaderCaps&, GrProcessorKeyBuilder* b) {
        b->add32(GenBaseGradientKey(processor));
    }

private:
    typedef GrGradientEffect::GLSLProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////

GrGLSLFragmentProcessor* GrLinearGradient::onCreateGLSLInstance() const {
    return new GrLinearGradient::GLSLLinearProcessor(*this);
}

void GrLinearGradient::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                             GrProcessorKeyBuilder* b) const {
    GrLinearGradient::GLSLLinearProcessor::GenKey(*this, caps, b);
}

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrLinearGradient);

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> GrLinearGradient::TestCreate(GrProcessorTestData* d) {
    SkPoint points[] = {{d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()},
                        {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()}};

    RandomGradientParams params(d->fRandom);
    auto shader = params.fUseColors4f ?
        SkGradientShader::MakeLinear(points, params.fColors4f, params.fColorSpace, params.fStops,
                                     params.fColorCount, params.fTileMode) :
        SkGradientShader::MakeLinear(points, params.fColors, params.fStops,
                                     params.fColorCount, params.fTileMode);
    GrTest::TestAsFPArgs asFPArgs(d);
    sk_sp<GrFragmentProcessor> fp = shader->asFragmentProcessor(asFPArgs.args());
    GrAlwaysAssert(fp);
    return fp;
}
#endif

/////////////////////////////////////////////////////////////////////

void GrLinearGradient::GLSLLinearProcessor::emitCode(EmitArgs& args) {
    const GrLinearGradient& ge = args.fFp.cast<GrLinearGradient>();
    this->emitUniforms(args.fUniformHandler, ge);
    SkString t = args.fFragBuilder->ensureCoords2D(args.fTransformedCoords[0]);
    t.append(".x");
    this->emitColor(args.fFragBuilder,
                    args.fUniformHandler,
                    args.fShaderCaps,
                    ge,
                    t.c_str(),
                    args.fOutputColor,
                    args.fInputColor,
                    args.fTexSamplers);
}

/////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> SkLinearGradient::asFragmentProcessor(const AsFPArgs& args) const {
    SkASSERT(args.fContext);

    SkMatrix matrix;
    if (!this->getLocalMatrix().invert(&matrix)) {
        return nullptr;
    }
    if (args.fLocalMatrix) {
        SkMatrix inv;
        if (!args.fLocalMatrix->invert(&inv)) {
            return nullptr;
        }
        matrix.postConcat(inv);
    }
    matrix.postConcat(fPtsToUnit);

    sk_sp<GrColorSpaceXform> colorSpaceXform = GrColorSpaceXform::Make(fColorSpace.get(),
                                                                       args.fDstColorSpace);
    sk_sp<GrFragmentProcessor> inner(GrLinearGradient::Make(
        GrGradientEffect::CreateArgs(args.fContext, this, &matrix, fTileMode,
                                     std::move(colorSpaceXform), SkToBool(args.fDstColorSpace))));
    return GrFragmentProcessor::MulOutputByInputAlpha(std::move(inner));
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

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkNx.h"

static const SkLinearGradient::LinearGradientContext::Rec*
find_forward(const SkLinearGradient::LinearGradientContext::Rec rec[], float tiledX) {
    SkASSERT(tiledX >= 0 && tiledX <= 1);

    SkASSERT(rec[0].fPos >= 0 && rec[0].fPos <= 1);
    SkASSERT(rec[1].fPos >= 0 && rec[1].fPos <= 1);
    SkASSERT(rec[0].fPos <= rec[1].fPos);
    rec += 1;
    while (rec->fPos < tiledX || rec->fPosScale == 0) {
        SkASSERT(rec[0].fPos >= 0 && rec[0].fPos <= 1);
        SkASSERT(rec[1].fPos >= 0 && rec[1].fPos <= 1);
        SkASSERT(rec[0].fPos <= rec[1].fPos);
        rec += 1;
    }
    return rec - 1;
}

static const SkLinearGradient::LinearGradientContext::Rec*
find_backward(const SkLinearGradient::LinearGradientContext::Rec rec[], float tiledX) {
    SkASSERT(tiledX >= 0 && tiledX <= 1);

    SkASSERT(rec[0].fPos >= 0 && rec[0].fPos <= 1);
    SkASSERT(rec[1].fPos >= 0 && rec[1].fPos <= 1);
    SkASSERT(rec[0].fPos <= rec[1].fPos);
    while (tiledX < rec->fPos || rec[1].fPosScale == 0) {
        rec -= 1;
        SkASSERT(rec[0].fPos >= 0 && rec[0].fPos <= 1);
        SkASSERT(rec[1].fPos >= 0 && rec[1].fPos <= 1);
        SkASSERT(rec[0].fPos <= rec[1].fPos);
    }
    return rec;
}

// As an optimization, we can apply the dither bias before interpolation -- but only when
// operating in premul space (apply_alpha == false).  When apply_alpha == true, we must
// defer the bias application until after premul.
//
// The following two helpers encapsulate this logic: pre_bias is called before interpolation,
// and effects the bias when apply_alpha == false, while post_bias is called after premul and
// effects the bias for the apply_alpha == true case.

template <bool apply_alpha>
Sk4f pre_bias(const Sk4f& x, const Sk4f& bias) {
    return apply_alpha ? x : x + bias;
}

template <bool apply_alpha>
Sk4f post_bias(const Sk4f& x, const Sk4f& bias) {
    return apply_alpha ? x + bias : x;
}

template <bool apply_alpha> SkPMColor trunc_from_255(const Sk4f& x, const Sk4f& bias) {
    SkPMColor c;
    Sk4f c4f255 = x;
    if (apply_alpha) {
        const float scale = x[SkPM4f::A] * (1 / 255.f);
        c4f255 *= Sk4f(scale, scale, scale, 1);
    }
    SkNx_cast<uint8_t>(post_bias<apply_alpha>(c4f255, bias)).store(&c);

    return c;
}

template <bool apply_alpha> void fill(SkPMColor dst[], int count,
                                      const Sk4f& c4, const Sk4f& bias0, const Sk4f& bias1) {
    const SkPMColor c0 = trunc_from_255<apply_alpha>(pre_bias<apply_alpha>(c4, bias0), bias0);
    const SkPMColor c1 = trunc_from_255<apply_alpha>(pre_bias<apply_alpha>(c4, bias1), bias1);
    sk_memset32_dither(dst, c0, c1, count);
}

template <bool apply_alpha> void fill(SkPMColor dst[], int count, const Sk4f& c4) {
    // Assumes that c4 does not need to be dithered.
    sk_memset32(dst, trunc_from_255<apply_alpha>(c4, 0), count);
}

/*
 *  TODOs
 *
 *  - tilemodes
 *  - interp before or after premul
 *  - perspective
 *  - optimizations
 *      - use fixed (32bit or 16bit) instead of floats?
 */

static Sk4f lerp_color(float fx, const SkLinearGradient::LinearGradientContext::Rec* rec) {
    SkASSERT(fx >= rec[0].fPos);
    SkASSERT(fx <= rec[1].fPos);

    const float p0 = rec[0].fPos;
    const Sk4f c0 = rec[0].fColor;
    const Sk4f c1 = rec[1].fColor;
    const Sk4f diffc = c1 - c0;
    const float scale = rec[1].fPosScale;
    const float t = (fx - p0) * scale;
    return c0 + Sk4f(t) * diffc;
}

template <bool apply_alpha> void ramp(SkPMColor dstC[], int n, const Sk4f& c, const Sk4f& dc,
                                      const Sk4f& dither0, const Sk4f& dither1) {
    Sk4f dc2 = dc + dc;
    Sk4f dc4 = dc2 + dc2;
    Sk4f cd0 = pre_bias<apply_alpha>(c     , dither0);
    Sk4f cd1 = pre_bias<apply_alpha>(c + dc, dither1);
    Sk4f cd2 = cd0 + dc2;
    Sk4f cd3 = cd1 + dc2;
    while (n >= 4) {
        if (!apply_alpha) {
            Sk4f_ToBytes((uint8_t*)dstC, cd0, cd1, cd2, cd3);
            dstC += 4;
        } else {
            *dstC++ = trunc_from_255<apply_alpha>(cd0, dither0);
            *dstC++ = trunc_from_255<apply_alpha>(cd1, dither1);
            *dstC++ = trunc_from_255<apply_alpha>(cd2, dither0);
            *dstC++ = trunc_from_255<apply_alpha>(cd3, dither1);
        }
        cd0 = cd0 + dc4;
        cd1 = cd1 + dc4;
        cd2 = cd2 + dc4;
        cd3 = cd3 + dc4;
        n -= 4;
    }
    if (n & 2) {
        *dstC++ = trunc_from_255<apply_alpha>(cd0, dither0);
        *dstC++ = trunc_from_255<apply_alpha>(cd1, dither1);
        cd0 = cd0 + dc2;
    }
    if (n & 1) {
        *dstC++ = trunc_from_255<apply_alpha>(cd0, dither0);
    }
}

template <bool apply_alpha, bool dx_is_pos>
void SkLinearGradient::LinearGradientContext::shade4_dx_clamp(SkPMColor dstC[], int count,
                                                              float fx, float dx, float invDx,
                                                              const float dither[2]) {
    Sk4f dither0(dither[0]);
    Sk4f dither1(dither[1]);
    const Rec* rec = fRecs.begin();

    const Sk4f dx4 = Sk4f(dx);
    SkDEBUGCODE(SkPMColor* endDstC = dstC + count;)

    if (dx_is_pos) {
        if (fx < 0) {
            // count is guaranteed to be positive, but the first arg may overflow int32 after
            // increment => casting to uint32 ensures correct clamping.
            int n = SkTMin<uint32_t>(static_cast<uint32_t>(SkFloatToIntFloor(-fx * invDx)) + 1,
                                     count);
            SkASSERT(n > 0);
            fill<apply_alpha>(dstC, n, rec[0].fColor);
            count -= n;
            dstC += n;
            fx += n * dx;
            SkASSERT(0 == count || fx >= 0);
            if (n & 1) {
                SkTSwap(dither0, dither1);
            }
        }
    } else { // dx < 0
        if (fx > 1) {
            // count is guaranteed to be positive, but the first arg may overflow int32 after
            // increment => casting to uint32 ensures correct clamping.
            int n = SkTMin<uint32_t>(static_cast<uint32_t>(SkFloatToIntFloor((1 - fx) * invDx)) + 1,
                                     count);
            SkASSERT(n > 0);
            fill<apply_alpha>(dstC, n, rec[fRecs.count() - 1].fColor);
            count -= n;
            dstC += n;
            fx += n * dx;
            SkASSERT(0 == count || fx <= 1);
            if (n & 1) {
                SkTSwap(dither0, dither1);
            }
        }
    }
    SkASSERT(count >= 0);

    const Rec* r;
    if (dx_is_pos) {
        r = fRecs.begin();                      // start at the beginning
    } else {
        r = fRecs.begin() + fRecs.count() - 2;  // start at the end
    }

    while (count > 0) {
        if (dx_is_pos) {
            if (fx >= 1) {
                fill<apply_alpha>(dstC, count, rec[fRecs.count() - 1].fColor);
                return;
            }
        } else {    // dx < 0
            if (fx <= 0) {
                fill<apply_alpha>(dstC, count, rec[0].fColor);
                return;
            }
        }

        if (dx_is_pos) {
            r = find_forward(r, fx);
        } else {
            r = find_backward(r, fx);
        }
        SkASSERT(r >= fRecs.begin() && r < fRecs.begin() + fRecs.count() - 1);

        const float p0 = r[0].fPos;
        const Sk4f c0 = r[0].fColor;
        const float p1 = r[1].fPos;
        const Sk4f diffc = Sk4f(r[1].fColor) - c0;
        const float scale = r[1].fPosScale;
        const float t = (fx - p0) * scale;
        const Sk4f c = c0 + Sk4f(t) * diffc;
        const Sk4f dc = diffc * dx4 * Sk4f(scale);

        int n;
        if (dx_is_pos) {
            n = SkTMin((int)((p1 - fx) * invDx) + 1, count);
        } else {
            n = SkTMin((int)((p0 - fx) * invDx) + 1, count);
        }

        fx += n * dx;
        // fx should now outside of the p0..p1 interval. However, due to float precision loss,
        // its possible that fx is slightly too small/large, so we clamp it.
        if (dx_is_pos) {
            fx = SkTMax(fx, p1);
        } else {
            fx = SkTMin(fx, p0);
        }

        ramp<apply_alpha>(dstC, n, c, dc, dither0, dither1);
        dstC += n;
        SkASSERT(dstC <= endDstC);

        if (n & 1) {
            SkTSwap(dither0, dither1);
        }

        count -= n;
        SkASSERT(count >= 0);
    }
}

void SkLinearGradient::LinearGradientContext::shade4_clamp(int x, int y, SkPMColor dstC[],
                                                           int count) {
    SkASSERT(count > 0);
    SkASSERT(kLinear_MatrixClass == fDstToIndexClass);

    SkPoint srcPt;
    fDstToIndexProc(fDstToIndex, x + SK_ScalarHalf, y + SK_ScalarHalf, &srcPt);
    float fx = srcPt.x();
    const float dx = fDstToIndex.getScaleX();

    // Default our dither bias values to 1/2, (rounding), which is no dithering
    float dither0 = 0.5f;
    float dither1 = 0.5f;
    if (fDither) {
        const float ditherCell[] = {
            1/8.0f,   5/8.0f,
            7/8.0f,   3/8.0f,
        };
        const int rowIndex = (y & 1) << 1;
        dither0 = ditherCell[rowIndex];
        dither1 = ditherCell[rowIndex + 1];
        if (x & 1) {
            SkTSwap(dither0, dither1);
        }
    }
    const float dither[2] = { dither0, dither1 };

    if (SkScalarNearlyZero(dx * count)) { // gradient is vertical
        const float pinFx = SkTPin(fx, 0.0f, 1.0f);
        Sk4f c = lerp_color(pinFx, find_forward(fRecs.begin(), pinFx));
        if (fApplyAlphaAfterInterp) {
            fill<true>(dstC, count, c, dither0, dither1);
        } else {
            fill<false>(dstC, count, c, dither0, dither1);
        }
        return;
    }

    SkASSERT(0.f != dx);
    const float invDx = 1 / dx;
    if (dx > 0) {
        if (fApplyAlphaAfterInterp) {
            this->shade4_dx_clamp<true, true>(dstC, count, fx, dx, invDx, dither);
        } else {
            this->shade4_dx_clamp<false, true>(dstC, count, fx, dx, invDx, dither);
        }
    } else {
        if (fApplyAlphaAfterInterp) {
            this->shade4_dx_clamp<true, false>(dstC, count, fx, dx, invDx, dither);
        } else {
            this->shade4_dx_clamp<false, false>(dstC, count, fx, dx, invDx, dither);
        }
    }
}
