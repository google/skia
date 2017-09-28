/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SampleCode.h"
#include "sk_tool_utils.h"

#include "SkCanvas.h"
#include "SkFontMgr.h"
#include "SkRandom.h"
#include "SkTypeface.h"
#include "SkTextBlob.h"

static void make_paint(SkPaint* paint, sk_sp<SkTypeface> typeface) {
  static const int kTextSize = 56;

  paint->setAntiAlias(true);
  paint->setColor(0xDE000000);
  paint->setTypeface(typeface);
  paint->setTextSize(kTextSize);
  paint->setTextEncoding(SkPaint::kUTF32_TextEncoding);
}

static sk_sp<SkTypeface> chinese_typeface() {
#ifdef SK_BUILD_FOR_ANDROID
    return MakeResourceAsTypeface("/fonts/NotoSansCJK-Regular.ttc");
#elif defined(SK_BUILD_FOR_WIN32)
    return SkTypeface::MakeFromName("SimSun", SkFontStyle());
#elif defined(SK_BUILD_FOR_MAC)
    return SkTypeface::MakeFromName("Hiragino Sans GB W3", SkFontStyle());
#elif defined(SK_BUILD_FOR_IOS)
    return SkTypeface::MakeFromName("Hiragino Sans GB W3", SkFontStyle());
#elif defined(SK_BUILD_FOR_UNIX)
    return SkTypeface::MakeFromName("Noto Sans CJK SC", SkFontStyle());
#else
    return nullptr;
#endif
}

class ChineseFlingView : public SampleView {
public:
    ChineseFlingView() {}

protected:
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "chinese-fling");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        if (!fInitialized) {
            this->init();
            fInitialized = true;
        }

        canvas->clear(0xFFDDDDDD);

        SkPaint paint;
        make_paint(&paint, fTypeface);

        int index = fIndex;
        for (SkScalar y = 0.0f; y < 1024.0f; ) {

            y += -fMetrics.fAscent;
            canvas->drawTextBlob(fBlobs[index], 0, y, paint);

            y += fMetrics.fDescent + fMetrics.fLeading;
            ++index;
            index %= fBlobs.count();
        }
        fIndex += fRand.nextRangeU(5, 20);
        fIndex %= fBlobs.count();

        this->inval(nullptr);
    }

private:
    void init() {
        fTypeface = chinese_typeface();

        SkPaint paint;
        make_paint(&paint, fTypeface);

        paint.getFontMetrics(&fMetrics);

        SkUnichar glyphs[16];
        for (int32_t i = 0; i < 200; ++i) {
            getUnicodeRow(glyphs);

            SkTextBlobBuilder builder;
            sk_tool_utils::add_to_text_blob_w_len(&builder, (const char*) glyphs, 16*4, paint,
                                                  0, 0);

            fBlobs.emplace_back(builder.make());
        }

        fIndex = 0;
    }

    void getUnicodeRow(SkUnichar glyphs[16]) {
        for (int i = 0; i < 16; ++i) {
            glyphs[i] = fRand.nextRangeU(0x4F00, 0x9FA0);
        }
    }

    bool                        fInitialized = false;
    sk_sp<SkTypeface>           fTypeface;
    SkPaint::FontMetrics        fMetrics;
    SkTArray<sk_sp<SkTextBlob>> fBlobs;
    SkRandom                    fRand;
    int                         fIndex;

    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ChineseFlingView; }
static SkViewRegister reg(MyFactory);
