/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPictureShader.h"

#include "SkBitmap.h"
#include "SkBitmapProcShader.h"
#include "SkCanvas.h"
#include "SkMatrixUtils.h"
#include "SkPicture.h"
#include "SkReadBuffer.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

SkPictureShader::SkPictureShader(const SkPicture* picture, TileMode tmx, TileMode tmy,
                                 const SkMatrix* localMatrix, const SkRect* tile)
    : INHERITED(localMatrix)
    , fPicture(SkRef(picture))
    , fTile(tile ? *tile : picture->cullRect())
    , fTmx(tmx)
    , fTmy(tmy) {
}

#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
SkPictureShader::SkPictureShader(SkReadBuffer& buffer) : INHERITED(buffer) {
    fTmx = static_cast<SkShader::TileMode>(buffer.read32());
    fTmy = static_cast<SkShader::TileMode>(buffer.read32());
    buffer.readRect(&fTile);
    fPicture = SkPicture::CreateFromBuffer(buffer);
}
#endif

SkPictureShader::~SkPictureShader() {
    fPicture->unref();
}

SkPictureShader* SkPictureShader::Create(const SkPicture* picture, TileMode tmx, TileMode tmy,
                                         const SkMatrix* localMatrix, const SkRect* tile) {
    if (!picture || picture->cullRect().isEmpty() || (tile && tile->isEmpty())) {
        return NULL;
    }
    return SkNEW_ARGS(SkPictureShader, (picture, tmx, tmy, localMatrix, tile));
}

SkFlattenable* SkPictureShader::CreateProc(SkReadBuffer& buffer) {
    SkMatrix lm;
    buffer.readMatrix(&lm);
    TileMode mx = (TileMode)buffer.read32();
    TileMode my = (TileMode)buffer.read32();
    SkRect tile;
    buffer.readRect(&tile);
    SkAutoTUnref<SkPicture> picture(SkPicture::CreateFromBuffer(buffer));
    return SkPictureShader::Create(picture, mx, my, &lm, &tile);
}

void SkPictureShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeMatrix(this->getLocalMatrix());
    buffer.write32(fTmx);
    buffer.write32(fTmy);
    buffer.writeRect(fTile);
    fPicture->flatten(buffer);
}

SkShader* SkPictureShader::refBitmapShader(const SkMatrix& matrix, const SkMatrix* localM) const {
    SkASSERT(fPicture && !fPicture->cullRect().isEmpty());

    SkMatrix m;
    m.setConcat(matrix, this->getLocalMatrix());
    if (localM) {
        m.preConcat(*localM);
    }

    // Use a rotation-invariant scale
    SkPoint scale;
    if (!SkDecomposeUpper2x2(m, NULL, &scale, NULL)) {
        // Decomposition failed, use an approximation.
        scale.set(SkScalarSqrt(m.getScaleX() * m.getScaleX() + m.getSkewX() * m.getSkewX()),
                  SkScalarSqrt(m.getScaleY() * m.getScaleY() + m.getSkewY() * m.getSkewY()));
    }
    SkSize scaledSize = SkSize::Make(scale.x() * fTile.width(), scale.y() * fTile.height());

    // Clamp the tile size to about 16M pixels
    static const SkScalar kMaxTileArea = 4096 * 4096;
    SkScalar tileArea = SkScalarMul(scaledSize.width(), scaledSize.height());
    if (tileArea > kMaxTileArea) {
        SkScalar clampScale = SkScalarSqrt(SkScalarDiv(kMaxTileArea, tileArea));
        scaledSize.set(SkScalarMul(scaledSize.width(), clampScale),
                       SkScalarMul(scaledSize.height(), clampScale));
    }

    SkISize tileSize = scaledSize.toRound();
    if (tileSize.isEmpty()) {
        return NULL;
    }

    // The actual scale, compensating for rounding & clamping.
    SkSize tileScale = SkSize::Make(SkIntToScalar(tileSize.width()) / fTile.width(),
                                    SkIntToScalar(tileSize.height()) / fTile.height());

    SkAutoMutexAcquire ama(fCachedBitmapShaderMutex);

    if (!fCachedBitmapShader || tileScale != fCachedTileScale) {
        SkBitmap bm;
        if (!bm.tryAllocN32Pixels(tileSize.width(), tileSize.height())) {
            return NULL;
        }
        bm.eraseColor(SK_ColorTRANSPARENT);

        SkCanvas canvas(bm);
        canvas.scale(tileScale.width(), tileScale.height());
        canvas.translate(fTile.x(), fTile.y());
        canvas.drawPicture(fPicture);

        fCachedTileScale = tileScale;

        SkMatrix shaderMatrix = this->getLocalMatrix();
        shaderMatrix.preScale(1 / tileScale.width(), 1 / tileScale.height());
        fCachedBitmapShader.reset(CreateBitmapShader(bm, fTmx, fTmy, &shaderMatrix));
    }

    // Increment the ref counter inside the mutex to ensure the returned pointer is still valid.
    // Otherwise, the pointer may have been overwritten on a different thread before the object's
    // ref count was incremented.
    fCachedBitmapShader.get()->ref();
    return fCachedBitmapShader;
}

