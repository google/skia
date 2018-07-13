/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkBitmapController.h"
#include "SkBitmapProcShader.h"
#include "SkBitmapProvider.h"
#include "SkColorSpaceXformSteps.h"
#include "SkEmptyShader.h"
#include "SkImage_Base.h"
#include "SkImageShader.h"
#include "SkPM4fPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "../jumper/SkJumper.h"

/**
 *  We are faster in clamp, so always use that tiling when we can.
 */
static SkShader::TileMode optimize(SkShader::TileMode tm, int dimension) {
    SkASSERT(dimension > 0);
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    // need to update frameworks/base/libs/hwui/tests/unit/SkiaBehaviorTests.cpp:55 to allow
    // for transforming to clamp.
    return tm;
#else
    return dimension == 1 ? SkShader::kClamp_TileMode : tm;
#endif
}

SkImageShader::SkImageShader(sk_sp<SkImage> img,
                             TileMode tmx, TileMode tmy,
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
    const TileMode tx = (TileMode)buffer.readUInt();
    const TileMode ty = (TileMode)buffer.readUInt();
    SkMatrix localMatrix;
    buffer.readMatrix(&localMatrix);
    sk_sp<SkImage> img = buffer.readImage();
    if (!img) {
        return nullptr;
    }
    return SkImageShader::Make(std::move(img), tx, ty, &localMatrix);
}

void SkImageShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeUInt(fTileModeX);
    buffer.writeUInt(fTileModeY);
    buffer.writeMatrix(this->getLocalMatrix());
    buffer.writeImage(fImage.get());
    SkASSERT(fClampAsIfUnpremul == false);
}

bool SkImageShader::isOpaque() const {
    return fImage->isOpaque() && fTileModeX != kDecal_TileMode && fTileModeY != kDecal_TileMode;
}

static bool legacy_shader_can_handle(const SkMatrix& inv) {
    if (!inv.isScaleTranslate()) {
        return false;
    }

    // legacy code uses SkFixed 32.32, so ensure the inverse doesn't map device coordinates
    // out of range.
    const SkScalar max_dev_coord = 32767.0f;
    SkRect src;
    SkAssertResult(inv.mapRect(&src, SkRect::MakeWH(max_dev_coord, max_dev_coord)));

    // take 1/4 of max signed 32bits so we have room to subtract local values
    const SkScalar max_fixed32dot32 = SK_MaxS32 * 0.25f;
    if (!SkRect::MakeLTRB(-max_fixed32dot32, -max_fixed32dot32,
                           max_fixed32dot32, max_fixed32dot32).contains(src)) {
        return false;
    }

    // legacy shader impl should be able to handle these matrices
    return true;
}

SkShaderBase::Context* SkImageShader::onMakeContext(const ContextRec& rec,
                                                    SkArenaAlloc* alloc) const {
    const auto info = as_IB(fImage)->onImageInfo();

    if (info.colorType() != kN32_SkColorType) {
        return nullptr;
    }
    if (info.alphaType() == kUnpremul_SkAlphaType) {
        return nullptr;
    }
#ifndef SK_SUPPORT_LEGACY_TILED_BITMAPS
    if (fTileModeX != fTileModeY) {
        return nullptr;
    }
#endif
    if (fTileModeX == kDecal_TileMode || fTileModeY == kDecal_TileMode) {
        return nullptr;
    }

    SkMatrix inv;
    if (!this->computeTotalInverse(*rec.fMatrix, rec.fLocalMatrix, &inv) ||
        !legacy_shader_can_handle(inv)) {
        return nullptr;
    }

    return SkBitmapProcLegacyShader::MakeContext(*this, fTileModeX, fTileModeY,
                                                 SkBitmapProvider(fImage.get()), rec, alloc);
}

SkImage* SkImageShader::onIsAImage(SkMatrix* texM, TileMode xy[]) const {
    if (texM) {
        *texM = this->getLocalMatrix();
    }
    if (xy) {
        xy[0] = (TileMode)fTileModeX;
        xy[1] = (TileMode)fTileModeY;
    }
    return const_cast<SkImage*>(fImage.get());
}

