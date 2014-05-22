/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"

namespace skiagm {

/**
 *  Test decoding an image from a PKM file and then
 *  from compressed ETC1 data.
 */
class ETC1BitmapGM : public GM {
public:
    ETC1BitmapGM() { }
    virtual ~ETC1BitmapGM() { }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("etc1bitmap");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return make_isize(512, 512);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {

        SkBitmap bm;
        SkString filename = SkOSPath::SkPathJoin(
                INHERITED::gResourcePath.c_str(), "mandrill_512.pkm");
        if (!SkImageDecoder::DecodeFile(filename.c_str(), &bm,
                                        SkBitmap::kARGB_8888_Config,
                                        SkImageDecoder::kDecodePixels_Mode)) {
            SkDebugf("Could not decode the file. Did you forget to set the "
                     "resourcePath?\n");
            return;
        }
        canvas->drawBitmap(bm, 0, 0);
    }

private:
    typedef GM INHERITED;
};

}  // namespace skiagm

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(skiagm::ETC1BitmapGM); )
