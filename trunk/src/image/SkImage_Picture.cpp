/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
#include "SkImagePriv.h"
#include "SkPicture.h"

class SkImage_Picture : public SkImage_Base {
public:
    SkImage_Picture(SkPicture*);
    virtual ~SkImage_Picture();

    virtual void onDraw(SkCanvas*, SkScalar, SkScalar, const SkPaint*) SK_OVERRIDE;

private:
    SkPicture*  fPicture;

    typedef SkImage_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkImage_Picture::SkImage_Picture(SkPicture* pict) : INHERITED(pict->width(), pict->height()) {
    pict->endRecording();
    pict->ref();
    fPicture = pict;
}

SkImage_Picture::~SkImage_Picture() {
    fPicture->unref();
}

void SkImage_Picture::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                             const SkPaint* paint) {
    SkImagePrivDrawPicture(canvas, fPicture, x, y, paint);
}

SkImage* SkNewImageFromPicture(const SkPicture* srcPicture) {
    /**
     *  We want to snapshot the playback status of the picture, w/o affecting
     *  its ability to continue recording (if needed).
     *
     *  Optimally this will shared as much data/buffers as it can with
     *  srcPicture, and srcPicture will perform a copy-on-write as needed if it
     *  needs to mutate them later on.
     */
    SkAutoTUnref<SkPicture> playback(SkNEW_ARGS(SkPicture, (*srcPicture)));

    return SkNEW_ARGS(SkImage_Picture, (playback));
}