#ifdef SK_SUPPORT_LEGACY_SHADER_ISABITMAP
bool SkImageShader::onIsABitmap(SkBitmap* texture, SkMatrix* texM, TileMode xy[]) const {
    const SkBitmap* bm = as_IB(fImage)->onPeekBitmap();
    if (!bm) {
        return false;
    }

    if (texture) {
        *texture = *bm;
    }
    if (texM) {
        *texM = this->getLocalMatrix();
    }
    if (xy) {
        xy[0] = (TileMode)fTileModeX;
        xy[1] = (TileMode)fTileModeY;
    }
    return true;
}
#endif

static bool bitmap_is_too_big(int w, int h) {
    // SkBitmapProcShader stores bitmap coordinates in a 16bit buffer, as it
    // communicates between its matrix-proc and its sampler-proc. Until we can
    // widen that, we have to reject bitmaps that are larger.
    //
    static const int kMaxSize = 65535;

    return w > kMaxSize || h > kMaxSize;
}

sk_sp<SkShader> SkImageShader::Make(sk_sp<SkImage> image,
                                    TileMode tx, TileMode ty,
                                    const SkMatrix* localMatrix,
                                    bool clampAsIfUnpremul) {
    if (!image || bitmap_is_too_big(image->width(), image->height())) {
        return sk_make_sp<SkEmptyShader>();
    }
    return sk_sp<SkShader>{ new SkImageShader(image, tx,ty, localMatrix, clampAsIfUnpremul) };
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrColorSpaceInfo.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "SkGr.h"
#include "effects/GrBicubicEffect.h"
#include "effects/GrSimpleTextureEffect.h"

static GrSamplerState::WrapMode tile_mode_to_wrap_mode(const SkShader::TileMode tileMode) {
    switch (tileMode) {
        case SkShader::TileMode::kClamp_TileMode:
            return GrSamplerState::WrapMode::kClamp;
        case SkShader::TileMode::kRepeat_TileMode:
            return GrSamplerState::WrapMode::kRepeat;
        case SkShader::TileMode::kMirror_TileMode:
            return GrSamplerState::WrapMode::kMirrorRepeat;
        case SkShader::kDecal_TileMode:
            // TODO: depending on caps, we should extend WrapMode for decal...
            return GrSamplerState::WrapMode::kClamp;
    }
    SK_ABORT("Unknown tile mode.");
    return GrSamplerState::WrapMode::kClamp;
}

std::unique_ptr<GrFragmentProcessor> SkImageShader::asFragmentProcessor(
        const GrFPArgs& args) const {
    const auto lm = this->totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix);
    SkMatrix lmInverse;
    if (!lm->invert(&lmInverse)) {
        return nullptr;
    }

    GrSamplerState::WrapMode wrapModes[] = {tile_mode_to_wrap_mode(fTileModeX),
                                            tile_mode_to_wrap_mode(fTileModeY)};

    // Must set wrap and filter on the sampler before requesting a texture. In two places below
    // we check the matrix scale factors to determine how to interpret the filter quality setting.
    // This completely ignores the complexity of the drawVertices case where explicit local coords
    // are provided by the caller.
    bool doBicubic;
    GrSamplerState::Filter textureFilterMode = GrSkFilterQualityToGrFilterMode(
            args.fFilterQuality, *args.fViewMatrix, *lm,
            args.fContext->contextPriv().sharpenMipmappedTextures(), &doBicubic);
    GrSamplerState samplerState(wrapModes, textureFilterMode);
    sk_sp<SkColorSpace> texColorSpace;
    SkScalar scaleAdjust[2] = { 1.0f, 1.0f };
    sk_sp<GrTextureProxy> proxy(as_IB(fImage)->asTextureProxyRef(
            args.fContext, samplerState, args.fDstColorSpaceInfo->colorSpace(), &texColorSpace,
            scaleAdjust));
    if (!proxy) {
        return nullptr;
    }

    GrPixelConfig config = proxy->config();
    bool isAlphaOnly = GrPixelConfigIsAlphaOnly(config);

    lmInverse.postScale(scaleAdjust[0], scaleAdjust[1]);

    std::unique_ptr<GrFragmentProcessor> inner;
    if (doBicubic) {
        inner = GrBicubicEffect::Make(std::move(proxy), lmInverse, wrapModes);
    } else {
        inner = GrSimpleTextureEffect::Make(std::move(proxy), lmInverse, samplerState);
    }
    inner = GrColorSpaceXformEffect::Make(std::move(inner), texColorSpace.get(),
                                          args.fDstColorSpaceInfo->colorSpace());
    if (isAlphaOnly) {
        return inner;
    }
    return GrFragmentProcessor::MulChildByInputAlpha(std::move(inner));
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkImagePriv.h"

sk_sp<SkShader> SkMakeBitmapShader(const SkBitmap& src, SkShader::TileMode tmx,
                                   SkShader::TileMode tmy, const SkMatrix* localMatrix,
                                   SkCopyPixelsMode cpm) {
    return SkImageShader::Make(SkMakeImageFromRasterBitmap(src, cpm),
                               tmx, tmy, localMatrix);
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkShaderBase)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkImageShader)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

