/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkCPUTypes.h"
#include "include/private/base/SkOnce.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkMask.h"
#include "src/core/SkMemset.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkRasterPipelineVizualizer.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"
#include "src/shaders/SkShaderBase.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

class SkColorSpace;
class SkShader;

static bool can_direct_blit(const SkPaint& paint) {
    if (paint.getShader() || paint.getColorFilter() || paint.isDither()) {
        return false;
    }
    if (auto mode = paint.asBlendMode()) {
        switch (mode.value()) {
            case SkBlendMode::kClear: [[fallthrough]];
            case SkBlendMode::kSrc:
                return true;
            case SkBlendMode::kSrcOver:
                return paint.getAlphaf() == 1;
            default:
                break;
        }
    }
    return false;
}

class SkRasterPipelineBlitter final : public SkBlitter {
public:
    // This is our common entrypoint for creating the blitter once we've sorted out shaders.
    static SkBlitter* Create(const SkPixmap& dst,
                             const SkPaint& paint,
                             const SkColor4f& dstPaintColor,
                             SkArenaAlloc* alloc,
                             const SkRasterPipeline& shaderPipeline,
                             bool is_opaque,
                             bool is_constant,
                             const SkShader* clipShader);

    SkRasterPipelineBlitter(SkPixmap dst,
                            const SkPaint& paint,
                            SkArenaAlloc* alloc)
        : fDst(std::move(dst))
        , fAlloc(alloc)
        , fColorPipeline(alloc)
        , fBlendPipeline(alloc)
        , fCanDirectBlit(can_direct_blit(paint))
        , fDirectBlitPaintColor(paint.getColor4f())
    {}

    void blitH     (int x, int y, int w)                            override;
    void blitAntiH (int x, int y, const SkAlpha[], const int16_t[]) override;
    void blitAntiH2(int x, int y, U8CPU a0, U8CPU a1)               override;
    void blitAntiV2(int x, int y, U8CPU a0, U8CPU a1)               override;
    void blitMask  (const SkMask&, const SkIRect& clip)             override;
    void blitRect  (int x, int y, int width, int height)            override;
    void blitV     (int x, int y, int height, SkAlpha alpha)        override;
    std::optional<DirectBlit> canDirectBlit()                       override;

private:
    void appendLoadDst      (SkRasterPipeline*) const;
    void appendStore        (SkRasterPipeline*) const;

    // these check internally, and only append if there was a native clipShader
    void appendClipScale    (SkRasterPipeline*) const;
    void appendClipLerp     (SkRasterPipeline*) const;

    SkPixmap               fDst;
    SkArenaAlloc*          fAlloc;
    SkRasterPipeline       fColorPipeline;
    SkRasterPipeline       fBlendPipeline;
    // If the blender is a blend-mode, we retain that information for late-stage optimizations
    std::optional<SkBlendMode> fBlendMode;
    // set to pipeline storage (for alpha) if we have a clipShader
    void*                  fClipShaderBuffer = nullptr; // "native" : float or U16

    bool fCanDirectBlit;
    const SkColor4f fDirectBlitPaintColor;
    std::optional<uint64_t> fDirectBlitValue;

    SkRasterPipelineContexts::MemoryCtx
        fDstPtr       = {nullptr,0},  // Always points to the top-left of fDst.
        fMaskPtr      = {nullptr,0};  // Updated each call to blitMask().
    SkRasterPipelineContexts::EmbossCtx fEmbossCtx;  // Used only for k3D_Format masks.

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

    using INHERITED = SkBlitter;
};

static SkColor4f paint_color_to_dst(const SkPaint& paint, const SkPixmap& dst) {
    SkColor4f paintColor = paint.getColor4f();
    SkColorSpaceXformSteps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                           dst.colorSpace(),    kUnpremul_SkAlphaType).apply(paintColor.vec());
    return paintColor;
}

