/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSWMaskHelper.h"

#include "GrPipelineBuilder.h"
#include "GrCaps.h"
#include "GrDrawTarget.h"
#include "GrGpu.h"

#include "SkData.h"
#include "SkDistanceFieldGen.h"
#include "SkStrokeRec.h"

// TODO: try to remove this #include
#include "GrContext.h"

namespace {

/*
 * Convert a boolean operation into a transfer mode code
 */
SkXfermode::Mode op_to_mode(SkRegion::Op op) {

    static const SkXfermode::Mode modeMap[] = {
        SkXfermode::kDstOut_Mode,   // kDifference_Op
        SkXfermode::kModulate_Mode, // kIntersect_Op
        SkXfermode::kSrcOver_Mode,  // kUnion_Op
        SkXfermode::kXor_Mode,      // kXOR_Op
        SkXfermode::kClear_Mode,    // kReverseDifference_Op
        SkXfermode::kSrc_Mode,      // kReplace_Op
    };

    return modeMap[op];
}

static inline GrPixelConfig fmt_to_config(SkTextureCompressor::Format fmt) {

    GrPixelConfig config;
    switch (fmt) {
        case SkTextureCompressor::kLATC_Format:
            config = kLATC_GrPixelConfig;
            break;

        case SkTextureCompressor::kR11_EAC_Format:
            config = kR11_EAC_GrPixelConfig;
            break;

        case SkTextureCompressor::kASTC_12x12_Format:
            config = kASTC_12x12_GrPixelConfig;
            break;

        case SkTextureCompressor::kETC1_Format:
            config = kETC1_GrPixelConfig;
            break;

        default:
            SkDEBUGFAIL("No GrPixelConfig for compression format!");
            // Best guess
            config = kAlpha_8_GrPixelConfig;
            break;
    }

    return config;
}

static bool choose_compressed_fmt(const GrCaps* caps,
                                  SkTextureCompressor::Format *fmt) {
    if (NULL == fmt) {
        return false;
    }

    // We can't use scratch textures without the ability to update
    // compressed textures...
    if (!(caps->compressedTexSubImageSupport())) {
        return false;
    }

    // Figure out what our preferred texture type is. If ASTC is available, that always
    // gives the biggest win. Otherwise, in terms of compression speed and accuracy,
    // LATC has a slight edge over R11 EAC.
    if (caps->isConfigTexturable(kASTC_12x12_GrPixelConfig)) {
        *fmt = SkTextureCompressor::kASTC_12x12_Format;
        return true;
    } else if (caps->isConfigTexturable(kLATC_GrPixelConfig)) {
        *fmt = SkTextureCompressor::kLATC_Format;
        return true;
    } else if (caps->isConfigTexturable(kR11_EAC_GrPixelConfig)) {
        *fmt = SkTextureCompressor::kR11_EAC_Format;
        return true;
    }

    return false;
}

}

/**
 * Draw a single rect element of the clip stack into the accumulation bitmap
 */
void GrSWMaskHelper::draw(const SkRect& rect, SkRegion::Op op,
                          bool antiAlias, uint8_t alpha) {
    SkPaint paint;

    SkXfermode* mode = SkXfermode::Create(op_to_mode(op));

    SkASSERT(kNone_CompressionMode == fCompressionMode);

    paint.setXfermode(mode);
    paint.setAntiAlias(antiAlias);
    paint.setColor(SkColorSetARGB(alpha, alpha, alpha, alpha));

    fDraw.drawRect(rect, paint);

    SkSafeUnref(mode);
}

/**
 * Draw a single path element of the clip stack into the accumulation bitmap
 */
void GrSWMaskHelper::draw(const SkPath& path, const SkStrokeRec& stroke, SkRegion::Op op,
                          bool antiAlias, uint8_t alpha) {

    SkPaint paint;
    if (stroke.isHairlineStyle()) {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SK_Scalar1);
    } else {
        if (stroke.isFillStyle()) {
            paint.setStyle(SkPaint::kFill_Style);
        } else {
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeJoin(stroke.getJoin());
            paint.setStrokeCap(stroke.getCap());
            paint.setStrokeWidth(stroke.getWidth());
        }
    }
    paint.setAntiAlias(antiAlias);

    SkTBlitterAllocator allocator;
    SkBlitter* blitter = NULL;
    if (kBlitter_CompressionMode == fCompressionMode) {
        SkASSERT(fCompressedBuffer.get());
        blitter = SkTextureCompressor::CreateBlitterForFormat(
            fPixels.width(), fPixels.height(), fCompressedBuffer.get(), &allocator,
                                                              fCompressedFormat);
    }

    if (SkRegion::kReplace_Op == op && 0xFF == alpha) {
        SkASSERT(0xFF == paint.getAlpha());
        fDraw.drawPathCoverage(path, paint, blitter);
    } else {
        paint.setXfermodeMode(op_to_mode(op));
        paint.setColor(SkColorSetARGB(alpha, alpha, alpha, alpha));
        fDraw.drawPath(path, paint, blitter);
    }
}

