/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/private/SkTo.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkUtils.h"
#include "src/shaders/SkShaderBase.h"

class SkRasterPipelineBlitter final : public SkBlitter {
public:
    // This is our common entrypoint for creating the blitter once we've sorted out shaders.
    static SkBlitter* Create(const SkPixmap&, const SkPaint&, SkArenaAlloc*,
                             const SkRasterPipeline& shaderPipeline,
                             bool is_opaque, bool is_constant,
                             sk_sp<SkShader> clipShader);

    SkRasterPipelineBlitter(SkPixmap dst,
                            SkBlendMode blend,
                            SkArenaAlloc* alloc)
        : fDst(dst)
        , fBlend(blend)
        , fAlloc(alloc)
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
    void append_load_dst      (SkRasterPipeline*) const;
    void append_store         (SkRasterPipeline*) const;

    // these check internally, and only append if there was a native clipShader
    void append_clip_scale    (SkRasterPipeline*) const;
    void append_clip_lerp     (SkRasterPipeline*) const;

    SkPixmap               fDst;
    SkBlendMode            fBlend;
    SkArenaAlloc*          fAlloc;
    SkRasterPipeline       fColorPipeline;
    // set to pipeline storage (for alpha) if we have a clipShader
    void*                  fClipShaderBuffer = nullptr; // "native" : float or U16

    SkRasterPipeline_MemoryCtx
        fDstPtr       = {nullptr,0},  // Always points to the top-left of fDst.
        fMaskPtr      = {nullptr,0};  // Updated each call to blitMask().
    SkRasterPipeline_EmbossCtx fEmbossCtx;  // Used only for k3D_Format masks.

    // We may be able to specialize blitH() or blitRect() into a memset.
    void   (*fMemset2D)(SkPixmap*, int x,int y, int w,int h, uint64_t color) = nullptr;
    uint64_t fMemsetColor = 0;   // Big enough for largest memsettable dst format, F16.

    // Built lazily on first use.
    std::function<void(size_t, size_t, size_t, size_t)> fBlitRect,
                                                        fBlitAntiH,
                                                        fBlitMaskA8,
                                                        fBlitMaskLCD16,
                                                        fBlitMask3D;

    // These values are pointed to by the blit pipelines above,
    // which allows us to adjust them from call to call.
    float fCurrentCoverage = 0.0f;
    float fDitherRate      = 0.0f;

    typedef SkBlitter INHERITED;
};

SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap& dst,
                                         const SkPaint& paint,
                                         const SkMatrix& ctm,
                                         SkArenaAlloc* alloc,
                                         sk_sp<SkShader> clipShader) {
    // For legacy to keep working, we need to sometimes still distinguish null dstCS from sRGB.
#if 0
    SkColorSpace* dstCS = dst.colorSpace() ? dst.colorSpace()
                                           : sk_srgb_singleton();
#else
    SkColorSpace* dstCS = dst.colorSpace();
#endif
    SkColorType dstCT = dst.colorType();
    SkColor4f paintColor = paint.getColor4f();
    SkColorSpaceXformSteps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                           dstCS,               kUnpremul_SkAlphaType).apply(paintColor.vec());

    auto shader = as_SB(paint.getShader());

    SkRasterPipeline_<256> shaderPipeline;
    if (!shader) {
        // Having no shader makes things nice and easy... just use the paint color.
        shaderPipeline.append_constant_color(alloc, paintColor.premul().vec());
        bool is_opaque    = paintColor.fA == 1.0f,
             is_constant  = true;
        return SkRasterPipelineBlitter::Create(dst, paint, alloc,
                                               shaderPipeline, is_opaque, is_constant,
                                               std::move(clipShader));
    }

    bool is_opaque    = shader->isOpaque() && paintColor.fA == 1.0f;
    bool is_constant  = shader->isConstant();

    if (shader->appendStages({&shaderPipeline, alloc, dstCT, dstCS, paint, nullptr, ctm})) {
        if (paintColor.fA != 1.0f) {
            shaderPipeline.append(SkRasterPipeline::scale_1_float,
                                  alloc->make<float>(paintColor.fA));
        }
        return SkRasterPipelineBlitter::Create(dst, paint, alloc,
                                               shaderPipeline, is_opaque, is_constant,
                                               std::move(clipShader));
    }

    // The shader has opted out of drawing anything.
    return alloc->make<SkNullBlitter>();
}

SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap& dst,
                                         const SkPaint& paint,
                                         const SkRasterPipeline& shaderPipeline,
                                         bool is_opaque,
                                         SkArenaAlloc* alloc,
                                         sk_sp<SkShader> clipShader) {
    bool is_constant = false;  // If this were the case, it'd be better to just set a paint color.
    return SkRasterPipelineBlitter::Create(dst, paint, alloc,
                                           shaderPipeline, is_opaque, is_constant,
                                           clipShader);
}

