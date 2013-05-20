/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "../images/SkImageDecoder.h"

class SkImage_Codec : public SkImage_Base {
public:
    static SkImage* NewEmpty();

    SkImage_Codec(SkData* encodedData, int width, int height);
    virtual ~SkImage_Codec();

    virtual void onDraw(SkCanvas*, SkScalar, SkScalar, const SkPaint*) SK_OVERRIDE;

private:
    SkData*     fEncodedData;
    SkBitmap    fBitmap;

    typedef SkImage_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkImage_Codec::SkImage_Codec(SkData* data, int width, int height) : INHERITED(width, height) {
    fEncodedData = data;
    fEncodedData->ref();
}

SkImage_Codec::~SkImage_Codec() {
    fEncodedData->unref();
}

void SkImage_Codec::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) {
    if (!fBitmap.pixelRef()) {
        if (!SkImageDecoder::DecodeMemory(fEncodedData->bytes(), fEncodedData->size(),
                                          &fBitmap)) {
            return;
        }
    }
    canvas->drawBitmap(fBitmap, x, y, paint);
}

///////////////////////////////////////////////////////////////////////////////

SkImage* SkImage::NewEncodedData(SkData* data) {
    if (NULL == data) {
        return NULL;
    }

    SkBitmap bitmap;
    if (!SkImageDecoder::DecodeMemory(data->bytes(), data->size(), &bitmap,
                                      SkBitmap::kNo_Config,
                                      SkImageDecoder::kDecodeBounds_Mode)) {
        return NULL;
    }

    return SkNEW_ARGS(SkImage_Codec, (data, bitmap.width(), bitmap.height()));
}

///////////////////////////////////////////////////////////////////////////////

// FIXME: Temporarily move this here so chromium can still build until we truly
// fix the core/images dependency issue (https://code.google.com/p/skia/issues/detail?id=1275)
#include "SkImage.h"
#include "../images/SkImageEncoder.h"

static const SkImage_Base* asIB(const SkImage* image) {
    return static_cast<const SkImage_Base*>(image);
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
