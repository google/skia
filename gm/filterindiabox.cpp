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

static void draw_cell(SkCanvas* canvas, const SkBitmap& bm, const SkMatrix& mat, SkScalar dx,
                      SkFilterQuality lvl) {
    SkPaint paint;
    paint.setFilterQuality(lvl);

    SkAutoCanvasRestore acr(canvas, true);

    canvas->translate(dx, 0);
    canvas->concat(mat);
    canvas->drawBitmap(bm, 0, 0, &paint);
}

static void draw_row(SkCanvas* canvas, const SkBitmap& bm, const SkMatrix& mat, SkScalar dx) {
    draw_cell(canvas, bm, mat, 0 * dx, kNone_SkFilterQuality);
    draw_cell(canvas, bm, mat, 1 * dx, kLow_SkFilterQuality);
    draw_cell(canvas, bm, mat, 2 * dx, kMedium_SkFilterQuality);
    draw_cell(canvas, bm, mat, 3 * dx, kHigh_SkFilterQuality);
}

class FilterIndiaBoxGM : public skiagm::GM {
    void onOnceBeforeDraw() override {
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
        this->setBGColor(sk_tool_utils::color_to_565(0xFFDDDDDD));
    }

    FilterIndiaBoxGM(const char filename[]) : fFilename(filename) {
        fName.printf("filterindiabox");
    }

protected:
    SkString onShortName() override {
        return fName;
    }

    SkISize onISize() override {
        return SkISize::Make(1024, 768);
    }

    void onDraw(SkCanvas* canvas) override {
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
          SkImageDecoder* codec = NULL;
          SkString resourcePath = GetResourcePath(fFilename.c_str());
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
