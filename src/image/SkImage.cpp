/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkImagePriv.h"
#include "SkImage_Base.h"

static SkImage_Base* as_IB(SkImage* image) {
    return static_cast<SkImage_Base*>(image);
}

static const SkImage_Base* as_IB(const SkImage* image) {
    return static_cast<const SkImage_Base*>(image);
}

uint32_t SkImage::NextUniqueID() {
    static int32_t gUniqueID;

    // never return 0;
    uint32_t id;
    do {
        id = sk_atomic_inc(&gUniqueID) + 1;
    } while (0 == id);
    return id;
}

void SkImage::draw(SkCanvas* canvas, SkScalar x, SkScalar y,
                   const SkPaint* paint) {
    as_IB(this)->onDraw(canvas, x, y, paint);
}

void SkImage::draw(SkCanvas* canvas, const SkRect* src, const SkRect& dst,
                   const SkPaint* paint) {
    as_IB(this)->onDrawRectToRect(canvas, src, dst, paint);
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

bool SkImage::readPixels(SkBitmap* bitmap, const SkIRect* subset) const {
    if (NULL == bitmap) {
        return false;
    }

    SkIRect bounds = SkIRect::MakeWH(this->width(), this->height());

    // trim against the bitmap, if its already been allocated
    if (bitmap->pixelRef()) {
        bounds.fRight = SkMin32(bounds.fRight, bitmap->width());
        bounds.fBottom = SkMin32(bounds.fBottom, bitmap->height());
        if (bounds.isEmpty()) {
            return false;
        }
    }

    if (subset && !bounds.intersect(*subset)) {
        // perhaps we could return true + empty-bitmap?
        return false;
    }
    return as_IB(this)->onReadPixels(bitmap, bounds);
}

GrTexture* SkImage::getTexture() {
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

///////////////////////////////////////////////////////////////////////////////

static bool raster_canvas_supports(const SkImageInfo& info) {
    switch (info.fColorType) {
        case kN32_SkColorType:
            return kUnpremul_SkAlphaType != info.fAlphaType;
        case kRGB_565_SkColorType:
            return true;
        case kAlpha_8_SkColorType:
            return true;
        default:
            break;
    }
    return false;
}

bool SkImage_Base::onReadPixels(SkBitmap* bitmap, const SkIRect& subset) const {
    if (bitmap->pixelRef()) {
        const SkImageInfo info = bitmap->info();
        if (kUnknown_SkColorType == info.colorType()) {
            return false;
        }
        if (!raster_canvas_supports(info)) {
            return false;
        }
    } else {
        const SkImageInfo info = SkImageInfo::MakeN32Premul(subset.width(), subset.height());
        SkBitmap tmp;
        if (!tmp.allocPixels(info)) {
            return false;
        }
        *bitmap = tmp;
    }

    SkRect srcR, dstR;
    srcR.set(subset);
    dstR = srcR;
    dstR.offset(-dstR.left(), -dstR.top());

    SkCanvas canvas(*bitmap);

    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kClear_Mode);
    canvas.drawRect(dstR, paint);

    const_cast<SkImage_Base*>(this)->onDrawRectToRect(&canvas, &srcR, dstR, NULL);
    return true;
}
