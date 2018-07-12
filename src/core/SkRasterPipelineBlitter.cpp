/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../jumper/SkJumper.h"
#include "SkArenaAlloc.h"
#include "SkBlendModePriv.h"
#include "SkBlitter.h"
#include "SkColor.h"
#include "SkColorFilter.h"
#include "SkColorSpaceXformer.h"
#include "SkColorSpaceXformSteps.h"
#include "SkOpts.h"
#include "SkPM4f.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
#include "SkShader.h"
#include "SkShaderBase.h"
#include "SkTo.h"
#include "SkUtils.h"

class SkRasterPipelineBlitter final : public SkBlitter {
public:
    // This is our common entrypoint for creating the blitter once we've sorted out shaders.
    static SkBlitter* Create(const SkPixmap&, const SkPaint&, SkArenaAlloc*,
                             const SkRasterPipeline& shaderPipeline,
                             SkShaderBase::Context*,
                             bool is_opaque, bool is_constant);

    SkRasterPipelineBlitter(SkPixmap dst,
                            SkBlendMode blend,
                            SkArenaAlloc* alloc,
                            SkShaderBase::Context* burstCtx)
        : fDst(dst)
        , fBlend(blend)
        , fAlloc(alloc)
        , fBurstCtx(burstCtx)
        , fColorPipeline(alloc)
    {}

    void blitH     (int x, int y, int w)                            override;
    void blitAntiH (int x, int y, const SkAlpha[], const int16_t[]) override;
    void blitAntiH2(int x, int y, U8CPU a0, U8CPU a1)               override;
    void blitAntiV2(int x, int y, U8CPU a0, U8CPU a1)               override;
    void blitMask  (const SkMask&, const SkIRect& clip)             override;
    void blitRect  (int x, int y, int width, int height)            override;
    void blitV     (int x, int y, int height, SkAlpha alpha)        override;

private:
    void append_load_dst(SkRasterPipeline*) const;
    void append_store   (SkRasterPipeline*) const;

    // If we have an burst context, use it to fill our shader buffer.
    void burst_shade(int x, int y, int w);

    SkPixmap               fDst;
    SkBlendMode            fBlend;
    SkArenaAlloc*          fAlloc;
    SkShaderBase::Context* fBurstCtx;
    SkRasterPipeline       fColorPipeline;

    SkJumper_MemoryCtx fShaderOutput = {nullptr,0},  // Possibly updated each call to burst_shade().
                       fDstPtr       = {nullptr,0},  // Always points to the top-left of fDst.
                       fMaskPtr      = {nullptr,0};  // Updated each call to blitMask().

    // We may be able to specialize blitH() or blitRect() into a memset.
    bool     fCanMemsetInBlitRect = false;
    uint64_t fMemsetColor      = 0;     // Big enough for largest dst format, F16.

    // Built lazily on first use.
    std::function<void(size_t, size_t, size_t, size_t)> fBlitRect,
                                                        fBlitAntiH,
                                                        fBlitMaskA8,
                                                        fBlitMaskLCD16;

    // These values are pointed to by the blit pipelines above,
    // which allows us to adjust them from call to call.
    float fCurrentCoverage = 0.0f;
    float fDitherRate      = 0.0f;

    std::vector<SkPM4f> fShaderBuffer;

    typedef SkBlitter INHERITED;
};

SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap& dst,
                                         const SkPaint& paint,
                                         const SkMatrix& ctm,
                                         SkArenaAlloc* alloc) {
    // For legacy/SkColorSpaceXformCanvas to keep working,
    // we need to sometimes still need to distinguish null dstCS from sRGB.
#if 0
    SkColorSpace* dstCS = dst.colorSpace() ? dst.colorSpace()
                                           : SkColorSpace::MakeSRGB().get();
#else
    SkColorSpace* dstCS = dst.colorSpace();
#endif
    SkPM4f paintColor = premul_in_dst_colorspace(paint.getColor(), dstCS);

    auto shader = as_SB(paint.getShader());

    SkRasterPipeline_<256> shaderPipeline;
    if (!shader) {
        // Having no shader makes things nice and easy... just use the paint color.
        shaderPipeline.append_constant_color(alloc, paintColor);
        bool is_opaque    = paintColor.a() == 1.0f,
             is_constant  = true;
        return SkRasterPipelineBlitter::Create(dst, paint, alloc,
                                               shaderPipeline, nullptr,
                                               is_opaque, is_constant);
    }

    bool is_opaque    = shader->isOpaque() && paintColor.a() == 1.0f;
    bool is_constant  = shader->isConstant();

    // Check whether the shader prefers to run in burst mode.
    if (auto* burstCtx = shader->makeBurstPipelineContext(
        SkShaderBase::ContextRec(paint, ctm, nullptr, SkShaderBase::ContextRec::kPM4f_DstType,
                                 dstCS), alloc)) {
        return SkRasterPipelineBlitter::Create(dst, paint, alloc,
                                               shaderPipeline, burstCtx,
                                               is_opaque, is_constant);
    }

    if (shader->appendStages({&shaderPipeline, alloc, dstCS, paint, nullptr, ctm})) {
        if (paintColor.a() != 1.0f) {
            shaderPipeline.append(SkRasterPipeline::scale_1_float,
                                  alloc->make<float>(paintColor.a()));
        }
        return SkRasterPipelineBlitter::Create(dst, paint, alloc, shaderPipeline, nullptr,
                                               is_opaque, is_constant);
    }

    // The shader has opted out of drawing anything.
    return alloc->make<SkNullBlitter>();
}

SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap& dst,
                                         const SkPaint& paint,
                                         const SkRasterPipeline& shaderPipeline,
                                         bool is_opaque,
                                         SkArenaAlloc* alloc) {
    bool is_constant = false;  // If this were the case, it'd be better to just set a paint color.
    return SkRasterPipelineBlitter::Create(dst, paint, alloc, shaderPipeline, nullptr,
                                           is_opaque, is_constant);
}

SkBlitter* SkRasterPipelineBlitter::Create(const SkPixmap& dst,
                                           const SkPaint& paint,
                                           SkArenaAlloc* alloc,
                                           const SkRasterPipeline& shaderPipeline,
                                           SkShaderBase::Context* burstCtx,
                                           bool is_opaque,
                                           bool is_constant) {
    auto blitter = alloc->make<SkRasterPipelineBlitter>(dst,
                                                        paint.getBlendMode(),
                                                        alloc,
                                                        burstCtx);

    // Our job in this factory is to fill out the blitter's color pipeline.
    // This is the common front of the full blit pipelines, each constructed lazily on first use.
    // The full blit pipelines handle reading and writing the dst, blending, coverage, dithering.
    auto colorPipeline = &blitter->fColorPipeline;

    // Let's get the shader in first.
    if (burstCtx) {
        colorPipeline->append(SkRasterPipeline::load_f32, &blitter->fShaderOutput);
    } else {
        colorPipeline->extend(shaderPipeline);
    }

    // If there's a color filter it comes next.
    if (auto colorFilter = paint.getColorFilter()) {
        colorFilter->appendStages(colorPipeline, dst.colorSpace(), alloc, is_opaque);
        is_opaque = is_opaque && (colorFilter->getFlags() & SkColorFilter::kAlphaUnchanged_Flag);
    }

    // Not all formats make sense to dither (think, F16).  We set their dither rate
    // to zero.  We need to decide if we're going to dither now to keep is_constant accurate.
    if (paint.isDither()) {
        switch (dst.info().colorType()) {
            default:                        blitter->fDitherRate =      0.0f; break;
            case kARGB_4444_SkColorType:    blitter->fDitherRate =   1/15.0f; break;
            case   kRGB_565_SkColorType:    blitter->fDitherRate =   1/63.0f; break;
            case    kGray_8_SkColorType:
            case  kRGB_888x_SkColorType:
            case kRGBA_8888_SkColorType:
            case kBGRA_8888_SkColorType:    blitter->fDitherRate =  1/255.0f; break;
            case kRGB_101010x_SkColorType:
            case kRGBA_1010102_SkColorType: blitter->fDitherRate = 1/1023.0f; break;
        }
        // TODO: for constant colors, we could try to measure the effect of dithering, and if
        //       it has no value (i.e. all variations result in the same 32bit color, then we
        //       could disable it (for speed, by not adding the stage).
    }
    is_constant = is_constant && (blitter->fDitherRate == 0.0f);

    // We're logically done here.  The code between here and return blitter is all optimization.

    // A pipeline that's still constant here can collapse back into a constant color.
    if (is_constant) {
        SkPM4f constantColor;
        SkJumper_MemoryCtx constantColorPtr = { &constantColor, 0 };
        colorPipeline->append(SkRasterPipeline::store_f32, &constantColorPtr);
        colorPipeline->run(0,0,1,1);
        colorPipeline->reset();
        colorPipeline->append_constant_color(alloc, constantColor);

        is_opaque = constantColor.a() == 1.0f;
    }

    // We can strength-reduce SrcOver into Src when opaque.
    if (is_opaque && blitter->fBlend == SkBlendMode::kSrcOver) {
        blitter->fBlend = SkBlendMode::kSrc;
    }

    // When we're drawing a constant color in Src mode, we can sometimes just memset.
    // (The previous two optimizations help find more opportunities for this one.)
    if (is_constant && blitter->fBlend == SkBlendMode::kSrc
                    && blitter->fDst.shiftPerPixel() <= 3 /*TODO: F32*/) {
        // Run our color pipeline all the way through to produce what we'd memset when we can.
        // Not all blits can memset, so we need to keep colorPipeline too.
        SkRasterPipeline_<256> p;
        p.extend(*colorPipeline);
        blitter->fDstPtr = SkJumper_MemoryCtx{&blitter->fMemsetColor, 0};
        blitter->append_store(&p);
        p.run(0,0,1,1);

        blitter->fCanMemsetInBlitRect = true;
    }

    blitter->fDstPtr = SkJumper_MemoryCtx{
        blitter->fDst.writable_addr(),
        blitter->fDst.rowBytesAsPixels(),
    };

    return blitter;
}

