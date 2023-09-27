/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_fontations.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/ports/SkFontHost_FreeType_common.h"
#include "tools/Resources.h"
#include "tools/TestFontDataProvider.h"

namespace skiagm {

namespace {

constexpr int kGmWidth = 1000;
constexpr int kMargin = 15;
constexpr float kFontSize = 24;
constexpr float kLangYIncrementScale = 1.9;

/** Compare bitmap A and B, in this case originating from text rendering results with FreeType and
 * Fontations + Skia path rendering, compute individual pixel differences for the rectangles that
 * must match in size. Produce a highlighted difference bitmap, in which any pixel becomes white for
 * which a difference was determined. */
void comparePixels(const SkBitmap& bitmapA,
                   const SkBitmap& bitmapB,
                   SkBitmap* outPixelDiffBitmap,
                   SkBitmap* outHighlightDiffBitmap) {
    SkASSERT(bitmapA.colorType() == bitmapB.colorType() &&
             bitmapA.colorType() == outPixelDiffBitmap->colorType() &&
             bitmapA.dimensions() == bitmapB.dimensions() &&
             bitmapA.dimensions() == outPixelDiffBitmap->dimensions());

    SkASSERT(bitmapA.bytesPerPixel() == 4);
    SkISize dimensions = bitmapA.dimensions();
    for (int32_t x = 0; x < dimensions.fWidth; x++) {
        for (int32_t y = 0; y < dimensions.fHeight; y++) {
            SkPMColor c0 = *bitmapA.getAddr32(x, y);
            SkPMColor c1 = *bitmapB.getAddr32(x, y);
            int dr = SkGetPackedR32(c0) - SkGetPackedR32(c1);
            int dg = SkGetPackedG32(c0) - SkGetPackedG32(c1);
            int db = SkGetPackedB32(c0) - SkGetPackedB32(c1);

            *(outPixelDiffBitmap->getAddr32(x, y)) =
                    SkPackARGB32(0xFF, SkAbs32(dr), SkAbs32(dg), SkAbs32(db));

            if (dr != 0 || dg != 0 || db != 0) {
                *(outHighlightDiffBitmap->getAddr32(x, y)) = SK_ColorWHITE;
            } else {
                *(outHighlightDiffBitmap->getAddr32(x, y)) = SK_ColorBLACK;
            }
        }
    }
}

}  // namespace

class FontationsFtCompareGM : public GM {
public:
    FontationsFtCompareGM(std::string testName,
                          std::string fontNameFilterRegexp,
                          std::string langFilterRegexp)
            : fTestDataIterator(fontNameFilterRegexp, langFilterRegexp)
            , fTestName(testName.c_str()) {
        this->setBGColor(SK_ColorWHITE);
    }

protected:
    SkString getName() const override {
        return SkStringPrintf("fontations_compare_ft_%s", fTestName.c_str());
    }

    SkISize getISize() override {
        TestFontDataProvider::TestSet testSet;
        fTestDataIterator.rewind();
        fTestDataIterator.next(&testSet);

        return SkISize::Make(kGmWidth,
                             testSet.langSamples.size() * kFontSize * kLangYIncrementScale + 100);
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);

        fTestDataIterator.rewind();
        TestFontDataProvider::TestSet testSet;

