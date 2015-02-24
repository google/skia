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
    SkPaint::FilterLevel fFilterLevel;

    DownsampleBitmapGM(SkPaint::FilterLevel filterLevel)
        : fFilterLevel(filterLevel)
    {
        this->setBGColor(0xFFDDDDDD);
        fBitmapMade = false;
    }

    const char* filterLevelToString() {
        static const char *filterLevelNames[] = {
            "none", "low", "medium", "high"
        };
        return filterLevelNames[fFilterLevel];
    }

protected:

    SkString onShortName() SK_OVERRIDE {
        return fName;
    }

    SkISize onISize() SK_OVERRIDE {
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

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        make_bitmap_wrapper();

        int curY = 0;
        int curHeight;
        float curScale = 1;
        do {

            SkMatrix matrix;
            matrix.setScale( curScale, curScale );

            SkPaint paint;
            paint.setFilterLevel(fFilterLevel);

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
      DownsampleBitmapTextGM(float textSize, SkPaint::FilterLevel filterLevel)
      : INHERITED(filterLevel), fTextSize(textSize)
        {
            fName.printf("downsamplebitmap_text_%s_%.2fpt", this->filterLevelToString(), fTextSize);
        }

  protected:
      float fTextSize;

      void make_bitmap() SK_OVERRIDE {
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
      typedef DownsampleBitmapGM INHERITED;
};

class DownsampleBitmapCheckerboardGM: public DownsampleBitmapGM {
  public:
      DownsampleBitmapCheckerboardGM(int size, int numChecks, SkPaint::FilterLevel filterLevel)
      : INHERITED(filterLevel), fSize(size), fNumChecks(numChecks)
        {
            fName.printf("downsamplebitmap_checkerboard_%s_%d_%d", this->filterLevelToString(), fSize, fNumChecks);
        }

  protected:
      int fSize;
      int fNumChecks;

      void make_bitmap() SK_OVERRIDE {
          make_checker(&fBM, fSize, fNumChecks);
      }
  private:
      typedef DownsampleBitmapGM INHERITED;
};

class DownsampleBitmapImageGM: public DownsampleBitmapGM {
  public:
      DownsampleBitmapImageGM(const char filename[], SkPaint::FilterLevel filterLevel)
      : INHERITED(filterLevel), fFilename(filename)
        {
            fName.printf("downsamplebitmap_image_%s_%s", this->filterLevelToString(), filename);
        }

  protected:
      SkString fFilename;
      int fSize;

      void make_bitmap() SK_OVERRIDE {
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
      typedef DownsampleBitmapGM INHERITED;
};

#include "SkMipMap.h"

static void release_mipmap(void*, void* context) {
    ((SkMipMap*)context)->unref();
}

class ShowMipLevels : public skiagm::GM {
public:
    SkBitmap    fBM;

    ShowMipLevels() {
        this->setBGColor(0xFFDDDDDD);
        make_checker(&fBM, 512, 256);
    }

protected:

    SkString onShortName() SK_OVERRIDE {
        return SkString("showmiplevels");
    }

    SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(fBM.width() + 8, 2 * fBM.height() + 80);
    }

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkScalar x = 4;
        SkScalar y = 4;
        canvas->drawBitmap(fBM, x, y, NULL);
        y += fBM.height() + 4;

        SkAutoTUnref<SkMipMap> mm(SkMipMap::Build(fBM, NULL));

        SkMipMap::Level level;
        SkScalar scale = 0.5f;
        while (mm->extractLevel(scale, &level)) {
            SkImageInfo info = SkImageInfo::MakeN32Premul(level.fWidth, level.fHeight);
            SkBitmap bm;
            bm.installPixels(info, level.fPixels, level.fRowBytes, NULL,
                             &release_mipmap, (void*)(SkRef(mm.get())));
            bm.setImmutable();
            canvas->drawBitmap(bm, x, y, NULL);
            y += bm.height() + 4;
            scale /= 2;
            if (info.width() == 1 || info.height() == 1) {
                break;
            }
        }
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new ShowMipLevels; )

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new DownsampleBitmapTextGM(72, SkPaint::kHigh_FilterLevel); )
DEF_GM( return new DownsampleBitmapCheckerboardGM(512,256, SkPaint::kHigh_FilterLevel); )
DEF_GM( return new DownsampleBitmapImageGM("mandrill_512.png", SkPaint::kHigh_FilterLevel); )
DEF_GM( return new DownsampleBitmapImageGM("mandrill_132x132_12x12.astc",
                                            SkPaint::kHigh_FilterLevel); )

DEF_GM( return new DownsampleBitmapTextGM(72, SkPaint::kMedium_FilterLevel); )
DEF_GM( return new DownsampleBitmapCheckerboardGM(512,256, SkPaint::kMedium_FilterLevel); )
DEF_GM( return new DownsampleBitmapImageGM("mandrill_512.png", SkPaint::kMedium_FilterLevel); )
DEF_GM( return new DownsampleBitmapImageGM("mandrill_132x132_12x12.astc",
                                           SkPaint::kMedium_FilterLevel); )

DEF_GM( return new DownsampleBitmapTextGM(72, SkPaint::kLow_FilterLevel); )
DEF_GM( return new DownsampleBitmapCheckerboardGM(512,256, SkPaint::kLow_FilterLevel); )
DEF_GM( return new DownsampleBitmapImageGM("mandrill_512.png", SkPaint::kLow_FilterLevel); )
DEF_GM( return new DownsampleBitmapImageGM("mandrill_132x132_12x12.astc",
                                           SkPaint::kLow_FilterLevel); )

DEF_GM( return new DownsampleBitmapTextGM(72, SkPaint::kNone_FilterLevel); )
DEF_GM( return new DownsampleBitmapCheckerboardGM(512,256, SkPaint::kNone_FilterLevel); )
DEF_GM( return new DownsampleBitmapImageGM("mandrill_512.png", SkPaint::kNone_FilterLevel); )
DEF_GM( return new DownsampleBitmapImageGM("mandrill_132x132_12x12.astc",
                                           SkPaint::kNone_FilterLevel); )
