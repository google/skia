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
        SkString filename(INHERITED::gResourcePath);
        if (!filename.endsWith("/") && !filename.endsWith("\\")) {
            filename.append("/");
        }

        // Copyright-free file from http://openclipart.org/detail/29213/paper-plane-by-ddoo
        filename.append("plane.png");

        SkFILEStream stream(filename.c_str());
        if (stream.isValid()) {
            stream.rewind();
            size_t length = stream.getLength();
            void* buffer = sk_malloc_throw(length);
            stream.read(buffer, length);
            SkAutoDataUnref data(SkData::NewFromMalloc(buffer, length));
            SkBitmapFactory::DecodeBitmap(&fBitmap, data);
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
