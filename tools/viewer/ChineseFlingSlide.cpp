/*
 * Copyright 2017 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkTo.h"
#include "src/core/SkRandom.h"
#include "src/core/SkTHash.h"
#include "src/gpu/MaskFormat.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/timer/TimeUtils.h"
#include "tools/viewer/Slide.h"

#include <array>

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrDirectContext.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#endif

using MaskFormat = skgpu::MaskFormat;

static sk_sp<SkTypeface> chinese_typeface() {
#ifdef SK_BUILD_FOR_ANDROID
    return ToolUtils::CreateTypefaceFromResource("fonts/NotoSansCJK-Regular.ttc");
#elif defined(SK_BUILD_FOR_WIN)
    return ToolUtils::CreateTestTypeface("SimSun", SkFontStyle());
#elif defined(SK_BUILD_FOR_MAC)
    return ToolUtils::CreateTestTypeface("Hiragino Sans GB W3", SkFontStyle());
#elif defined(SK_BUILD_FOR_IOS)
    return ToolUtils::CreateTestTypeface("Hiragino Sans GB W3", SkFontStyle());
#elif defined(SK_BUILD_FOR_UNIX)
    return ToolUtils::CreateTestTypeface("Noto Sans CJK SC", SkFontStyle());
#else
    return nullptr;
#endif
}

class ChineseFlingSlide : public Slide {
    inline static constexpr int kNumBlobs = 200;
    inline static constexpr int kWordLength = 16;

    sk_sp<SkTypeface>    fTypeface;
    SkFontMetrics        fMetrics;
    std::array<sk_sp<SkTextBlob>, kNumBlobs> fBlobs;
    SkRandom             fRand;
    int                  fIndex = 0;

public:
    ChineseFlingSlide() { fName = "chinese-fling"; }

    void draw(SkCanvas* canvas) override {
        canvas->clear(0xFFDDDDDD);

        SkPaint paint;
        paint.setColor(0xDE000000);

        // draw a consistent run of the 'words' - one word per line
        int index = fIndex;
        for (SkScalar y = 0.0f; y < 1024.0f; ) {

            y += -fMetrics.fAscent;
            canvas->drawTextBlob(fBlobs[index], 0, y, paint);

            y += fMetrics.fDescent + fMetrics.fLeading;
            ++index;
            index %= kNumBlobs;
        }
        // now "fling" a random amount
        fIndex += fRand.nextRangeU(5, 20);
        fIndex %= kNumBlobs;
    }

    void load(SkScalar w, SkScalar h) override {
        fTypeface = chinese_typeface();

        SkFont font(fTypeface, 56);
        font.getMetrics(&fMetrics);

        std::array<SkUnichar, kWordLength> glyphs;
        for (int32_t i = 0; i < kNumBlobs; ++i) {
            this->createRandomWord(glyphs);

            SkTextBlobBuilder builder;
            ToolUtils::add_to_text_blob_w_len(&builder,
                                              reinterpret_cast<const char*>(glyphs.data()),
                                              glyphs.size() * sizeof(SkUnichar),
                                              SkTextEncoding::kUTF32,
                                              font,
                                              0,
                                              0);

            fBlobs[i] = builder.make();
        }
    }

    // Construct a random kWordLength character 'word' drawing from the full Chinese set
    void createRandomWord(SkSpan<SkUnichar, kWordLength> glyphs) {
        for (int i = 0; i < kWordLength; ++i) {
            glyphs[i] = fRand.nextRangeU(0x4F00, 0x9FA0);
        }
    }
};

class ChineseZoomSlide : public Slide {
    inline static constexpr int kNumBlobs = 8;
    inline static constexpr int kParagraphLength = 175;

    bool                 fAfterFirstFrame = false;
    sk_sp<SkTypeface>    fTypeface;
    SkFontMetrics        fMetrics;
    std::array<sk_sp<SkTextBlob>, kNumBlobs> fBlobs;
    SkRandom             fRand;
    SkScalar             fScale = 15;
    SkScalar             fTranslate = 0;

public:
    ChineseZoomSlide() { fName = "chinese-zoom"; }

    bool onChar(SkUnichar uni) override {
            if ('>' == uni) {
                fScale += 0.125f;
                return true;
            }
            if ('<' == uni) {
                fScale -= 0.125f;
                return true;
            }
            return false;
    }

    void draw(SkCanvas* canvas) override {
        canvas->clear(0xFFDDDDDD);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xDE000000);

        if (fAfterFirstFrame) {
#if defined(SK_GANESH)
            auto direct = GrAsDirectContext(canvas->recordingContext());
            if (direct) {
                sk_sp<SkImage> image = direct->priv().testingOnly_getFontAtlasImage(MaskFormat::kA8,
                                                                                    0);
                canvas->drawImageRect(image,
                                      SkRect::MakeXYWH(10.0f, 10.0f, 512.0f, 512.0),
                                      SkSamplingOptions(), &paint);
                image = direct->priv().testingOnly_getFontAtlasImage(MaskFormat::kA8, 1);
                canvas->drawImageRect(image,
                                      SkRect::MakeXYWH(522.0f, 10.0f, 512.f, 512.0f),
                                      SkSamplingOptions(), &paint);
                image = direct->priv().testingOnly_getFontAtlasImage(MaskFormat::kA8, 2);
                canvas->drawImageRect(image,
                                      SkRect::MakeXYWH(10.0f, 522.0f, 512.0f, 512.0f),
                                      SkSamplingOptions(), &paint);
                image = direct->priv().testingOnly_getFontAtlasImage(MaskFormat::kA8, 3);
                canvas->drawImageRect(image,
                                      SkRect::MakeXYWH(522.0f, 522.0f, 512.0f, 512.0f),
                                      SkSamplingOptions(), &paint);
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
        if (!fAfterFirstFrame) {
            fAfterFirstFrame = true;
        }
    }

    void load(SkScalar w, SkScalar h) override {
        fTypeface = chinese_typeface();

        SkFont font(fTypeface, 11);
        font.getMetrics(&fMetrics);

        SkPaint paint;
        paint.setColor(0xDE000000);

        std::array<SkUnichar, 45> glyphs;
        for (int32_t i = 0; i < kNumBlobs; ++i) {
            SkTextBlobBuilder builder;
            auto paragraphLength = kParagraphLength;
            SkScalar y = 0;
            while (paragraphLength - 45 > 0) {
                auto currentLineLength = SkToSizeT(std::min(45, paragraphLength - 45));
                SkSpan<SkUnichar> line = SkSpan<SkUnichar>(glyphs).first(currentLineLength);
                this->createRandomLine(line);

                ToolUtils::add_to_text_blob_w_len(&builder,
                                                  reinterpret_cast<const char*>(line.data()),
                                                  line.size_bytes(),
                                                  SkTextEncoding::kUTF32,
                                                  font,
                                                  0,
                                                  y);
                y += fMetrics.fDescent - fMetrics.fAscent + fMetrics.fLeading;
                paragraphLength -= 45;
            }
            fBlobs[i] = builder.make();
        }
    }

    // Construct a random kWordLength character 'word' drawing from the full Chinese set
    void createRandomLine(SkSpan<SkUnichar> glyphs) {
        for (SkUnichar& g : glyphs) {
            g = fRand.nextRangeU(0x4F00, 0x9FA0);
        }
    }
};

/**
 * Scrolls a large number of unique glyphs so the set of glyphs that need to be active in the cache
 * evolves over time and glyphs age out of being needed in current frame.
 */
