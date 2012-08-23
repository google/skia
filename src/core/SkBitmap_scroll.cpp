
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBitmap.h"
#include "SkRegion.h"

bool SkBitmap::scrollRect(const SkIRect* subset, int dx, int dy,
                          SkRegion* inval) const
{
    if (this->isImmutable()) {
        return false;
    }

    if (NULL != subset) {
        SkBitmap tmp;

        return  this->extractSubset(&tmp, *subset) &&
                // now call again with no rectangle
                tmp.scrollRect(NULL, dx, dy, inval);
    }

    int shift;

    switch (this->config()) {
    case kIndex8_Config:
    case kA8_Config:
        shift = 0;
        break;
    case kARGB_4444_Config:
    case kRGB_565_Config:
        shift = 1;
        break;
    case kARGB_8888_Config:
        shift = 2;
        break;
    default:
        // can't scroll this config
        return false;
    }

    int width = this->width();
    int height = this->height();

    // check if there's nothing to do
    if ((dx | dy) == 0 || width <= 0 || height <= 0) {
        if (NULL != inval) {
            inval->setEmpty();
        }
        return true;
    }

    // compute the inval region now, before we see if there are any pixels
    if (NULL != inval) {
        SkIRect r;

        r.set(0, 0, width, height);
        // initial the region with the entire bounds
        inval->setRect(r);
        // do the "scroll"
        r.offset(dx, dy);

        // check if we scrolled completely away
        if (!SkIRect::Intersects(r, inval->getBounds())) {
            // inval has already been updated...
            return true;
        }

        // compute the dirty area
        inval->op(r, SkRegion::kDifference_Op);
    }

    SkAutoLockPixels    alp(*this);
    // if we have no pixels, just return (inval is already updated)
    // don't call readyToDraw(), since we don't require a colortable per se
    if (this->getPixels() == NULL) {
        return true;
    }

    char*       dst = (char*)this->getPixels();
    const char* src = dst;
    int         rowBytes = this->rowBytes();    // need rowBytes to be signed

    if (dy <= 0) {
        src -= dy * rowBytes;
        height += dy;
    } else {
        dst += dy * rowBytes;
        height -= dy;
        // now jump src/dst to the last scanline
        src += (height - 1) * rowBytes;
        dst += (height - 1) * rowBytes;
        // now invert rowbytes so we copy backwards in the loop
        rowBytes = -rowBytes;
    }

    if (dx <= 0) {
        src -= dx << shift;
        width += dx;
    } else {
        dst += dx << shift;
        width -= dx;
    }

    // If the X-translation would push it completely beyond the region,
    // then there's nothing to draw.
    if (width <= 0) {
        return true;
    }

    width <<= shift;    // now width is the number of bytes to move per line
    while (--height >= 0) {
        memmove(dst, src, width);
        dst += rowBytes;
        src += rowBytes;
    }

    this->notifyPixelsChanged();
    return true;
}
