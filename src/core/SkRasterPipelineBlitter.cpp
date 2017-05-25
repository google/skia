/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkBlitter.h"
#include "SkBlendModePriv.h"
#include "SkColor.h"
#include "SkColorFilter.h"
#include "SkColorSpaceXformer.h"
#include "SkOpts.h"
#include "SkPM4f.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
#include "SkShader.h"
#include "SkShaderBase.h"
#include "SkUtils.h"
#include "../jumper/SkJumper.h"

class SkRasterPipelineBlitter final : public SkBlitter {
public:
    // This is our common entrypoint for creating the blitter once we've sorted out shaders.
    static SkBlitter* Create(const SkPixmap&, const SkPaint&, SkArenaAlloc*,
                             const SkRasterPipeline& shaderPipeline,
                             SkShaderBase::Context* shaderCtx,
                             bool is_opaque, bool is_constant, bool wants_dither);

    SkRasterPipelineBlitter(SkPixmap dst,
                            SkBlendMode blend,
                            SkArenaAlloc* alloc,
                            SkShaderBase::Context* shaderCtx)
        : fDst(dst)
        , fBlend(blend)
        , fAlloc(alloc)
        , fShaderCtx(shaderCtx)
        , fColorPipeline(alloc)
    {}

    void blitH    (int x, int y, int w)                            override;
    void blitAntiH(int x, int y, const SkAlpha[], const int16_t[]) override;
    void blitMask (const SkMask&, const SkIRect& clip)             override;

    // TODO: The default implementations of the other blits look fine,
    // but some of them like blitV could probably benefit from custom
    // blits using something like a SkRasterPipeline::runFew() method.

private:
    void append_load_d(SkRasterPipeline*) const;
    void append_blend (SkRasterPipeline*) const;
    void maybe_clamp  (SkRasterPipeline*) const;
    void append_store (SkRasterPipeline*) const;

    // If we have an SkShader::Context, use it to fill our shader buffer.
    void maybe_shade(int x, int y, int w);

    SkPixmap               fDst;
    SkBlendMode            fBlend;
    SkArenaAlloc*          fAlloc;
    SkShaderBase::Context* fShaderCtx;
    SkRasterPipeline       fColorPipeline;

    // We may be able to specialize blitH() into a memset.
    bool     fCanMemsetInBlitH = false;
    uint64_t fMemsetColor      = 0;     // Big enough for largest dst format, F16.

    // Built lazily on first use.
    std::function<void(size_t, size_t)> fBlitH,
                                        fBlitAntiH,
                                        fBlitMaskA8,
                                        fBlitMaskLCD16;

    // These values are pointed to by the blit pipelines above,
    // which allows us to adjust them from call to call.
    void*              fShaderOutput    = nullptr;
    void*              fDstPtr          = nullptr;
    const void*        fMaskPtr         = nullptr;
    float              fCurrentCoverage = 0.0f;
    int                fCurrentY        = 0;
    SkJumper_DitherCtx fDitherCtx = { &fCurrentY, 0.0f };

    std::vector<SkPM4f> fShaderBuffer;

    typedef SkBlitter INHERITED;
};

SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap& dst,
                                         const SkPaint& paint,
                                         const SkMatrix& ctm,
                                         SkArenaAlloc* alloc) {
    SkColorSpace* dstCS = dst.colorSpace();
    auto paintColor = alloc->make<SkPM4f>(SkPM4f_from_SkColor(paint.getColor(), dstCS));
    auto shader = as_SB(paint.getShader());

    SkRasterPipeline_<256> shaderPipeline;
    if (!shader) {
        // Having no shader makes things nice and easy... just use the paint color.
        shaderPipeline.append(SkRasterPipeline::constant_color, paintColor);
        bool is_opaque    = paintColor->a() == 1.0f,
             is_constant  = true,
             wants_dither = false;
        return SkRasterPipelineBlitter::Create(dst, paint, alloc,
                                               shaderPipeline, nullptr,
                                               is_opaque, is_constant, wants_dither);
    }

    bool is_opaque    = shader->isOpaque() && paintColor->a() == 1.0f;
    bool is_constant  = shader->isConstant();
    bool wants_dither = shader->asAGradient(nullptr) >= SkShader::kLinear_GradientType;

    // See if the shader can express itself by appending pipeline stages.
    if (shader->appendStages(&shaderPipeline, dstCS, alloc, ctm, paint)) {
        if (paintColor->a() != 1.0f) {
            shaderPipeline.append(SkRasterPipeline::scale_1_float, &paintColor->fVec[SkPM4f::A]);
        }
        return SkRasterPipelineBlitter::Create(dst, paint, alloc,
                                               shaderPipeline, nullptr,
                                               is_opaque, is_constant, wants_dither);
    }

    // No, the shader wants us to create a context and call shadeSpan4f().
    SkASSERT(!is_constant);  // All constant shaders should be able to appendStages().

    if (dstCS) {
        // We need to transform the shader into the dst color space, and extend its lifetime.
        sk_sp<SkShader> in_dstCS = SkColorSpaceXformer::Make(sk_ref_sp(dstCS))->apply(shader);
        shader = as_SB(in_dstCS.get());
        alloc->make<sk_sp<SkShader>>(std::move(in_dstCS));
    }
    SkShaderBase::ContextRec rec(paint, ctm, nullptr, SkShaderBase::ContextRec::kPM4f_DstType,
                                 dstCS);
    SkShaderBase::Context* shaderCtx = shader->makeContext(rec, alloc);
    if (!shaderCtx) {
        // When a shader fails to create a context, it has vetoed drawing entirely.
        return alloc->make<SkNullBlitter>();
    }
    return SkRasterPipelineBlitter::Create(dst, paint, alloc,
                                           shaderPipeline, shaderCtx,
                                           is_opaque, is_constant, wants_dither);
}

SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap& dst,
                                         const SkPaint& paint,
                                         const SkRasterPipeline& shaderPipeline,
                                         bool is_opaque,
                                         bool wants_dither,
                                         SkArenaAlloc* alloc) {
    bool is_constant = false;  // If this were the case, it'd be better to just set a paint color.
    return SkRasterPipelineBlitter::Create(dst, paint, alloc,
                                           shaderPipeline, nullptr,
                                           is_opaque, is_constant, wants_dither);
}

SkBlitter* SkRasterPipelineBlitter::Create(const SkPixmap& dst,
                                           const SkPaint& paint,
                                           SkArenaAlloc* alloc,
                                           const SkRasterPipeline& shaderPipeline,
                                           SkShaderBase::Context* shaderCtx,
                                           bool is_opaque,
                                           bool is_constant,
                                           bool wants_dither) {
    auto blitter = alloc->make<SkRasterPipelineBlitter>(dst,
                                                        paint.getBlendMode(),
                                                        alloc,
                                                        shaderCtx);

    // Our job in this factory is to fill out the blitter's color pipeline.
    // This is the common front of the full blit pipelines, each constructed lazily on first use.
    // The full blit pipelines handle reading and writing the dst, blending, coverage, dithering.
    auto colorPipeline = &blitter->fColorPipeline;

    // Let's get the shader in first.
    if (shaderCtx) {
        colorPipeline->append(SkRasterPipeline::load_f32, &blitter->fShaderOutput);
    } else {
        // If the shader's not constant, it'll need seeding with x,y.
        if (!is_constant) {
            colorPipeline->append(SkRasterPipeline::seed_shader, &blitter->fCurrentY);
        }
        colorPipeline->extend(shaderPipeline);
    }

    // If there's a color filter it comes next.
    if (auto colorFilter = paint.getColorFilter()) {
        colorFilter->appendStages(colorPipeline, dst.colorSpace(), alloc, is_opaque);
        is_opaque = is_opaque && (colorFilter->getFlags() & SkColorFilter::kAlphaUnchanged_Flag);
    }

    // We'll dither if the shader wants to, or if we're drawing 565 and the paint wants to.
    // Not all formats make sense to dither (think, F16).  We set their dither rate to zero.
    // We need to decide if we're going to dither now to keep is_constant accurate.
    if (wants_dither ||
            (paint.isDither() && dst.info().colorType() == kRGB_565_SkColorType)) {
        switch (dst.info().colorType()) {
            default:                     blitter->fDitherCtx.rate =     0.0f; break;
            case   kRGB_565_SkColorType: blitter->fDitherCtx.rate =  1/63.0f; break;
            case kRGBA_8888_SkColorType:
            case kBGRA_8888_SkColorType: blitter->fDitherCtx.rate = 1/255.0f; break;
        }
    }
    is_constant = is_constant && (blitter->fDitherCtx.rate == 0.0f);

    // We're logically done here.  The code between here and return blitter is all optimization.

    // A pipeline that's still constant here can collapse back into a constant color.
    if (is_constant) {
        auto constantColor = alloc->make<SkPM4f>();
        colorPipeline->append(SkRasterPipeline::store_f32, &constantColor);
        colorPipeline->run(0,1);
        colorPipeline->reset();
        colorPipeline->append(SkRasterPipeline::constant_color, constantColor);

        is_opaque = constantColor->a() == 1.0f;
    }

    // We can strength-reduce SrcOver into Src when opaque.
    if (is_opaque && blitter->fBlend == SkBlendMode::kSrcOver) {
        blitter->fBlend = SkBlendMode::kSrc;
    }

    // When we're drawing a constant color in Src mode, we can sometimes just memset.
    // (The previous two optimizations help find more opportunities for this one.)
    if (is_constant && blitter->fBlend == SkBlendMode::kSrc) {
        // Run our color pipeline all the way through to produce what we'd memset when we can.
        // Not all blits can memset, so we need to keep colorPipeline too.
        SkRasterPipeline_<256> p;
        p.extend(*colorPipeline);
        blitter->fDstPtr = &blitter->fMemsetColor;
        blitter->append_store(&p);
        p.run(0,1);

        blitter->fCanMemsetInBlitH = true;
    }

    return blitter;
}