class ChineseScrollSlide : public Slide {
    static constexpr int   kNumBlobs     = 50;
    static constexpr int   kLinesPerBlob = 6;
    static constexpr int   kLineLength   = 45;
    static constexpr float kFontSize     = 25;

    static constexpr int kDefaultScrollUnitsPerSecondLog2 = 7;

    sk_sp<SkTypeface> fTypeface;
    std::array<sk_sp<SkTextBlob>, kNumBlobs> fBlobs;
    float             fVerticalPeriod;
    SkRandom          fRand;
    float             fTranslate = 0;
    double            fPrevNs = std::numeric_limits<double>::infinity();
    // The scroll speed is sign(value)*2^abs(value) where sign is tri-state (-1, 0, or 1)
    int               fScrollUnitsPerSecondLog2 = kDefaultScrollUnitsPerSecondLog2;

public:
    ChineseScrollSlide() { fName = "chinese-scroll"; }

    bool onChar(SkUnichar uni) override {
        if (',' == uni) {
            --fScrollUnitsPerSecondLog2;
            return true;
        }
        if ('.' == uni) {
            ++fScrollUnitsPerSecondLog2;
            return true;
        }
        return false;
    }

    bool animate(double ns) override {
        if (!fScrollUnitsPerSecondLog2) {
            fPrevNs = std::numeric_limits<double>::infinity();
            return false;
        }
        if (!std::isfinite(fPrevNs)) {
            fPrevNs = ns;
            return true;
        }
        float scrollUnitsPerSecond = std::pow(2, std::abs(fScrollUnitsPerSecondLog2));
        if (fScrollUnitsPerSecondLog2 < 0) {
            scrollUnitsPerSecond = -scrollUnitsPerSecond;
        }
        float dt = TimeUtils::NanosToSeconds(ns - fPrevNs) * scrollUnitsPerSecond;
        fTranslate -= dt;
        fPrevNs = ns;
        return true;
    }

