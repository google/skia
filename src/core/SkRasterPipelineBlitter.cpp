/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitter.h"
#include "SkBlendModePriv.h"
#include "SkColor.h"
#include "SkColorFilter.h"
#include "SkFixedAlloc.h"
#include "SkOpts.h"
#include "SkPM4f.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
#include "SkShader.h"
#include "SkUtils.h"


class SkRasterPipelineBlitter : public SkBlitter {
public:
    static SkBlitter* Create(const SkPixmap&, const SkPaint&, const SkMatrix& ctm,
                             SkTBlitterAllocator*);

    SkRasterPipelineBlitter(SkPixmap dst, SkBlendMode blend, SkPM4f paintColor)
        : fDst(dst)
        , fBlend(blend)
        , fPaintColor(paintColor)
        , fScratchAlloc(fScratch, sizeof(fScratch))
        , fScratchFallback(&fScratchAlloc)
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

    // These functions are compiled lazily when first used.
    std::function<void(size_t, size_t, size_t)> fBlitH         = nullptr,
                                                fBlitAntiH     = nullptr,
                                                fBlitMaskA8    = nullptr,
                                                fBlitMaskLCD16 = nullptr;

    // These values are pointed to by the compiled blit functions
    // above, which allows us to adjust them from call to call.
    void*       fDstPtr          = nullptr;
    const void* fMaskPtr         = nullptr;
    float       fCurrentCoverage = 0.0f;

    // Scratch space for shaders and color filters to use.
    char            fScratch[64];
    SkFixedAlloc    fScratchAlloc;
    SkFallbackAlloc fScratchFallback;

    typedef SkBlitter INHERITED;
};

SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap& dst,
                                         const SkPaint& paint,
                                         const SkMatrix& ctm,
                                         SkTBlitterAllocator* alloc) {
    return SkRasterPipelineBlitter::Create(dst, paint, ctm, alloc);
}

static bool supported(const SkImageInfo& info) {
    switch (info.colorType()) {
        case kN32_SkColorType:      return info.gammaCloseToSRGB();
        case kRGBA_F16_SkColorType: return true;
        case kRGB_565_SkColorType:  return true;
        default:                    return false;
    }
}

SkBlitter* SkRasterPipelineBlitter::Create(const SkPixmap& dst,
                                           const SkPaint& paint,
                                           const SkMatrix& ctm,
                                           SkTBlitterAllocator* alloc) {
    auto blitter = alloc->createT<SkRasterPipelineBlitter>(
            dst,
            paint.getBlendMode(),
            SkPM4f_from_SkColor(paint.getColor(), dst.colorSpace()));

    auto earlyOut = [&] {
        alloc->deleteLast();
        return nullptr;
    };

    SkBlendMode*      blend       = &blitter->fBlend;
    SkPM4f*           paintColor  = &blitter->fPaintColor;
    SkRasterPipeline* pipeline    = &blitter->fShader;

    SkShader*      shader      = paint.getShader();
    SkColorFilter* colorFilter = paint.getColorFilter();

    // TODO: all temporary
    if (!supported(dst.info()) || !SkBlendMode_AppendStages(*blend)) {
        return earlyOut();
    }

    bool is_opaque   = paintColor->a() == 1.0f,
         is_constant = true;
    if (shader) {
        if (!shader->appendStages(pipeline, dst.colorSpace(), &blitter->fScratchFallback,
                                  ctm, paint)) {
            return earlyOut();
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
        if (!colorFilter->appendStages(pipeline, dst.colorSpace(), &blitter->fScratchFallback,
                                       is_opaque)) {
            return earlyOut();
        }
        is_opaque = is_opaque && (colorFilter->getFlags() & SkColorFilter::kAlphaUnchanged_Flag);
    }

    if (is_constant) {
        pipeline->append(SkRasterPipeline::store_f32, &paintColor);
        pipeline->run(0,0, 1);

        *pipeline = SkRasterPipeline();
        pipeline->append(SkRasterPipeline::constant_color, paintColor);

        is_opaque = paintColor->a() == 1.0f;
    }

    if (is_opaque && *blend == SkBlendMode::kSrcOver) {
        *blend = SkBlendMode::kSrc;
    }

    if (is_constant && *blend == SkBlendMode::kSrc) {
        uint64_t color;  // Big enough for largest dst format, F16.
        SkRasterPipeline p;
        p.extend(*pipeline);
        blitter->fDstPtr = &color;
        blitter->append_store(&p);
        p.run(0,0, 1);

        switch (dst.shiftPerPixel()) {
            case 1:
                blitter->fBlitH = [blitter,color](size_t x, size_t, size_t n) {
                    sk_memset16((uint16_t*)blitter->fDstPtr + x, color, n);
                };
                break;
            case 2:
                blitter->fBlitH = [blitter,color](size_t x, size_t, size_t n) {
                    sk_memset32((uint32_t*)blitter->fDstPtr + x, color, n);
                };
                break;
            case 3:
                blitter->fBlitH = [blitter,color](size_t x, size_t, size_t n) {
                    sk_memset64((uint64_t*)blitter->fDstPtr + x, color, n);
                };
                break;
            default: break;
        }
    }

    return blitter;
}

void SkRasterPipelineBlitter::append_load_d(SkRasterPipeline* p) const {
    SkASSERT(supported(fDst.info()));

    p->append(SkRasterPipeline::move_src_dst);
    switch (fDst.info().colorType()) {
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
    if (!fBlitH) {
        SkRasterPipeline p;
        p.extend(fShader);
        if (fBlend != SkBlendMode::kSrc) {
            this->append_load_d(&p);
            this->append_blend(&p);
            this->maybe_clamp(&p);
        }
        this->append_store(&p);
        fBlitH = p.compile();
    }
    fDstPtr = fDst.writable_addr(0,y);
    fBlitH(x,y, w);
}

void SkRasterPipelineBlitter::blitAntiH(int x, int y, const SkAlpha aa[], const int16_t runs[]) {
    if (!fBlitAntiH) {
        SkRasterPipeline p;
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
        fBlitAntiH = p.compile();
    }

    fDstPtr = fDst.writable_addr(0,y);
    for (int16_t run = *runs; run > 0; run = *runs) {
        switch (*aa) {
            case 0x00:                       break;
            case 0xff: this->blitH(x,y,run); break;
            default:
                fCurrentCoverage = *aa * (1/255.0f);
                fBlitAntiH(x,y, run);
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
        SkRasterPipeline p;
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
        fBlitMaskA8 = p.compile();
    }

    if (mask.fFormat == SkMask::kLCD16_Format && !fBlitMaskLCD16) {
        SkRasterPipeline p;
        p.extend(fShader);
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

        switch (mask.fFormat) {
            case SkMask::kA8_Format:
                fMaskPtr = mask.getAddr8(x,y)-x;
                fBlitMaskA8(x,y, clip.width());
                break;
            case SkMask::kLCD16_Format:
                fMaskPtr = mask.getAddrLCD16(x,y)-x;
                fBlitMaskLCD16(x,y, clip.width());
                break;
            default:
                // TODO
                break;
        }
    }
}