void SkRasterPipelineBlitter::append_load_d(SkRasterPipeline* p) const {
    p->append(SkRasterPipeline::move_src_dst);
    switch (fDst.info().colorType()) {
        case kAlpha_8_SkColorType:   p->append(SkRasterPipeline::load_a8,   &fDstPtr); break;
        case kRGB_565_SkColorType:   p->append(SkRasterPipeline::load_565,  &fDstPtr); break;
        case kBGRA_8888_SkColorType:
        case kRGBA_8888_SkColorType: p->append(SkRasterPipeline::load_8888, &fDstPtr); break;
        case kRGBA_F16_SkColorType:  p->append(SkRasterPipeline::load_f16,  &fDstPtr); break;
        default: break;
    }
    if (fDst.info().colorType() == kBGRA_8888_SkColorType) {
        p->append(SkRasterPipeline::swap_rb);
    }
    if (fDst.info().gammaCloseToSRGB()) {
        p->append_from_srgb(fDst.info().alphaType());
    }
    p->append(SkRasterPipeline::swap);
}

void SkRasterPipelineBlitter::append_store(SkRasterPipeline* p) const {
    if (fDst.info().gammaCloseToSRGB()) {
        p->append(SkRasterPipeline::to_srgb);
    }
    if (fDitherCtx.rate > 0.0f) {
        // We dither after any sRGB transfer function to make sure our 1/255.0f is sensible
        // over the whole range.  If we did it before, 1/255.0f is too big a rate near zero.
        p->append(SkRasterPipeline::dither, &fDitherCtx);
    }

    if (fDst.info().colorType() == kBGRA_8888_SkColorType) {
        p->append(SkRasterPipeline::swap_rb);
    }
    switch (fDst.info().colorType()) {
        case kAlpha_8_SkColorType:   p->append(SkRasterPipeline::store_a8,   &fDstPtr); break;
        case kRGB_565_SkColorType:   p->append(SkRasterPipeline::store_565,  &fDstPtr); break;
        case kBGRA_8888_SkColorType:
        case kRGBA_8888_SkColorType: p->append(SkRasterPipeline::store_8888, &fDstPtr); break;
        case kRGBA_F16_SkColorType:  p->append(SkRasterPipeline::store_f16,  &fDstPtr); break;
        default: break;
    }
}

void SkRasterPipelineBlitter::append_blend(SkRasterPipeline* p) const {
    SkBlendMode_AppendStages(fBlend, p);
}

void SkRasterPipelineBlitter::maybe_clamp(SkRasterPipeline* p) const {
    if (SkBlendMode_CanOverflow(fBlend)) {
        p->append(SkRasterPipeline::clamp_a);
    }
}

void SkRasterPipelineBlitter::maybe_shade(int x, int y, int w) {
    if (fShaderCtx) {
        if (w > SkToInt(fShaderBuffer.size())) {
            fShaderBuffer.resize(w);
        }
        fShaderCtx->shadeSpan4f(x,y, fShaderBuffer.data(), w);
        // We'll be reading from fShaderOutput + x.
        fShaderOutput = fShaderBuffer.data() - x;
    }
}

