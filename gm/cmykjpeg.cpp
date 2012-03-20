/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkImageDecoder.h"
#include "SkStream.h"

namespace skiagm {

/** Draw a CMYK encoded jpeg - libjpeg doesn't support CMYK->RGB
    conversion so this tests Skia's internal processing
*/
class CMYKJpegGM : public GM {
public:
    CMYKJpegGM() {

        // parameters to the "decode" call
        bool dither = false;
        SkBitmap::Config prefConfig = SkBitmap::kARGB_8888_Config;

        SkString filename(INHERITED::gResourcePath);
        if (!filename.endsWith("/") && !filename.endsWith("\\")) {
            filename.append("/");
        }

        filename.append("CMYK.jpg");

        SkFILEStream stream(filename.c_str());
        SkImageDecoder* codec = SkImageDecoder::Factory(&stream);
        if (codec) {
            stream.rewind();
            codec->setDitherImage(dither);
            codec->decode(&stream, &fBitmap, prefConfig,
                          SkImageDecoder::kDecodePixels_Mode);
        }
    }

protected:
    virtual SkString onShortName() {
        return SkString("cmykjpeg");
    }

    virtual SkISize onISize() {
        return make_isize(640, 480);
    }

    virtual void onDraw(SkCanvas* canvas) {

        canvas->translate(20*SK_Scalar1, 20*SK_Scalar1);
        canvas->drawBitmap(fBitmap, 0, 0);
    }

private:
    SkBitmap fBitmap;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new CMYKJpegGM; }
static GMRegistry reg(MyFactory);

}
