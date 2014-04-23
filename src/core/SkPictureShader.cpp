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
    : fPicture(SkRef(picture))
    , fTmx(tmx)
    , fTmy(tmy) { }

SkPictureShader::SkPictureShader(SkReadBuffer& buffer)
        : INHERITED(buffer) {
    fTmx = static_cast<SkShader::TileMode>(buffer.read32());
    fTmy = static_cast<SkShader::TileMode>(buffer.read32());
    fPicture = SkPicture::CreateFromBuffer(buffer);
}

SkPictureShader::~SkPictureShader() {
    fPicture->unref();
}

SkPictureShader* SkPictureShader::Create(SkPicture* picture, TileMode tmx, TileMode tmy) {
    if (!picture || 0 == picture->width() || 0 == picture->height()) {
        return NULL;
    }
    return SkNEW_ARGS(SkPictureShader, (picture, tmx, tmy));
}

void SkPictureShader::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.write32(fTmx);
    buffer.write32(fTmy);
    fPicture->flatten(buffer);
}

bool SkPictureShader::buildBitmapShader(const SkMatrix& matrix) const {
    SkASSERT(fPicture && fPicture->width() > 0 && fPicture->height() > 0);

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
        return false;
    }

    // The actual scale, compensating for rounding.
    SkSize tileScale = SkSize::Make(SkIntToScalar(tileSize.width()) / fPicture->width(),
                                    SkIntToScalar(tileSize.height()) / fPicture->height());

    if (!fCachedShader || tileScale != fCachedTileScale) {
        SkBitmap bm;
        if (!bm.allocN32Pixels(tileSize.width(), tileSize.height())) {
            return false;
        }
        bm.eraseColor(SK_ColorTRANSPARENT);

        SkCanvas canvas(bm);
        canvas.scale(tileScale.width(), tileScale.height());
        canvas.drawPicture(*fPicture);

        fCachedShader.reset(CreateBitmapShader(bm, fTmx, fTmy));
        fCachedTileScale = tileScale;
    }

    SkMatrix shaderMatrix = this->getLocalMatrix();
    shaderMatrix.preScale(1 / tileScale.width(), 1 / tileScale.height());
    fCachedShader->setLocalMatrix(shaderMatrix);

    return true;
}

bool SkPictureShader::setContext(const SkBitmap& device,
                                 const SkPaint& paint,
                                 const SkMatrix& matrix) {
    if (!this->buildBitmapShader(matrix)) {
        return false;
    }

    if (!this->INHERITED::setContext(device, paint, matrix)) {
        return false;
    }

    SkASSERT(fCachedShader);
    if (!fCachedShader->setContext(device, paint, matrix)) {
        this->INHERITED::endContext();
        return false;
    }

    return true;
}

void SkPictureShader::endContext() {
    SkASSERT(fCachedShader);
    fCachedShader->endContext();

    this->INHERITED::endContext();
}

uint32_t SkPictureShader::getFlags() {
    if (NULL != fCachedShader) {
        return fCachedShader->getFlags();
    }
    return 0;
}

SkShader::ShadeProc SkPictureShader::asAShadeProc(void** ctx) {
    if (fCachedShader) {
        return fCachedShader->asAShadeProc(ctx);
    }
    return NULL;
}

void SkPictureShader::shadeSpan(int x, int y, SkPMColor dstC[], int count) {
    SkASSERT(fCachedShader);
    fCachedShader->shadeSpan(x, y, dstC, count);
}

void SkPictureShader::shadeSpan16(int x, int y, uint16_t dstC[], int count) {
    SkASSERT(fCachedShader);
    fCachedShader->shadeSpan16(x, y, dstC, count);
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
    if (!this->buildBitmapShader(context->getMatrix())) {
        return NULL;
    }
    SkASSERT(fCachedShader);
    return fCachedShader->asNewEffect(context, paint);
}
#endif
