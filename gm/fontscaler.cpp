/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "sk_tool_utils.h"
#include "SkTypeface.h"
#include "SkFontMgr.h"

namespace skiagm {

class FontScalerGM : public GM {
public:
    FontScalerGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:

    SkString onShortName() override {
        SkString name("fontscaler");
        name.append(sk_tool_utils::major_platform_os_name());
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(1450, 750);
    }

    void onDraw(SkCanvas* canvas) override {
      SkPaint paint;

      paint.setAntiAlias(true);
      paint.setLCDRenderText(true);
      paint.setSubpixelText(true);

      sk_sp<SkFontMgr> fontMgr = SkFontMgr::RefDefault();
      sk_sp<SkTypeface> typeface(fontMgr->matchFamilyStyle("Calibri", SkFontStyle()));

      const char* text = "Hamburgefons ooo mmm";
      const size_t textLen = strlen(text);

      SkScalar positions[] = {0, 13.489258f, 9.666992f, 16.365234f, 11.169922f, 10.752441f,
                              6.605469f,
                              11.188477f,
                              9.936035f,
                              5.946777f,
                              11.132813f,
                              10.752441f,
                              8.062012f,
                              5.204590f,
                              11.132813f,
                              11.132813f,
                              11.132813f,
                              5.204590f,
                              16.365234f,
                              16.365234f,
                              16.365234f};

      for (int i = 0; i < textLen -1; ++i) {
        positions[i+1] = positions[i] + positions[i+1];
      }

      // paint.setTextSize(SkIntToScalar(19));
      // printf("Subpixel true");
      // for (size_t i = 0; i < textLen; ++i) {
      //   SkScalar sk_width;
      //   paint.getTextWidths(&text[i], 1, &sk_width, nullptr);
      //   printf("width of %d: %f\n", text[i], sk_width);
      // }
      // paint.setSubpixelText(false);
      // printf("Subpixel false");
      // for (size_t i = 0; i < textLen; ++i) {
      //   SkScalar sk_width;
      //   paint.getTextWidths(&text[i], 1, &sk_width, nullptr);
      //   printf("width of %d: %f\n", text[i], sk_width);
      // }
      // paint.setSubpixelText(true);


      paint.setTypeface(typeface);
      SkScalar x = SkIntToScalar(10);
      SkScalar y = SkIntToScalar(20);

      paint.setEmbeddedBitmapText(false);
      paint.setTextSize(SkIntToScalar(19));
      static const int subpixelSteps = 4;
      for (int i = 0; i <= subpixelSteps; ++i) {
        //        canvas->drawText(text, textLen, x, y, paint);
        canvas->drawPosTextH(text, textLen, positions, y, paint);
        canvas->translate(1.0f/(float)(subpixelSteps) * (float)i, 20);
      }

      canvas->translate(0, 40);
      paint.setEmbeddedBitmapText(true);
      paint.setTextSize(SkIntToScalar(19));
      for (int i = 0; i <= subpixelSteps; ++i) {
        //        canvas->drawText(text, textLen, x, y, paint);
        canvas->drawPosTextH(text, textLen, positions, y, paint);
        canvas->translate(1.0f/(float)(subpixelSteps) * (float)i, 20);
      }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new FontScalerGM; }
static GMRegistry reg(MyFactory);

}
