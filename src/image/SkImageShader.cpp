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
#include "SkColorTable.h"
#include "SkEmptyShader.h"
#include "SkImage_Base.h"
#include "SkImageShader.h"
#include "SkPM4fPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "../jumper/SkJumper.h"

SkImageShader::SkImageShader(sk_sp<SkImage> img, TileMode tmx, TileMode tmy, const SkMatrix* matrix)
    : INHERITED(matrix)
    , fImage(std::move(img))
    , fTileModeX(tmx)
    , fTileModeY(tmy)
{}

sk_sp<SkFlattenable> SkImageShader::CreateProc(SkReadBuffer& buffer) {
    const TileMode tx = (TileMode)buffer.readUInt();
    const TileMode ty = (TileMode)buffer.readUInt();
    SkMatrix matrix;
    buffer.readMatrix(&matrix);
    sk_sp<SkImage> img = buffer.readImage();
    if (!img) {
        return nullptr;
    }
    return SkImageShader::Make(std::move(img), tx, ty, &matrix);
}

void SkImageShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeUInt(fTileModeX);
    buffer.writeUInt(fTileModeY);
    buffer.writeMatrix(this->getLocalMatrix());
    buffer.writeImage(fImage.get());
}

bool SkImageShader::isOpaque() const {
    return fImage->isOpaque();
}