        while (fTestDataIterator.next(&testSet)) {
            sk_sp<SkTypeface> testTypeface = SkTypeface_Make_Fontations(
                    SkStream::MakeFromFile(testSet.fontFilename.c_str()), SkFontArguments());
            sk_sp<SkTypeface> ftTypeface = SkTypeface_FreeType::MakeFromStream(
                    SkStream::MakeFromFile(testSet.fontFilename.c_str()), SkFontArguments());

            if (!testTypeface || !ftTypeface) {
                *errorMsg = "Unable to initialize typeface.";
                return DrawResult::kSkip;
            }

            auto configureFont = [](SkFont& font) {
                font.setSize(kFontSize);
                font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
                font.setSubpixel(true);
                font.setHinting(SkFontHinting::kNone);
            };

            SkFont font(testTypeface);
            configureFont(font);

            SkFont ftFont(ftTypeface);
            configureFont(ftFont);
            enum class DrawPhase { Fontations, FreeType, Comparison };

            SkRect maxBounds = SkRect::MakeEmpty();
            for (auto phase : {DrawPhase::Fontations, DrawPhase::FreeType, DrawPhase::Comparison}) {
                SkScalar yCoord = kFontSize * 1.5f;

                for (auto& langEntry : testSet.langSamples) {
                    auto shapeAndDrawToCanvas = [canvas, paint, langEntry](const SkFont& font,
                                                                           SkPoint coord) {
                        std::string testString(langEntry.sampleShort.c_str(),
                                               langEntry.sampleShort.size());
                        SkTextBlobBuilderRunHandler textBlobBuilder(testString.c_str(), {0, 0});
                        std::unique_ptr<SkShaper> shaper = SkShaper::Make();
                        shaper->shape(testString.c_str(),
                                      testString.size(),
                                      font,
                                      true,
                                      999999, /* Don't linebreak. */
                                      &textBlobBuilder);
                        sk_sp<const SkTextBlob> blob = textBlobBuilder.makeBlob();
                        canvas->drawTextBlob(blob.get(), coord.x(), coord.y(), paint);
                        return blob->bounds();
                    };

                    auto roundToDevicePixels = [canvas](SkPoint& point) {
                        SkMatrix ctm = canvas->getLocalToDeviceAs3x3();
                        SkPoint mapped = ctm.mapPoint(point);
                        SkPoint mappedRounded =
                                SkPoint::Make(roundf(mapped.x()), roundf(mapped.y()));
                        SkMatrix inverse;
                        bool inverseExists = ctm.invert(&inverse);
                        SkASSERT(inverseExists);
                        if (inverseExists) {
                            point = inverse.mapPoint(mappedRounded);
                        }
                    };

                    auto fontationsCoord = [yCoord, roundToDevicePixels]() {
                        SkPoint fontationsCoord = SkPoint::Make(kMargin, yCoord);
                        roundToDevicePixels(fontationsCoord);
                        return fontationsCoord;
                    };

                    auto freetypeCoord = [yCoord, maxBounds, roundToDevicePixels]() {
                        SkPoint freetypeCoord = SkPoint::Make(
                                2 * kMargin + maxBounds.left() + maxBounds.width(), yCoord);
                        roundToDevicePixels(freetypeCoord);
                        return freetypeCoord;
                    };

                    switch (phase) {
                        case DrawPhase::Fontations: {
                            SkRect boundsFontations = shapeAndDrawToCanvas(font, fontationsCoord());
                            /* Determine maximum of column width across all language samples. */
                            boundsFontations.roundOut();
                            maxBounds.join(boundsFontations);
                            break;
                        }
                        case DrawPhase::FreeType: {
                            shapeAndDrawToCanvas(ftFont, freetypeCoord());
                            break;
                        }
                        case DrawPhase::Comparison: {
                            /* Read back pixels from equally sized rectangles from the space in
                             * SkCanvas where Fontations and FreeType sample texts were drawn,
                             * compare them using pixel comparisons similar to SkDiff, draw a
                             * comparison as faint pixel differences, and as an amplified
                             * visualization in which each differing pixel is drawn as white. */
                            SkPoint copyBoxFontationsCoord = fontationsCoord();
                            SkPoint copyBoxFreetypeCoord = freetypeCoord();
                            SkRect copyBoxFontations(maxBounds);
                            copyBoxFontations.roundOut(&copyBoxFontations);
                            SkRect copyBoxFreetype(copyBoxFontations);
                            copyBoxFontations.offset(copyBoxFontationsCoord.x(),
                                                     copyBoxFontationsCoord.y());
                            copyBoxFreetype.offset(copyBoxFreetypeCoord.x(),
                                                   copyBoxFreetypeCoord.y());

                            SkMatrix ctm = canvas->getLocalToDeviceAs3x3();
                            ctm.mapRect(&copyBoxFontations, copyBoxFontations);
                            ctm.mapRect(&copyBoxFreetype, copyBoxFreetype);

                            SkISize pixelDimensions(copyBoxFontations.roundOut().size());
                            SkImageInfo canvasImageInfo = canvas->imageInfo();
                            SkImageInfo copyImageInfo =
                                    SkImageInfo::Make(pixelDimensions,
                                                      canvasImageInfo.colorType(),
                                                      canvasImageInfo.alphaType());

                            SkBitmap fontationsBitmap, freetypeBitmap, diffBitmap,
                                    highlightDiffBitmap;
                            fontationsBitmap.allocPixels(copyImageInfo, 0);
                            freetypeBitmap.allocPixels(copyImageInfo, 0);
                            diffBitmap.allocPixels(copyImageInfo, 0);
                            highlightDiffBitmap.allocPixels(copyImageInfo, 0);

                            canvas->readPixels(
                                    fontationsBitmap, copyBoxFontations.x(), copyBoxFontations.y());
                            canvas->readPixels(
                                    freetypeBitmap, copyBoxFreetype.x(), copyBoxFreetype.y());

                            comparePixels(fontationsBitmap,
                                          freetypeBitmap,
                                          &diffBitmap,
                                          &highlightDiffBitmap);

                            /* Place comparison results as two extra columns, shift up to account
                               for placement of rectangles vs. SkTextBlobs (baseline shift). */
                            SkPoint comparisonCoord = ctm.mapPoint(SkPoint::Make(
                                    3 * kMargin + maxBounds.width() * 2, yCoord + maxBounds.top()));
                            SkPoint whiteCoord = ctm.mapPoint(SkPoint::Make(
                                    4 * kMargin + maxBounds.width() * 3, yCoord + maxBounds.top()));

                            canvas->writePixels(
                                    diffBitmap, comparisonCoord.x(), comparisonCoord.y());
                            canvas->writePixels(
                                    highlightDiffBitmap, whiteCoord.x(), whiteCoord.y());
                            break;
                        }
                    }

                    yCoord += font.getSize() * kLangYIncrementScale;
                }
            }
        }

