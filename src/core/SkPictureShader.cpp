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

SkPictureShader::SkPictureShader(SkPicture* picture, TileMode tmx, TileMode tmy)
    : fPicture(picture)
    , fTmx(tmx)
    , fTmy(tmy) {
    SkSafeRef(fPicture);
}

SkPictureShader::SkPictureShader(SkReadBuffer& buffer)
        : INHERITED(buffer) {
    fTmx = static_cast<SkShader::TileMode>(buffer.read32());
    fTmy = static_cast<SkShader::TileMode>(buffer.read32());
    if (buffer.readBool()) {
        fPicture = SkPicture::CreateFromBuffer(buffer);
    } else {
        fPicture = NULL;
    }
}

SkPictureShader::~SkPictureShader() {
    SkSafeUnref(fPicture);
}

SkPictureShader* SkPictureShader::Create(SkPicture* picture, TileMode tmx, TileMode tmy) {
    return SkNEW_ARGS(SkPictureShader, (picture, tmx, tmy));
}

void SkPictureShader::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.write32(fTmx);
    buffer.write32(fTmy);
    buffer.writeBool(NULL != fPicture);
    if (fPicture) {
        fPicture->flatten(buffer);
    }
}

SkShader* SkPictureShader::refBitmapShader(const SkMatrix& matrix) const {
    if (!fPicture || (0 == fPicture->width() && 0 == fPicture->height())) {
        return NULL;
    }

    SkMatrix m;
    if (this->hasLocalMatrix()) {
        m.setConcat(matrix, this->getLocalMatrix());
    } else {
        m = matrix;
    }

    // Use a rotation-invariant scale
    SkPoint scale;
    if (!SkDecomposeUpper2x2(m, NULL, &scale, NULL)) {
        // Decomposition failed, use an approximation.
        scale.set(SkScalarSqrt(m.getScaleX() * m.getScaleX() + m.getSkewX() * m.getSkewX()),
                  SkScalarSqrt(m.getScaleY() * m.getScaleY() + m.getSkewY() * m.getSkewY()));
    }
    SkSize scaledSize = SkSize::Make(scale.x() * fPicture->width(), scale.y() * fPicture->height());

    SkISize tileSize = scaledSize.toRound();
    if (tileSize.isEmpty()) {
        return NULL;
    }

    // The actual scale, compensating for rounding.
    SkSize tileScale = SkSize::Make(SkIntToScalar(tileSize.width()) / fPicture->width(),
                                    SkIntToScalar(tileSize.height()) / fPicture->height());

    SkAutoMutexAcquire ama(fCachedBitmapShaderMutex);

    if (!fCachedBitmapShader || tileScale != fCachedTileScale ||
        this->getLocalMatrix() != fCachedLocalMatrix) {
        SkBitmap bm;
        if (!bm.allocN32Pixels(tileSize.width(), tileSize.height())) {
            return NULL;
        }
        bm.eraseColor(SK_ColorTRANSPARENT);

        SkCanvas canvas(bm);
        canvas.scale(tileScale.width(), tileScale.height());
        canvas.drawPicture(*fPicture);

        fCachedBitmapShader.reset(CreateBitmapShader(bm, fTmx, fTmy));
        fCachedTileScale = tileScale;
        fCachedLocalMatrix = this->getLocalMatrix();

        SkMatrix shaderMatrix = this->getLocalMatrix();
        shaderMatrix.preScale(1 / tileScale.width(), 1 / tileScale.height());
        fCachedBitmapShader->setLocalMatrix(shaderMatrix);
    }

    // Increment the ref counter inside the mutex to ensure the returned pointer is still valid.
    // Otherwise, the pointer may have been overwritten on a different thread before the object's
    // ref count was incremented.
    fCachedBitmapShader.get()->ref();
    return fCachedBitmapShader;
}

SkShader* SkPictureShader::validInternal(const SkBitmap& device, const SkPaint& paint,
                                         const SkMatrix& matrix, SkMatrix* totalInverse) const {
    if (!this->INHERITED::validContext(device, paint, matrix, totalInverse)) {
        return NULL;
    }

    SkAutoTUnref<SkShader> bitmapShader(this->refBitmapShader(matrix));
    if (!bitmapShader || !bitmapShader->validContext(device, paint, matrix)) {
        return NULL;
    }

    return bitmapShader.detach();
}

bool SkPictureShader::validContext(const SkBitmap& device, const SkPaint& paint,
                                   const SkMatrix& matrix, SkMatrix* totalInverse) const {
    SkAutoTUnref<SkShader> shader(this->validInternal(device, paint, matrix, totalInverse));
    return shader != NULL;
}

SkShader::Context* SkPictureShader::createContext(const SkBitmap& device, const SkPaint& paint,
                                                  const SkMatrix& matrix, void* storage) const {
    SkAutoTUnref<SkShader> bitmapShader(this->validInternal(device, paint, matrix, NULL));
    if (!bitmapShader) {
        return NULL;
    }

    return SkNEW_PLACEMENT_ARGS(storage, PictureShaderContext,
                                (*this, device, paint, matrix, bitmapShader.detach()));
}

size_t SkPictureShader::contextSize() const {
    return sizeof(PictureShaderContext);
}

SkPictureShader::PictureShaderContext::PictureShaderContext(
        const SkPictureShader& shader, const SkBitmap& device,
        const SkPaint& paint, const SkMatrix& matrix, SkShader* bitmapShader)
    : INHERITED(shader, device, paint, matrix)
    , fBitmapShader(bitmapShader)
{
    SkASSERT(fBitmapShader);
    fBitmapShaderContextStorage = sk_malloc_throw(fBitmapShader->contextSize());
    fBitmapShaderContext = fBitmapShader->createContext(
            device, paint, matrix, fBitmapShaderContextStorage);
    SkASSERT(fBitmapShaderContext);
}

SkPictureShader::PictureShaderContext::~PictureShaderContext() {
    fBitmapShaderContext->SkShader::Context::~Context();
    sk_free(fBitmapShaderContextStorage);
}

uint32_t SkPictureShader::PictureShaderContext::getFlags() const {
    return fBitmapShaderContext->getFlags();
}

SkShader::Context::ShadeProc SkPictureShader::PictureShaderContext::asAShadeProc(void** ctx) {
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

    str->appendf("PictureShader: [%d:%d] ",
                 fPicture ? fPicture->width() : 0,
                 fPicture ? fPicture->height() : 0);

    str->appendf("(%s, %s)", gTileModeName[fTmx], gTileModeName[fTmy]);

    this->INHERITED::toString(str);
}
#endif

#if SK_SUPPORT_GPU
GrEffectRef* SkPictureShader::asNewEffect(GrContext* context, const SkPaint& paint) const {
    SkAutoTUnref<SkShader> bitmapShader(this->refBitmapShader(context->getMatrix()));
    if (!bitmapShader) {
        return NULL;
    }
    return bitmapShader->asNewEffect(context, paint);
}
#endif