SkShader::Context* SkImageShader::onMakeContext(const ContextRec& rec, SkArenaAlloc* alloc) const {
    return SkBitmapProcLegacyShader::MakeContext(*this, fTileModeX, fTileModeY,
                                                 SkBitmapProvider(fImage.get(), rec.fDstColorSpace),
                                                 rec, alloc);
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

sk_sp<SkShader> SkImageShader::Make(sk_sp<SkImage> image, TileMode tx, TileMode ty,
                                    const SkMatrix* localMatrix) {
    if (!image || bitmap_is_too_big(image->width(), image->height())) {
        return sk_make_sp<SkEmptyShader>();
    } else {
        return sk_make_sp<SkImageShader>(image, tx, ty, localMatrix);
    }
}

#ifndef SK_IGNORE_TO_STRING
void SkImageShader::toString(SkString* str) const {
    const char* gTileModeName[SkShader::kTileModeCount] = {
        "clamp", "repeat", "mirror"
    };

    str->appendf("ImageShader: ((%s %s) ", gTileModeName[fTileModeX], gTileModeName[fTileModeY]);
    fImage->toString(str);
    this->INHERITED::toString(str);
    str->append(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "SkGr.h"
#include "GrContext.h"
#include "effects/GrSimpleTextureEffect.h"
#include "effects/GrBicubicEffect.h"
#include "effects/GrSimpleTextureEffect.h"

sk_sp<GrFragmentProcessor> SkImageShader::asFragmentProcessor(const AsFPArgs& args) const {

    SkMatrix lmInverse;
    if (!this->getLocalMatrix().invert(&lmInverse)) {
        return nullptr;
    }
    if (args.fLocalMatrix) {
        SkMatrix inv;
        if (!args.fLocalMatrix->invert(&inv)) {
            return nullptr;
        }
        lmInverse.postConcat(inv);
    }

    SkShader::TileMode tm[] = { fTileModeX, fTileModeY };

    // Must set wrap and filter on the sampler before requesting a texture. In two places below
    // we check the matrix scale factors to determine how to interpret the filter quality setting.
    // This completely ignores the complexity of the drawVertices case where explicit local coords
    // are provided by the caller.
    bool doBicubic;
    GrSamplerParams::FilterMode textureFilterMode =
    GrSkFilterQualityToGrFilterMode(args.fFilterQuality, *args.fViewMatrix, this->getLocalMatrix(),
                                    &doBicubic);
    GrSamplerParams params(tm, textureFilterMode);
    sk_sp<SkColorSpace> texColorSpace;
    SkScalar scaleAdjust[2] = { 1.0f, 1.0f };
    sk_sp<GrTextureProxy> proxy(as_IB(fImage)->asTextureProxyRef(args.fContext, params,
                                                                 args.fDstColorSpace,
                                                                 &texColorSpace, scaleAdjust));
    if (!proxy) {
        return nullptr;
    }

    bool isAlphaOnly = GrPixelConfigIsAlphaOnly(proxy->config());

    lmInverse.postScale(scaleAdjust[0], scaleAdjust[1]);

    sk_sp<GrColorSpaceXform> colorSpaceXform = GrColorSpaceXform::Make(texColorSpace.get(),
                                                                       args.fDstColorSpace);
    sk_sp<GrFragmentProcessor> inner;
    if (doBicubic) {
        inner = GrBicubicEffect::Make(args.fContext->resourceProvider(), std::move(proxy),
                                      std::move(colorSpaceXform), lmInverse, tm);
    } else {
        inner = GrSimpleTextureEffect::Make(args.fContext->resourceProvider(), std::move(proxy),
                                            std::move(colorSpaceXform), lmInverse, params);
    }

    if (isAlphaOnly) {
        return inner;
    }
    return sk_sp<GrFragmentProcessor>(GrFragmentProcessor::MulOutputByInputAlpha(std::move(inner)));
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

static sk_sp<SkFlattenable> SkBitmapProcShader_CreateProc(SkReadBuffer& buffer) {
    SkMatrix lm;
    buffer.readMatrix(&lm);
    sk_sp<SkImage> image = buffer.readBitmapAsImage();
    SkShader::TileMode mx = (SkShader::TileMode)buffer.readUInt();
    SkShader::TileMode my = (SkShader::TileMode)buffer.readUInt();
    return image ? image->makeShader(mx, my, &lm) : nullptr;
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkShader)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkImageShader)
SkFlattenable::Register("SkBitmapProcShader", SkBitmapProcShader_CreateProc, kSkShader_Type);
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END


bool SkImageShader::onAppendStages(SkRasterPipeline* p, SkColorSpace* dst, SkArenaAlloc* scratch,
                                   const SkMatrix& ctm, const SkPaint& paint,
                                   const SkMatrix* localM) const {
    auto matrix = SkMatrix::Concat(ctm, this->getLocalMatrix());
    if (localM) {
        matrix.preConcat(*localM);
    }

    if (!matrix.invert(&matrix)) {
        return false;
    }
    auto quality = paint.getFilterQuality();

    SkBitmapProvider provider(fImage.get(), dst);
    SkDefaultBitmapController controller(SkDefaultBitmapController::CanShadeHQ::kYes);
    std::unique_ptr<SkBitmapController::State> state {
        controller.requestBitmap(provider, matrix, quality)
    };
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


    struct MiscCtx {
        std::unique_ptr<SkBitmapController::State> state;
        SkColor4f paint_color;
        float     width;
        float     height;
        float     matrix[9];
    };
    auto misc = scratch->make<MiscCtx>();
    misc->state       = std::move(state);  // Extend lifetime to match the pipeline's.
    misc->paint_color = SkColor4f_from_SkColor(paint.getColor(), dst);
    misc->width       = (float)pm.width();
    misc->height      = (float)pm.height();
    if (matrix.asAffine(misc->matrix)) {
        p->append(SkRasterPipeline::matrix_2x3, misc->matrix);
    } else {
        matrix.get9(misc->matrix);
        p->append(SkRasterPipeline::matrix_perspective, misc->matrix);
    }

    auto gather = scratch->make<SkJumper_GatherCtx>();
    gather->pixels  = pm.addr();
    gather->ctable  = pm.ctable() ? pm.ctable()->readColors() : nullptr;
    gather->stride  = pm.rowBytesAsPixels();

    auto append_tiling_and_gather = [&] {
        switch (fTileModeX) {
            case kClamp_TileMode:  p->append(SkRasterPipeline::clamp_x,  &misc->width); break;
            case kMirror_TileMode: p->append(SkRasterPipeline::mirror_x, &misc->width); break;
            case kRepeat_TileMode: p->append(SkRasterPipeline::repeat_x, &misc->width); break;
        }
        switch (fTileModeY) {
            case kClamp_TileMode:  p->append(SkRasterPipeline::clamp_y,  &misc->height); break;
            case kMirror_TileMode: p->append(SkRasterPipeline::mirror_y, &misc->height); break;
            case kRepeat_TileMode: p->append(SkRasterPipeline::repeat_y, &misc->height); break;
        }
        switch (info.colorType()) {
            case kAlpha_8_SkColorType:   p->append(SkRasterPipeline::gather_a8,   gather); break;
            case kIndex_8_SkColorType:   p->append(SkRasterPipeline::gather_i8,   gather); break;
            case kGray_8_SkColorType:    p->append(SkRasterPipeline::gather_g8,   gather); break;
            case kRGB_565_SkColorType:   p->append(SkRasterPipeline::gather_565,  gather); break;
            case kARGB_4444_SkColorType: p->append(SkRasterPipeline::gather_4444, gather); break;
            case kRGBA_8888_SkColorType:
            case kBGRA_8888_SkColorType: p->append(SkRasterPipeline::gather_8888, gather); break;
            case kRGBA_F16_SkColorType:  p->append(SkRasterPipeline::gather_f16,  gather); break;
            default: SkASSERT(false);
        }
        if (info.gammaCloseToSRGB() && dst != nullptr) {
            p->append_from_srgb(info.alphaType());
        }
    };

    SkJumper_SamplerCtx* sampler = nullptr;
    if (quality != kNone_SkFilterQuality) {
        sampler = scratch->make<SkJumper_SamplerCtx>();
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

    auto effective_color_type = [](SkColorType ct) {
        return ct == kIndex_8_SkColorType ? kN32_SkColorType : ct;
    };

    if (effective_color_type(info.colorType()) == kBGRA_8888_SkColorType) {
        p->append(SkRasterPipeline::swap_rb);
    }
    if (info.colorType() == kAlpha_8_SkColorType) {
        p->append(SkRasterPipeline::set_rgb, &misc->paint_color);
    }
    if (info.colorType() == kAlpha_8_SkColorType || info.alphaType() == kUnpremul_SkAlphaType) {
        p->append(SkRasterPipeline::premul);
    }
    if (quality > kLow_SkFilterQuality) {
        // Bicubic filtering naturally produces out of range values on both sides.
        p->append(SkRasterPipeline::clamp_0);
        p->append(SkRasterPipeline::clamp_a);
    }
    return append_gamut_transform(p, scratch, info.colorSpace(), dst);
}
