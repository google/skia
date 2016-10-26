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
#include "SkOpts.h"
#include "SkPM4f.h"
#include "SkRasterPipeline.h"
#include "SkShader.h"
#include "SkXfermode.h"


class SkRasterPipelineBlitter : public SkBlitter {
public:
    static SkBlitter* Create(const SkPixmap&, const SkPaint&, SkTBlitterAllocator*);

    SkRasterPipelineBlitter(SkPixmap dst,
                            SkRasterPipeline shader,
                            SkBlendMode blend,
                            SkPM4f paintColor)
        : fDst(dst)
        , fShader(shader)
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
    void append_store (SkRasterPipeline*) const;
    void append_blend (SkRasterPipeline*) const;

    SkPixmap         fDst;
    SkRasterPipeline fShader;
    SkBlendMode      fBlend;
    SkPM4f           fPaintColor;

    // These functions are compiled lazily when first used.
    std::function<void(size_t, size_t)> fBlitH         = nullptr,
                                        fBlitAntiH     = nullptr,
                                        fBlitMaskA8    = nullptr,
                                        fBlitMaskLCD16 = nullptr;

    // These values are pointed to by the compiled blit functions
    // above, which allows us to adjust them from call to call.
    void*       fDstPtr           = nullptr;
    const void* fMaskPtr          = nullptr;
    float       fConstantCoverage = 0.0f;

    typedef SkBlitter INHERITED;
};

SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap& dst,
                                         const SkPaint& paint,
                                         SkTBlitterAllocator* alloc) {
    return SkRasterPipelineBlitter::Create(dst, paint, alloc);
}

static bool supported(const SkImageInfo& info) {
    switch (info.colorType()) {
        case kN32_SkColorType:      return info.gammaCloseToSRGB();
        case kRGBA_F16_SkColorType: return true;
        case kRGB_565_SkColorType:  return true;
        default:                    return false;
    }
}

template <typename Effect>
static bool append_effect_stages(const Effect* effect, SkRasterPipeline* pipeline) {
    return !effect || effect->appendStages(pipeline);
}


SkBlitter* SkRasterPipelineBlitter::Create(const SkPixmap& dst,
                                           const SkPaint& paint,
                                           SkTBlitterAllocator* alloc) {
    if (!supported(dst.info())) {
        return nullptr;
    }
    if (paint.getShader()) {
        return nullptr;  // TODO: need to work out how shaders and their contexts work
    }
    SkBlendMode blend = paint.getBlendMode();
    if (!SkBlendMode_AppendStages(blend)) {
        return nullptr;  // TODO
    }

    SkRasterPipeline shader, colorFilter;
    if (paint.getColorFilter() && !paint.getColorFilter()->appendStages(&colorFilter)) {
        return nullptr;
    }

    uint32_t paintColor = paint.getColor();

    SkColor4f color;
    if (SkImageInfoIsGammaCorrect(dst.info())) {
        color = SkColor4f::FromColor(paintColor);
    } else {
        swizzle_rb(SkNx_cast<float>(Sk4b::Load(&paintColor)) * (1/255.0f)).store(&color);
    }

    auto blitter = alloc->createT<SkRasterPipelineBlitter>(dst, shader, blend, color.premul());

    if (!paint.getShader()) {
        blitter->fShader.append(SkRasterPipeline::constant_color, &blitter->fPaintColor);
    }
    blitter->fShader.extend(colorFilter);

    return blitter;
}

void SkRasterPipelineBlitter::append_load_d(SkRasterPipeline* p) const {
    SkASSERT(supported(fDst.info()));

    switch (fDst.info().colorType()) {
        case kN32_SkColorType:
            if (fDst.info().gammaCloseToSRGB()) {
                p->append(SkRasterPipeline::load_d_srgb, &fDstPtr);
            }
            break;
        case kRGBA_F16_SkColorType:
            p->append(SkRasterPipeline::load_d_f16, &fDstPtr);
            break;
        case kRGB_565_SkColorType:
            p->append(SkRasterPipeline::load_d_565, &fDstPtr);
            break;
        default: break;
    }
}

void SkRasterPipelineBlitter::append_store(SkRasterPipeline* p) const {
    SkASSERT(supported(fDst.info()));

    switch (fDst.info().colorType()) {
        case kN32_SkColorType:
            if (fDst.info().gammaCloseToSRGB()) {
                p->append(SkRasterPipeline::store_srgb, &fDstPtr);
            }
            break;
        case kRGBA_F16_SkColorType:
            p->append(SkRasterPipeline::store_f16, &fDstPtr);
            break;
        case kRGB_565_SkColorType:
            p->append(SkRasterPipeline::store_565, &fDstPtr);
            break;
        default: break;
    }
}

void SkRasterPipelineBlitter::append_blend(SkRasterPipeline* p) const {
    SkAssertResult(SkBlendMode_AppendStages(fBlend, p));
}

void SkRasterPipelineBlitter::blitH(int x, int y, int w) {
    if (!fBlitH) {
        SkRasterPipeline p;
        p.extend(fShader);
        this->append_load_d(&p);
        this->append_blend(&p);
        this->append_store(&p);
        fBlitH = p.compile();
    }

    fDstPtr = fDst.writable_addr(0,y);
    fBlitH(x,w);
}

void SkRasterPipelineBlitter::blitAntiH(int x, int y, const SkAlpha aa[], const int16_t runs[]) {
    if (!fBlitAntiH) {
        SkRasterPipeline p;
        p.extend(fShader);
        this->append_load_d(&p);
        this->append_blend(&p);
        p.append(SkRasterPipeline::lerp_constant_float, &fConstantCoverage);
        this->append_store(&p);
        fBlitAntiH = p.compile();
    }

    fDstPtr = fDst.writable_addr(0,y);
    for (int16_t run = *runs; run > 0; run = *runs) {
        fConstantCoverage = *aa * (1/255.0f);
        fBlitAntiH(x, run);

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
        this->append_load_d(&p);
        this->append_blend(&p);
        p.append(SkRasterPipeline::lerp_u8, &fMaskPtr);
        this->append_store(&p);
        fBlitMaskA8 = p.compile();
    }

    if (mask.fFormat == SkMask::kLCD16_Format && !fBlitMaskLCD16) {
        SkRasterPipeline p;
        p.extend(fShader);
        this->append_load_d(&p);
        this->append_blend(&p);
        p.append(SkRasterPipeline::lerp_565, &fMaskPtr);
        this->append_store(&p);
        fBlitMaskLCD16 = p.compile();
    }

    int x = clip.left();
    for (int y = clip.top(); y < clip.bottom(); y++) {
        fDstPtr = fDst.writable_addr(0,y);

        switch (mask.fFormat) {
            case SkMask::kA8_Format:
                fMaskPtr = mask.getAddr8(x,y)-x;
                fBlitMaskA8(x, clip.width());
                break;
            case SkMask::kLCD16_Format:
                fMaskPtr = mask.getAddrLCD16(x,y)-x;
                fBlitMaskLCD16(x, clip.width());
                break;
            default:
                // TODO
                break;
        }
    }
}
