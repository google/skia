
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkImageGenerator.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"

#include "SkStream.h"

static void make_image(SkBitmap* bm, SkColorType ct, int configIndex) {
    const int   width = 98;
    const int   height = 100;
    const SkImageInfo info = SkImageInfo::Make(width, height, ct, kPremul_SkAlphaType);

    SkBitmap    device;
    device.allocN32Pixels(width, height);
    SkCanvas    canvas(device);
    SkPaint     paint;

    paint.setAntiAlias(true);
    canvas.drawColor(SK_ColorRED);
    paint.setColor(SK_ColorBLUE);
    canvas.drawCircle(SkIntToScalar(width)/2, SkIntToScalar(height)/2,
                      SkIntToScalar(width)/2, paint);

    switch (ct) {
        case kN32_SkColorType:
            bm->swap(device);
            break;
        case kRGB_565_SkColorType: {
            bm->allocPixels(info);
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    *bm->getAddr16(x, y) = SkPixel32ToPixel16(*device.getAddr32(x, y));
                }
            }
            break;
        }
        case kIndex_8_SkColorType: {
            SkPMColor colors[256];
            for (int i = 0; i < 256; i++) {
                if (configIndex & 1) {
                    colors[i] = SkPackARGB32(255-i, 0, 0, 255-i);
                } else {
                    colors[i] = SkPackARGB32(0xFF, i, 0, 255-i);
                }
            }
            SkColorTable* ctable = new SkColorTable(colors, 256);
            bm->allocPixels(info, NULL, ctable);
            ctable->unref();
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    *bm->getAddr8(x, y) = SkGetPackedR32(*device.getAddr32(x, y));
                }
            }
            break;
        }
        default:
            SkASSERT(0);
    }
}

// configs to build the original bitmap in. Can be at most these 3
static const SkColorType gColorTypes[] = {
    kN32_SkColorType,
    kRGB_565_SkColorType,
    kIndex_8_SkColorType,   // opaque
    kIndex_8_SkColorType    // alpha
};

static const char* const gConfigLabels[] = {
    "8888", "565", "Index8",  "Index8 alpha"
};

// types to encode into. Can be at most these 3. Must match up with gExt[]
static const SkImageEncoder::Type gTypes[] = {
    SkImageEncoder::kJPEG_Type,
    SkImageEncoder::kPNG_Type
};

// must match up with gTypes[]
static const char* const gExt[] = {
    ".jpg", ".png"
};

#include <sys/stat.h>

class EncodeView : public SampleView {
public:
    SkBitmap*        fBitmaps;
    SkAutoDataUnref* fEncodedPNGs;
    SkAutoDataUnref* fEncodedJPEGs;
    int              fBitmapCount;

    EncodeView() {
        fBitmapCount = SK_ARRAY_COUNT(gColorTypes);
        fBitmaps = new SkBitmap[fBitmapCount];
        fEncodedPNGs = new SkAutoDataUnref[fBitmapCount];
        fEncodedJPEGs = new SkAutoDataUnref[fBitmapCount];
        for (int i = 0; i < fBitmapCount; i++) {
            make_image(&fBitmaps[i], gColorTypes[i], i);

            for (size_t j = 0; j < SK_ARRAY_COUNT(gTypes); j++) {
                SkAutoTDelete<SkImageEncoder> codec(
                    SkImageEncoder::Create(gTypes[j]));
                if (NULL == codec.get()) {
                    SkDebugf("[%s:%d] failed to encode %s%s\n",
                             __FILE__, __LINE__,gConfigLabels[i], gExt[j]);
                    continue;
                }
                SkAutoDataUnref data(codec->encodeData(fBitmaps[i], 100));
                if (NULL == data.get()) {
                    SkDebugf("[%s:%d] failed to encode %s%s\n",
                             __FILE__, __LINE__,gConfigLabels[i], gExt[j]);
                    continue;
                }
                if (SkImageEncoder::kJPEG_Type == gTypes[j]) {
                    fEncodedJPEGs[i].reset(data.detach());
                } else if (SkImageEncoder::kPNG_Type == gTypes[j]) {
                    fEncodedPNGs[i].reset(data.detach());
                }
            }
        }
        this->setBGColor(0xFFDDDDDD);
    }

    virtual ~EncodeView() {
        delete[] fBitmaps;
        delete[] fEncodedPNGs;
        delete[] fEncodedJPEGs;
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "ImageEncoder");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        if (fBitmapCount == 0) {
            return;
        }

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextAlign(SkPaint::kCenter_Align);

        canvas->translate(SkIntToScalar(10), SkIntToScalar(20));

        SkScalar x = 0, y = 0, maxX = 0;
        const int SPACER = 10;

        for (int i = 0; i < fBitmapCount; i++) {
            canvas->drawText(gConfigLabels[i], strlen(gConfigLabels[i]),
                             x + SkIntToScalar(fBitmaps[i].width()) / 2, 0,
                             paint);
            y = paint.getTextSize();

            canvas->drawBitmap(fBitmaps[i], x, y);

            SkScalar yy = y;
            for (size_t j = 0; j < SK_ARRAY_COUNT(gTypes); j++) {
                yy += SkIntToScalar(fBitmaps[i].height() + 10);

                SkBitmap bm;
                SkData* encoded = NULL;
                if (SkImageEncoder::kJPEG_Type == gTypes[j]) {
                    encoded = fEncodedJPEGs[i].get();
                } else if (SkImageEncoder::kPNG_Type == gTypes[j]) {
                    encoded = fEncodedPNGs[i].get();
                }
                if (encoded) {
                    if (!SkInstallDiscardablePixelRef(encoded, &bm)) {
                    SkDebugf("[%s:%d] failed to decode %s%s\n",
                             __FILE__, __LINE__,gConfigLabels[i], gExt[j]);
                    }
                    canvas->drawBitmap(bm, x, yy);
                }
            }

            x += SkIntToScalar(fBitmaps[i].width() + SPACER);
            if (x > maxX) {
                maxX = x;
            }
        }

        y = (paint.getTextSize() + SkIntToScalar(fBitmaps[0].height())) * 3 / 2;
        x = maxX + SkIntToScalar(10);
        paint.setTextAlign(SkPaint::kLeft_Align);

        for (size_t j = 0; j < SK_ARRAY_COUNT(gExt); j++) {
            canvas->drawText(gExt[j], strlen(gExt[j]), x, y, paint);
            y += SkIntToScalar(fBitmaps[0].height() + SPACER);
        }
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y,
                                              unsigned modi) {
        this->inval(NULL);
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new EncodeView; }
static SkViewRegister reg(MyFactory);
