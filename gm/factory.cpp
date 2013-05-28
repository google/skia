/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBitmapFactory.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkImageDecoder.h"
#include "SkLruImageCache.h"
#include "SkOSFile.h"
#include "SkStream.h"

namespace skiagm {

/**
 *  Draw a PNG created by SkBitmapFactory.
 */
class FactoryGM : public GM {
public:
    FactoryGM() {}

protected:
    virtual void onOnceBeforeDraw() SK_OVERRIDE {
        // Copyright-free file from http://openclipart.org/detail/29213/paper-plane-by-ddoo
        SkString filename = SkOSPath::SkPathJoin(INHERITED::gResourcePath.c_str(),
                                                 "plane.png");

        SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(filename.c_str()));
        if (NULL != stream.get()) {
            stream->rewind();
            size_t length = stream->getLength();
            void* buffer = sk_malloc_throw(length);
            stream->read(buffer, length);
            SkAutoDataUnref data(SkData::NewFromMalloc(buffer, length));
            SkBitmapFactory factory(&SkImageDecoder::DecodeMemoryToTarget);
            // Create a cache which will boot the pixels out anytime the
            // bitmap is unlocked.
            SkAutoTUnref<SkLruImageCache> cache(SkNEW_ARGS(SkLruImageCache, (1)));
            factory.setImageCache(cache);
            factory.installPixelRef(data, &fBitmap);
        }
    }

    virtual SkString onShortName() {
        return SkString("factory");
    }

    virtual SkISize onISize() {
        return make_isize(640, 480);
    }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawBitmap(fBitmap, 0, 0);
    }

private:
    SkBitmap fBitmap;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new FactoryGM; }
static GMRegistry reg(MyFactory);

}
