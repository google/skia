/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkTraceEvent.h"

#include "modules/svg/include/SkSVGDOM.h"

#include <filesystem>
#include <regex>
#include <string>
#include <vector>

#if !defined(OS_LINUX)
#include "include/ports/SkFontMgr_empty.h"
#endif

constexpr unsigned kPixelSize = 400;
constexpr float kFontSizeScale = 0.85f;

class ColrV1vsSVGBench : public Benchmark {
public:
    enum BenchMode { kSvg, kColrV1 };

    ColrV1vsSVGBench(BenchMode mode) : fMode(mode) {}

    const char* onGetName() override {
        fName.printf("colrv1vssvg_%s", fMode == kColrV1 ? "colrv1" : "svg");
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override { return backend == kRaster_Backend; }

    void onDelayedSetup() override {
        sk_sp<SkData> font_data = SkData::MakeFromFileName(
                "third-party/externals/color-fonts/fonts/noto-glyf_colr_1.ttf");
        sk_sp<SkData> data_in_memory = SkData::MakeWithCopy(font_data->bytes(), font_data->size());

#if !defined(OS_LINUX)
        auto font_mgr = sk_sp<SkFontMgr>(SkFontMgr_New_Custom_Empty());
#else
        auto font_mgr = sk_sp<SkFontMgr>(SkFontMgr::RefDefault());
#endif
        std::unique_ptr<SkStreamAsset> font_memory_stream =
                std::make_unique<SkMemoryStream>(data_in_memory);

        fNotoEmojiTypeface = font_mgr->makeFromStream(std::move(font_memory_stream));

        SkASSERT(fNotoEmojiTypeface);
        SkASSERT(fNotoEmojiTypeface->countGlyphs());
        fEmojiFont = SkFont(fNotoEmojiTypeface, kPixelSize * kFontSizeScale);

        std::string path = "third_party/externals/noto-emoji/svg/";
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            std::regex base_emoji_regex(".*emoji_u([0-9a-f]*).svg");
            std::string entry_path = entry.path().c_str();
            std::smatch emoji_codepoint_match;
            if (std::regex_match(entry_path, emoji_codepoint_match, base_emoji_regex)) {
                fFileList.push_back(entry_path);
                std::string codepoint_string = emoji_codepoint_match[1].str();

                SkGlyphID glyphId = fNotoEmojiTypeface->unicharToGlyph(strtoul(codepoint_string.c_str(), nullptr, 16));
                SkASSERT(glyphId);
                fGlyphIds.push_back(glyphId);
            }
        }

        SkASSERT(fFileList.size() == fGlyphIds.size());
        for (size_t i = 0; i < fFileList.size(); ++i) {
            sk_sp<SkData> svg_data = SkData::MakeFromFileName(fFileList[i].c_str());
            if (!svg_data) {
                printf("svg loading failed for %s\n", fFileList[i].c_str());
            }
            sk_sp<SkData> data_in_memory =
                    SkData::MakeWithCopy(svg_data->bytes(), svg_data->size());
            SkASSERT(data_in_memory->size());
            fSvgs.push_back(data_in_memory);
        }

        SkDebugf("Glyph count: %ld\n", fGlyphIds.size()); // 1369 SVG files matching the pattern.
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        switch (fMode) {
            case BenchMode::kSvg:
                onDrawSvg(loops, canvas);
                break;
            case BenchMode::kColrV1:
                onDrawColrV1(loops, canvas);
                break;
        }
    }

    SkIPoint onGetSize() override { return {kPixelSize + 100, kPixelSize + 100}; }

    void onPreDraw(SkCanvas* canvas) override { canvas->clear(SK_ColorWHITE); }

private:
    void drawSvgGlyph(int index, SkCanvas* canvas) {
        TRACE_EVENT0("skia", TRACE_FUNC);
        SkMemoryStream svg_stream(fSvgs[index]);
        sk_sp<SkSVGDOM> svgDom = SkSVGDOM::MakeFromStream(svg_stream);
        svgDom->setContainerSize(SkSize::Make(kPixelSize, kPixelSize));
        svgDom->render(canvas);
    }

    void drawColrV1Glyph(int index, SkCanvas* canvas) {
        TRACE_EVENT0("skia", TRACE_FUNC);
        const uint16_t testCodepoint = fGlyphIds[index];
        SkPaint paint;
        canvas->drawSimpleText(
                &testCodepoint, 2, SkTextEncoding::kGlyphID, 20, kPixelSize + 20, fEmojiFont, paint);
    }

      void onDrawSvg(int loops, SkCanvas* canvas) {
        for (int l = 0; l < loops; ++l) {
            SkASSERT(fSvgs.size() == fGlyphIds.size());
            for (size_t i = 0; i < fSvgs.size(); ++i) {
              drawSvgGlyph(i, canvas);
            }
        }
    }

    void onDrawColrV1(int loops, SkCanvas* canvas) {
        for (int l = 0; l < loops; ++l) {
            SkASSERT(fSvgs.size() == fGlyphIds.size());
            for (size_t i = 0; i < fGlyphIds.size(); ++i) {
              drawColrV1Glyph(i, canvas);
            }
        }
    }

    std::vector<std::string> fFileList;
    std::vector<SkGlyphID> fGlyphIds;

    BenchMode fMode;
    SkString fName;
    SkFont fEmojiFont;
    sk_sp<SkTypeface> fNotoEmojiTypeface;
    std::vector<sk_sp<SkData>> fSvgs;
};

DEF_BENCH(return new ColrV1vsSVGBench(ColrV1vsSVGBench::BenchMode::kSvg);)
DEF_BENCH(return new ColrV1vsSVGBench(ColrV1vsSVGBench::BenchMode::kColrV1);)