SkBlitter* SkRasterPipelineBlitter::Create(const SkPixmap& dst,
                                           const SkPaint& paint,
                                           SkArenaAlloc* alloc,
                                           const SkRasterPipeline& shaderPipeline,
                                           bool is_opaque,
                                           bool is_constant,
                                           sk_sp<SkShader> clipShader) {
    auto blitter = alloc->make<SkRasterPipelineBlitter>(dst,
                                                        paint.getBlendMode(),
                                                        alloc);


    // Our job in this factory is to fill out the blitter's color pipeline.
    // This is the common front of the full blit pipelines, each constructed lazily on first use.
    // The full blit pipelines handle reading and writing the dst, blending, coverage, dithering.
    auto colorPipeline = &blitter->fColorPipeline;

    if (clipShader) {
        auto clipP = colorPipeline;
        SkPaint clipPaint;  // just need default values
        SkColorType clipCT = kRGBA_8888_SkColorType;
        SkColorSpace* clipCS = nullptr;
        SkMatrix clipM = SkMatrix::I();
        SkStageRec rec = {clipP, alloc, clipCT, clipCS, clipPaint, nullptr, clipM};
        if (as_SB(clipShader)->appendStages(rec)) {
            struct Storage {
                // large enough for highp (float) or lowp(U16)
                float   fA[SkRasterPipeline_kMaxStride];
            };
            auto storage = alloc->make<Storage>();
            clipP->append(SkRasterPipeline::store_src_a, storage->fA);
            blitter->fClipShaderBuffer = storage->fA;
            is_constant = false;
        }
    }

    // Let's get the shader in first.
    colorPipeline->extend(shaderPipeline);

    // If there's a color filter it comes next.
    if (auto colorFilter = paint.getColorFilter()) {
        SkStageRec rec = {
            colorPipeline, alloc, dst.colorType(), dst.colorSpace(), paint, nullptr, SkMatrix::I()
        };
        colorFilter->appendStages(rec, is_opaque);
        is_opaque = is_opaque && (colorFilter->getFlags() & SkColorFilter::kAlphaUnchanged_Flag);
    }

    // Not all formats make sense to dither (think, F16).  We set their dither rate
    // to zero.  We need to decide if we're going to dither now to keep is_constant accurate.
    if (paint.isDither()) {
        switch (dst.info().colorType()) {
            case kARGB_4444_SkColorType:    blitter->fDitherRate =   1/15.0f; break;
            case   kRGB_565_SkColorType:    blitter->fDitherRate =   1/63.0f; break;
            case    kGray_8_SkColorType:
            case  kRGB_888x_SkColorType:
            case kRGBA_8888_SkColorType:
            case kBGRA_8888_SkColorType:    blitter->fDitherRate =  1/255.0f; break;
            case kRGB_101010x_SkColorType:
            case kRGBA_1010102_SkColorType:
            case kBGR_101010x_SkColorType:
            case kBGRA_1010102_SkColorType: blitter->fDitherRate = 1/1023.0f; break;

            case kUnknown_SkColorType:
            case kAlpha_8_SkColorType:
            case kRGBA_F16_SkColorType:
            case kRGBA_F16Norm_SkColorType:
            case kRGBA_F32_SkColorType:
            case kR8G8_unorm_SkColorType:
            case kA16_float_SkColorType:
            case kA16_unorm_SkColorType:
            case kR16G16_float_SkColorType:
            case kR16G16_unorm_SkColorType:
            case kR16G16B16A16_unorm_SkColorType: blitter->fDitherRate = 0.0f; break;
        }
        // TODO: for constant colors, we could try to measure the effect of dithering, and if
        //       it has no value (i.e. all variations result in the same 32bit color, then we
        //       could disable it (for speed, by not adding the stage).
    }
    is_constant = is_constant && (blitter->fDitherRate == 0.0f);

    // We're logically done here.  The code between here and return blitter is all optimization.

    // A pipeline that's still constant here can collapse back into a constant color.
    if (is_constant) {
        SkColor4f constantColor;
        SkRasterPipeline_MemoryCtx constantColorPtr = { &constantColor, 0 };
        colorPipeline->append_gamut_clamp_if_normalized(dst.info());
        colorPipeline->append(SkRasterPipeline::store_f32, &constantColorPtr);
        colorPipeline->run(0,0,1,1);
        colorPipeline->reset();
        colorPipeline->append_constant_color(alloc, constantColor);

        is_opaque = constantColor.fA == 1.0f;
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
        p.append_gamut_clamp_if_normalized(dst.info());
        blitter->fDstPtr = SkRasterPipeline_MemoryCtx{&blitter->fMemsetColor, 0};
        blitter->append_store(&p);
        p.run(0,0,1,1);

        switch (blitter->fDst.shiftPerPixel()) {
            case 0: blitter->fMemset2D = [](SkPixmap* dst, int x,int y, int w,int h, uint64_t c) {
                void* p = dst->writable_addr(x,y);
                while (h --> 0) {
                    memset(p, c, w);
                    p = SkTAddOffset<void>(p, dst->rowBytes());
                }
            }; break;

            case 1: blitter->fMemset2D = [](SkPixmap* dst, int x,int y, int w,int h, uint64_t c) {
                SkOpts::rect_memset16(dst->writable_addr16(x,y), c, w, dst->rowBytes(), h);
            }; break;

            case 2: blitter->fMemset2D = [](SkPixmap* dst, int x,int y, int w,int h, uint64_t c) {
                SkOpts::rect_memset32(dst->writable_addr32(x,y), c, w, dst->rowBytes(), h);
            }; break;

            case 3: blitter->fMemset2D = [](SkPixmap* dst, int x,int y, int w,int h, uint64_t c) {
                SkOpts::rect_memset64(dst->writable_addr64(x,y), c, w, dst->rowBytes(), h);
            }; break;

            // TODO(F32)?
        }
    }

    blitter->fDstPtr = SkRasterPipeline_MemoryCtx{
        blitter->fDst.writable_addr(),
        blitter->fDst.rowBytesAsPixels(),
    };

    return blitter;
}

