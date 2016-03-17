/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SkGradientShader.h"
#include "SkTypeface.h"
#include "SkImageDecoder.h"
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

static void setTypeface(SkPaint* paint, const char name[], SkTypeface::Style style) {
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

          setTypeface(&paint, "serif", SkTypeface::kNormal);
          canvas.drawText("Hamburgefons", 12, fTextSize/2, 1.2f*fTextSize, paint);
          setTypeface(&paint, "serif", SkTypeface::kBold);
          canvas.drawText("Hamburgefons", 12, fTextSize/2, 2.4f*fTextSize, paint);
          setTypeface(&paint, "serif", SkTypeface::kItalic);
          canvas.drawText("Hamburgefons", 12, fTextSize/2, 3.6f*fTextSize, paint);
          setTypeface(&paint, "serif", SkTypeface::kBoldItalic);
          canvas.drawText("Hamburgefons", 12, fTextSize/2, 4.8f*fTextSize, paint);
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
      DownsampleBitmapImageGM(const char filename[], SkFilterQuality filterQuality)
      : INHERITED(filterQuality), fFilename(filename)
        {
            fName.printf("downsamplebitmap_image_%s_%s", this->filterQualityToString(), filename);
        }

  protected:
      SkString fFilename;
      int fSize;

      void make_bitmap() override {
          SkImageDecoder* codec = nullptr;
          SkString resourcePath = GetResourcePath(fFilename.c_str());
          SkFILEStream stream(resourcePath.c_str());
          if (stream.isValid()) {
              codec = SkImageDecoder::Factory(&stream);
          }
          if (codec) {
              stream.rewind();
              codec->decode(&stream, &fBM, kN32_SkColorType, SkImageDecoder::kDecodePixels_Mode);
              delete codec;
          } else {
              fBM.allocN32Pixels(1, 1);
              *(fBM.getAddr32(0,0)) = 0xFF0000FF; // red == bad
          }
          fSize = fBM.height();
      }
  private:
      typedef DownsampleBitmapGM INHERITED;
};

DEF_GM( return new DownsampleBitmapTextGM(72, kHigh_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapCheckerboardGM(512,256, kHigh_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapImageGM("mandrill_512.png", kHigh_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapImageGM("mandrill_132x132_12x12.astc",
                                            kHigh_SkFilterQuality); )

DEF_GM( return new DownsampleBitmapTextGM(72, kMedium_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapCheckerboardGM(512,256, kMedium_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapImageGM("mandrill_512.png", kMedium_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapImageGM("mandrill_132x132_12x12.astc",
                                           kMedium_SkFilterQuality); )

DEF_GM( return new DownsampleBitmapTextGM(72, kLow_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapCheckerboardGM(512,256, kLow_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapImageGM("mandrill_512.png", kLow_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapImageGM("mandrill_132x132_12x12.astc",
                                           kLow_SkFilterQuality); )

DEF_GM( return new DownsampleBitmapTextGM(72, kNone_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapCheckerboardGM(512,256, kNone_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapImageGM("mandrill_512.png", kNone_SkFilterQuality); )
DEF_GM( return new DownsampleBitmapImageGM("mandrill_132x132_12x12.astc",
                                           kNone_SkFilterQuality); )