void SkRasterPipelineBlitter::append_load_dst(SkRasterPipeline* p) const {
    const void* ctx = &fDstPtr;
    switch (fDst.info().colorType()) {
        default: break;

        case kGray_8_SkColorType:       p->append(SkRasterPipeline::load_g8_dst,      ctx); break;
        case kAlpha_8_SkColorType:      p->append(SkRasterPipeline::load_a8_dst,      ctx); break;
        case kRGB_565_SkColorType:      p->append(SkRasterPipeline::load_565_dst,     ctx); break;
        case kARGB_4444_SkColorType:    p->append(SkRasterPipeline::load_4444_dst,    ctx); break;
        case kBGRA_8888_SkColorType:    p->append(SkRasterPipeline::load_bgra_dst,    ctx); break;
        case kRGBA_8888_SkColorType:    p->append(SkRasterPipeline::load_8888_dst,    ctx); break;
        case kRGBA_1010102_SkColorType: p->append(SkRasterPipeline::load_1010102_dst, ctx); break;
        case kRGBA_F16_SkColorType:     p->append(SkRasterPipeline::load_f16_dst,     ctx); break;
        case kRGBA_F32_SkColorType:     p->append(SkRasterPipeline::load_f32_dst,     ctx); break;

        case kRGB_888x_SkColorType:     p->append(SkRasterPipeline::load_8888_dst,    ctx);
                                        p->append(SkRasterPipeline::force_opaque_dst     ); break;
        case kRGB_101010x_SkColorType:  p->append(SkRasterPipeline::load_1010102_dst, ctx);
                                        p->append(SkRasterPipeline::force_opaque_dst     ); break;
    }
    if (fDst.info().alphaType() == kUnpremul_SkAlphaType) {
        p->append(SkRasterPipeline::premul_dst);
    }
}

void SkRasterPipelineBlitter::append_store(SkRasterPipeline* p) const {
    if (fDst.info().alphaType() == kUnpremul_SkAlphaType) {
        p->append(SkRasterPipeline::unpremul);
    }
    if (fDitherRate > 0.0f) {
        p->append(SkRasterPipeline::dither, &fDitherRate);
    }

    const void* ctx = &fDstPtr;
    switch (fDst.info().colorType()) {
        default: break;

        case kGray_8_SkColorType:       p->append(SkRasterPipeline::luminance_to_alpha);
                                        p->append(SkRasterPipeline::store_a8,      ctx); break;
        case kAlpha_8_SkColorType:      p->append(SkRasterPipeline::store_a8,      ctx); break;
        case kRGB_565_SkColorType:      p->append(SkRasterPipeline::store_565,     ctx); break;
        case kARGB_4444_SkColorType:    p->append(SkRasterPipeline::store_4444,    ctx); break;
        case kBGRA_8888_SkColorType:    p->append(SkRasterPipeline::store_bgra,    ctx); break;
        case kRGBA_8888_SkColorType:    p->append(SkRasterPipeline::store_8888,    ctx); break;
        case kRGBA_1010102_SkColorType: p->append(SkRasterPipeline::store_1010102, ctx); break;
        case kRGBA_F16_SkColorType:     p->append(SkRasterPipeline::store_f16,     ctx); break;
        case kRGBA_F32_SkColorType:     p->append(SkRasterPipeline::store_f32,     ctx); break;

        case kRGB_888x_SkColorType:     p->append(SkRasterPipeline::force_opaque         );
                                        p->append(SkRasterPipeline::store_8888,       ctx); break;
        case kRGB_101010x_SkColorType:  p->append(SkRasterPipeline::force_opaque         );
                                        p->append(SkRasterPipeline::store_1010102,    ctx); break;
    }
}