void SkRasterPipelineBlitter::append_load_dst(SkRasterPipeline* p) const {
    p->append_load_dst(fDst.info().colorType(), &fDstPtr);
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

    p->append_store(fDst.info().colorType(), &fDstPtr);
}

void SkRasterPipelineBlitter::append_clip_scale(SkRasterPipeline* p) const {
    if (fClipShaderBuffer) {
        p->append(SkRasterPipeline::scale_native, fClipShaderBuffer);
    }
}

void SkRasterPipelineBlitter::append_clip_lerp(SkRasterPipeline* p) const {
    if (fClipShaderBuffer) {
        p->append(SkRasterPipeline::lerp_native, fClipShaderBuffer);
    }
}

void SkRasterPipelineBlitter::blitH(int x, int y, int w) {
    this->blitRect(x,y,w,1);
}

void SkRasterPipelineBlitter::blitRect(int x, int y, int w, int h) {
    if (fMemset2D) {
        fMemset2D(&fDst, x,y, w,h, fMemsetColor);
        return;
    }

    if (!fBlitRect) {
        SkRasterPipeline p(fAlloc);
        p.extend(fColorPipeline);
        p.append_gamut_clamp_if_normalized(fDst.info());
        if (fBlend == SkBlendMode::kSrcOver
                && (fDst.info().colorType() == kRGBA_8888_SkColorType ||
                    fDst.info().colorType() == kBGRA_8888_SkColorType)
                && !fDst.colorSpace()
                && fDst.info().alphaType() != kUnpremul_SkAlphaType
                && fDitherRate == 0.0f) {
            if (fDst.info().colorType() == kBGRA_8888_SkColorType) {
                p.append(SkRasterPipeline::swap_rb);
            }
            this->append_clip_scale(&p);
            p.append(SkRasterPipeline::srcover_rgba_8888, &fDstPtr);
        } else {
            if (fBlend != SkBlendMode::kSrc) {
                this->append_load_dst(&p);
                SkBlendMode_AppendStages(fBlend, &p);
                this->append_clip_lerp(&p);
            } else if (fClipShaderBuffer) {
                this->append_load_dst(&p);
                this->append_clip_lerp(&p);
            }
            this->append_store(&p);
        }
        fBlitRect = p.compile();
    }

    fBlitRect(x,y,w,h);
}

