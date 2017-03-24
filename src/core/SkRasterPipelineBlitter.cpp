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
#include "SkOpts.h"
#include "SkPM4f.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
#include "SkShader.h"
#include "SkUtils.h"


class SkRasterPipelineBlitter : public SkBlitter {
public:
    static SkBlitter* Create(const SkPixmap&, const SkPaint&, const SkMatrix& ctm,
                             SkArenaAlloc*);

    SkRasterPipelineBlitter(SkPixmap dst, SkBlendMode blend, SkPM4f paintColor)
        : fDst(dst)
        , fBlend(blend)
        , fPaintColor(paintColor)
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

    SkPixmap         fDst;
    SkBlendMode      fBlend;
    SkPM4f           fPaintColor;
    SkRasterPipeline fShader;

    // We may be able to specialize blitH() into a memset.
    bool     fCanMemsetInBlitH = false;
    uint64_t fMemsetColor      = 0;     // Big enough for largest dst format, F16.

    // Built lazily on first use.
    SkRasterPipeline fBlitH,
                     fBlitAntiH,
                     fBlitMaskA8,
                     fBlitMaskLCD16;

    // These values are pointed to by the blit pipelines above,
    // which allows us to adjust them from call to call.
    void*       fDstPtr          = nullptr;
    const void* fMaskPtr         = nullptr;
    float       fCurrentCoverage = 0.0f;
    int         fCurrentY        = 0;

    typedef SkBlitter INHERITED;
};

SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap& dst,
                                         const SkPaint& paint,
                                         const SkMatrix& ctm,
                                         SkArenaAlloc* alloc) {
    return SkRasterPipelineBlitter::Create(dst, paint, ctm, alloc);
}

static bool supported(const SkImageInfo& info) {
    switch (info.colorType()) {
        case kAlpha_8_SkColorType:  return true;
        case kRGB_565_SkColorType:  return true;
        case kN32_SkColorType:      return info.gammaCloseToSRGB();
        case kRGBA_F16_SkColorType: return true;
        default:                    return false;
    }
}

SkBlitter* SkRasterPipelineBlitter::Create(const SkPixmap& dst,
                                           const SkPaint& paint,
                                           const SkMatrix& ctm,
                                           SkArenaAlloc* alloc) {
    auto blitter = alloc->make<SkRasterPipelineBlitter>(
            dst,
            paint.getBlendMode(),
            SkPM4f_from_SkColor(paint.getColor(), dst.colorSpace()));


    SkBlendMode*      blend       = &blitter->fBlend;
    SkPM4f*           paintColor  = &blitter->fPaintColor;
    SkRasterPipeline* pipeline    = &blitter->fShader;

    SkShader*      shader      = paint.getShader();
    SkColorFilter* colorFilter = paint.getColorFilter();

    // TODO: all temporary
    if (!supported(dst.info()) || !SkBlendMode_AppendStages(*blend)) {
        return nullptr;
    }

    bool is_opaque   = paintColor->a() == 1.0f,
         is_constant = true;
    if (shader) {
        pipeline->append(SkRasterPipeline::seed_shader, &blitter->fCurrentY);
        if (!shader->appendStages(pipeline, dst.colorSpace(), alloc, ctm, paint)) {
            return nullptr;
        }
        if (!is_opaque) {
            pipeline->append(SkRasterPipeline::scale_1_float,
                             &paintColor->fVec[SkPM4f::A]);
        }

        is_opaque   = is_opaque && shader->isOpaque();
        is_constant = shader->isConstant();
    } else {
        pipeline->append(SkRasterPipeline::constant_color, paintColor);
    }

    if (colorFilter) {
        if (!colorFilter->appendStages(pipeline, dst.colorSpace(), alloc, is_opaque)) {
            return nullptr;
        }
        is_opaque = is_opaque && (colorFilter->getFlags() & SkColorFilter::kAlphaUnchanged_Flag);
    }

    if (is_constant) {
        pipeline->append(SkRasterPipeline::store_f32, &paintColor);
        pipeline->run(0,1);

        *pipeline = SkRasterPipeline();
        pipeline->append(SkRasterPipeline::constant_color, paintColor);

        is_opaque = paintColor->a() == 1.0f;
    }

    if (is_opaque && *blend == SkBlendMode::kSrcOver) {
        *blend = SkBlendMode::kSrc;
    }

    if (is_constant && *blend == SkBlendMode::kSrc) {
        SkRasterPipeline p;
        p.extend(*pipeline);
        blitter->fDstPtr = &blitter->fMemsetColor;
        blitter->append_store(&p);
        p.run(0,1);

        blitter->fCanMemsetInBlitH = true;
    }

    return blitter;
}

void SkRasterPipelineBlitter::append_load_d(SkRasterPipeline* p) const {
    SkASSERT(supported(fDst.info()));

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
    if (fDst.info().colorType() == kBGRA_8888_SkColorType) {
        p->append(SkRasterPipeline::swap_rb);
    }

    SkASSERT(supported(fDst.info()));
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
    SkAssertResult(SkBlendMode_AppendStages(fBlend, p));
}

void SkRasterPipelineBlitter::maybe_clamp(SkRasterPipeline* p) const {
    if (SkBlendMode_CanOverflow(fBlend)) {
        p->append(SkRasterPipeline::clamp_a);
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

    auto& p = fBlitH;
    if (p.empty()) {
        p.extend(fShader);
        if (fBlend != SkBlendMode::kSrc) {
            this->append_load_d(&p);
            this->append_blend(&p);
            this->maybe_clamp(&p);
        }
        this->append_store(&p);
    }
    p.run(x,w);
}

void SkRasterPipelineBlitter::blitAntiH(int x, int y, const SkAlpha aa[], const int16_t runs[]) {
    auto& p = fBlitAntiH;
    if (p.empty()) {
        p.extend(fShader);
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
    }

    fDstPtr = fDst.writable_addr(0,y);
    fCurrentY = y;
    for (int16_t run = *runs; run > 0; run = *runs) {
        switch (*aa) {
            case 0x00:                       break;
            case 0xff: this->blitH(x,y,run); break;
            default:
                fCurrentCoverage = *aa * (1/255.0f);
                p.run(x,run);
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

    if (mask.fFormat == SkMask::kA8_Format && fBlitMaskA8.empty()) {
        auto& p = fBlitMaskA8;
        p.extend(fShader);
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
    }

    if (mask.fFormat == SkMask::kLCD16_Format && fBlitMaskLCD16.empty()) {
        auto& p = fBlitMaskLCD16;
        p.extend(fShader);
        this->append_load_d(&p);
        this->append_blend(&p);
        p.append(SkRasterPipeline::lerp_565, &fMaskPtr);
        this->maybe_clamp(&p);
        this->append_store(&p);
    }

    int x = clip.left();
    for (int y = clip.top(); y < clip.bottom(); y++) {
        fDstPtr = fDst.writable_addr(0,y);
        fCurrentY = y;

        switch (mask.fFormat) {
            case SkMask::kA8_Format:
                fMaskPtr = mask.getAddr8(x,y)-x;
                fBlitMaskA8.run(x,clip.width());
                break;
            case SkMask::kLCD16_Format:
                fMaskPtr = mask.getAddrLCD16(x,y)-x;
                fBlitMaskLCD16.run(x,clip.width());
                break;
            default:
                // TODO
                break;
        }
    }
}