void SkRasterPipelineBlitter::burst_shade(int x, int y, int w) {
    SkASSERT(fBurstCtx);
    if (w > SkToInt(fShaderBuffer.size())) {
        fShaderBuffer.resize(w);
    }
    fBurstCtx->shadeSpan4f(x,y, fShaderBuffer.data(), w);
    // We'll be reading from fShaderOutput.pixels + x, so back up by x.
    fShaderOutput = SkJumper_MemoryCtx{ fShaderBuffer.data() - x, 0 };
}

void SkRasterPipelineBlitter::blitH(int x, int y, int w) {
    this->blitRect(x,y,w,1);
}

void SkRasterPipelineBlitter::blitRect(int x, int y, int w, int h) {
    if (fCanMemsetInBlitRect) {
        for (int ylimit = y+h; y < ylimit; y++) {
            switch (fDst.shiftPerPixel()) {
                case 0:    memset  (fDst.writable_addr8 (x,y), fMemsetColor, w); break;
                case 1: sk_memset16(fDst.writable_addr16(x,y), fMemsetColor, w); break;
                case 2: sk_memset32(fDst.writable_addr32(x,y), fMemsetColor, w); break;
                case 3: sk_memset64(fDst.writable_addr64(x,y), fMemsetColor, w); break;
                default: SkASSERT(false); break;
            }
        }
        return;
    }

    if (!fBlitRect) {
        SkRasterPipeline p(fAlloc);
        p.extend(fColorPipeline);
        if (fBlend == SkBlendMode::kSrcOver
                && (fDst.info().colorType() == kRGBA_8888_SkColorType ||
                    fDst.info().colorType() == kBGRA_8888_SkColorType)
                && !fDst.colorSpace()
                && fDst.info().alphaType() != kUnpremul_SkAlphaType
                && fDitherRate == 0.0f) {
            auto stage = fDst.info().colorType() == kRGBA_8888_SkColorType
                       ? SkRasterPipeline::srcover_rgba_8888
                       : SkRasterPipeline::srcover_bgra_8888;
            p.append(stage, &fDstPtr);
        } else {
            if (fBlend != SkBlendMode::kSrc) {
                this->append_load_dst(&p);
                SkBlendMode_AppendStages(fBlend, &p);
            }
            this->append_store(&p);
        }
        fBlitRect = p.compile();
    }

    if (fBurstCtx) {
        // We can only burst shade one row at a time.
        for (int ylimit = y+h; y < ylimit; y++) {
            this->burst_shade(x,y,w);
            fBlitRect(x,y, w,1);
        }
    } else {
        // If not bursting we can blit the entire rect at once.
        fBlitRect(x,y,w,h);
    }
}

void SkRasterPipelineBlitter::blitAntiH(int x, int y, const SkAlpha aa[], const int16_t runs[]) {
    if (!fBlitAntiH) {
        SkRasterPipeline p(fAlloc);
        p.extend(fColorPipeline);
        if (SkBlendMode_ShouldPreScaleCoverage(fBlend, /*rgb_coverage=*/false)) {
            p.append(SkRasterPipeline::scale_1_float, &fCurrentCoverage);
            this->append_load_dst(&p);
            SkBlendMode_AppendStages(fBlend, &p);
        } else {
            this->append_load_dst(&p);
            SkBlendMode_AppendStages(fBlend, &p);
            p.append(SkRasterPipeline::lerp_1_float, &fCurrentCoverage);
        }

        this->append_store(&p);
        fBlitAntiH = p.compile();
    }

    for (int16_t run = *runs; run > 0; run = *runs) {
        switch (*aa) {
            case 0x00:                       break;
            case 0xff: this->blitH(x,y,run); break;
            default:
                fCurrentCoverage = *aa * (1/255.0f);
                if (fBurstCtx) {
                    this->burst_shade(x,y,run);
                }
                fBlitAntiH(x,y,run,1);
        }
        x    += run;
        runs += run;
        aa   += run;
    }
}

void SkRasterPipelineBlitter::blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) {
    SkIRect clip = {x,y, x+2,y+1};
    uint8_t coverage[] = { (uint8_t)a0, (uint8_t)a1 };

    SkMask mask;
    mask.fImage    = coverage;
    mask.fBounds   = clip;
    mask.fRowBytes = 2;
    mask.fFormat   = SkMask::kA8_Format;

    this->blitMask(mask, clip);
}

