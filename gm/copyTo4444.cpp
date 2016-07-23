/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SkCanvas.h"
#include "SkOSFile.h"

namespace skiagm {

/**
 *  Test copying an image from 8888 to 4444.
 */
class CopyTo4444GM : public GM {
public:
    CopyTo4444GM() {}

protected:
    virtual SkString onShortName() {
        return SkString("copyTo4444");
    }

    virtual SkISize onISize() {
        return SkISize::Make(1024, 512);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkBitmap bm, bm4444;
        if (!GetResourceAsBitmap("mandrill_512.png", &bm)) {
            SkDebugf("Could not decode the file. Did you forget to set the "
                     "resourcePath?\n");
            return;
        }
        canvas->drawBitmap(bm, 0, 0);
        SkAssertResult(bm.copyTo(&bm4444, kARGB_4444_SkColorType));
        canvas->drawBitmap(bm4444, SkIntToScalar(bm.width()), 0);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new CopyTo4444GM; }
static GMRegistry reg(MyFactory);

}
