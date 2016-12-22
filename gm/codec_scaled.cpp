/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkCodec.h"
#include "SkCodecImageGenerator.h"
#include "SkColor.h"
#include "SkCommandLineFlags.h"
#include "SkImageGenerator.h"
#include "SkString.h"
#include "Resources.h"

DEFINE_string(codec_scaled, "brickwork-texture.jpg", "Image in resources/ to draw scaled.");

class CodecScaledGM : public skiagm::GM {
private:
    // FIXME: Once generateScaledPixels is plumbed to SkImage, store an SkImage
    // and call SkImage::scalePixels.
    std::unique_ptr<SkImageGenerator>   fGenerator;

public:
    CodecScaledGM()
    {}

private:
    SkString onShortName() override {
        return SkString("codec_scaled");
    }

    SkISize onISize() override {
        if (this->initCodec()) {
            SkISize dim = fGenerator->getInfo().dimensions();
            // Wide enough to show 8 versions, corresponding to the options JPEG supports.
            dim.fWidth *= 8;
            // Tall enough to display 2 versions - one using computed dimensions, and one
            // with scaling.
            dim.fHeight *= 2;
            return dim;
        }
        return SkISize::Make(640, 480);
    }

    void onDrawBackground(SkCanvas* canvas) override {
        canvas->clear(SK_ColorWHITE);
    }

    bool initCodec() {
        if (fGenerator) {
            return true;
        }

        if (FLAGS_codec_scaled.isEmpty()) {
            SkDebugf("Nothing specified for --codec_scaled!");
            return false;
        }

        SkString path = GetResourcePath(FLAGS_codec_scaled[0]);
        sk_sp<SkData> data(SkData::MakeFromFileName(path.c_str()));
        if (!data) {
            return false;
        }

        fGenerator.reset(SkCodecImageGenerator::NewFromEncodedCodec(data));
        if (!fGenerator) {
            SkDebugf("Could create codec from %s", FLAGS_codec_scaled[0]);
            return false;
        }

        return true;
    }

    void onDraw(SkCanvas* canvas) override {
        if (!this->initCodec()) {
            return;
        }

        SkAutoCanvasRestore acr(canvas, true);
        for (float scale : { 1.0f, .875f, .750f, .625f, .5f, .375f, .25f, .125f }) {
            // generateScaledPixels does not support index8
            const auto info = fGenerator->getInfo().makeColorType(kN32_SkColorType);
            auto scaledInfo = info;
            SkImageGenerator::SupportedSizes sizes;
            if (fGenerator->computeScaledDimensions(scale, &sizes)) {
                scaledInfo = info.makeWH(sizes.fSizes[0].fWidth, sizes.fSizes[0].fHeight);
            }

            SkBitmap bm;
            bm.allocPixels(scaledInfo);
            SkPixmap pixmap(scaledInfo, bm.getPixels(), bm.rowBytes());
            if (fGenerator->generateScaledPixels(pixmap)) {
                canvas->drawBitmap(bm, 0, 0);
            }

            bm.allocPixels(info);
            if (fGenerator->getPixels(info, bm.getPixels(), bm.rowBytes())) {
                SkAutoCanvasRestore acr2(canvas, true);
                canvas->translate(0, SkIntToScalar(info.height()));
                canvas->scale(SkFloatToScalar(scale), SkFloatToScalar(scale));
                canvas->drawBitmap(bm, 0, 0);
            }

            canvas->translate(SkIntToScalar(info.width()), 0);
        }
    }
};

DEF_GM(return new CodecScaledGM);
