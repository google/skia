/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SkGradientShader.h"
#include "SkImageDecoder.h"
#include "SkStream.h"
#include "SkTypeface.h"

static void setTypeface(SkPaint* paint, const char name[], SkTypeface::Style style) {
    sk_tool_utils::set_portable_typeface(paint, name, style);
}

static SkSize computeSize(const SkBitmap& bm, const SkMatrix& mat) {
    SkRect bounds = SkRect::MakeWH(SkIntToScalar(bm.width()),
                                   SkIntToScalar(bm.height()));
    mat.mapRect(&bounds);
    return SkSize::Make(bounds.width(), bounds.height());
}

static void draw_cell(SkCanvas* canvas, const SkBitmap& bm, const SkMatrix& mat, SkScalar dx,
                      SkPaint::FilterLevel lvl) {
    SkPaint paint;
    paint.setFilterLevel(lvl);

    SkAutoCanvasRestore acr(canvas, true);
    canvas->translate(dx, 0);
    canvas->concat(mat);
    canvas->drawBitmap(bm, 0, 0, &paint);
}

static void draw_row(SkCanvas* canvas, const SkBitmap& bm, const SkMatrix& mat, SkScalar dx) {
    draw_cell(canvas, bm, mat, 0 * dx, SkPaint::kNone_FilterLevel);
    draw_cell(canvas, bm, mat, 1 * dx, SkPaint::kLow_FilterLevel);
    draw_cell(canvas, bm, mat, 2 * dx, SkPaint::kMedium_FilterLevel);
    draw_cell(canvas, bm, mat, 3 * dx, SkPaint::kHigh_FilterLevel);
}

class FilterBitmapGM : public skiagm::GM {
    void onOnceBeforeDraw() SK_OVERRIDE {

        this->makeBitmap();

        SkScalar cx = SkScalarHalf(fBM.width());
        SkScalar cy = SkScalarHalf(fBM.height());
        SkScalar scale = this->getScale();

        // these two matrices use a scale factor configured by the subclass
        fMatrix[0].setScale(scale, scale);
        fMatrix[1].setRotate(30, cx, cy); fMatrix[1].postScale(scale, scale);

        // up/down scaling mix
        fMatrix[2].setScale(0.7f, 1.05f);
    }

public:
    SkBitmap    fBM;
    SkMatrix    fMatrix[3];
    SkString    fName;

    FilterBitmapGM()
    {
        this->setBGColor(0xFFDDDDDD);
    }

protected:

    SkString onShortName() SK_OVERRIDE {
        return fName;
    }

    SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(1024, 768);
    }

    virtual void makeBitmap() = 0;
    virtual SkScalar getScale() = 0;

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {

        canvas->translate(10, 10);
        for (size_t i = 0; i < SK_ARRAY_COUNT(fMatrix); ++i) {
            SkSize size = computeSize(fBM, fMatrix[i]);
            size.fWidth += 20;
            size.fHeight += 20;

            draw_row(canvas, fBM, fMatrix[i], size.fWidth);
            canvas->translate(0, size.fHeight);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

class FilterBitmapTextGM: public FilterBitmapGM {
  public:
      FilterBitmapTextGM(float textSize)
      : fTextSize(textSize)
        {
            fName.printf("filterbitmap_text_%.2fpt", fTextSize);
        }

  protected:
      float fTextSize;

      SkScalar getScale() SK_OVERRIDE {
          return 32.f/fTextSize;
      }

      void makeBitmap() SK_OVERRIDE {
          fBM.allocN32Pixels(int(fTextSize * 8), int(fTextSize * 6));
          SkCanvas canvas(fBM);
          canvas.drawColor(SK_ColorWHITE);

          SkPaint paint;
          paint.setAntiAlias(true);
          paint.setSubpixelText(true);
          paint.setTextSize(fTextSize);

          setTypeface(&paint, "Times", SkTypeface::kNormal);
          canvas.drawText("Hamburgefons", 12, fTextSize/2, 1.2f*fTextSize, paint);
          setTypeface(&paint, "Times", SkTypeface::kBold);
          canvas.drawText("Hamburgefons", 12, fTextSize/2, 2.4f*fTextSize, paint);
          setTypeface(&paint, "Times", SkTypeface::kItalic);
          canvas.drawText("Hamburgefons", 12, fTextSize/2, 3.6f*fTextSize, paint);
          setTypeface(&paint, "Times", SkTypeface::kBoldItalic);
          canvas.drawText("Hamburgefons", 12, fTextSize/2, 4.8f*fTextSize, paint);
      }
  private:
      typedef FilterBitmapGM INHERITED;
};

class FilterBitmapCheckerboardGM: public FilterBitmapGM {
  public:
      FilterBitmapCheckerboardGM(int size, int num_checks)
      : fSize(size), fNumChecks(num_checks)
        {
            fName.printf("filterbitmap_checkerboard_%d_%d", fSize, fNumChecks);
        }

  protected:
      int fSize;
      int fNumChecks;

      SkScalar getScale() SK_OVERRIDE {
          return 192.f/fSize;
      }

      void makeBitmap() SK_OVERRIDE {
          fBM.allocN32Pixels(fSize, fSize);
          for (int y = 0; y < fSize; y ++) {
              for (int x = 0; x < fSize; x ++) {
                  SkPMColor* s = fBM.getAddr32(x, y);
                  int cx = (x * fNumChecks) / fSize;
                  int cy = (y * fNumChecks) / fSize;
                  if ((cx+cy)%2) {
                      *s = 0xFFFFFFFF;
                  } else {
                      *s = 0xFF000000;
                  }
              }
          }
      }
  private:
      typedef FilterBitmapGM INHERITED;
};

class FilterBitmapImageGM: public FilterBitmapGM {
  public:
      FilterBitmapImageGM(const char filename[])
      : fFilename(filename)
        {
            fName.printf("filterbitmap_image_%s", filename);
        }

  protected:
      SkString fFilename;
      int fSize;

      SkScalar getScale() SK_OVERRIDE {
          return 192.f/fSize;
      }

      void makeBitmap() SK_OVERRIDE {
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
      typedef FilterBitmapGM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new FilterBitmapTextGM(3); )
DEF_GM( return new FilterBitmapTextGM(7); )
DEF_GM( return new FilterBitmapTextGM(10); )
DEF_GM( return new FilterBitmapCheckerboardGM(4,4); )
DEF_GM( return new FilterBitmapCheckerboardGM(32,32); )
DEF_GM( return new FilterBitmapCheckerboardGM(32,8); )
DEF_GM( return new FilterBitmapCheckerboardGM(32,2); )
DEF_GM( return new FilterBitmapCheckerboardGM(192,192); )
DEF_GM( return new FilterBitmapImageGM("mandrill_16.png"); )
DEF_GM( return new FilterBitmapImageGM("mandrill_32.png"); )
DEF_GM( return new FilterBitmapImageGM("mandrill_64.png"); )
DEF_GM( return new FilterBitmapImageGM("mandrill_128.png"); )
DEF_GM( return new FilterBitmapImageGM("mandrill_256.png"); )
DEF_GM( return new FilterBitmapImageGM("mandrill_512.png"); )
DEF_GM( return new FilterBitmapImageGM("color_wheel.png"); )