bool GrSWMaskHelper::init(const SkIRect& resultBounds,
                          const SkMatrix* matrix,
                          bool allowCompression) {
    if (matrix) {
        fMatrix = *matrix;
    } else {
        fMatrix.setIdentity();
    }

    // Now translate so the bound's UL corner is at the origin
    fMatrix.postTranslate(-resultBounds.fLeft * SK_Scalar1,
                          -resultBounds.fTop * SK_Scalar1);
    SkIRect bounds = SkIRect::MakeWH(resultBounds.width(),
                                     resultBounds.height());

    if (allowCompression &&
        fContext->caps()->drawPathMasksToCompressedTexturesSupport() &&
        choose_compressed_fmt(fContext->caps(), &fCompressedFormat)) {
        fCompressionMode = kCompress_CompressionMode;
    }

    // Make sure that the width is a multiple of the desired block dimensions
    // to allow for specialized SIMD instructions that compress multiple blocks at a time.
    int cmpWidth = bounds.fRight;
    int cmpHeight = bounds.fBottom;
    if (kCompress_CompressionMode == fCompressionMode) {
        int dimX, dimY;
        SkTextureCompressor::GetBlockDimensions(fCompressedFormat, &dimX, &dimY);
        cmpWidth = dimX * ((cmpWidth + (dimX - 1)) / dimX);
        cmpHeight = dimY * ((cmpHeight + (dimY - 1)) / dimY);

        // Can we create a blitter?
        if (SkTextureCompressor::ExistsBlitterForFormat(fCompressedFormat)) {
            int cmpSz = SkTextureCompressor::GetCompressedDataSize(
                fCompressedFormat, cmpWidth, cmpHeight);

            SkASSERT(cmpSz > 0);
            SkASSERT(NULL == fCompressedBuffer.get());
            fCompressedBuffer.reset(cmpSz);
            fCompressionMode = kBlitter_CompressionMode;
        }
    } 

    sk_bzero(&fDraw, sizeof(fDraw));

    // If we don't have a custom blitter, then we either need a bitmap to compress
    // from or a bitmap that we're going to use as a texture. In any case, we should
    // allocate the pixels for a bitmap
    const SkImageInfo bmImageInfo = SkImageInfo::MakeA8(cmpWidth, cmpHeight);
    if (kBlitter_CompressionMode != fCompressionMode) {
        if (!fPixels.tryAlloc(bmImageInfo)) {
            return false;
        }
        fPixels.erase(0);
    } else {
        // Otherwise, we just need to remember how big the buffer is...
        fPixels.reset(bmImageInfo);
    }
    fDraw.fDst      = fPixels;
    fRasterClip.setRect(bounds);
    fDraw.fRC       = &fRasterClip;
    fDraw.fClip     = &fRasterClip.bwRgn();
    fDraw.fMatrix   = &fMatrix;
    return true;
}

/**
 * Get a texture (from the texture cache) of the correct size & format.
 */
GrTexture* GrSWMaskHelper::createTexture() {
    GrSurfaceDesc desc;
    desc.fWidth = fPixels.width();
    desc.fHeight = fPixels.height();
    desc.fConfig = kAlpha_8_GrPixelConfig;

    if (kNone_CompressionMode != fCompressionMode) {

#ifdef SK_DEBUG
        int dimX, dimY;
        SkTextureCompressor::GetBlockDimensions(fCompressedFormat, &dimX, &dimY);
        SkASSERT((desc.fWidth % dimX) == 0);
        SkASSERT((desc.fHeight % dimY) == 0);
#endif

        desc.fConfig = fmt_to_config(fCompressedFormat);
        SkASSERT(fContext->caps()->isConfigTexturable(desc.fConfig));
    }

    return fContext->textureProvider()->createApproxTexture(desc);
}

