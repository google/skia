/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

#include "Resources.h"
#include "SkGradientShader.h"
#include "SkTypeface.h"
#include "SkStream.h"
#include "SkPaint.h"

static void make_checker(SkBitmap* bm, int size, int numChecks) {
    bm->allocN32Pixels(size, size);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            SkPMColor* s = bm->getAddr32(x, y);
            int cx = (x * numChecks) / size;
            int cy = (y * numChecks) / size;
            if ((cx+cy)%2) {
                *s = 0xFFFFFFFF;
            } else {
                *s = 0xFF000000;
            }
        }
    }
}

static void setTypeface(SkPaint* paint, const char name[], SkFontStyle style) {
    sk_tool_utils::set_portable_typeface(paint, name, style);
}

class DownsampleBitmapGM : public skiagm::GM {
public:
    SkBitmap    fBM;
    SkString    fName;
    bool        fBitmapMade;
    SkFilterQuality fFilterQuality;

    DownsampleBitmapGM(SkFilterQuality filterQuality)
        : fFilterQuality(filterQuality)
    {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFDDDDDD));
        fBitmapMade = false;
    }

    const char* filterQualityToString() {
        static const char *filterQualityNames[] = {
            "none", "low", "medium", "high"
        };
        return filterQualityNames[fFilterQuality];
    }

protected:

    SkString onShortName() override {
        return fName;
    }

    SkISize onISize() override {
        make_bitmap_wrapper();
        return SkISize::Make(fBM.width(), 4 * fBM.height());
    }

    void make_bitmap_wrapper() {
        if (!fBitmapMade) {
            fBitmapMade = true;
            make_bitmap();
        }
    }

    virtual void make_bitmap() = 0;

    void onDraw(SkCanvas* canvas) override {
        make_bitmap_wrapper();

        int curY = 0;
        int curHeight;
        float curScale = 1;
        do {

            SkMatrix matrix;
            matrix.setScale( curScale, curScale );

            SkPaint paint;
            paint.setFilterQuality(fFilterQuality);

            canvas->save();
            canvas->translate(0, (SkScalar)curY);
            canvas->concat(matrix);
            canvas->drawBitmap(fBM, 0, 0, &paint);
            canvas->restore();

            curHeight = (int) (fBM.height() * curScale + 2);
            curY += curHeight;
            curScale *= 0.75f;
        } while (curHeight >= 2 && curY < 4*fBM.height());
    }

private:
    typedef skiagm::GM INHERITED;
};

class DownsampleBitmapTextGM: public DownsampleBitmapGM {
  public:
      DownsampleBitmapTextGM(float textSize, SkFilterQuality filterQuality)
      : INHERITED(filterQuality), fTextSize(textSize)
        {
            fName.printf("downsamplebitmap_text_%s_%.2fpt", this->filterQualityToString(), fTextSize);
        }

  protected:
      float fTextSize;

      void make_bitmap() override {
          fBM.allocN32Pixels(int(fTextSize * 8), int(fTextSize * 6));
          SkCanvas canvas(fBM);
          canvas.drawColor(SK_ColorWHITE);

          SkPaint paint;
          paint.setAntiAlias(true);
          paint.setSubpixelText(true);
          paint.setTextSize(fTextSize);

          setTypeface(&paint, "serif", SkFontStyle());
          canvas.drawString("Hamburgefons", fTextSize/2, 1.2f*fTextSize, paint);
          setTypeface(&paint, "serif", SkFontStyle::Bold());
          canvas.drawString("Hamburgefons", fTextSize/2, 2.4f*fTextSize, paint);
          setTypeface(&paint, "serif", SkFontStyle::Italic());
          canvas.drawString("Hamburgefons", fTextSize/2, 3.6f*fTextSize, paint);
          setTypeface(&paint, "serif", SkFontStyle::BoldItalic());
          canvas.drawString("Hamburgefons", fTextSize/2, 4.8f*fTextSize, paint);
      }
  private:
      typedef DownsampleBitmapGM INHERITED;
};

class DownsampleBitmapCheckerboardGM: public DownsampleBitmapGM {
  public:
      DownsampleBitmapCheckerboardGM(int size, int numChecks, SkFilterQuality filterQuality)
      : INHERITED(filterQuality), fSize(size), fNumChecks(numChecks)
        {
            fName.printf("downsamplebitmap_checkerboard_%s_%d_%d", this->filterQualityToString(), fSize, fNumChecks);
        }

  protected:
      int fSize;
      int fNumChecks;

      void make_bitmap() override {
          make_checker(&fBM, fSize, fNumChecks);
      }
  private:
      typedef DownsampleBitmapGM INHERITED;
};

class DownsampleBitmapImageGM: public DownsampleBitmapGM {
  public:
      explicit DownsampleBitmapImageGM(SkFilterQuality filterQuality)
        : INHERITED(filterQuality) {
          fName.printf("downsamplebitmap_image_%s", this->filterQualityToString());
      }

  protected:
      int fSize;

      void make_bitmap() override {
        if (!GetResourceAsBitmap("images/mandrill_512.png", &fBM)) {
            fBM.allocN32Pixels(1, 1);
            fBM.eraseARGB(255, 255, 0 , 0); // red == bad
        }
        fSize = fBM.height();
      }
  private:
      typedef DownsampleBitmapGM INHERITED;
};

DEF_GM( return new DownsampleBitmapTextGM(72, kHigh_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapCheckerboardGM(512,256, kHigh_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapImageGM(kHigh_SkFilterQuality); )

DEF_GM( return new DownsampleBitmapTextGM(72, kMedium_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapCheckerboardGM(512,256, kMedium_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapImageGM(kMedium_SkFilterQuality); )

DEF_GM( return new DownsampleBitmapTextGM(72, kLow_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapCheckerboardGM(512,256, kLow_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapImageGM(kLow_SkFilterQuality); )

DEF_GM( return new DownsampleBitmapTextGM(72, kNone_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapCheckerboardGM(512,256, kNone_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapImageGM(kNone_SkFilterQuality); )
