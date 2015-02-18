/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkImageGenerator.h"
#include "SkImagePriv.h"
#include "SkImage_Base.h"
#include "SkReadPixelsRec.h"
#include "SkString.h"
#include "SkSurface.h"

uint32_t SkImage::NextUniqueID() {
    static int32_t gUniqueID;

    // never return 0;
    uint32_t id;
    do {
        id = sk_atomic_inc(&gUniqueID) + 1;
    } while (0 == id);
    return id;
}

void SkImage::draw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) const {
    as_IB(this)->onDraw(canvas, x, y, paint);
}

void SkImage::drawRect(SkCanvas* canvas, const SkRect* src, const SkRect& dst,
                   const SkPaint* paint) const {
    as_IB(this)->onDrawRect(canvas, src, dst, paint);
}

const void* SkImage::peekPixels(SkImageInfo* info, size_t* rowBytes) const {
    SkImageInfo infoStorage;
    size_t rowBytesStorage;
    if (NULL == info) {
        info = &infoStorage;
    }
    if (NULL == rowBytes) {
        rowBytes = &rowBytesStorage;
    }
    return as_IB(this)->onPeekPixels(info, rowBytes);
}

bool SkImage::readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                           int srcX, int srcY) const {
    SkReadPixelsRec rec(dstInfo, dstPixels, dstRowBytes, srcX, srcY);
    if (!rec.trim(this->width(), this->height())) {
        return false;
    }
    return as_IB(this)->onReadPixels(rec.fInfo, rec.fPixels, rec.fRowBytes, rec.fX, rec.fY);
}

GrTexture* SkImage::getTexture() const {
    return as_IB(this)->onGetTexture();
}

SkShader* SkImage::newShader(SkShader::TileMode tileX,
                             SkShader::TileMode tileY,
                             const SkMatrix* localMatrix) const {
    return as_IB(this)->onNewShader(tileX, tileY, localMatrix);
}

SkData* SkImage::encode(SkImageEncoder::Type type, int quality) const {
    SkBitmap bm;
    if (as_IB(this)->getROPixels(&bm)) {
        return SkImageEncoder::EncodeData(bm, type, quality);
    }
    return NULL;
}

SkImage* SkImage::NewFromData(SkData* data) {
    if (NULL == data) {
        return NULL;
    }
    SkImageGenerator* generator = SkImageGenerator::NewFromData(data);
    return generator ? SkImage::NewFromGenerator(generator) : NULL;
}

SkSurface* SkImage::newSurface(const SkImageInfo& info, const SkSurfaceProps* props) const {
    if (NULL == props) {
        props = &as_IB(this)->props();
    }
    return as_IB(this)->onNewSurface(info, *props);
}

const char* SkImage::toString(SkString* str) const {
    str->appendf("image: (id:%d (%d, %d) %s)", this->uniqueID(), this->width(), this->height(),
                 this->isOpaque() ? "opaque" : "");
    return str->c_str();
}

SkImage* SkImage::newImage(int newWidth, int newHeight, const SkIRect* subset,
                           SkFilterQuality quality) const {
    if (newWidth <= 0 || newHeight <= 0) {
        return NULL;
    }

    const SkIRect bounds = SkIRect::MakeWH(this->width(), this->height());

    if (subset) {
        if (!bounds.contains(*subset)) {
            return NULL;
        }
        if (bounds == *subset) {
            subset = NULL;  // and fall through to check below
        }
    }

    if (NULL == subset && this->width() == newWidth && this->height() == newHeight) {
        return SkRef(const_cast<SkImage*>(this));
    }

    return as_IB(this)->onNewImage(newWidth, newHeight, subset, quality);
}

///////////////////////////////////////////////////////////////////////////////

static bool raster_canvas_supports(const SkImageInfo& info) {
    switch (info.colorType()) {
        case kN32_SkColorType:
            return kUnpremul_SkAlphaType != info.alphaType();
        case kRGB_565_SkColorType:
            return true;
        case kAlpha_8_SkColorType:
            return true;
        default:
            break;
    }
    return false;
}

bool SkImage_Base::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                                int srcX, int srcY) const {
    if (!raster_canvas_supports(dstInfo)) {
        return false;
    }

    SkBitmap bm;
    bm.installPixels(dstInfo, dstPixels, dstRowBytes);
    SkCanvas canvas(bm);

    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    canvas.drawImage(this, -SkIntToScalar(srcX), -SkIntToScalar(srcY), &paint);

    return true;
}

SkImage* SkImage_Base::onNewImage(int newWidth, int newHeight, const SkIRect* subset,
                                  SkFilterQuality quality) const {
    const bool opaque = this->isOpaque();
    const SkImageInfo info = SkImageInfo::Make(newWidth, newHeight, kN32_SkColorType,
                                               opaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(this->newSurface(info, NULL));
    if (!surface.get()) {
        return NULL;
    }

    SkRect src;
    if (subset) {
        src.set(*subset);
    } else {
        src = SkRect::MakeIWH(this->width(), this->height());
    }

    surface->getCanvas()->scale(newWidth / src.width(), newHeight / src.height());
    surface->getCanvas()->translate(-src.x(), -src.y());

    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    paint.setFilterQuality(quality);
    surface->getCanvas()->drawImage(this, 0, 0, &paint);
    return surface->newImageSnapshot();
}