static bool create_pipeline_for_blitter(const SkPixmap& dst,
                                        const SkPaint& paint,
                                        const SkMatrix& ctm,
                                        SkArenaAlloc* alloc,
                                        const SkSurfaceProps& props,
                                        SkRasterPipeline* shaderPipeline,
                                        SkColor4f* dstPaintColor,
                                        bool* isOpaqueOut,
                                        bool* isConstantOut) {
    *dstPaintColor = paint_color_to_dst(paint, dst);

    auto shader = as_SB(paint.getShader());

    if (!shader) {
        // Having no shader makes things nice and easy... just use the paint color
        shaderPipeline->appendConstantColor(alloc, dstPaintColor->premul().vec());
        *isOpaqueOut = dstPaintColor->fA == 1.0f;
        *isConstantOut = true;
        return true;
    }

    SkColorSpace* dstCS = dst.colorSpace();
    SkColorType dstCT = dst.colorType();
    *isOpaqueOut = shader->isOpaque() && dstPaintColor->fA == 1.0f;
    *isConstantOut = shader->isConstant();
    if (shader->appendRootStages(
                SkStageRec{shaderPipeline, alloc, dstCT, dstCS, *dstPaintColor, props}, ctm)) {
        if (dstPaintColor->fA != 1.0f) {
            shaderPipeline->append(SkRasterPipelineOp::scale_1_float,
                                   alloc->make<float>(dstPaintColor->fA));
        }
        return true;
    }
    // The shader can't draw with SkRasterPipeline.
    return false;
}

SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap& dst,
                                         const SkPaint& paint,
                                         const SkMatrix& ctm,
                                         SkArenaAlloc* alloc,
                                         sk_sp<SkShader> clipShader,
                                         const SkSurfaceProps& props) {
    SkRasterPipeline_<256> shaderPipeline;
    SkColor4f dstPaintColor;
    bool is_opaque, is_constant;
    if (!create_pipeline_for_blitter(dst,
                                     paint,
                                     ctm,
                                     alloc,
                                     props,
                                     &shaderPipeline,
                                     &dstPaintColor,
                                     &is_opaque,
                                     &is_constant)) {
        return nullptr;
    }

    return SkRasterPipelineBlitter::Create(dst,
                                           paint,
                                           dstPaintColor,
                                           alloc,
                                           shaderPipeline,
                                           is_opaque,
                                           is_constant,
                                           clipShader.get());
}

namespace SkRasterPipelineVisualizer {

SkBlitter* CreateBlitter(const SkPixmap& dst,
                         const std::vector<DebugStage>& debugStages,
                         const SkPaint& paint,
                         const SkMatrix& ctm,
                         SkArenaAlloc* alloc,
                         sk_sp<SkShader> clipShader,
                         const SkSurfaceProps& props) {
    SkRasterPipeline_<256> shaderPipeline;
    SkColor4f dstPaintColor;
    bool is_opaque, is_constant;
    if (!create_pipeline_for_blitter(dst,
                                     paint,
                                     ctm,
                                     alloc,
                                     props,
                                     &shaderPipeline,
                                     &dstPaintColor,
                                     &is_opaque,
                                     &is_constant)) {
        return nullptr;
    }

    static SkOnce once;
    once([&] { shaderPipeline.dump(); });

    std::vector<SkRasterPipeline::StageList> stages;
    for (auto st = shaderPipeline.getStageList(); st; st = st->prev) {
        stages.push_back(*st);
    }
    std::reverse(stages.begin(), stages.end());

    SkASSERT_RELEASE(stages.size() == debugStages.size());
    SkRasterPipeline_<256> shaderPipeline2;

    for (size_t i = 0; i < stages.size(); i++) {
        shaderPipeline2.append(stages[i].stage, stages[i].ctx);
        auto stage = debugStages[i];
        SkASSERT_RELEASE(stage.panels.size() == stage.ops.size());
        for (size_t j = 0; j < stage.panels.size() && j < stage.ops.size(); j++) {
            auto panelCtx = alloc->make<SkRasterPipelineContexts::MemoryCtx>();
            panelCtx->pixels = stage.panels[j].pixmap().writable_addr();
            panelCtx->stride = stage.panels[j].pixmap().rowBytesAsPixels();
            shaderPipeline2.append(stage.ops[j], panelCtx);
        }
    }

    return SkRasterPipelineBlitter::Create(dst,
                                           paint,
                                           dstPaintColor,
                                           alloc,
                                           shaderPipeline2,
                                           is_opaque,
                                           is_constant,
                                           clipShader.get());
}

}  // namespace SkRasterPipelineVisualizer

SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap& dst,
                                         const SkPaint& paint,
                                         const SkRasterPipeline& shaderPipeline,
                                         bool is_opaque,
                                         SkArenaAlloc* alloc,
                                         sk_sp<SkShader> clipShader) {
    bool is_constant = false;  // If this were the case, it'd be better to just set a paint color.
    return SkRasterPipelineBlitter::Create(dst, paint, paint_color_to_dst(paint, dst), alloc,
                                           shaderPipeline, is_opaque, is_constant,
                                           clipShader.get());
}

SkBlitter* SkRasterPipelineBlitter::Create(const SkPixmap& dst,
                                           const SkPaint& paint,
                                           const SkColor4f& dstPaintColor,
                                           SkArenaAlloc* alloc,
                                           const SkRasterPipeline& shaderPipeline,
                                           bool is_opaque,
                                           bool is_constant,
                                           const SkShader* clipShader) {
    auto blitter = alloc->make<SkRasterPipelineBlitter>(dst, paint, alloc);

    // Our job in this factory is to fill out the blitter's color and blend pipelines.
    // The color pipeline is the common front of the full blit pipeline. The blend pipeline is just
    // the portion that does the actual blending math (and assumes that src and dst are already
    // loaded).
    //
    // The full blit pipelines are each constructed lazily on first use, and include the color
    // pipeline, reading the dst, the blend pipeline, coverage, dithering, and writing the dst.

    // Start with the color pipeline
    auto colorPipeline = &blitter->fColorPipeline;

    if (clipShader) {
        auto clipP = colorPipeline;
        SkColorType clipCT = kRGBA_8888_SkColorType;
        SkColorSpace* clipCS = nullptr;
        SkSurfaceProps props{}; // default OK; clipShader doesn't render text
        SkStageRec rec = {clipP, alloc, clipCT, clipCS, SkColors::kBlack, props};
        if (as_SB(clipShader)->appendRootStages(rec, SkMatrix::I())) {
            struct Storage {
                // large enough for highp (float) or lowp(U16)
                float fA[SkRasterPipelineContexts::kMaxStride];
            };
            auto storage = alloc->make<Storage>();
            clipP->append(SkRasterPipelineOp::store_src_a, storage->fA);
            blitter->fClipShaderBuffer = storage->fA;
            is_constant = false;
        } else {
            return nullptr;
        }
    }

    // Let's get the shader in first.
    colorPipeline->extend(shaderPipeline);

    // If there's a color filter it comes next.
    if (auto colorFilter = paint.getColorFilter()) {
        SkSurfaceProps props{}; // default OK; colorFilter doesn't render text
        SkStageRec rec = {
                colorPipeline, alloc, dst.colorType(), dst.colorSpace(), dstPaintColor, props};
        if (!as_CFB(colorFilter)->appendStages(rec, is_opaque)) {
            return nullptr;
        }
        is_opaque = is_opaque && as_CFB(colorFilter)->isAlphaUnchanged();
    }

    // Not all formats make sense to dither (think, F16).  We set their dither rate
    // to zero.  We only dither non-constant shaders, so is_constant won't change here.
    if (paint.isDither() && !is_constant) {
        switch (dst.info().colorType()) {
            case kARGB_4444_SkColorType:
                blitter->fDitherRate = 1 / 15.0f;
                break;
            case kRGB_565_SkColorType:
                blitter->fDitherRate = 1 / 63.0f;
                break;
            case kGray_8_SkColorType:
            case kRGB_888x_SkColorType:
            case kRGBA_8888_SkColorType:
            case kBGRA_8888_SkColorType:
            case kSRGBA_8888_SkColorType:
            case kR8_unorm_SkColorType:
                blitter->fDitherRate = 1 / 255.0f;
                break;
            case kRGB_101010x_SkColorType:
            case kRGBA_1010102_SkColorType:
            case kBGR_101010x_SkColorType:
            case kBGRA_1010102_SkColorType:
            case kBGRA_10101010_XR_SkColorType:
            case kRGBA_10x6_SkColorType:
                blitter->fDitherRate = 1 / 1023.0f;
                break;

            case kUnknown_SkColorType:
            case kAlpha_8_SkColorType:
            case kBGR_101010x_XR_SkColorType:
            case kRGBA_F16_SkColorType:
            case kRGB_F16F16F16x_SkColorType:
            case kRGBA_F16Norm_SkColorType:
            case kRGBA_F32_SkColorType:
            case kR8G8_unorm_SkColorType:
            case kA16_float_SkColorType:
            case kA16_unorm_SkColorType:
            case kR16G16_float_SkColorType:
            case kR16G16_unorm_SkColorType:
            case kR16G16B16A16_unorm_SkColorType:
                blitter->fDitherRate = 0.0f;
                break;
        }
        if (blitter->fDitherRate > 0.0f) {
            colorPipeline->append(SkRasterPipelineOp::dither, &blitter->fDitherRate);
        }
    }

    // Optimization: A pipeline that's still constant here can collapse back into a constant color.
    if (is_constant) {
        SkColor4f constantColor;
        SkRasterPipelineContexts::MemoryCtx constantColorPtr = {&constantColor, 0};
        // We could remove this clamp entirely, but if the destination is 8888, doing the clamp
        // here allows the color pipeline to still run in lowp (we'll use uniform_color, rather than
        // unbounded_uniform_color).
        colorPipeline->appendClampIfNormalized(dst.info());
        colorPipeline->append(SkRasterPipelineOp::store_f32, &constantColorPtr);
        colorPipeline->run(0,0,1,1);
        colorPipeline->reset();
        colorPipeline->appendConstantColor(alloc, constantColor);

        is_opaque = constantColor.fA == 1.0f;
    }

    // Now we'll build the blend pipeline
    auto blendPipeline = &blitter->fBlendPipeline;

    sk_sp<SkBlender> blender = paint.refBlender();
    if (!blender) {
        blender = SkBlender::Mode(SkBlendMode::kSrcOver);
    }

    // We can strength-reduce SrcOver into Src when opaque.
    if (is_opaque && as_BB(blender)->asBlendMode() == SkBlendMode::kSrcOver) {
        blender = SkBlender::Mode(SkBlendMode::kSrc);
    }

    // When we're drawing a constant color in Src mode, we can sometimes just memset.
    // (The previous two optimizations help find more opportunities for this one.)
    if (is_constant && as_BB(blender)->asBlendMode() == SkBlendMode::kSrc &&
        dst.info().bytesPerPixel() <= static_cast<int>(sizeof(blitter->fMemsetColor))) {
        // Run our color pipeline all the way through to produce what we'd memset when we can.
        // Not all blits can memset, so we need to keep colorPipeline too.
        SkRasterPipeline_<256> p;
        p.extend(*colorPipeline);
        blitter->fDstPtr = SkRasterPipelineContexts::MemoryCtx{&blitter->fMemsetColor, 0};
        blitter->appendStore(&p);
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

    {
        SkSurfaceProps props{};  // default OK; blender doesn't render text
        SkStageRec rec = {
                blendPipeline, alloc, dst.colorType(), dst.colorSpace(), dstPaintColor, props};
        if (!as_BB(blender)->appendStages(rec)) {
            return nullptr;
        }
        blitter->fBlendMode = as_BB(blender)->asBlendMode();
    }

    blitter->fDstPtr = SkRasterPipelineContexts::MemoryCtx{
        blitter->fDst.writable_addr(),
        blitter->fDst.rowBytesAsPixels(),
    };

    return blitter;
}

void SkRasterPipelineBlitter::appendLoadDst(SkRasterPipeline* p) const {
    p->appendLoadDst(fDst.info().colorType(), &fDstPtr);
    if (fDst.info().alphaType() == kUnpremul_SkAlphaType) {
        p->append(SkRasterPipelineOp::premul_dst);
    }
}

void SkRasterPipelineBlitter::appendStore(SkRasterPipeline* p) const {
    if (fDst.info().alphaType() == kUnpremul_SkAlphaType) {
        p->append(SkRasterPipelineOp::unpremul);
    }
    p->appendStore(fDst.info().colorType(), &fDstPtr);
}

void SkRasterPipelineBlitter::appendClipScale(SkRasterPipeline* p) const {
    if (fClipShaderBuffer) {
        p->append(SkRasterPipelineOp::scale_native, fClipShaderBuffer);
    }
}

void SkRasterPipelineBlitter::appendClipLerp(SkRasterPipeline* p) const {
    if (fClipShaderBuffer) {
        p->append(SkRasterPipelineOp::lerp_native, fClipShaderBuffer);
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
        p.appendClampIfNormalized(fDst.info());
        if (fBlendMode == SkBlendMode::kSrcOver
                && (fDst.info().colorType() == kRGBA_8888_SkColorType ||
                    fDst.info().colorType() == kBGRA_8888_SkColorType)
                && !fDst.colorSpace()
                && fDst.info().alphaType() != kUnpremul_SkAlphaType
                && fDitherRate == 0.0f) {
            if (fDst.info().colorType() == kBGRA_8888_SkColorType) {
                p.append(SkRasterPipelineOp::swap_rb);
            }
            this->appendClipScale(&p);
            p.append(SkRasterPipelineOp::srcover_rgba_8888, &fDstPtr);
        } else {
            if (fBlendMode != SkBlendMode::kSrc) {
                this->appendLoadDst(&p);
                p.extend(fBlendPipeline);
                this->appendClipLerp(&p);
            } else if (fClipShaderBuffer) {
                this->appendLoadDst(&p);
                this->appendClipLerp(&p);
            }
            this->appendStore(&p);
        }
        fBlitRect = p.compile();
    }

    fBlitRect(x,y,w,h);
}

void SkRasterPipelineBlitter::blitAntiH(int x, int y, const SkAlpha aa[], const int16_t runs[]) {
    if (!fBlitAntiH) {
        SkRasterPipeline p(fAlloc);
        p.extend(fColorPipeline);
        p.appendClampIfNormalized(fDst.info());
        if (fBlendMode.has_value() &&
            SkBlendMode_ShouldPreScaleCoverage(*fBlendMode, /*rgb_coverage=*/false)) {
            p.append(SkRasterPipelineOp::scale_1_float, &fCurrentCoverage);
            this->appendClipScale(&p);
            this->appendLoadDst(&p);
            p.extend(fBlendPipeline);
        } else {
            this->appendLoadDst(&p);
            p.extend(fBlendPipeline);
            p.append(SkRasterPipelineOp::lerp_1_float, &fCurrentCoverage);
            this->appendClipLerp(&p);
        }

        this->appendStore(&p);
        fBlitAntiH = p.compile();
    }

    for (int16_t run = *runs; run > 0; run = *runs) {
        switch (*aa) {
            case 0x00:                             break;
            case 0xff: this->blitRect(x,y,run, 1); break;
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
    SkMask mask(coverage, clip, 2, SkMask::kA8_Format);
    this->blitMask(mask, clip);
}

void SkRasterPipelineBlitter::blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) {
    SkIRect clip = {x,y, x+1,y+2};
    uint8_t coverage[] = { (uint8_t)a0, (uint8_t)a1 };
    SkMask mask(coverage, clip, 1, SkMask::kA8_Format);
    this->blitMask(mask, clip);
}

void SkRasterPipelineBlitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkIRect clip = {x,y, x+1,y+height};
    SkMask mask(&alpha, clip,
                0,     // so we reuse the 1 "row" for all of height
                SkMask::kA8_Format);
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

    auto extract_mask_plane = [&mask](int plane, SkRasterPipelineContexts::MemoryCtx* ctx) {
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
        p.appendClampIfNormalized(fDst.info());
        if (fBlendMode.has_value() &&
            SkBlendMode_ShouldPreScaleCoverage(*fBlendMode, /*rgb_coverage=*/false)) {
            p.append(SkRasterPipelineOp::scale_u8, &fMaskPtr);
            this->appendClipScale(&p);
            this->appendLoadDst(&p);
            p.extend(fBlendPipeline);
        } else {
            this->appendLoadDst(&p);
            p.extend(fBlendPipeline);
            p.append(SkRasterPipelineOp::lerp_u8, &fMaskPtr);
            this->appendClipLerp(&p);
        }
        this->appendStore(&p);
        fBlitMaskA8 = p.compile();
    }
    if (mask.fFormat == SkMask::kLCD16_Format && !fBlitMaskLCD16) {
        SkRasterPipeline p(fAlloc);
        p.extend(fColorPipeline);
        p.appendClampIfNormalized(fDst.info());
        if (fBlendMode.has_value() &&
            SkBlendMode_ShouldPreScaleCoverage(*fBlendMode, /*rgb_coverage=*/true)) {
            // Somewhat unusually, scale_565 needs dst loaded first.
            this->appendLoadDst(&p);
            p.append(SkRasterPipelineOp::scale_565, &fMaskPtr);
            this->appendClipScale(&p);
            p.extend(fBlendPipeline);
        } else {
            this->appendLoadDst(&p);
            p.extend(fBlendPipeline);
            p.append(SkRasterPipelineOp::lerp_565, &fMaskPtr);
            this->appendClipLerp(&p);
        }
        this->appendStore(&p);
        fBlitMaskLCD16 = p.compile();
    }
    if (mask.fFormat == SkMask::k3D_Format && !fBlitMask3D) {
        SkRasterPipeline p(fAlloc);
        p.extend(fColorPipeline);
        // This bit is where we differ from kA8_Format:
        p.append(SkRasterPipelineOp::emboss, &fEmbossCtx);
        // Now onward just as kA8.
        p.appendClampIfNormalized(fDst.info());
        if (fBlendMode.has_value() &&
            SkBlendMode_ShouldPreScaleCoverage(*fBlendMode, /*rgb_coverage=*/false)) {
            p.append(SkRasterPipelineOp::scale_u8, &fMaskPtr);
            this->appendClipScale(&p);
            this->appendLoadDst(&p);
            p.extend(fBlendPipeline);
        } else {
            this->appendLoadDst(&p);
            p.extend(fBlendPipeline);
            p.append(SkRasterPipelineOp::lerp_u8, &fMaskPtr);
            this->appendClipLerp(&p);
        }
        this->appendStore(&p);
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

std::optional<SkBlitter::DirectBlit> SkRasterPipelineBlitter::canDirectBlit() {
    if (fCanDirectBlit) {
        if (!fDirectBlitValue.has_value()) {
            // want maximum alignment (8) but legal punning for smaller int types
            union {
                uint64_t u8[1];
                uint32_t u4[2];
                uint16_t u2[4];
                 uint8_t u1[8];
            } dstBuffer;
            auto dst = SkImageInfo::Make(1, 1, fDst.info().colorType(), fDst.info().alphaType());
            auto src = SkImageInfo::Make(1, 1, kRGBA_F32_SkColorType, kUnpremul_SkAlphaType);
            if (!SkConvertPixels(dst, &dstBuffer, sizeof(dstBuffer),
                                 src, &fDirectBlitPaintColor, sizeof(fDirectBlitPaintColor))) {
                goto FAIL;
            }
            switch (dst.bytesPerPixel()) {
                case 1: fDirectBlitValue = dstBuffer.u1[0]; break;
                case 2: fDirectBlitValue = dstBuffer.u2[0]; break;
                case 4: fDirectBlitValue = dstBuffer.u4[0]; break;
                case 8: fDirectBlitValue = dstBuffer.u8[0]; break;
                default: goto FAIL;
            }
        }
        return {{fDst, fDirectBlitValue.value()}};
    }
FAIL:
    fCanDirectBlit = false;
    return {};
}