void SkRasterPipelineBlitter::blitAntiH(int x, int y, const SkAlpha aa[], const int16_t runs[]) {
    if (!fBlitAntiH) {
        SkRasterPipeline p(fAlloc);
        p.extend(fColorPipeline);
        p.append_gamut_clamp_if_normalized(fDst.info());
        if (SkBlendMode_ShouldPreScaleCoverage(fBlend, /*rgb_coverage=*/false)) {
            p.append(SkRasterPipeline::scale_1_float, &fCurrentCoverage);
            this->append_clip_scale(&p);
            this->append_load_dst(&p);
            SkBlendMode_AppendStages(fBlend, &p);
        } else {
            this->append_load_dst(&p);
            SkBlendMode_AppendStages(fBlend, &p);
            p.append(SkRasterPipeline::lerp_1_float, &fCurrentCoverage);
            this->append_clip_lerp(&p);
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

    // ARGB and SDF masks shouldn't make it here.
    SkASSERT(mask.fFormat == SkMask::kA8_Format
          || mask.fFormat == SkMask::kLCD16_Format
          || mask.fFormat == SkMask::k3D_Format);

    auto extract_mask_plane = [&mask](int plane, SkRasterPipeline_MemoryCtx* ctx) {
        // LCD is 16-bit per pixel; A8 and 3D are 8-bit per pixel.
        size_t bpp = mask.fFormat == SkMask::kLCD16_Format ? 2 : 1;

        // Select the right mask plane.  Usually plane == 0 and this is just mask.fImage.
        auto ptr = (uintptr_t)mask.fImage
                 + plane * mask.computeImageSize();

        // Update ctx to point "into" this current mask, but lined up with fDstPtr at (0,0).
        // This sort of trickery upsets UBSAN (pointer-overflow) so our ptr must be a uintptr_t.
        // mask.fRowBytes is a uint32_t, which would break our addressing math on 64-bit builds.
        size_t rowBytes = mask.fRowBytes;
        ctx->stride = rowBytes / bpp;
        ctx->pixels = (void*)(ptr - mask.fBounds.left() * bpp
                                  - mask.fBounds.top()  * rowBytes);
    };

    extract_mask_plane(0, &fMaskPtr);
    if (mask.fFormat == SkMask::k3D_Format) {
        extract_mask_plane(1, &fEmbossCtx.mul);
        extract_mask_plane(2, &fEmbossCtx.add);
    }

    // Lazily build whichever pipeline we need, specialized for each mask format.
    if (mask.fFormat == SkMask::kA8_Format && !fBlitMaskA8) {
        SkRasterPipeline p(fAlloc);
        p.extend(fColorPipeline);
        p.append_gamut_clamp_if_normalized(fDst.info());
        if (SkBlendMode_ShouldPreScaleCoverage(fBlend, /*rgb_coverage=*/false)) {
            p.append(SkRasterPipeline::scale_u8, &fMaskPtr);
            this->append_clip_scale(&p);
            this->append_load_dst(&p);
            SkBlendMode_AppendStages(fBlend, &p);
        } else {
            this->append_load_dst(&p);
            SkBlendMode_AppendStages(fBlend, &p);
            p.append(SkRasterPipeline::lerp_u8, &fMaskPtr);
            this->append_clip_lerp(&p);
        }
        this->append_store(&p);
        fBlitMaskA8 = p.compile();
    }
    if (mask.fFormat == SkMask::kLCD16_Format && !fBlitMaskLCD16) {
        SkRasterPipeline p(fAlloc);
        p.extend(fColorPipeline);
        p.append_gamut_clamp_if_normalized(fDst.info());
        if (SkBlendMode_ShouldPreScaleCoverage(fBlend, /*rgb_coverage=*/true)) {
            // Somewhat unusually, scale_565 needs dst loaded first.
            this->append_load_dst(&p);
            p.append(SkRasterPipeline::scale_565, &fMaskPtr);
            this->append_clip_scale(&p);
            SkBlendMode_AppendStages(fBlend, &p);
        } else {
            this->append_load_dst(&p);
            SkBlendMode_AppendStages(fBlend, &p);
            p.append(SkRasterPipeline::lerp_565, &fMaskPtr);
            this->append_clip_lerp(&p);
        }
        this->append_store(&p);
        fBlitMaskLCD16 = p.compile();
    }
    if (mask.fFormat == SkMask::k3D_Format && !fBlitMask3D) {
        SkRasterPipeline p(fAlloc);
        p.extend(fColorPipeline);
        // This bit is where we differ from kA8_Format:
        p.append(SkRasterPipeline::emboss, &fEmbossCtx);
        // Now onward just as kA8.
        p.append_gamut_clamp_if_normalized(fDst.info());
        if (SkBlendMode_ShouldPreScaleCoverage(fBlend, /*rgb_coverage=*/false)) {
            p.append(SkRasterPipeline::scale_u8, &fMaskPtr);
            this->append_clip_scale(&p);
            this->append_load_dst(&p);
            SkBlendMode_AppendStages(fBlend, &p);
        } else {
            this->append_load_dst(&p);
            SkBlendMode_AppendStages(fBlend, &p);
            p.append(SkRasterPipeline::lerp_u8, &fMaskPtr);
            this->append_clip_lerp(&p);
        }
        this->append_store(&p);
        fBlitMask3D = p.compile();
    }

    std::function<void(size_t,size_t,size_t,size_t)>* blitter = nullptr;
    switch (mask.fFormat) {
        case SkMask::kA8_Format:    blitter = &fBlitMaskA8;    break;
        case SkMask::kLCD16_Format: blitter = &fBlitMaskLCD16; break;
        case SkMask::k3D_Format:    blitter = &fBlitMask3D;    break;
        default:
            SkASSERT(false);
            return;
    }

    SkASSERT(blitter);
    (*blitter)(clip.left(),clip.top(), clip.width(),clip.height());
}
