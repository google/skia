/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "tools/Resources.h"

namespace skiagm {

class EncodeGM : public GM {
public:
    EncodeGM() {}

protected:
    SkString getName() const override { return SkString("encode"); }

    SkISize getISize() override { return SkISize::Make(1024, 600); }

    void onDraw(SkCanvas* canvas) override {
        SkBitmap orig;
        GetResourceAsBitmap("images/mandrill_512_q075.jpg", &orig);
        SkDynamicMemoryWStream stream;
        SkASSERT_RELEASE(SkPngEncoder::Encode(&stream, orig.pixmap(), {}));
        sk_sp<SkData> pngData = stream.detachAsData();
        stream.reset();

        SkASSERT_RELEASE(SkJpegEncoder::Encode(&stream, orig.pixmap(), {}));
        sk_sp<SkData> jpgData = stream.detachAsData();

        sk_sp<SkImage> pngImage = SkImages::DeferredFromEncodedData(pngData);
        sk_sp<SkImage> jpgImage = SkImages::DeferredFromEncodedData(jpgData);
        canvas->drawImage(pngImage.get(), 0.0f, 0.0f);
        canvas->drawImage(jpgImage.get(), 512.0f, 0.0f);

        SkFont font;
        font.setEdging(SkFont::Edging::kAlias);
        canvas->drawString("Images should look identical.", 450.0f, 550.0f, font, SkPaint());
    }

private:
    using INHERITED = GM;
};

DEF_GM( return new EncodeGM; )
}  // namespace skiagm

///////////

#if 0
DEF_SIMPLE_GM(jpeg_orientation, canvas, 1000, 1000) {
    static sk_sp<SkImage> imgs[8];
    if (!imgs[0]) {
        for (int i = 0; i < 8; ++i) {
            SkString path;
            path.printf("/skia/orientation/Landscape_%d.jpg", i + 1);
            auto stream = SkStream::MakeFromFile(path.c_str());
            auto data = SkData::MakeFromStream(stream.get(), stream->getLength());
            imgs[i] = SkImages::DeferredFromEncodedData(data, nullptr);
        }
    }
    canvas->scale(0.25, 0.25);
    for (int i = 0; i < 8; ++i) {
        SkImage* img = imgs[i].get();
        canvas->drawImage(img, 0, 0, nullptr);
        canvas->translate(0, img->height());
        if (i == 3) {
            canvas->translate(img->width(), -4*img->height());
        }
    }
}
#endif
