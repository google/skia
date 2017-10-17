/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "sk_tool_utils.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkImageEncoder.h"
#include "Resources.h"

namespace skiagm {

class EncodeGM : public GM {
public:
    EncodeGM() {}

protected:
    SkString onShortName() override {
        return SkString("encode");
    }

    SkISize onISize() override {
        return SkISize::Make(1024, 600);
    }

    void onDraw(SkCanvas* canvas) override {
        SkBitmap orig;
        GetResourceAsBitmap("mandrill_512_q075.jpg", &orig);
        sk_sp<SkData> pngData(sk_tool_utils::EncodeImageToData(orig, SkEncodedImageFormat::kPNG, 100));
        sk_sp<SkData> jpegData(sk_tool_utils::EncodeImageToData(orig, SkEncodedImageFormat::kJPEG, 100));

        sk_sp<SkImage> pngImage = SkImage::MakeFromEncoded(pngData);
        sk_sp<SkImage> jpegImage = SkImage::MakeFromEncoded(jpegData);
        canvas->drawImage(pngImage.get(), 0.0f, 0.0f);
        canvas->drawImage(jpegImage.get(), 512.0f, 0.0f);

        const char text[] = "Images should look identical.";
        canvas->drawText(text, sizeof(text) - 1, 450.0f, 550.0f, SkPaint());
    }

private:
    typedef GM INHERITED;
};

DEF_GM( return new EncodeGM; )
}