    void draw(SkCanvas* canvas) override {
        canvas->clear(0xFFDDDDDD);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xDE000000);

        canvas->translate(0, fTranslate);

        // We want the text to repeat in y. If we assume the total height of the text
        // is larger than the window height then we need at most 2 repetitions to
        // cover the window.
        float n = std::floor(-fTranslate / fVerticalPeriod);
        float y = n * fVerticalPeriod;
        for (int i = 0; i < 2; ++i) {
            for (int b = 0; b < kNumBlobs; ++b) {
                canvas->drawTextBlob(fBlobs[b], 0, y, paint);
            }
            y += fVerticalPeriod;
        }
    }

    void load(SkScalar w, SkScalar h) override {
        fTypeface = chinese_typeface();

        SkFont font(fTypeface, kFontSize);
        SkFontMetrics metrics;
        font.getMetrics(&metrics);

        SkPaint paint;
        paint.setColor(0xDE000000);

        skia_private::THashSet<SkUnichar> usedGlyphs;
        float y = 0;
        for (int i = 0; i < kNumBlobs; ++i) {
            SkTextBlobBuilder builder;
            for (int j = 0; j < kLinesPerBlob; ++j) {
                std::array<SkUnichar, kLineLength> glyphs;
                this->createRandomLine(glyphs, usedGlyphs);

                ToolUtils::add_to_text_blob_w_len(&builder,
                                                  reinterpret_cast<const char*>(glyphs.data()),
                                                  glyphs.size() * sizeof(SkUnichar),
                                                  SkTextEncoding::kUTF32,
                                                  font,
                                                  0,
                                                  y);
                y += metrics.fDescent - metrics.fAscent + metrics.fLeading;
            }
            fBlobs[i] = builder.make();
        }
        fVerticalPeriod = std::ceil(y);
    }

    // Construct a random lineLength character 'word' drawing from the full Chinese set
    // but not using glyphs in usedGlyphs.
    void createRandomLine(SkSpan<SkUnichar> glyphs, auto& usedGlyphs) {
        for (SkUnichar& g : glyphs) {
            do {
                g = fRand.nextRangeU(0x4F00, 0x9FA0);
            } while (usedGlyphs.contains(g));
            usedGlyphs.add(g);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_SLIDE( return new ChineseFlingSlide(); )
DEF_SLIDE( return new ChineseZoomSlide(); )
DEF_SLIDE( return new ChineseScrollSlide(); )
