/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SkBitmapProcState.h"
#include "SkBitmapScaler.h"
#include "SkGradientShader.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkStream.h"
#include "SkTypeface.h"

static SkSize computeSize(const SkBitmap& bm, const SkMatrix& mat) {
    SkRect bounds = SkRect::MakeWH(SkIntToScalar(bm.width()),
                                   SkIntToScalar(bm.height()));
    mat.mapRect(&bounds);
    return SkSize::Make(bounds.width(), bounds.height());
}

static void draw_row(SkCanvas* canvas, const SkBitmap& bm, const SkMatrix& mat, SkScalar dx) {
    SkPaint paint;

    SkAutoCanvasRestore acr(canvas, true);

    canvas->drawBitmapMatrix(bm, mat, &paint);

    paint.setFilterLevel(SkPaint::kLow_FilterLevel);
    canvas->translate(dx, 0);
    canvas->drawBitmapMatrix(bm, mat, &paint);

    paint.setFilterLevel(SkPaint::kMedium_FilterLevel);
    canvas->translate(dx, 0);
    canvas->drawBitmapMatrix(bm, mat, &paint);

    paint.setFilterLevel(SkPaint::kHigh_FilterLevel);
    canvas->translate(dx, 0);
    canvas->drawBitmapMatrix(bm, mat, &paint);
}

class FilterIndiaBoxGM : public skiagm::GM {
    void onOnceBeforeDraw() {
        this->makeBitmap();

        SkScalar cx = SkScalarHalf(fBM.width());
        SkScalar cy = SkScalarHalf(fBM.height());

        float vertScale = 30.0f/55.0f;
        float horizScale = 150.0f/200.0f;

        fMatrix[0].setScale(horizScale, vertScale);
        fMatrix[1].setRotate(30, cx, cy); fMatrix[1].postScale(horizScale, vertScale);
    }

public:
    SkBitmap    fBM;
    SkMatrix    fMatrix[2];
    SkString    fName;

    FilterIndiaBoxGM() {
        this->setBGColor(0xFFDDDDDD);
    }

    FilterIndiaBoxGM(const char filename[]) : fFilename(filename) {
        fName.printf("filterindiabox");
    }

protected:
    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        return kSkipTiled_Flag;
    }

    virtual SkString onShortName() SK_OVERRIDE {
        return fName;
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(1024, 768);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        canvas->translate(10, 10);
        for (size_t i = 0; i < SK_ARRAY_COUNT(fMatrix); ++i) {
            SkSize size = computeSize(fBM, fMatrix[i]);
            size.fWidth += 20;
            size.fHeight += 20;

            draw_row(canvas, fBM, fMatrix[i], size.fWidth);
            canvas->translate(0, size.fHeight);
        }
    }

  protected:
      SkString fFilename;
      int fSize;

      SkScalar getScale() {
          return 192.f/fSize;
      }

      void makeBitmap() {
          SkString resourcePath = GetResourcePath();
          resourcePath.append("/");
          resourcePath.append(fFilename);

          SkImageDecoder* codec = NULL;
          SkFILEStream stream(resourcePath.c_str());
          if (stream.isValid()) {
              codec = SkImageDecoder::Factory(&stream);
          }
          if (codec) {
              stream.rewind();
              codec->decode(&stream, &fBM, kN32_SkColorType, SkImageDecoder::kDecodePixels_Mode);
              SkDELETE(codec);
          } else {
              fBM.allocN32Pixels(1, 1);
              *(fBM.getAddr32(0,0)) = 0xFF0000FF; // red == bad
          }
          fSize = fBM.height();
      }
  private:
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////


DEF_GM( return new FilterIndiaBoxGM("box.gif"); )