void SkRasterPipelineBlitter::blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) {
    SkIRect clip = {x,y, x+1,y+2};
    uint8_t coverage[] = { (uint8_t)a0, (uint8_t)a1 };

    SkMask mask;
    mask.fImage    = coverage;
    mask.fBounds   = clip;
    mask.fRowBytes = 1;
    mask.fFormat   = SkMask::kA8_Format;

    this->blitMask(mask, clip);
}

void SkRasterPipelineBlitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkIRect clip = {x,y, x+1,y+height};

    SkMask mask;
    mask.fImage    = &alpha;
    mask.fBounds   = clip;
    mask.fRowBytes = 0;     // so we reuse the 1 "row" for all of height
    mask.fFormat   = SkMask::kA8_Format;

    this->blitMask(mask, clip);
}

void SkRasterPipelineBlitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    if (mask.fFormat == SkMask::kBW_Format) {
        // TODO: native BW masks?
        return INHERITED::blitMask(mask, clip);
    }

    // We'll use the first (A8) plane of any mask and ignore the other two, just like Ganesh.
    SkMask::Format effectiveMaskFormat = mask.fFormat == SkMask::k3D_Format ? SkMask::kA8_Format
                                                                            : mask.fFormat;


    // Lazily build whichever pipeline we need, specialized for each mask format.
    if (effectiveMaskFormat == SkMask::kA8_Format && !fBlitMaskA8) {
        SkRasterPipeline p(fAlloc);
        p.extend(fColorPipeline);
        if (SkBlendMode_ShouldPreScaleCoverage(fBlend, /*rgb_coverage=*/false)) {
            p.append(SkRasterPipeline::scale_u8, &fMaskPtr);
            this->append_load_dst(&p);
            SkBlendMode_AppendStages(fBlend, &p);
        } else {
            this->append_load_dst(&p);
            SkBlendMode_AppendStages(fBlend, &p);
            p.append(SkRasterPipeline::lerp_u8, &fMaskPtr);
        }
        this->append_store(&p);
        fBlitMaskA8 = p.compile();
    }
    if (effectiveMaskFormat == SkMask::kLCD16_Format && !fBlitMaskLCD16) {
        SkRasterPipeline p(fAlloc);
        p.extend(fColorPipeline);
        if (SkBlendMode_ShouldPreScaleCoverage(fBlend, /*rgb_coverage=*/true)) {
            // Somewhat unusually, scale_565 needs dst loaded first.
            this->append_load_dst(&p);
            p.append(SkRasterPipeline::scale_565, &fMaskPtr);
            SkBlendMode_AppendStages(fBlend, &p);
        } else {
            this->append_load_dst(&p);
            SkBlendMode_AppendStages(fBlend, &p);
            p.append(SkRasterPipeline::lerp_565, &fMaskPtr);
        }
        this->append_store(&p);
        fBlitMaskLCD16 = p.compile();
    }

    std::function<void(size_t,size_t,size_t,size_t)>* blitter = nullptr;
    // Update fMaskPtr to point "into" this current mask, but lined up with fDstPtr at (0,0).
    // This sort of trickery upsets UBSAN (pointer-overflow) so we do our math in uintptr_t.

    // mask.fRowBytes is a uint32_t, which would break our addressing math on 64-bit builds.
    size_t rowBytes = mask.fRowBytes;
    switch (effectiveMaskFormat) {
        case SkMask::kA8_Format:
            fMaskPtr.stride = rowBytes;
            fMaskPtr.pixels = (void*)((uintptr_t)mask.fImage - mask.fBounds.left() * (size_t)1
                                                             - mask.fBounds.top()  * rowBytes);
            blitter = &fBlitMaskA8;
            break;
        case SkMask::kLCD16_Format:
            fMaskPtr.stride = rowBytes / 2;
            fMaskPtr.pixels = (void*)((uintptr_t)mask.fImage - mask.fBounds.left() * (size_t)2
                                                             - mask.fBounds.top()  * rowBytes);
            blitter = &fBlitMaskLCD16;
            break;
        default:
            return;
    }

    SkASSERT(blitter);
    if (fBurstCtx) {
        // We can only burst shade one row at a time.
        int x = clip.left();
        for (int y = clip.top(); y < clip.bottom(); y++) {
            this->burst_shade(x,y,clip.width());
            (*blitter)(x,y, clip.width(),1);
        }
    } else {
        // If not bursting we can blit the entire mask at once.
        (*blitter)(clip.left(),clip.top(), clip.width(),clip.height());
    }
}