void GrSWMaskHelper::sendTextureData(GrTexture *texture, const GrSurfaceDesc& desc,
                                     const void *data, size_t rowbytes) {
    // If we aren't reusing scratch textures we don't need to flush before
    // writing since no one else will be using 'texture'
    bool reuseScratch = fContext->caps()->reuseScratchTextures();

    // Since we're uploading to it, and it's compressed, 'texture' shouldn't
    // have a render target.
    SkASSERT(NULL == texture->asRenderTarget());

    texture->writePixels(0, 0, desc.fWidth, desc.fHeight,
                         desc.fConfig, data, rowbytes,
                         reuseScratch ? 0 : GrContext::kDontFlush_PixelOpsFlag);
}

void GrSWMaskHelper::compressTextureData(GrTexture *texture, const GrSurfaceDesc& desc) {

    SkASSERT(GrPixelConfigIsCompressed(desc.fConfig));
    SkASSERT(fmt_to_config(fCompressedFormat) == desc.fConfig);

    SkAutoDataUnref cmpData(SkTextureCompressor::CompressBitmapToFormat(fPixels,
                                                                        fCompressedFormat));
    SkASSERT(cmpData);

    this->sendTextureData(texture, desc, cmpData->data(), 0);
}

/**
 * Move the result of the software mask generation back to the gpu
 */
void GrSWMaskHelper::toTexture(GrTexture *texture) {
    GrSurfaceDesc desc;
    desc.fWidth = fPixels.width();
    desc.fHeight = fPixels.height();
    desc.fConfig = texture->config();
        
    // First see if we should compress this texture before uploading.
    switch (fCompressionMode) {
        case kNone_CompressionMode:
            this->sendTextureData(texture, desc, fPixels.addr(), fPixels.rowBytes());
            break;

        case kCompress_CompressionMode:
            this->compressTextureData(texture, desc);
            break;

        case kBlitter_CompressionMode:
            SkASSERT(fCompressedBuffer.get());
            this->sendTextureData(texture, desc, fCompressedBuffer.get(), 0);
            break;
    }
}

/**
 * Convert mask generation results to a signed distance field
 */
void GrSWMaskHelper::toSDF(unsigned char* sdf) {
    SkGenerateDistanceFieldFromA8Image(sdf, (const unsigned char*)fPixels.addr(),
                                       fPixels.width(), fPixels.height(), fPixels.rowBytes());
}

////////////////////////////////////////////////////////////////////////////////
/**
 * Software rasterizes path to A8 mask (possibly using the context's matrix)
 * and uploads the result to a scratch texture. Returns the resulting
 * texture on success; NULL on failure.
 */
GrTexture* GrSWMaskHelper::DrawPathMaskToTexture(GrContext* context,
                                                 const SkPath& path,
                                                 const SkStrokeRec& stroke,
                                                 const SkIRect& resultBounds,
                                                 bool antiAlias,
                                                 const SkMatrix* matrix) {
    GrSWMaskHelper helper(context);

    if (!helper.init(resultBounds, matrix)) {
        return NULL;
    }

    helper.draw(path, stroke, SkRegion::kReplace_Op, antiAlias, 0xFF);

    GrTexture* texture(helper.createTexture());
    if (!texture) {
        return NULL;
    }

    helper.toTexture(texture);

    return texture;
}

void GrSWMaskHelper::DrawToTargetWithPathMask(GrTexture* texture,
                                              GrDrawTarget* target,
                                              GrPipelineBuilder* pipelineBuilder,
                                              GrColor color,
                                              const SkMatrix& viewMatrix,
                                              const SkIRect& rect) {
    SkMatrix invert;
    if (!viewMatrix.invert(&invert)) {
        return;
    }
    GrPipelineBuilder::AutoRestoreFragmentProcessorState arfps(*pipelineBuilder);

    SkRect dstRect = SkRect::MakeLTRB(SK_Scalar1 * rect.fLeft,
                                      SK_Scalar1 * rect.fTop,
                                      SK_Scalar1 * rect.fRight,
                                      SK_Scalar1 * rect.fBottom);

    // We use device coords to compute the texture coordinates. We take the device coords and apply
    // a translation so that the top-left of the device bounds maps to 0,0, and then a scaling
    // matrix to normalized coords.
    SkMatrix maskMatrix;
    maskMatrix.setIDiv(texture->width(), texture->height());
    maskMatrix.preTranslate(SkIntToScalar(-rect.fLeft), SkIntToScalar(-rect.fTop));

    pipelineBuilder->addCoverageProcessor(
                         GrSimpleTextureEffect::Create(pipelineBuilder->getProcessorDataManager(),
                                                       texture,
                                                       maskMatrix,
                                                       GrTextureParams::kNone_FilterMode,
                                                       kDevice_GrCoordSet))->unref();

    target->drawBWRect(*pipelineBuilder, color, SkMatrix::I(), dstRect, NULL, &invert);
}