size_t SkPictureShader::contextSize() const {
    return sizeof(PictureShaderContext);
}

SkShader::Context* SkPictureShader::onCreateContext(const ContextRec& rec, void* storage) const {
    SkAutoTUnref<SkShader> bitmapShader(this->refBitmapShader(*rec.fMatrix, rec.fLocalMatrix));
    if (NULL == bitmapShader.get()) {
        return NULL;
    }
    return PictureShaderContext::Create(storage, *this, rec, bitmapShader);
}

/////////////////////////////////////////////////////////////////////////////////////////

SkShader::Context* SkPictureShader::PictureShaderContext::Create(void* storage,
                   const SkPictureShader& shader, const ContextRec& rec, SkShader* bitmapShader) {
    PictureShaderContext* ctx = SkNEW_PLACEMENT_ARGS(storage, PictureShaderContext,
                                                     (shader, rec, bitmapShader));
    if (NULL == ctx->fBitmapShaderContext) {
        ctx->~PictureShaderContext();
        ctx = NULL;
    }
    return ctx;
}

SkPictureShader::PictureShaderContext::PictureShaderContext(
        const SkPictureShader& shader, const ContextRec& rec, SkShader* bitmapShader)
    : INHERITED(shader, rec)
    , fBitmapShader(SkRef(bitmapShader))
{
    fBitmapShaderContextStorage = sk_malloc_throw(bitmapShader->contextSize());
    fBitmapShaderContext = bitmapShader->createContext(rec, fBitmapShaderContextStorage);
    //if fBitmapShaderContext is null, we are invalid
}

SkPictureShader::PictureShaderContext::~PictureShaderContext() {
    if (fBitmapShaderContext) {
        fBitmapShaderContext->~Context();
    }
    sk_free(fBitmapShaderContextStorage);
}

uint32_t SkPictureShader::PictureShaderContext::getFlags() const {
    SkASSERT(fBitmapShaderContext);
    return fBitmapShaderContext->getFlags();
}

SkShader::Context::ShadeProc SkPictureShader::PictureShaderContext::asAShadeProc(void** ctx) {
    SkASSERT(fBitmapShaderContext);
    return fBitmapShaderContext->asAShadeProc(ctx);
}

void SkPictureShader::PictureShaderContext::shadeSpan(int x, int y, SkPMColor dstC[], int count) {
    SkASSERT(fBitmapShaderContext);
    fBitmapShaderContext->shadeSpan(x, y, dstC, count);
}

void SkPictureShader::PictureShaderContext::shadeSpan16(int x, int y, uint16_t dstC[], int count) {
    SkASSERT(fBitmapShaderContext);
    fBitmapShaderContext->shadeSpan16(x, y, dstC, count);
}

#ifndef SK_IGNORE_TO_STRING
void SkPictureShader::toString(SkString* str) const {
    static const char* gTileModeName[SkShader::kTileModeCount] = {
        "clamp", "repeat", "mirror"
    };

    str->appendf("PictureShader: [%f:%f:%f:%f] ",
                 fPicture ? fPicture->cullRect().fLeft : 0,
                 fPicture ? fPicture->cullRect().fTop : 0,
                 fPicture ? fPicture->cullRect().fRight : 0,
                 fPicture ? fPicture->cullRect().fBottom : 0);

    str->appendf("(%s, %s)", gTileModeName[fTmx], gTileModeName[fTmy]);

    this->INHERITED::toString(str);
}
#endif

#if SK_SUPPORT_GPU
bool SkPictureShader::asFragmentProcessor(GrContext* context, const SkPaint& paint,
                                          const SkMatrix* localMatrix, GrColor* paintColor,
                                          GrFragmentProcessor** fp) const {
    SkAutoTUnref<SkShader> bitmapShader(this->refBitmapShader(context->getMatrix(), localMatrix));
    if (!bitmapShader) {
        return false;
    }
    return bitmapShader->asFragmentProcessor(context, paint, NULL, paintColor, fp);
}
#else
bool SkPictureShader::asFragmentProcessor(GrContext*, const SkPaint&, const SkMatrix*, GrColor*,
                                          GrFragmentProcessor**) const {
    SkDEBUGFAIL("Should not call in GPU-less build");
    return false;
}
#endif
