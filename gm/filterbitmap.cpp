/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

static void setTypeface(SkFont* font, const char name[], SkFontStyle style) {
    font->setTypeface(ToolUtils::create_portable_typeface(name, style));
}

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

class FilterBitmapGM : public skiagm::GM {
    void onOnceBeforeDraw() override {

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

    SkString onShortName() override {
        return fName;
    }

    SkISize onISize() override {
        return SkISize::Make(1024, 768);
    }

    virtual void makeBitmap() = 0;
    virtual SkScalar getScale() = 0;

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

      SkScalar getScale() override {
          return 32.f/fTextSize;
      }

      void makeBitmap() override {
          fBM.allocN32Pixels(int(fTextSize * 8), int(fTextSize * 6));
          SkCanvas canvas(fBM);
          canvas.drawColor(SK_ColorWHITE);

          SkPaint paint;
          SkFont font;
          font.setSize(fTextSize);
          font.setSubpixel(true);

          setTypeface(&font, "serif", SkFontStyle::Normal());
          canvas.drawString("Hamburgefons", fTextSize/2, 1.2f*fTextSize, font, paint);
          setTypeface(&font, "serif", SkFontStyle::Bold());
          canvas.drawString("Hamburgefons", fTextSize/2, 2.4f*fTextSize, font, paint);
          setTypeface(&font, "serif", SkFontStyle::Italic());
          canvas.drawString("Hamburgefons", fTextSize/2, 3.6f*fTextSize, font, paint);
          setTypeface(&font, "serif", SkFontStyle::BoldItalic());
          canvas.drawString("Hamburgefons", fTextSize/2, 4.8f*fTextSize, font, paint);
      }
  private:
      typedef FilterBitmapGM INHERITED;
};

class FilterBitmapCheckerboardGM: public FilterBitmapGM {
public:
    FilterBitmapCheckerboardGM(int size, int num_checks, bool convertToG8 = false)
        : fSize(size), fNumChecks(num_checks), fConvertToG8(convertToG8)
    {
        fName.printf("filterbitmap_checkerboard_%d_%d%s",
                     fSize, fNumChecks, convertToG8 ? "_g8" : "");
    }

  protected:
      int fSize;
      int fNumChecks;

      SkScalar getScale() override {
          return 192.f/fSize;
      }

      void makeBitmap() override {
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
          if (fConvertToG8) {
              SkBitmap tmp;
              ToolUtils::copy_to_g8(&tmp, fBM);
              fBM = tmp;
          }
      }
private:
    const bool fConvertToG8;
    typedef FilterBitmapGM INHERITED;
};

class FilterBitmapImageGM: public FilterBitmapGM {
public:
    FilterBitmapImageGM(const char filename[], bool convertToG8 = false)
        : fFilename(filename), fConvertToG8(convertToG8)
    {
        fName.printf("filterbitmap_image_%s%s", filename, convertToG8 ? "_g8" : "");
    }

protected:
      SkString fFilename;
      int fSize;

      SkScalar getScale() override {
          return 192.f/fSize;
      }

      void makeBitmap() override {
        SkString resource = SkStringPrintf("images/%s", fFilename.c_str());
        if (!GetResourceAsBitmap(resource.c_str(), &fBM)) {
            fBM.allocN32Pixels(1, 1);
            fBM.eraseARGB(255, 255, 0 , 0); // red == bad
        }
        fSize = fBM.height();

        if (fConvertToG8) {
            SkBitmap tmp;
            ToolUtils::copy_to_g8(&tmp, fBM);
            fBM = tmp;
        }
      }
private:
    const bool fConvertToG8;
    typedef FilterBitmapGM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new FilterBitmapTextGM(3); )
DEF_GM( return new FilterBitmapTextGM(7); )
DEF_GM( return new FilterBitmapTextGM(10); )
DEF_GM( return new FilterBitmapCheckerboardGM(4,4); )
DEF_GM( return new FilterBitmapCheckerboardGM(32,32); )
DEF_GM( return new FilterBitmapCheckerboardGM(32,32, true); )
DEF_GM( return new FilterBitmapCheckerboardGM(32,8); )
DEF_GM( return new FilterBitmapCheckerboardGM(32,2); )
DEF_GM( return new FilterBitmapCheckerboardGM(192,192); )
DEF_GM( return new FilterBitmapImageGM("mandrill_16.png"); )
DEF_GM( return new FilterBitmapImageGM("mandrill_32.png"); )
DEF_GM( return new FilterBitmapImageGM("mandrill_64.png"); )
DEF_GM( return new FilterBitmapImageGM("mandrill_64.png", true); )
DEF_GM( return new FilterBitmapImageGM("mandrill_128.png"); )
DEF_GM( return new FilterBitmapImageGM("mandrill_256.png"); )
DEF_GM( return new FilterBitmapImageGM("mandrill_512.png"); )
DEF_GM( return new FilterBitmapImageGM("color_wheel.png"); )
