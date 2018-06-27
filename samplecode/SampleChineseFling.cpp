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

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

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
    return MakeResourceAsTypeface("fonts/NotoSansCJK-Regular.ttc");
#elif defined(SK_BUILD_FOR_WIN)
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
    ChineseFlingView() : fBlobs(kNumBlobs) {}

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

        // draw a consistent run of the 'words' - one word per line
        int index = fIndex;
        for (SkScalar y = 0.0f; y < 1024.0f; ) {

            y += -fMetrics.fAscent;
            canvas->drawTextBlob(fBlobs[index], 0, y, paint);

            y += fMetrics.fDescent + fMetrics.fLeading;
            ++index;
            index %= fBlobs.count();
        }
        // now "fling" a random amount
        fIndex += fRand.nextRangeU(5, 20);
        fIndex %= fBlobs.count();
    }

private:
    static constexpr auto kNumBlobs = 200;
    static constexpr auto kWordLength = 16;

    void init() {
        fTypeface = chinese_typeface();

        SkPaint paint;
        make_paint(&paint, fTypeface);

        paint.getFontMetrics(&fMetrics);

        SkUnichar glyphs[kWordLength];
        for (int32_t i = 0; i < kNumBlobs; ++i) {
            this->createRandomWord(glyphs);

            SkTextBlobBuilder builder;
            sk_tool_utils::add_to_text_blob_w_len(&builder, (const char*) glyphs, kWordLength*4,
                                                  paint, 0, 0);

            fBlobs.emplace_back(builder.make());
        }

        fIndex = 0;
    }

    // Construct a random kWordLength character 'word' drawing from the full Chinese set
    void createRandomWord(SkUnichar glyphs[kWordLength]) {
        for (int i = 0; i < kWordLength; ++i) {
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

class ChineseZoomView : public SampleView {
public:
    ChineseZoomView() : fBlobs(kNumBlobs), fScale(15.0f), fTranslate(0.0f) {}

protected:
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "chinese-zoom");
            return true;
        }
        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
            if ('>' == uni) {
                fScale += 0.125f;
                return true;
            }
            if ('<' == uni) {
                fScale -= 0.125f;
                return true;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        bool afterFirstFrame = fInitialized;
        if (!fInitialized) {
            this->init();
            fInitialized = true;
        }

        canvas->clear(0xFFDDDDDD);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xDE000000);
        paint.setTypeface(fTypeface);
        paint.setTextSize(11);
        paint.setTextEncoding(SkPaint::kUTF32_TextEncoding);

        if (afterFirstFrame) {
#if SK_SUPPORT_GPU
            GrContext* grContext = canvas->getGrContext();
            if (grContext) {
                sk_sp<SkImage> image =
                grContext->getFontAtlasImage_ForTesting(GrMaskFormat::kA8_GrMaskFormat, 0);
                canvas->drawImageRect(image,
                                      SkRect::MakeXYWH(10.0f, 10.0f, 512.0f, 512.0), &paint);
                image = grContext->getFontAtlasImage_ForTesting(GrMaskFormat::kA8_GrMaskFormat, 1);
                canvas->drawImageRect(image,
                                      SkRect::MakeXYWH(522.0f, 10.0f, 512.f, 512.0f), &paint);
                image = grContext->getFontAtlasImage_ForTesting(GrMaskFormat::kA8_GrMaskFormat, 2);
                canvas->drawImageRect(image,
                                      SkRect::MakeXYWH(10.0f, 522.0f, 512.0f, 512.0f), &paint);
                image = grContext->getFontAtlasImage_ForTesting(GrMaskFormat::kA8_GrMaskFormat, 3);
                canvas->drawImageRect(image,
                                      SkRect::MakeXYWH(522.0f, 522.0f, 512.0f, 512.0f), &paint);
            }
#endif
        }

        canvas->scale(fScale, fScale);
        canvas->translate(0, fTranslate);
        fTranslate -= 0.5f;

        // draw a consistent run of the 'words' - one word per line
        SkScalar y = 0;
        for (int index = 0; index < kNumBlobs; ++index) {
            y += -fMetrics.fAscent;
            canvas->drawTextBlob(fBlobs[index], 0, y, paint);

            y += 3*(fMetrics.fDescent - fMetrics.fAscent + fMetrics.fLeading);
        }
    }

private:
    static constexpr auto kNumBlobs = 8;
    static constexpr auto kParagraphLength = 175;

    void init() {
        fTypeface = chinese_typeface();

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xDE000000);
        paint.setTypeface(fTypeface);
        paint.setTextSize(11);
        paint.setTextEncoding(SkPaint::kUTF32_TextEncoding);

        paint.getFontMetrics(&fMetrics);

        SkUnichar glyphs[45];
        for (int32_t i = 0; i < kNumBlobs; ++i) {
            SkTextBlobBuilder builder;
            auto paragraphLength = kParagraphLength;
            SkScalar y = 0;
            while (paragraphLength - 45 > 0) {
                auto currentLineLength = SkTMin(45, paragraphLength - 45);
                this->createRandomLine(glyphs, currentLineLength);

                sk_tool_utils::add_to_text_blob_w_len(&builder, (const char*) glyphs,
                                                      currentLineLength*4, paint, 0, y);
                y += fMetrics.fDescent - fMetrics.fAscent + fMetrics.fLeading;
                paragraphLength -= 45;
            }
            fBlobs.emplace_back(builder.make());
        }

        fIndex = 0;
    }

    // Construct a random kWordLength character 'word' drawing from the full Chinese set
    void createRandomLine(SkUnichar glyphs[45], int lineLength) {
        for (auto i = 0; i < lineLength; ++i) {
            glyphs[i] = fRand.nextRangeU(0x4F00, 0x9FA0);
        }
    }

    bool                        fInitialized = false;
    sk_sp<SkTypeface>           fTypeface;
    SkPaint::FontMetrics        fMetrics;
    SkTArray<sk_sp<SkTextBlob>> fBlobs;
    SkRandom                    fRand;
    SkScalar                    fScale;
    SkScalar                    fTranslate;
    int                         fIndex;

    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* FlingFactory() { return new ChineseFlingView; }
static SkViewRegister regFling(FlingFactory);

static SkView* ZoomFactory() { return new ChineseZoomView; }
static SkViewRegister regZoom(ZoomFactory);
