/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
#include "SkImagePriv.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "../images/SkImageEncoder.h"

SK_DEFINE_INST_COUNT(SkImage)

static SkImage_Base* asIB(SkImage* image) {
    return static_cast<SkImage_Base*>(image);
}

static const SkImage_Base* asIB(const SkImage* image) {
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
    asIB(this)->onDraw(canvas, x, y, paint);
}

GrTexture* SkImage::getTexture() {
    return asIB(this)->onGetTexture();
}

static const struct {
    SkImageEncoder::Type    fIE;
    SkImage::EncodeType     fET;
} gTable[] = {
    { SkImageEncoder::kBMP_Type,    SkImage::kBMP_EncodeType  },
    { SkImageEncoder::kGIF_Type,    SkImage::kGIF_EncodeType  },
    { SkImageEncoder::kICO_Type,    SkImage::kICO_EncodeType  },
    { SkImageEncoder::kJPEG_Type,   SkImage::kJPEG_EncodeType },
    { SkImageEncoder::kPNG_Type,    SkImage::kPNG_EncodeType  },
    { SkImageEncoder::kWBMP_Type,   SkImage::kWBMP_EncodeType },
    { SkImageEncoder::kWEBP_Type,   SkImage::kWEBP_EncodeType },
};

SkData* SkImage::encode(EncodeType et, int quality) const {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gTable); ++i) {
        if (gTable[i].fET == et) {
            SkBitmap bm;
            if (asIB(this)->getROPixels(&bm)) {
                return SkImageEncoder::EncodeData(bm, gTable[i].fIE, quality);
            }
            break;
        }
    }
    return NULL;
}