bool SkImageShader::onAppendStages(const StageRec& rec) const {
    SkRasterPipeline* p = rec.fPipeline;
    SkArenaAlloc* alloc = rec.fAlloc;

    SkMatrix matrix;
    if (!this->computeTotalInverse(rec.fCTM, rec.fLocalM, &matrix)) {
        return false;
    }
    auto quality = rec.fPaint.getFilterQuality();

    SkBitmapProvider provider(fImage.get());
    SkDefaultBitmapController controller;
    SkBitmapController::State* state = controller.requestBitmap(provider, matrix, quality, alloc);
    if (!state) {
        return false;
    }

    const SkPixmap& pm = state->pixmap();
    matrix  = state->invMatrix();
    quality = state->quality();
    auto info = pm.info();

    // When the matrix is just an integer translate, bilerp == nearest neighbor.
    if (quality == kLow_SkFilterQuality &&
        matrix.getType() <= SkMatrix::kTranslate_Mask &&
        matrix.getTranslateX() == (int)matrix.getTranslateX() &&
        matrix.getTranslateY() == (int)matrix.getTranslateY()) {
        quality = kNone_SkFilterQuality;
    }

    // See skia:4649 and the GM image_scale_aligned.
    if (quality == kNone_SkFilterQuality) {
        if (matrix.getScaleX() >= 0) {
            matrix.setTranslateX(nextafterf(matrix.getTranslateX(),
                                            floorf(matrix.getTranslateX())));
        }
        if (matrix.getScaleY() >= 0) {
            matrix.setTranslateY(nextafterf(matrix.getTranslateY(),
                                            floorf(matrix.getTranslateY())));
        }
    }

    p->append(SkRasterPipeline::seed_shader);

    struct MiscCtx {
        SkColor4f paint_color;
    };
    auto misc = alloc->make<MiscCtx>();
    swizzle_rb(Sk4f_fromL32(rec.fPaint.getColor())).store(misc->paint_color.vec());  // sRGBA floats
    p->append_matrix(alloc, matrix);

    auto gather = alloc->make<SkJumper_GatherCtx>();
    gather->pixels = pm.addr();
    gather->stride = pm.rowBytesAsPixels();
    gather->width  = pm.width();
    gather->height = pm.height();

    auto limit_x = alloc->make<SkJumper_TileCtx>(),
         limit_y = alloc->make<SkJumper_TileCtx>();
    limit_x->scale = pm.width();
    limit_x->invScale = 1.0f / pm.width();
    limit_y->scale = pm.height();
    limit_y->invScale = 1.0f / pm.height();

    SkJumper_DecalTileCtx* decal_ctx = nullptr;
    bool decal_x_and_y = fTileModeX == kDecal_TileMode && fTileModeY == kDecal_TileMode;
    if (fTileModeX == kDecal_TileMode || fTileModeY == kDecal_TileMode) {
        decal_ctx = alloc->make<SkJumper_DecalTileCtx>();
        decal_ctx->limit_x = limit_x->scale;
        decal_ctx->limit_y = limit_y->scale;
    }

    auto append_tiling_and_gather = [&] {
        if (decal_x_and_y) {
            p->append(SkRasterPipeline::decal_x_and_y,  decal_ctx);
        } else {
            switch (fTileModeX) {
                case kClamp_TileMode:  /* The gather_xxx stage will clamp for us. */     break;
                case kMirror_TileMode: p->append(SkRasterPipeline::mirror_x, limit_x);   break;
                case kRepeat_TileMode: p->append(SkRasterPipeline::repeat_x, limit_x);   break;
                case kDecal_TileMode:  p->append(SkRasterPipeline::decal_x,  decal_ctx); break;
            }
            switch (fTileModeY) {
                case kClamp_TileMode:  /* The gather_xxx stage will clamp for us. */     break;
                case kMirror_TileMode: p->append(SkRasterPipeline::mirror_y, limit_y);   break;
                case kRepeat_TileMode: p->append(SkRasterPipeline::repeat_y, limit_y);   break;
                case kDecal_TileMode:  p->append(SkRasterPipeline::decal_y,  decal_ctx); break;
            }
        }

        void* ctx = gather;
        switch (info.colorType()) {
            case kAlpha_8_SkColorType:      p->append(SkRasterPipeline::gather_a8,      ctx); break;
            case kGray_8_SkColorType:       p->append(SkRasterPipeline::gather_g8,      ctx); break;
            case kRGB_565_SkColorType:      p->append(SkRasterPipeline::gather_565,     ctx); break;
            case kARGB_4444_SkColorType:    p->append(SkRasterPipeline::gather_4444,    ctx); break;
            case kBGRA_8888_SkColorType:    p->append(SkRasterPipeline::gather_bgra,    ctx); break;
            case kRGBA_8888_SkColorType:    p->append(SkRasterPipeline::gather_8888,    ctx); break;
            case kRGBA_1010102_SkColorType: p->append(SkRasterPipeline::gather_1010102, ctx); break;
            case kRGBA_F16_SkColorType:     p->append(SkRasterPipeline::gather_f16,     ctx); break;
            case kRGBA_F32_SkColorType:     p->append(SkRasterPipeline::gather_f32,     ctx); break;

            case kRGB_888x_SkColorType:     p->append(SkRasterPipeline::gather_8888,    ctx);
                                            p->append(SkRasterPipeline::force_opaque       ); break;
            case kRGB_101010x_SkColorType:  p->append(SkRasterPipeline::gather_1010102, ctx);
                                            p->append(SkRasterPipeline::force_opaque       ); break;

            default: SkASSERT(false);
        }
        if (decal_ctx) {
            p->append(SkRasterPipeline::check_decal_mask, decal_ctx);
        }
    };

    auto append_misc = [&] {
        // TODO: if ref.fDstCS isn't null, we'll premul here then immediately unpremul
        // to do the color space transformation.  Might be possible to streamline.
        if (info.colorType() == kAlpha_8_SkColorType) {
            // The color for A8 images comes from the (sRGB) paint color.
            p->append(SkRasterPipeline::set_rgb, &misc->paint_color);
            p->append(SkRasterPipeline::premul);
        } else if (info.alphaType() == kUnpremul_SkAlphaType) {
            // Convert unpremul images to premul before we carry on with the rest of the pipeline.
            p->append(SkRasterPipeline::premul);
        }

        if (quality > kLow_SkFilterQuality) {
            // Bicubic filtering naturally produces out of range values on both sides.
            p->append(SkRasterPipeline::clamp_0);
            p->append(fClampAsIfUnpremul ? SkRasterPipeline::clamp_1
                                         : SkRasterPipeline::clamp_a);
        }

        if (rec.fDstCS) {
            // If color managed, convert from premul source all the way to premul dst color space.
            auto srcCS = info.colorSpace();
            if (!srcCS || info.colorType() == kAlpha_8_SkColorType) {
                // We treat untagged images as sRGB.
                // A8 images get their r,g,b from the paint color, so they're also sRGB.
                srcCS = SkColorSpace::MakeSRGB().get();
            }
            alloc->make<SkColorSpaceXformSteps>(srcCS, kPremul_SkAlphaType, rec.fDstCS)
                ->apply(p);
        }

        return true;
    };

    // We've got a fast path for 8888 bilinear clamp/clamp sampling.
    auto ct = info.colorType();
    if (true
        && (ct == kRGBA_8888_SkColorType || ct == kBGRA_8888_SkColorType)
        && quality == kLow_SkFilterQuality
        && fTileModeX == SkShader::kClamp_TileMode
        && fTileModeY == SkShader::kClamp_TileMode) {

        p->append(SkRasterPipeline::bilerp_clamp_8888, gather);
        if (ct == kBGRA_8888_SkColorType) {
            p->append(SkRasterPipeline::swap_rb);
        }
        return append_misc();
    }

    SkJumper_SamplerCtx* sampler = nullptr;
    if (quality != kNone_SkFilterQuality) {
        sampler = alloc->make<SkJumper_SamplerCtx>();
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
