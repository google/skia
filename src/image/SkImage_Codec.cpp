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
