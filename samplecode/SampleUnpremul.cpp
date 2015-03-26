/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "sk_tool_utils.h"
#include "Resources.h"
#include "SampleCode.h"
#include "SkBlurMask.h"
#include "SkBlurDrawLooper.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkForceLinking.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkSystemEventTypes.h"
#include "SkTypes.h"
#include "SkUtils.h"
#include "SkView.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

/**
 *  Interprets c as an unpremultiplied color, and returns the
 *  premultiplied equivalent.
 */
static SkPMColor premultiply_unpmcolor(SkPMColor c) {
    U8CPU a = SkGetPackedA32(c);
    U8CPU r = SkGetPackedR32(c);
    U8CPU g = SkGetPackedG32(c);
    U8CPU b = SkGetPackedB32(c);
    return SkPreMultiplyARGB(a, r, g, b);
}

class UnpremulView : public SampleView {
public:
    UnpremulView(SkString res)
    : fResPath(res)
    , fPremul(true)
    , fDecodeSucceeded(false) {
        this->nextImage();
    }

protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "unpremul");
            return true;
        }
        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
            char utf8[kMaxBytesInUTF8Sequence];
            size_t size = SkUTF8_FromUnichar(uni, utf8);
            // Only consider events for single char keys
            if (1 == size) {
                switch (utf8[0]) {
                    case fNextImageChar:
                        this->nextImage();
                        return true;
                    case fTogglePremulChar:
                        this->togglePremul();
                        return true;
                    default:
                        break;
                }
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawBackground(SkCanvas* canvas) override {
        sk_tool_utils::draw_checkerboard(canvas, 0xFFCCCCCC, 0xFFFFFFFF, 12);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(24));
        SkAutoTUnref<SkBlurDrawLooper> looper(
            SkBlurDrawLooper::Create(SK_ColorBLUE,
                                     SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(2)),
                                     0, 0));
        paint.setLooper(looper);
        SkScalar height = paint.getFontMetrics(NULL);
        if (!fDecodeSucceeded) {
            SkString failure;
            if (fResPath.size() == 0) {
                failure.printf("resource path is required!");
            } else {
                failure.printf("Failed to decode %s", fCurrFile.c_str());
            }
            canvas->drawText(failure.c_str(), failure.size(), 0, height, paint);
            return;
        }

        // Name, size of the file, and whether or not it is premultiplied.
        SkString header(SkOSPath::Basename(fCurrFile.c_str()));
        header.appendf("     [%dx%d]     %s", fBitmap.width(), fBitmap.height(),
                       (fPremul ? "premultiplied" : "unpremultiplied"));
        canvas->drawText(header.c_str(), header.size(), 0, height, paint);
        canvas->translate(0, height);

        // Help messages
        header.printf("Press '%c' to move to the next image.'", fNextImageChar);
        canvas->drawText(header.c_str(), header.size(), 0, height, paint);
        canvas->translate(0, height);

        header.printf("Press '%c' to toggle premultiplied decode.", fTogglePremulChar);
        canvas->drawText(header.c_str(), header.size(), 0, height, paint);

        // Now draw the image itself.
        canvas->translate(height * 2, height * 2);
        if (!fPremul) {
            // A premultiplied bitmap cannot currently be drawn.
            SkAutoLockPixels alp(fBitmap);
            // Copy it to a bitmap which can be drawn, converting
            // to premultiplied:
            SkBitmap bm;
            bm.allocN32Pixels(fBitmap.width(), fBitmap.height());
            for (int i = 0; i < fBitmap.width(); ++i) {
                for (int j = 0; j < fBitmap.height(); ++j) {
                    *bm.getAddr32(i, j) = premultiply_unpmcolor(*fBitmap.getAddr32(i, j));
                }
            }
            canvas->drawBitmap(bm, 0, 0);
        } else {
            canvas->drawBitmap(fBitmap, 0, 0);
        }
    }

private:
    const SkString  fResPath;
    SkString        fCurrFile;
    bool            fPremul;
    bool            fDecodeSucceeded;
    SkBitmap        fBitmap;
    SkOSFile::Iter  fFileIter;

    static const char   fNextImageChar      = 'j';
    static const char   fTogglePremulChar   = 'h';

    void nextImage() {
        if (fResPath.size() == 0) {
            return;
        }
        SkString basename;
        if (!fFileIter.next(&basename)) {
            fFileIter.reset(fResPath.c_str());
            if (!fFileIter.next(&basename)) {
                // Perhaps this should draw some error message?
                return;
            }
        }
        fCurrFile = SkOSPath::Join(fResPath.c_str(), basename.c_str());
        this->decodeCurrFile();
    }

    void decodeCurrFile() {
        if (fCurrFile.size() == 0) {
            fDecodeSucceeded = false;
            return;
        }
        SkFILEStream stream(fCurrFile.c_str());
        SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(&stream));
        if (NULL == decoder.get()) {
            fDecodeSucceeded = false;
            return;
        }
        if (!fPremul) {
            decoder->setRequireUnpremultipliedColors(true);
        }
        fDecodeSucceeded = decoder->decode(&stream, &fBitmap, kN32_SkColorType,
                SkImageDecoder::kDecodePixels_Mode) != SkImageDecoder::kFailure;
        this->inval(NULL);
    }

    void togglePremul() {
        fPremul = !fPremul;
        this->decodeCurrFile();
    }

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() {
    return new UnpremulView(GetResourcePath());
}
static SkViewRegister reg(MyFactory);
