/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkArenaAlloc.h"
#include "src/core/SkBitmapController.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkBitmapProcShader.h"
#include "src/shaders/SkEmptyShader.h"
#include "src/shaders/SkImageShader.h"

/**
 *  We are faster in clamp, so always use that tiling when we can.
 */
static SkTileMode optimize(SkTileMode tm, int dimension) {
    SkASSERT(dimension > 0);
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    // need to update frameworks/base/libs/hwui/tests/unit/SkiaBehaviorTests.cpp:55 to allow
    // for transforming to clamp.
    return tm;
#else
    return dimension == 1 ? SkTileMode::kClamp : tm;
#endif
}

SkImageShader::SkImageShader(sk_sp<SkImage> img,
                             SkTileMode tmx, SkTileMode tmy,
                             const SkMatrix* localMatrix,
                             bool clampAsIfUnpremul)
    : INHERITED(localMatrix)
    , fImage(std::move(img))
    , fTileModeX(optimize(tmx, fImage->width()))
    , fTileModeY(optimize(tmy, fImage->height()))
    , fClampAsIfUnpremul(clampAsIfUnpremul)
{}

// fClampAsIfUnpremul is always false when constructed through public APIs,
// so there's no need to read or write it here.

sk_sp<SkFlattenable> SkImageShader::CreateProc(SkReadBuffer& buffer) {
    auto tmx = buffer.read32LE<SkTileMode>(SkTileMode::kLastTileMode);
    auto tmy = buffer.read32LE<SkTileMode>(SkTileMode::kLastTileMode);
    SkMatrix localMatrix;
    buffer.readMatrix(&localMatrix);
    sk_sp<SkImage> img = buffer.readImage();
    if (!img) {
        return nullptr;
    }
    return SkImageShader::Make(std::move(img), tmx, tmy, &localMatrix);
}

void SkImageShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeUInt((unsigned)fTileModeX);
    buffer.writeUInt((unsigned)fTileModeY);
    buffer.writeMatrix(this->getLocalMatrix());
    buffer.writeImage(fImage.get());
    SkASSERT(fClampAsIfUnpremul == false);
}

bool SkImageShader::isOpaque() const {
    return fImage->isOpaque() &&
           fTileModeX != SkTileMode::kDecal && fTileModeY != SkTileMode::kDecal;
}

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
static bool legacy_shader_can_handle(const SkMatrix& inv) {
    if (inv.hasPerspective()) {
        return false;
    }

    // Scale+translate methods are always present, but affine might not be.
    if (!SkOpts::S32_alpha_D32_filter_DXDY && !inv.isScaleTranslate()) {
        return false;
    }

    // legacy code uses SkFixed 32.32, so ensure the inverse doesn't map device coordinates
    // out of range.
    const SkScalar max_dev_coord = 32767.0f;
    const SkRect src = inv.mapRect(SkRect::MakeWH(max_dev_coord, max_dev_coord));

    // take 1/4 of max signed 32bits so we have room to subtract local values
    const SkScalar max_fixed32dot32 = float(SK_MaxS32) * 0.25f;
    if (!SkRect::MakeLTRB(-max_fixed32dot32, -max_fixed32dot32,
                          +max_fixed32dot32, +max_fixed32dot32).contains(src)) {
        return false;
    }

    // legacy shader impl should be able to handle these matrices
    return true;
}

SkShaderBase::Context* SkImageShader::onMakeContext(const ContextRec& rec,
                                                    SkArenaAlloc* alloc) const {
    if (fImage->alphaType() == kUnpremul_SkAlphaType) {
        return nullptr;
    }
    if (fImage->colorType() != kN32_SkColorType) {
        return nullptr;
    }
#if !defined(SK_SUPPORT_LEGACY_TILED_BITMAPS)
    if (fTileModeX != fTileModeY) {
        return nullptr;
    }
#endif
    if (fTileModeX == SkTileMode::kDecal || fTileModeY == SkTileMode::kDecal) {
        return nullptr;
    }

    // SkBitmapProcShader stores bitmap coordinates in a 16bit buffer,
    // so it can't handle bitmaps larger than 65535.
    //
    // We back off another bit to 32767 to make small amounts of
    // intermediate math safe, e.g. in
    //
    //     SkFixed fx = ...;
    //     fx = tile(fx + SK_Fixed1);
    //
    // we want to make sure (fx + SK_Fixed1) never overflows.
    if (fImage-> width() > 32767 ||
        fImage->height() > 32767) {
        return nullptr;
    }

    SkMatrix inv;
    if (!this->computeTotalInverse(*rec.fMatrix, rec.fLocalMatrix, &inv) ||
        !legacy_shader_can_handle(inv)) {
        return nullptr;
    }

    if (!rec.isLegacyCompatible(fImage->colorSpace())) {
        return nullptr;
    }

    return SkBitmapProcLegacyShader::MakeContext(*this, fTileModeX, fTileModeY,
                                                 as_IB(fImage.get()), rec, alloc);
}
#endif