void SkRasterPipelineBlitter::blitH(int x, int y, int w) {
    fDstPtr = fDst.writable_addr(0,y);
    fCurrentY = y;

    if (fCanMemsetInBlitH) {
        switch (fDst.shiftPerPixel()) {
            case 0:    memset  ((uint8_t *)fDstPtr + x, fMemsetColor, w); return;
            case 1: sk_memset16((uint16_t*)fDstPtr + x, fMemsetColor, w); return;
            case 2: sk_memset32((uint32_t*)fDstPtr + x, fMemsetColor, w); return;
            case 3: sk_memset64((uint64_t*)fDstPtr + x, fMemsetColor, w); return;
            default: break;
        }
    }

    if (!fBlitH) {
        SkRasterPipeline p(fAlloc);
        p.extend(fColorPipeline);
        if (fBlend == SkBlendMode::kSrcOver
                && fDst.info().colorType() == kRGBA_8888_SkColorType
                && !fDst.colorSpace()
                && fDitherCtx.rate == 0.0f) {
            p.append(SkRasterPipeline::srcover_rgba_8888, &fDstPtr);
        } else {
            if (fBlend != SkBlendMode::kSrc) {
                this->append_load_d(&p);
                this->append_blend(&p);
                this->maybe_clamp(&p);
            }
            this->append_store(&p);
        }
        fBlitH = p.compile();
    }
    this->maybe_shade(x,y,w);
    fBlitH(x,w);
}

void SkRasterPipelineBlitter::blitAntiH(int x, int y, const SkAlpha aa[], const int16_t runs[]) {
    if (!fBlitAntiH) {
        SkRasterPipeline p(fAlloc);
        p.extend(fColorPipeline);
        if (fBlend == SkBlendMode::kSrcOver) {
            p.append(SkRasterPipeline::scale_1_float, &fCurrentCoverage);
            this->append_load_d(&p);
            this->append_blend(&p);
        } else {
            this->append_load_d(&p);
            this->append_blend(&p);
            p.append(SkRasterPipeline::lerp_1_float, &fCurrentCoverage);
        }
        this->maybe_clamp(&p);
        this->append_store(&p);
        fBlitAntiH = p.compile();
    }

    fDstPtr = fDst.writable_addr(0,y);
    fCurrentY = y;
    for (int16_t run = *runs; run > 0; run = *runs) {
        switch (*aa) {
            case 0x00:                       break;
            case 0xff: this->blitH(x,y,run); break;
            default:
                this->maybe_shade(x,y,run);
                fCurrentCoverage = *aa * (1/255.0f);
                fBlitAntiH(x,run);
        }
        x    += run;
        runs += run;
        aa   += run;
    }
}

void SkRasterPipelineBlitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    if (mask.fFormat == SkMask::kBW_Format) {
        // TODO: native BW masks?
        return INHERITED::blitMask(mask, clip);
    }

    if (mask.fFormat == SkMask::kA8_Format && !fBlitMaskA8) {
        SkRasterPipeline p(fAlloc);
        p.extend(fColorPipeline);
        if (fBlend == SkBlendMode::kSrcOver) {
            p.append(SkRasterPipeline::scale_u8, &fMaskPtr);
            this->append_load_d(&p);
            this->append_blend(&p);
        } else {
            this->append_load_d(&p);
            this->append_blend(&p);
            p.append(SkRasterPipeline::lerp_u8, &fMaskPtr);
        }
        this->maybe_clamp(&p);
        this->append_store(&p);
        fBlitMaskA8 = p.compile();
    }

    if (mask.fFormat == SkMask::kLCD16_Format && !fBlitMaskLCD16) {
        SkRasterPipeline p(fAlloc);
        p.extend(fColorPipeline);
        this->append_load_d(&p);
        this->append_blend(&p);
        p.append(SkRasterPipeline::lerp_565, &fMaskPtr);
        this->maybe_clamp(&p);
        this->append_store(&p);
        fBlitMaskLCD16 = p.compile();
    }

    int x = clip.left();
    for (int y = clip.top(); y < clip.bottom(); y++) {
        fDstPtr = fDst.writable_addr(0,y);
        fCurrentY = y;

        this->maybe_shade(x,y,clip.width());
        switch (mask.fFormat) {
            case SkMask::kA8_Format:
                fMaskPtr = mask.getAddr8(x,y)-x;
                fBlitMaskA8(x,clip.width());
                break;
            case SkMask::kLCD16_Format:
                fMaskPtr = mask.getAddrLCD16(x,y)-x;
                fBlitMaskLCD16(x,clip.width());
                break;
            default:
                // TODO
                break;
        }
    }
}
