/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkDecodingImageGenerator.h"
#include "SkDiscardableMemoryPool.h"
#include "SkDiscardablePixelRef.h"
#include "SkImageDecoder.h"
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
        SkAutoDataUnref data(SkData::NewFromFileName(filename.c_str()));
        if (NULL != data.get()) {
            // Create a cache which will boot the pixels out anytime the
            // bitmap is unlocked.
            SkAutoTUnref<SkDiscardableMemoryPool> pool(
                SkNEW_ARGS(SkDiscardableMemoryPool, (1)));
            SkAssertResult(SkDecodingImageGenerator::Install(data,
                                                             &fBitmap, pool));
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

    // Skip cross process pipe due to https://code.google.com/p/skia/issues/detail?id=1520
    virtual uint32_t onGetFlags() const {
        return INHERITED::onGetFlags() | kSkipPipeCrossProcess_Flag;
    }

private:
    SkBitmap fBitmap;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new FactoryGM; }
static GMRegistry reg(MyFactory);

}
