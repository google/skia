/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXformer.h"
#include "SkRadialGradient.h"
#include "SkNx.h"

namespace {

// GCC doesn't like using static functions as template arguments.  So force these to be non-static.
inline SkFixed mirror_tileproc_nonstatic(SkFixed x) {
    return mirror_tileproc(x);
}

inline SkFixed repeat_tileproc_nonstatic(SkFixed x) {
    return repeat_tileproc(x);
}

SkMatrix rad_to_unit_matrix(const SkPoint& center, SkScalar radius) {
    SkScalar    inv = SkScalarInvert(radius);

    SkMatrix matrix;
    matrix.setTranslate(-center.fX, -center.fY);
    matrix.postScale(inv, inv);
    return matrix;
}


}  // namespace

/////////////////////////////////////////////////////////////////////

SkRadialGradient::SkRadialGradient(const SkPoint& center, SkScalar radius, const Descriptor& desc)
    : SkGradientShaderBase(desc, rad_to_unit_matrix(center, radius))
    , fCenter(center)
    , fRadius(radius) {
}

SkShaderBase::Context* SkRadialGradient::onMakeContext(
    const ContextRec& rec, SkArenaAlloc* alloc) const
{
    return CheckedMakeContext<RadialGradientContext>(alloc, *this, rec);
}

SkRadialGradient::RadialGradientContext::RadialGradientContext(
        const SkRadialGradient& shader, const ContextRec& rec)
    : INHERITED(shader, rec) {}

SkShader::GradientType SkRadialGradient::asAGradient(GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fCenter;
        info->fRadius[0] = fRadius;
    }
    return kRadial_GradientType;
}

sk_sp<SkFlattenable> SkRadialGradient::CreateProc(SkReadBuffer& buffer) {
    DescriptorScope desc;
    if (!desc.unflatten(buffer)) {
        return nullptr;
    }
    const SkPoint center = buffer.readPoint();
    const SkScalar radius = buffer.readScalar();
    return SkGradientShader::MakeRadial(center, radius, desc.fColors, std::move(desc.fColorSpace),
                                        desc.fPos, desc.fCount, desc.fTileMode, desc.fGradFlags,
                                        desc.fLocalMatrix);
}

void SkRadialGradient::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fCenter);
    buffer.writeScalar(fRadius);
}

