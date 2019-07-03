/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkBlurDrawLooper.h"
#include "samplecode/DecodeFile.h"
#include "samplecode/Sample.h"
#include "src/core/SkBlurMask.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "src/utils/SkUTF.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

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

class UnpremulView : public Sample {
public:
    UnpremulView(SkString res)
    : fResPath(res)
    , fPremul(true)
    , fDecodeSucceeded(false) {
        this->nextImage();
    }

protected:
    SkString name() override { return SkString("unpremul"); }

    bool onChar(SkUnichar uni) override {
            char utf8[SkUTF::kMaxBytesInUTF8Sequence];
            size_t size = SkUTF::ToUTF8(uni, utf8);
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
            return false;
    }

    void onDrawBackground(SkCanvas* canvas) override {
        ToolUtils::draw_checkerboard(canvas, 0xFFCCCCCC, 0xFFFFFFFF, 12);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);

        SkFont font;
        font.setSize(24);
        auto looper(
            SkBlurDrawLooper::Make(SK_ColorBLUE, SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(2)),
                                   0, 0));
        paint.setLooper(looper);
        SkScalar height = font.getMetrics(nullptr);
        if (!fDecodeSucceeded) {
            SkString failure;
            if (fResPath.size() == 0) {
                failure.printf("resource path is required!");
            } else {
                failure.printf("Failed to decode %s", fCurrFile.c_str());
            }
            canvas->drawString(failure, 0, height, font, paint);
            return;
        }

        // Name, size of the file, and whether or not it is premultiplied.
        SkString header(SkOSPath::Basename(fCurrFile.c_str()));
        header.appendf("     [%dx%d]     %s", fBitmap.width(), fBitmap.height(),
                       (fPremul ? "premultiplied" : "unpremultiplied"));
        canvas->drawString(header, 0, height, font, paint);
        canvas->translate(0, height);

        // Help messages
        header.printf("Press '%c' to move to the next image.'", fNextImageChar);
        canvas->drawString(header, 0, height, font, paint);
        canvas->translate(0, height);

        header.printf("Press '%c' to toggle premultiplied decode.", fTogglePremulChar);
        canvas->drawString(header, 0, height, font, paint);

        // Now draw the image itself.
        canvas->translate(height * 2, height * 2);
        if (!fPremul) {
            // A premultiplied bitmap cannot currently be drawn.
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
        fDecodeSucceeded = decode_file(fCurrFile.c_str(), &fBitmap, kN32_SkColorType, !fPremul);
    }

    void togglePremul() {
        fPremul = !fPremul;
        this->decodeCurrFile();
    }

    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new UnpremulView(GetResourcePath("images")); )