SkImage* SkImageShader::onIsAImage(SkMatrix* texM, SkTileMode xy[]) const {
    if (texM) {
        *texM = this->getLocalMatrix();
    }
    if (xy) {
        xy[0] = fTileModeX;
        xy[1] = fTileModeY;
    }
    return const_cast<SkImage*>(fImage.get());
}

sk_sp<SkShader> SkImageShader::Make(sk_sp<SkImage> image,
                                    SkTileMode tmx, SkTileMode tmy,
                                    const SkMatrix* localMatrix,
                                    bool clampAsIfUnpremul) {
    if (!image) {
        return sk_make_sp<SkEmptyShader>();
    }
    return sk_sp<SkShader>{ new SkImageShader(image, tmx, tmy, localMatrix, clampAsIfUnpremul) };
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBicubicEffect.h"
#include "src/gpu/effects/GrTextureEffect.h"

static GrSamplerState::WrapMode tile_mode_to_wrap_mode(const SkTileMode tileMode) {
    switch (tileMode) {
        case SkTileMode::kClamp:
            return GrSamplerState::WrapMode::kClamp;
        case SkTileMode::kRepeat:
            return GrSamplerState::WrapMode::kRepeat;
        case SkTileMode::kMirror:
            return GrSamplerState::WrapMode::kMirrorRepeat;
        case SkTileMode::kDecal:
            return GrSamplerState::WrapMode::kClampToBorder;
    }
    SK_ABORT("Unknown tile mode.");
}

std::unique_ptr<GrFragmentProcessor> SkImageShader::asFragmentProcessor(
        const GrFPArgs& args) const {
    const auto lm = this->totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix);
    SkMatrix lmInverse;
    if (!lm->invert(&lmInverse)) {
        return nullptr;
    }

    GrSamplerState::WrapMode wm[] = {tile_mode_to_wrap_mode(fTileModeX),
                                     tile_mode_to_wrap_mode(fTileModeY)};

    // Must set wrap and filter on the sampler before requesting a texture. In two places below
    // we check the matrix scale factors to determine how to interpret the filter quality setting.
    // This completely ignores the complexity of the drawVertices case where explicit local coords
    // are provided by the caller.
    bool doBicubic;
    GrSamplerState::Filter textureFilterMode = GrSkFilterQualityToGrFilterMode(
            fImage->width(), fImage->height(), args.fFilterQuality, *args.fViewMatrix, *lm,
            args.fContext->priv().options().fSharpenMipmappedTextures, &doBicubic);
    GrSamplerState samplerState(wm, textureFilterMode);
    SkScalar scaleAdjust[2] = { 1.0f, 1.0f };
    GrSurfaceProxyView view = as_IB(fImage)->refView(args.fContext, samplerState, scaleAdjust);
    if (!view) {
        return nullptr;
    }

    SkAlphaType srcAlphaType = fImage->alphaType();

    lmInverse.postScale(scaleAdjust[0], scaleAdjust[1]);

    const auto& caps = *args.fContext->priv().caps();

    std::unique_ptr<GrFragmentProcessor> inner;
    if (doBicubic) {
        static constexpr auto kDir = GrBicubicEffect::Direction::kXY;
        inner = GrBicubicEffect::Make(std::move(view), srcAlphaType, lmInverse, wm[0], wm[1], kDir,
                                      caps);
    } else {
        inner = GrTextureEffect::Make(std::move(view), srcAlphaType, lmInverse, samplerState, caps);
    }
    inner = GrColorSpaceXformEffect::Make(std::move(inner), fImage->colorSpace(), srcAlphaType,
                                          args.fDstColorInfo->colorSpace());

    bool isAlphaOnly = SkColorTypeIsAlphaOnly(fImage->colorType());
    if (isAlphaOnly) {
        return inner;
    } else if (args.fInputColorIsOpaque) {
        return GrFragmentProcessor::OverrideInput(std::move(inner), SK_PMColor4fWHITE, false);
    }
    return GrFragmentProcessor::MulChildByInputAlpha(std::move(inner));
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "src/core/SkImagePriv.h"

sk_sp<SkShader> SkMakeBitmapShader(const SkBitmap& src, SkTileMode tmx, SkTileMode tmy,
                                   const SkMatrix* localMatrix, SkCopyPixelsMode cpm) {
    return SkImageShader::Make(SkMakeImageFromRasterBitmap(src, cpm),
                               tmx, tmy, localMatrix);
}

sk_sp<SkShader> SkMakeBitmapShaderForPaint(const SkPaint& paint, const SkBitmap& src,
                                           SkTileMode tmx, SkTileMode tmy,
                                           const SkMatrix* localMatrix, SkCopyPixelsMode mode) {
    auto s = SkMakeBitmapShader(src, tmx, tmy, localMatrix, mode);
    if (!s) {
        return nullptr;
    }
    if (src.colorType() == kAlpha_8_SkColorType && paint.getShader()) {
        // Compose the image shader with the paint's shader. Alpha images+shaders should output the
        // texture's alpha multiplied by the shader's color. DstIn (d*sa) will achieve this with
        // the source image and dst shader (MakeBlend takes dst first, src second).
        s = SkShaders::Blend(SkBlendMode::kDstIn, paint.refShader(), std::move(s));
    }
    return s;
}

void SkShaderBase::RegisterFlattenables() { SK_REGISTER_FLATTENABLE(SkImageShader); }

class SkImageStageUpdater : public SkStageUpdater {
public:
    SkImageStageUpdater(const SkImageShader* shader, bool usePersp)
        : fShader(shader), fUsePersp(usePersp)
    {}

    const SkImageShader* fShader;
    const bool           fUsePersp; // else use affine

    // large enough for perspective, though often we just use 2x3
    float fMatrixStorage[9];

#if 0   // TODO: when we support mipmaps
    SkRasterPipeline_GatherCtx* fGather;
    SkRasterPipeline_TileCtx* fLimitX;
    SkRasterPipeline_TileCtx* fLimitY;
    SkRasterPipeline_DecalTileCtx* fDecal;
#endif

    void append_matrix_stage(SkRasterPipeline* p) {
        if (fUsePersp) {
            p->append(SkRasterPipeline::matrix_perspective, fMatrixStorage);
        } else {
            p->append(SkRasterPipeline::matrix_2x3, fMatrixStorage);
        }
    }

    bool update(const SkMatrix& ctm, const SkMatrix* localM) override {
        SkMatrix matrix;
        if (fShader->computeTotalInverse(ctm, localM, &matrix)) {
            if (fUsePersp) {
                matrix.get9(fMatrixStorage);
            } else {
               SkAssertResult(matrix.asAffine(fMatrixStorage));
            }
            return true;
        }
        return false;
    }
};

static void tweak_quality_and_inv_matrix(SkFilterQuality* quality, SkMatrix* matrix) {
    // When the matrix is just an integer translate, bilerp == nearest neighbor.
    if (*quality == kLow_SkFilterQuality &&
            matrix->getType() <= SkMatrix::kTranslate_Mask &&
            matrix->getTranslateX() == (int)matrix->getTranslateX() &&
            matrix->getTranslateY() == (int)matrix->getTranslateY()) {
        *quality = kNone_SkFilterQuality;
    }

    // See skia:4649 and the GM image_scale_aligned.
    if (*quality == kNone_SkFilterQuality) {
        if (matrix->getScaleX() >= 0) {
            matrix->setTranslateX(nextafterf(matrix->getTranslateX(),
                                             floorf(matrix->getTranslateX())));
        }
        if (matrix->getScaleY() >= 0) {
            matrix->setTranslateY(nextafterf(matrix->getTranslateY(),
                                             floorf(matrix->getTranslateY())));
        }
    }
}

bool SkImageShader::doStages(const SkStageRec& rec, SkImageStageUpdater* updater) const {
    if (updater && rec.fPaint.getFilterQuality() == kMedium_SkFilterQuality) {
        // TODO: medium: recall RequestBitmap and update width/height accordingly
        return false;
    }

    SkRasterPipeline* p = rec.fPipeline;
    SkArenaAlloc* alloc = rec.fAlloc;
    auto quality = rec.fPaint.getFilterQuality();

    SkMatrix matrix;
    if (!this->computeTotalInverse(rec.fCTM, rec.fLocalM, &matrix)) {
        return false;
    }

    const auto* state = SkBitmapController::RequestBitmap(as_IB(fImage.get()),
                                                          matrix, quality, alloc);
    if (!state) {
        return false;
    }

    const SkPixmap& pm = state->pixmap();
    matrix  = state->invMatrix();
    quality = state->quality();
    auto info = pm.info();

    p->append(SkRasterPipeline::seed_shader);

    if (updater) {
        updater->append_matrix_stage(p);
    } else {
        tweak_quality_and_inv_matrix(&quality, &matrix);
        p->append_matrix(alloc, matrix);
    }

    auto gather = alloc->make<SkRasterPipeline_GatherCtx>();
    gather->pixels = pm.addr();
    gather->stride = pm.rowBytesAsPixels();
    gather->width  = pm.width();
    gather->height = pm.height();

    auto limit_x = alloc->make<SkRasterPipeline_TileCtx>(),
         limit_y = alloc->make<SkRasterPipeline_TileCtx>();
    limit_x->scale = pm.width();
    limit_x->invScale = 1.0f / pm.width();
    limit_y->scale = pm.height();
    limit_y->invScale = 1.0f / pm.height();

    SkRasterPipeline_DecalTileCtx* decal_ctx = nullptr;
    bool decal_x_and_y = fTileModeX == SkTileMode::kDecal && fTileModeY == SkTileMode::kDecal;
    if (fTileModeX == SkTileMode::kDecal || fTileModeY == SkTileMode::kDecal) {
        decal_ctx = alloc->make<SkRasterPipeline_DecalTileCtx>();
        decal_ctx->limit_x = limit_x->scale;
        decal_ctx->limit_y = limit_y->scale;
    }

#if 0   // TODO: when we support kMedium
    if (updator && (quality == kMedium_SkFilterQuality)) {
        // if we change levels in mipmap, we need to update the scales (and invScales)
        updator->fGather = gather;
        updator->fLimitX = limit_x;
        updator->fLimitY = limit_y;
        updator->fDecal = decal_ctx;
    }
#endif

    auto append_tiling_and_gather = [&] {
        if (decal_x_and_y) {
            p->append(SkRasterPipeline::decal_x_and_y,  decal_ctx);
        } else {
            switch (fTileModeX) {
                case SkTileMode::kClamp:  /* The gather_xxx stage will clamp for us. */     break;
                case SkTileMode::kMirror: p->append(SkRasterPipeline::mirror_x, limit_x);   break;
                case SkTileMode::kRepeat: p->append(SkRasterPipeline::repeat_x, limit_x);   break;
                case SkTileMode::kDecal:  p->append(SkRasterPipeline::decal_x,  decal_ctx); break;
            }
            switch (fTileModeY) {
                case SkTileMode::kClamp:  /* The gather_xxx stage will clamp for us. */     break;
                case SkTileMode::kMirror: p->append(SkRasterPipeline::mirror_y, limit_y);   break;
                case SkTileMode::kRepeat: p->append(SkRasterPipeline::repeat_y, limit_y);   break;
                case SkTileMode::kDecal:  p->append(SkRasterPipeline::decal_y,  decal_ctx); break;
            }
        }

        void* ctx = gather;
        switch (info.colorType()) {
            case kAlpha_8_SkColorType:      p->append(SkRasterPipeline::gather_a8,      ctx); break;
            case kA16_unorm_SkColorType:    p->append(SkRasterPipeline::gather_a16,     ctx); break;
            case kA16_float_SkColorType:    p->append(SkRasterPipeline::gather_af16,    ctx); break;
            case kRGB_565_SkColorType:      p->append(SkRasterPipeline::gather_565,     ctx); break;
            case kARGB_4444_SkColorType:    p->append(SkRasterPipeline::gather_4444,    ctx); break;
            case kR8G8_unorm_SkColorType:   p->append(SkRasterPipeline::gather_rg88,    ctx); break;
            case kR16G16_unorm_SkColorType: p->append(SkRasterPipeline::gather_rg1616,  ctx); break;
            case kR16G16_float_SkColorType: p->append(SkRasterPipeline::gather_rgf16,  ctx);  break;
            case kRGBA_8888_SkColorType:    p->append(SkRasterPipeline::gather_8888,    ctx); break;
            case kRGBA_1010102_SkColorType: p->append(SkRasterPipeline::gather_1010102, ctx); break;
            case kR16G16B16A16_unorm_SkColorType:
                                            p->append(SkRasterPipeline::gather_16161616,ctx); break;
            case kRGBA_F16Norm_SkColorType:
            case kRGBA_F16_SkColorType:     p->append(SkRasterPipeline::gather_f16,     ctx); break;
            case kRGBA_F32_SkColorType:     p->append(SkRasterPipeline::gather_f32,     ctx); break;

            case kGray_8_SkColorType:       p->append(SkRasterPipeline::gather_a8,      ctx);
                                            p->append(SkRasterPipeline::alpha_to_gray      ); break;

            case kRGB_888x_SkColorType:     p->append(SkRasterPipeline::gather_8888,    ctx);
                                            p->append(SkRasterPipeline::force_opaque       ); break;

            case kBGRA_1010102_SkColorType: p->append(SkRasterPipeline::gather_1010102, ctx);
                                            p->append(SkRasterPipeline::swap_rb            ); break;

            case kRGB_101010x_SkColorType:  p->append(SkRasterPipeline::gather_1010102, ctx);
                                            p->append(SkRasterPipeline::force_opaque       ); break;

            case kBGR_101010x_SkColorType:  p->append(SkRasterPipeline::gather_1010102, ctx);
                                            p->append(SkRasterPipeline::force_opaque       );
                                            p->append(SkRasterPipeline::swap_rb            ); break;

            case kBGRA_8888_SkColorType:    p->append(SkRasterPipeline::gather_8888,    ctx);
                                            p->append(SkRasterPipeline::swap_rb            ); break;

            case kUnknown_SkColorType: SkASSERT(false);
        }
        if (decal_ctx) {
            p->append(SkRasterPipeline::check_decal_mask, decal_ctx);
        }
    };

    auto append_misc = [&] {
        // This is an inessential optimization... it's logically safe to set this to false.
        // But if...
        //      - we know the image is definitely normalized, and
        //      - we're doing some color space conversion, and
        //      - sRGB curves are involved,
        // then we can use slightly faster math that doesn't work well outside [0,1].
        bool src_is_normalized = SkColorTypeIsNormalized(info.colorType());

        SkColorSpace* cs = info.colorSpace();
        SkAlphaType   at = info.alphaType();

        // Color for A8 images comes from the paint.  TODO: all alpha images?  none?
        if (info.colorType() == kAlpha_8_SkColorType) {
            SkColor4f rgb = rec.fPaint.getColor4f();
            p->append_set_rgb(alloc, rgb);

            src_is_normalized = rgb.fitsInBytes();
            cs = sk_srgb_singleton();
            at = kUnpremul_SkAlphaType;
        }

        // Bicubic filtering naturally produces out of range values on both sides of [0,1].
        if (quality == kHigh_SkFilterQuality) {
            p->append(SkRasterPipeline::clamp_0);
            p->append(at == kUnpremul_SkAlphaType || fClampAsIfUnpremul
                          ? SkRasterPipeline::clamp_1
                          : SkRasterPipeline::clamp_a);
            src_is_normalized = true;
        }

        // Transform color space and alpha type to match shader convention (dst CS, premul alpha).
        alloc->make<SkColorSpaceXformSteps>(cs, at,
                                            rec.fDstCS, kPremul_SkAlphaType)
            ->apply(p, src_is_normalized);

        return true;
    };

    // Check for fast-path stages.
    auto ct = info.colorType();
    if (true
        && (ct == kRGBA_8888_SkColorType || ct == kBGRA_8888_SkColorType)
        && quality == kLow_SkFilterQuality
        && fTileModeX == SkTileMode::kClamp && fTileModeY == SkTileMode::kClamp) {

        p->append(SkRasterPipeline::bilerp_clamp_8888, gather);
        if (ct == kBGRA_8888_SkColorType) {
            p->append(SkRasterPipeline::swap_rb);
        }
        return append_misc();
    }
    if (true
        && (ct == kRGBA_8888_SkColorType || ct == kBGRA_8888_SkColorType) // TODO: all formats
        && quality == kLow_SkFilterQuality
        && fTileModeX != SkTileMode::kDecal // TODO decal too?
        && fTileModeY != SkTileMode::kDecal) {

        auto ctx = alloc->make<SkRasterPipeline_SamplerCtx2>();
        *(SkRasterPipeline_GatherCtx*)(ctx) = *gather;
        ctx->ct = ct;
        ctx->tileX = fTileModeX;
        ctx->tileY = fTileModeY;
        ctx->invWidth  = 1.0f / ctx->width;
        ctx->invHeight = 1.0f / ctx->height;
        p->append(SkRasterPipeline::bilinear, ctx);
        return append_misc();
    }
    if (true
        && (ct == kRGBA_8888_SkColorType || ct == kBGRA_8888_SkColorType)
        && quality == kHigh_SkFilterQuality
        && fTileModeX == SkTileMode::kClamp && fTileModeY == SkTileMode::kClamp) {

        p->append(SkRasterPipeline::bicubic_clamp_8888, gather);
        if (ct == kBGRA_8888_SkColorType) {
            p->append(SkRasterPipeline::swap_rb);
        }
        return append_misc();
    }
    if (true
        && (ct == kRGBA_8888_SkColorType || ct == kBGRA_8888_SkColorType) // TODO: all formats
        && quality == kHigh_SkFilterQuality
        && fTileModeX != SkTileMode::kDecal // TODO decal too?
        && fTileModeY != SkTileMode::kDecal) {

        auto ctx = alloc->make<SkRasterPipeline_SamplerCtx2>();
        *(SkRasterPipeline_GatherCtx*)(ctx) = *gather;
        ctx->ct = ct;
        ctx->tileX = fTileModeX;
        ctx->tileY = fTileModeY;
        ctx->invWidth  = 1.0f / ctx->width;
        ctx->invHeight = 1.0f / ctx->height;
        p->append(SkRasterPipeline::bicubic, ctx);
        return append_misc();
    }

    SkRasterPipeline_SamplerCtx* sampler = nullptr;
    if (quality != kNone_SkFilterQuality) {
        sampler = alloc->make<SkRasterPipeline_SamplerCtx>();
    }

    auto sample = [&](SkRasterPipeline::StockStage setup_x,
                      SkRasterPipeline::StockStage setup_y) {
        p->append(setup_x, sampler);
        p->append(setup_y, sampler);
        append_tiling_and_gather();
        p->append(SkRasterPipeline::accumulate, sampler);
    };

    if (quality == kNone_SkFilterQuality) {
        append_tiling_and_gather();
    } else if (quality == kLow_SkFilterQuality) {
        p->append(SkRasterPipeline::save_xy, sampler);

        sample(SkRasterPipeline::bilinear_nx, SkRasterPipeline::bilinear_ny);
        sample(SkRasterPipeline::bilinear_px, SkRasterPipeline::bilinear_ny);
        sample(SkRasterPipeline::bilinear_nx, SkRasterPipeline::bilinear_py);
        sample(SkRasterPipeline::bilinear_px, SkRasterPipeline::bilinear_py);

        p->append(SkRasterPipeline::move_dst_src);

    } else {
        SkASSERT(quality == kHigh_SkFilterQuality);
        p->append(SkRasterPipeline::save_xy, sampler);

        sample(SkRasterPipeline::bicubic_n3x, SkRasterPipeline::bicubic_n3y);
        sample(SkRasterPipeline::bicubic_n1x, SkRasterPipeline::bicubic_n3y);
        sample(SkRasterPipeline::bicubic_p1x, SkRasterPipeline::bicubic_n3y);
        sample(SkRasterPipeline::bicubic_p3x, SkRasterPipeline::bicubic_n3y);

        sample(SkRasterPipeline::bicubic_n3x, SkRasterPipeline::bicubic_n1y);
        sample(SkRasterPipeline::bicubic_n1x, SkRasterPipeline::bicubic_n1y);
        sample(SkRasterPipeline::bicubic_p1x, SkRasterPipeline::bicubic_n1y);
        sample(SkRasterPipeline::bicubic_p3x, SkRasterPipeline::bicubic_n1y);

        sample(SkRasterPipeline::bicubic_n3x, SkRasterPipeline::bicubic_p1y);
        sample(SkRasterPipeline::bicubic_n1x, SkRasterPipeline::bicubic_p1y);
        sample(SkRasterPipeline::bicubic_p1x, SkRasterPipeline::bicubic_p1y);
        sample(SkRasterPipeline::bicubic_p3x, SkRasterPipeline::bicubic_p1y);

        sample(SkRasterPipeline::bicubic_n3x, SkRasterPipeline::bicubic_p3y);
        sample(SkRasterPipeline::bicubic_n1x, SkRasterPipeline::bicubic_p3y);
        sample(SkRasterPipeline::bicubic_p1x, SkRasterPipeline::bicubic_p3y);
        sample(SkRasterPipeline::bicubic_p3x, SkRasterPipeline::bicubic_p3y);

        p->append(SkRasterPipeline::move_dst_src);
    }

    return append_misc();
}

bool SkImageShader::onAppendStages(const SkStageRec& rec) const {
    return this->doStages(rec, nullptr);
}

SkStageUpdater* SkImageShader::onAppendUpdatableStages(const SkStageRec& rec) const {
    bool usePersp = rec.fCTM.hasPerspective();
    auto updater = rec.fAlloc->make<SkImageStageUpdater>(this, usePersp);
    return this->doStages(rec, updater) ? updater : nullptr;
}

bool SkImageShader::onProgram(skvm::Builder* p,
                              const SkMatrix& ctm, const SkMatrix* localM,
                              SkFilterQuality quality, SkColorSpace* dstCS,
                              skvm::Uniforms* uniforms, SkArenaAlloc* alloc,
                              skvm::F32 x, skvm::F32 y,
                              skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32* a) const {
    SkMatrix inv;
    if (!this->computeTotalInverse(ctm, localM, &inv)) {
        return false;
    }

    // We use RequestBitmap() to make sure our SkBitmapController::State lives in the alloc.
    // This lets the SkVMBlitter hang on to this state and keep our image alive.
    auto state = SkBitmapController::RequestBitmap(as_IB(fImage.get()), inv, quality, alloc);
    if (!state) {
        return false;
    }
    const SkPixmap& pm = state->pixmap();
    inv     = state->invMatrix();
    quality = state->quality();
    tweak_quality_and_inv_matrix(&quality, &inv);
    inv.normalizePerspective();

    // Apply matrix to convert dst coords to sample center coords.
    SkShaderBase::ApplyMatrix(p, inv, &x,&y,uniforms);

    // Bail out if sample() can't yet handle our image's color type.
    switch (pm.colorType()) {
        default: return false;
        case   kRGB_565_SkColorType:
        case  kRGB_888x_SkColorType:
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        case kRGBA_1010102_SkColorType:
        case kBGRA_1010102_SkColorType:
        case  kRGB_101010x_SkColorType:
        case  kBGR_101010x_SkColorType: break;
    }

    // We can exploit image opacity to skip work unpacking alpha channels.
    const bool input_is_opaque = SkAlphaTypeIsOpaque(pm.alphaType())
                              || SkColorTypeIsAlwaysOpaque(pm.colorType());

    // Each call to sample() will try to rewrite the same uniforms over and over,
    // so remember where we start and reset back there each time.  That way each
    // sample() call uses the same uniform offsets.
    const size_t uniforms_before_sample = uniforms->buf.size();

    auto sample = [&](skvm::F32 sx, skvm::F32 sy) -> skvm::Color {
        uniforms->buf.resize(uniforms_before_sample);

        // repeat() and mirror() are written assuming they'll be followed by a [0,scale) clamp.
        auto repeat = [&](skvm::F32 v, float scale) {
            skvm::F32 S = p->uniformF(uniforms->pushF(     scale)),
                      I = p->uniformF(uniforms->pushF(1.0f/scale));
            // v - floor(v/scale)*scale
            return p->sub(v, p->mul(p->floor(p->mul(v,I)), S));
        };
        auto mirror = [&](skvm::F32 v, float scale) {
            skvm::F32 S  = p->uniformF(uniforms->pushF(     scale)),
                      I2 = p->uniformF(uniforms->pushF(0.5f/scale));
            // abs( (v-scale) - (2*scale)*floor((v-scale)*(0.5f/scale)) - scale )
            //      {---A---}   {------------------B------------------}
            skvm::F32 A = p->sub(v,S),
                      B = p->mul(p->add(S,S), p->floor(p->mul(A,I2)));
            return p->abs(p->sub(p->sub(A,B), S));
        };
        switch (fTileModeX) {
            case SkTileMode::kDecal:  /* handled after gather */ break;
            case SkTileMode::kClamp:  /*    we always clamp   */ break;
            case SkTileMode::kRepeat: sx = repeat(sx, pm.width()); break;
            case SkTileMode::kMirror: sx = mirror(sx, pm.width()); break;
        }
        switch (fTileModeY) {
            case SkTileMode::kDecal:  /* handled after gather */  break;
            case SkTileMode::kClamp:  /*    we always clamp   */  break;
            case SkTileMode::kRepeat: sy = repeat(sy, pm.height()); break;
            case SkTileMode::kMirror: sy = mirror(sy, pm.height()); break;
        }

        // Always clamp sample coordinates to [0,width), [0,height), both for memory
        // safety and to handle the clamps still needed by kClamp, kRepeat, and kMirror.
        auto clamp = [&](skvm::F32 v, float limit) {
            // Subtract an ulp so the upper clamp limit excludes limit itself.
            int bits;
            memcpy(&bits, &limit, 4);
            return p->clamp(v, p->splat(0.0f), p->uniformF(uniforms->push(bits-1)));
        };
        skvm::F32 clamped_x = clamp(sx, pm. width()),
                  clamped_y = clamp(sy, pm.height());

        // Load pixels from pm.addr()[(int)sx + (int)sy*stride].
        skvm::Builder::Uniform img = uniforms->pushPtr(pm.addr());
        skvm::I32 index = p->add(p->trunc(clamped_x),
                          p->mul(p->trunc(clamped_y),
                                 p->uniform32(uniforms->push(pm.rowBytesAsPixels()))));
        skvm::Color c;
        switch (pm.colorType()) {
            default: SkUNREACHABLE;
            case   kRGB_565_SkColorType: c = p->unpack_565 (p->gather16(img, index)); break;

            case  kRGB_888x_SkColorType: [[fallthrough]];
            case kRGBA_8888_SkColorType: c = p->unpack_8888(p->gather32(img, index));
                                         break;
            case kBGRA_8888_SkColorType: c = p->unpack_8888(p->gather32(img, index));
                                         std::swap(c.r, c.b);
                                         break;

            case  kRGB_101010x_SkColorType: [[fallthrough]];
            case kRGBA_1010102_SkColorType: c = p->unpack_1010102(p->gather32(img, index));
                                            break;

            case  kBGR_101010x_SkColorType: [[fallthrough]];
            case kBGRA_1010102_SkColorType: c = p->unpack_1010102(p->gather32(img, index));
                                            std::swap(c.r, c.b);
                                            break;
        }
        // If we know the image is opaque, jump right to alpha = 1.0f, skipping work to unpack it.
        if (input_is_opaque) {
            c.a = p->splat(1.0f);
        }

        // Mask away any pixels that we tried to sample outside the bounds in kDecal.
        if (fTileModeX == SkTileMode::kDecal || fTileModeY == SkTileMode::kDecal) {
            skvm::I32 mask = p->splat(~0);
            if (fTileModeX == SkTileMode::kDecal) { mask = p->bit_and(mask, p->eq(sx, clamped_x)); }
            if (fTileModeY == SkTileMode::kDecal) { mask = p->bit_and(mask, p->eq(sy, clamped_y)); }
            c.r = p->bit_cast(p->bit_and(mask, p->bit_cast(c.r)));
            c.g = p->bit_cast(p->bit_and(mask, p->bit_cast(c.g)));
            c.b = p->bit_cast(p->bit_and(mask, p->bit_cast(c.b)));
            c.a = p->bit_cast(p->bit_and(mask, p->bit_cast(c.a)));
            // Notice that even if input_is_opaque, c.a might now be 0.
        }

        return c;
    };

    if (quality == kNone_SkFilterQuality) {
        skvm::Color c = sample(x,y);
        *r = c.r;
        *g = c.g;
        *b = c.b;
        *a = c.a;
    } else if (quality == kLow_SkFilterQuality) {
        // Our four sample points are the corners of a logical 1x1 pixel
        // box surrounding (x,y) at (0.5,0.5) off-center.
        skvm::F32 left   = p->sub(x, p->splat(0.5f)),
                  top    = p->sub(y, p->splat(0.5f)),
                  right  = p->add(x, p->splat(0.5f)),
                  bottom = p->add(y, p->splat(0.5f));

        // The fractional parts of right and bottom are our lerp factors in x and y respectively.
        skvm::F32 fx = p->fract(right ),
                  fy = p->fract(bottom);

        skvm::Color c = p->lerp(p->lerp(sample(left,top   ), sample(right,top   ), fx),
                                p->lerp(sample(left,bottom), sample(right,bottom), fx), fy);
        *r = c.r;
        *g = c.g;
        *b = c.b;
        *a = c.a;
    } else {
        SkASSERT(quality == kHigh_SkFilterQuality);

        // All bicubic samples have the same fractional offset (fx,fy) from the center.
        // They're either the 16 corners of a 3x3 grid/ surrounding (x,y) at (0.5,0.5) off-center.
        skvm::F32 fx = p->fract(p->add(x, p->splat(0.5f))),
                  fy = p->fract(p->add(y, p->splat(0.5f)));

        // See GrCubicEffect for details of these weights.
        // TODO: these maybe don't seem right looking at gm/bicubic and GrBicubicEffect.
        auto near = [&](skvm::F32 t) {
            // 1/18 + 9/18t + 27/18t^2 - 21/18t^3 == t ( t ( -21/18t + 27/18) + 9/18) + 1/18
            return p->mad(t,
                   p->mad(t,
                   p->mad(t, p->splat(-21/18.0f),
                             p->splat( 27/18.0f)),
                             p->splat(  9/18.0f)),
                             p->splat(  1/18.0f));
        };
        auto far = [&](skvm::F32 t) {
            // 0/18 + 0/18*t - 6/18t^2 + 7/18t^3 == t^2 (7/18t - 6/18)
            return p->mul(p->mul(t,t), p->mad(t, p->splat( 7/18.0f),
                                                 p->splat(-6/18.0f)));
        };
        const skvm::F32 wx[] =  {
            far (p->sub(p->splat(1.0f), fx)),
            near(p->sub(p->splat(1.0f), fx)),
            near(                       fx ),
            far (                       fx ),
        };
        const skvm::F32 wy[] = {
            far (p->sub(p->splat(1.0f), fy)),
            near(p->sub(p->splat(1.0f), fy)),
            near(                       fy ),
            far (                       fy ),
        };

        *r = *g = *b = *a = p->splat(0.0f);

        skvm::F32 sy = p->add(y, p->splat(-1.5f));
        for (int j = 0; j < 4; j++, sy = p->add(sy, p->splat(1.0f))) {
            skvm::F32 sx = p->add(x, p->splat(-1.5f));
            for (int i = 0; i < 4; i++, sx = p->add(sx, p->splat(1.0f))) {
                skvm::Color c = sample(sx,sy);
                skvm::F32 w = p->mul(wx[i], wy[j]);

                *r = p->mad(c.r,w, *r);
                *g = p->mad(c.g,w, *g);
                *b = p->mad(c.b,w, *b);
                *a = p->mad(c.a,w, *a);
            }
        }
    }

    // If the input is opaque and we're not in decal mode, that means the output is too.
    // Forcing *a to 1.0 here will retroactively skip any work we did to interpolate sample alphas.
    if (input_is_opaque
            && fTileModeX != SkTileMode::kDecal
            && fTileModeY != SkTileMode::kDecal) {
        *a = p->splat(1.0f);
    }

    if (quality == kHigh_SkFilterQuality) {
        // Bicubic filtering naturally produces out of range values on both sides of [0,1].
        *a = p->clamp(*a, p->splat(0.0f), p->splat(1.0f));

        skvm::F32 limit = (pm.alphaType() == kUnpremul_SkAlphaType || fClampAsIfUnpremul)
                        ? p->splat(1.0f)
                        : *a;
        *r = p->clamp(*r, p->splat(0.0f), limit);
        *g = p->clamp(*g, p->splat(0.0f), limit);
        *b = p->clamp(*b, p->splat(0.0f), limit);
    }

    // Follow SkColorSpaceXformSteps to match shader output convention (dstCS, premul).
    // TODO: may need to extend lifetime once doing actual transforms?  maybe all in uniforms.
    auto flags = SkColorSpaceXformSteps{pm.colorSpace(), pm.alphaType(),
                                        dstCS, kPremul_SkAlphaType}.flags;

    // TODO: once this all works, move it to SkColorSpaceXformSteps
    if (flags.unpremul)        { p->unpremul(r,g,b,*a); }
    if (flags.linearize)       { return false; }
    if (flags.gamut_transform) { return false; }
    if (flags.encode)          { return false; }
    if (flags.premul)          { p->premul(r,g,b,*a); }
    return true;
}