        return DrawResult::kOk;
    }

private:
    using INHERITED = GM;

    TestFontDataProvider fTestDataIterator;
    SkString fTestName;
    sk_sp<SkTypeface> fReportTypeface;
    std::unique_ptr<SkFontArguments::VariationPosition::Coordinate[]> fCoordinates;
};

DEF_GM(return new FontationsFtCompareGM(
        "NotoSans",
        "Noto Sans",
        "en_Latn|es_Latn|pt_Latn|id_Latn|ru_Cyrl|fr_Latn|tr_Latn|vi_Latn|de_"
        "Latn|it_Latn|pl_Latn|nl_Latn|uk_Cyrl|gl_Latn|ro_Latn|cs_Latn|hu_Latn|"
        "el_Grek|se_Latn|da_Latn|bg_Latn|sk_Latn|fi_Latn|bs_Latn|ca_Latn|no_"
        "Latn|sr_Latn|sr_Cyrl|lt_Latn|hr_Latn|sl_Latn|uz_Latn|uz_Cyrl|lv_Latn|"
        "et_Latn|az_Latn|az_Cyrl|la_Latn|tg_Latn|tg_Cyrl|sw_Latn|mn_Cyrl|kk_"
        "Latn|kk_Cyrl|sq_Latn|af_Latn|ha_Latn|ky_Cyrl"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Deva",
                                        "Noto Sans Devanagari",
                                        "hi_Deva|mr_Deva"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_ar_Arab",
                                        "Noto Sans Arabic",
                                        "ar_Arab|uz_Arab|kk_Arab|ky_Arab"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Beng", "Noto Sans Bengali", "bn_Beng"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Jpan", "Noto Sans JP", "ja_Jpan"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Thai", "Noto Sans Thai", "th_Thai"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Hans", "Noto Sans SC", "zh_Hans"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Hant", "Noto Sans TC", "zh_Hant"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Kore", "Noto Sans KR", "ko_Kore"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Taml", "Noto Sans Tamil", "ta_Taml"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Newa", "Noto Sans Newa", "new_Newa"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Knda", "Noto Sans Kannada", "kn_Knda"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Tglg", "Noto Sans Tagalog", "fil_Tglg"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Telu", "Noto Sans Telugu", "te_Telu"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Gujr", "Noto Sans Gujarati", "gu_Gujr"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Geor", "Noto Sans Georgian", "ka_Geor"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Mlym", "Noto Sans Malayalam", "ml_Mlym"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Khmr", "Noto Sans Khmer", "km_Khmr"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Sinh", "Noto Sans Sinhala", "si_Sinh"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Mymr", "Noto Sans Myanmar", "my_Mymr"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Java", "Noto Sans Javanese", "jv_Java"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Mong", "Noto Sans Mongolian", "mn_Mong"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Armn", "Noto Sans Armenian", "hy_Armn"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Elba", "Noto Sans Elbasan", "sq_Elba"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Vith", "Noto Sans Vithkuqi", "sq_Vith"));

DEF_GM(return new FontationsFtCompareGM("NotoSans_Guru", "Noto Sans Gurmukhi", "pa_Guru"));

}  // namespace skiagm