namespace {

inline bool radial_completely_pinned(SkScalar fx, SkScalar dx, SkScalar fy, SkScalar dy) {
    // fast, overly-conservative test: checks unit square instead of unit circle
    bool xClamped = (fx >= 1 && dx >= 0) || (fx <= -1 && dx <= 0);
    bool yClamped = (fy >= 1 && dy >= 0) || (fy <= -1 && dy <= 0);
    return xClamped || yClamped;
}

typedef void (* RadialShadeProc)(SkScalar sfx, SkScalar sdx,
        SkScalar sfy, SkScalar sdy,
        SkPMColor* dstC, const SkPMColor* cache,
        int count, int toggle);

static inline Sk4f fast_sqrt(const Sk4f& R) {
    return R * R.rsqrt();
}

static inline Sk4f sum_squares(const Sk4f& a, const Sk4f& b) {
    return a * a + b * b;
}

void shadeSpan_radial_clamp2(SkScalar sfx, SkScalar sdx, SkScalar sfy, SkScalar sdy,
                             SkPMColor* SK_RESTRICT dstC, const SkPMColor* SK_RESTRICT cache,
                             int count, int toggle) {
    if (radial_completely_pinned(sfx, sdx, sfy, sdy)) {
        unsigned fi = SkGradientShaderBase::kCache32Count - 1;
        sk_memset32_dither(dstC,
                           cache[toggle + fi],
                           cache[next_dither_toggle(toggle) + fi],
                           count);
    } else {
        const Sk4f min(SK_ScalarNearlyZero);
        const Sk4f max(255);
        const float scale = 255;
        sfx *= scale;
        sfy *= scale;
        sdx *= scale;
        sdy *= scale;
        const Sk4f fx4(sfx, sfx + sdx, sfx + 2*sdx, sfx + 3*sdx);
        const Sk4f fy4(sfy, sfy + sdy, sfy + 2*sdy, sfy + 3*sdy);
        const Sk4f dx4(sdx * 4);
        const Sk4f dy4(sdy * 4);

        Sk4f tmpxy = fx4 * dx4 + fy4 * dy4;
        Sk4f tmpdxdy = sum_squares(dx4, dy4);
        Sk4f R = Sk4f::Max(sum_squares(fx4, fy4), min);
        Sk4f dR = tmpxy + tmpxy + tmpdxdy;
        const Sk4f ddR = tmpdxdy + tmpdxdy;

        for (int i = 0; i < (count >> 2); ++i) {
            Sk4f dist = Sk4f::Min(fast_sqrt(R), max);
            R = Sk4f::Max(R + dR, min);
            dR = dR + ddR;

            uint8_t fi[4];
            SkNx_cast<uint8_t>(dist).store(fi);

            for (int i = 0; i < 4; i++) {
                *dstC++ = cache[toggle + fi[i]];
                toggle = next_dither_toggle(toggle);
            }
        }
        count &= 3;
        if (count) {
            Sk4f dist = Sk4f::Min(fast_sqrt(R), max);

            uint8_t fi[4];
            SkNx_cast<uint8_t>(dist).store(fi);
            for (int i = 0; i < count; i++) {
                *dstC++ = cache[toggle + fi[i]];
                toggle = next_dither_toggle(toggle);
            }
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
    const SkPMColor* SK_RESTRICT cache = fCache->getCache32();
    int toggle = init_dither_toggle(x, y);

    dstProc(fDstToIndex, SkIntToScalar(x) + SK_ScalarHalf,
                         SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
    SkScalar sdx = fDstToIndex.getScaleX();
    SkScalar sdy = fDstToIndex.getSkewY();

    RadialShadeProc shadeProc = shadeSpan_radial_repeat;
    if (SkShader::kClamp_TileMode == radialGradient.fTileMode) {
        shadeProc = shadeSpan_radial_clamp2;
    } else if (SkShader::kMirror_TileMode == radialGradient.fTileMode) {
        shadeProc = shadeSpan_radial_mirror;
    } else {
        SkASSERT(SkShader::kRepeat_TileMode == radialGradient.fTileMode);
    }
    (*shadeProc)(srcPt.fX, sdx, srcPt.fY, sdy, dstC, cache, count, toggle);
}

/////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "SkGr.h"
#include "GrShaderCaps.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

class GrRadialGradient : public GrGradientEffect {
public:
    class GLSLRadialProcessor;

    static sk_sp<GrFragmentProcessor> Make(const CreateArgs& args) {
        auto processor = sk_sp<GrRadialGradient>(new GrRadialGradient(args));
        return processor->isValid() ? std::move(processor) : nullptr;
    }

    ~GrRadialGradient() override {}

    const char* name() const override { return "Radial Gradient"; }

private:
    GrRadialGradient(const CreateArgs& args) : INHERITED(args, args.fShader->colorsAreOpaque()) {
        this->initClassID<GrRadialGradient>();
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    virtual void onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                       GrProcessorKeyBuilder* b) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrGradientEffect INHERITED;
};

/////////////////////////////////////////////////////////////////////

class GrRadialGradient::GLSLRadialProcessor : public GrGradientEffect::GLSLProcessor {
public:
    GLSLRadialProcessor(const GrProcessor&) {}
    ~GLSLRadialProcessor() override {}

    virtual void emitCode(EmitArgs&) override;

    static void GenKey(const GrProcessor& processor, const GrShaderCaps&, GrProcessorKeyBuilder* b) {
        b->add32(GenBaseGradientKey(processor));
    }

private:
    typedef GrGradientEffect::GLSLProcessor INHERITED;

};

/////////////////////////////////////////////////////////////////////

GrGLSLFragmentProcessor* GrRadialGradient::onCreateGLSLInstance() const {
    return new GrRadialGradient::GLSLRadialProcessor(*this);
}

void GrRadialGradient::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                             GrProcessorKeyBuilder* b) const {
    GrRadialGradient::GLSLRadialProcessor::GenKey(*this, caps, b);
}

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrRadialGradient);

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> GrRadialGradient::TestCreate(GrProcessorTestData* d) {
    sk_sp<SkShader> shader;
    do {
        RandomGradientParams params(d->fRandom);
        SkPoint center = {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()};
        SkScalar radius = d->fRandom->nextUScalar1();
        shader = params.fUseColors4f
                         ? SkGradientShader::MakeRadial(center, radius, params.fColors4f,
                                                        params.fColorSpace, params.fStops,
                                                        params.fColorCount, params.fTileMode)
                         : SkGradientShader::MakeRadial(center, radius, params.fColors,
                                                        params.fStops, params.fColorCount,
                                                        params.fTileMode);
    } while (!shader);
    GrTest::TestAsFPArgs asFPArgs(d);
    sk_sp<GrFragmentProcessor> fp = as_SB(shader)->asFragmentProcessor(asFPArgs.args());
    GrAlwaysAssert(fp);
    return fp;
}
#endif

/////////////////////////////////////////////////////////////////////

void GrRadialGradient::GLSLRadialProcessor::emitCode(EmitArgs& args) {
    const GrRadialGradient& ge = args.fFp.cast<GrRadialGradient>();
    this->emitUniforms(args.fUniformHandler, ge);
    SkString t("length(");
    t.append(args.fFragBuilder->ensureCoords2D(args.fTransformedCoords[0]));
    t.append(")");
    this->emitColor(args.fFragBuilder,
                    args.fUniformHandler,
                    args.fShaderCaps,
                    ge, t.c_str(),
                    args.fOutputColor,
                    args.fInputColor,
                    args.fTexSamplers);
}

/////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> SkRadialGradient::asFragmentProcessor(const AsFPArgs& args) const {
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
    sk_sp<GrFragmentProcessor> inner(GrRadialGradient::Make(
        GrGradientEffect::CreateArgs(args.fContext, this, &matrix, fTileMode,
                                     std::move(colorSpaceXform), SkToBool(args.fDstColorSpace))));
    if (!inner) {
        return nullptr;
    }
    return GrFragmentProcessor::MulOutputByInputAlpha(std::move(inner));
}

#endif

sk_sp<SkShader> SkRadialGradient::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    SkSTArray<8, SkColor> xformedColors(fColorCount);
    xformer->apply(xformedColors.begin(), fOrigColors, fColorCount);
    return SkGradientShader::MakeRadial(fCenter, fRadius, xformedColors.begin(), fOrigPos,
                                        fColorCount, fTileMode, fGradFlags,
                                        &this->getLocalMatrix());
}

bool SkRadialGradient::adjustMatrixAndAppendStages(SkArenaAlloc* alloc,
                                 SkMatrix* matrix,
                                 SkRasterPipeline* p,
                                 SkRasterPipeline*) const {
    matrix->postTranslate(-fCenter.fX, -fCenter.fY);
    matrix->postScale(1/fRadius, 1/fRadius);

    p->append(SkRasterPipeline::xy_to_radius);
    return true;
}

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
