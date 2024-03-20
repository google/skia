// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/skplaintexteditor/src/shape.h"

#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTFitsIn.h"
#include "modules/skplaintexteditor/src/word_boundaries.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/base/SkUTF.h"
#include "src/core/SkTextBlobPriv.h"

#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
#include "modules/skshaper/include/SkShaper_harfbuzz.h"
#include "modules/skshaper/include/SkShaper_skunicode.h"
#include "modules/skunicode/include/SkUnicode.h"
#endif

#include <cfloat>
#include <climits>
#include <cstring>


using namespace SkPlainTextEditor;

namespace {
class RunHandler final : public SkShaper::RunHandler {
public:
    RunHandler(const char* utf8Text, size_t) : fUtf8Text(utf8Text) {}
    using RunCallback = void (*)(void* context,
                                 const char* utf8Text,
                                  size_t utf8TextBytes,
                                  size_t glyphCount,
                                  const SkGlyphID* glyphs,
                                  const SkPoint* positions,
                                  const uint32_t* clusters,
                                  const SkFont& font);
    void setRunCallback(RunCallback f, void* context) {
        fCallbackContext = context;
        fCallbackFunction = f;
    }

    sk_sp<SkTextBlob> makeBlob();
    SkPoint endPoint() const { return fOffset; }
    SkPoint finalPosition() const { return fCurrentPosition; }

    void beginLine() override;
    void runInfo(const RunInfo&) override;
    void commitRunInfo() override;
    SkShaper::RunHandler::Buffer runBuffer(const RunInfo&) override;
    void commitRunBuffer(const RunInfo&) override;
    void commitLine() override;

    const std::vector<size_t>& lineEndOffsets() const { return fLineEndOffsets; }

    SkRect finalRect(const SkFont& font) const {
        if (0 == fMaxRunAscent || 0 == fMaxRunDescent) {
            SkFontMetrics metrics;
            font.getMetrics(&metrics);
            return {fCurrentPosition.x(),
                    fCurrentPosition.y(),
                    fCurrentPosition.x() + font.getSize(),
                    fCurrentPosition.y() + metrics.fDescent - metrics.fAscent};
        } else {
            return {fCurrentPosition.x(),
                    fCurrentPosition.y() + fMaxRunAscent,
                    fCurrentPosition.x() + font.getSize(),
                    fCurrentPosition.y() + fMaxRunDescent};
        }
    }


private:
    SkTextBlobBuilder fBuilder;
    std::vector<size_t> fLineEndOffsets;
    const SkGlyphID* fCurrentGlyphs = nullptr;
    const SkPoint* fCurrentPoints = nullptr;
    void* fCallbackContext = nullptr;
    RunCallback fCallbackFunction = nullptr;
    char const * const fUtf8Text;
    size_t fTextOffset = 0;
    uint32_t* fClusters = nullptr;
    int fClusterOffset = 0;
    int fGlyphCount = 0;
    SkScalar fMaxRunAscent = 0;
    SkScalar fMaxRunDescent = 0;
    SkScalar fMaxRunLeading = 0;
    SkPoint fCurrentPosition = {0, 0};
    SkPoint fOffset = {0, 0};
};
}  // namespace

void RunHandler::beginLine() {
    fCurrentPosition = fOffset;
    fMaxRunAscent = 0;
    fMaxRunDescent = 0;
    fMaxRunLeading = 0;
}

void RunHandler::runInfo(const SkShaper::RunHandler::RunInfo& info) {
    SkFontMetrics metrics;
    info.fFont.getMetrics(&metrics);
    fMaxRunAscent = std::min(fMaxRunAscent, metrics.fAscent);
    fMaxRunDescent = std::max(fMaxRunDescent, metrics.fDescent);
    fMaxRunLeading = std::max(fMaxRunLeading, metrics.fLeading);
}

void RunHandler::commitRunInfo() {
    fCurrentPosition.fY -= fMaxRunAscent;
}

SkShaper::RunHandler::Buffer RunHandler::runBuffer(const RunInfo& info) {
    int glyphCount = SkTFitsIn<int>(info.glyphCount) ? info.glyphCount : INT_MAX;
    int utf8RangeSize = SkTFitsIn<int>(info.utf8Range.size()) ? info.utf8Range.size() : INT_MAX;

    const auto& runBuffer = fBuilder.allocRunTextPos(info.fFont, glyphCount, utf8RangeSize);
    fCurrentGlyphs = runBuffer.glyphs;
    fCurrentPoints = runBuffer.points();

    if (runBuffer.utf8text && fUtf8Text) {
        memcpy(runBuffer.utf8text, fUtf8Text + info.utf8Range.begin(), utf8RangeSize);
    }
    fClusters = runBuffer.clusters;
    fGlyphCount = glyphCount;
    fClusterOffset = info.utf8Range.begin();

    return {runBuffer.glyphs,
            runBuffer.points(),
            nullptr,
            runBuffer.clusters,
            fCurrentPosition};
}

void RunHandler::commitRunBuffer(const RunInfo& info) {
    // for (size_t i = 0; i < info.glyphCount; ++i) {
    //     SkASSERT(fClusters[i] >= info.utf8Range.begin());
    //     // this fails for khmer example.
    //     SkASSERT(fClusters[i] <  info.utf8Range.end());
    // }
    if (fCallbackFunction) {
        fCallbackFunction(fCallbackContext,
                          fUtf8Text,
                          info.utf8Range.end(),
                          info.glyphCount,
                          fCurrentGlyphs,
                          fCurrentPoints,
                          fClusters,
                          info.fFont);
    }
    SkASSERT(0 <= fClusterOffset);
    for (int i = 0; i < fGlyphCount; ++i) {
        SkASSERT(fClusters[i] >= (unsigned)fClusterOffset);
        fClusters[i] -= fClusterOffset;
    }
    fCurrentPosition += info.fAdvance;
    fTextOffset = std::max(fTextOffset, info.utf8Range.end());
}

void RunHandler::commitLine() {
    if (fLineEndOffsets.empty() || fTextOffset > fLineEndOffsets.back()) {
        // Ensure that fLineEndOffsets is monotonic.
        fLineEndOffsets.push_back(fTextOffset);
    }
    fOffset += { 0, fMaxRunDescent + fMaxRunLeading - fMaxRunAscent };
}

sk_sp<SkTextBlob> RunHandler::makeBlob() {
    return fBuilder.make();
}

static SkRect selection_box(const SkFontMetrics& metrics,
                            float advance,
                            SkPoint pos) {
    if (fabsf(advance) < 1.0f) {
        advance = copysignf(1.0f, advance);
    }
    return SkRect{pos.x(),
                  pos.y() + metrics.fAscent,
                  pos.x() + advance,
                  pos.y() + metrics.fDescent}.makeSorted();
}

static void set_character_bounds(void* context,
                                 const char* utf8Text,
                                 size_t utf8TextBytes,
                                 size_t glyphCount,
                                 const SkGlyphID* glyphs,
                                 const SkPoint* positions,
                                 const uint32_t* clusters,
                                 const SkFont& font)
{
    SkASSERT(context);
    SkASSERT(glyphCount > 0);
    SkRect* cursors = (SkRect*)context;

    SkFontMetrics metrics;
    font.getMetrics(&metrics);
    std::unique_ptr<float[]> advances(new float[glyphCount]);
    font.getWidths(glyphs, glyphCount, advances.get());

    // Loop over each cluster in this run.
    size_t clusterStart = 0;
    for (size_t glyphIndex = 0; glyphIndex < glyphCount; ++glyphIndex) {
        if (glyphIndex + 1 < glyphCount  // more glyphs
            && clusters[glyphIndex] == clusters[glyphIndex + 1]) {
            continue; // multi-glyph cluster
        }
        unsigned textBegin = clusters[glyphIndex];
        unsigned textEnd = utf8TextBytes;
        for (size_t i = 0; i < glyphCount; ++i) {
            if (clusters[i] >= textEnd) {
                textEnd = clusters[i] + 1;
            }
        }
        for (size_t i = 0; i < glyphCount; ++i) {
            if (clusters[i] > textBegin && clusters[i] < textEnd) {
                textEnd = clusters[i];
                if (textEnd == textBegin + 1) { break; }
            }
        }
        SkASSERT(glyphIndex + 1 > clusterStart);
        unsigned clusterGlyphCount = glyphIndex + 1 - clusterStart;
        const SkPoint* clusterGlyphPositions = &positions[clusterStart];
        const float* clusterAdvances = &advances[clusterStart];
        clusterStart = glyphIndex + 1;  // for next loop

        SkRect clusterBox = selection_box(metrics, clusterAdvances[0], clusterGlyphPositions[0]);
        for (unsigned i = 1; i < clusterGlyphCount; ++i) { // multiple glyphs
            clusterBox.join(selection_box(metrics, clusterAdvances[i], clusterGlyphPositions[i]));
        }
        if (textBegin + 1 == textEnd) {  // single byte, fast path.
            cursors[textBegin] = clusterBox;
            continue;
        }
        int textCount = textEnd - textBegin;
        int codePointCount = SkUTF::CountUTF8(utf8Text + textBegin, textCount);
        if (codePointCount == 1) {  // single codepoint, fast path.
            cursors[textBegin] = clusterBox;
            continue;
        }

        float width = clusterBox.width() / codePointCount;
        SkASSERT(width > 0);
        const char* ptr = utf8Text + textBegin;
        const char* end = utf8Text + textEnd;
        float x = clusterBox.left();
        while (ptr < end) {  // for each codepoint in cluster
            const char* nextPtr = ptr;
            SkUTF::NextUTF8(&nextPtr, end);
            int firstIndex = ptr - utf8Text;
            float nextX = x + width;
            cursors[firstIndex] = SkRect{x, clusterBox.top(), nextX, clusterBox.bottom()};
            x = nextX;
            ptr = nextPtr;
        }
    }
}

ShapeResult SkPlainTextEditor::Shape(const char* utf8Text,
                                     size_t textByteLen,
                                     const SkFont& font,
                                     sk_sp<SkFontMgr> fontMgr,
                                     const char* locale,
                                     float width)
{
    ShapeResult result;
    if (SkUTF::CountUTF8(utf8Text, textByteLen) < 0) {
        utf8Text = nullptr;
        textByteLen = 0;
    }
    std::unique_ptr<SkShaper> shaper = nullptr;
#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
    auto unicode = SkUnicode::Make();
    shaper = SkShapers::HB::ShaperDrivenWrapper(std::move(unicode), fontMgr);
#else
    shaper = SkShapers::Primitive();
#endif
    SkASSERT(shaper);

    float height = font.getSpacing();
    RunHandler runHandler(utf8Text, textByteLen);
    if (textByteLen) {
        result.glyphBounds.resize(textByteLen);
        for (SkRect& c : result.glyphBounds) {
            c = SkRect{-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};
        }
        runHandler.setRunCallback(set_character_bounds, result.glyphBounds.data());

        static constexpr uint8_t kBidiLevelLTR = 0;
        std::unique_ptr<SkShaper::BiDiRunIterator> bidi = nullptr;
#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
        auto unicode2 = SkUnicode::Make();
        bidi = SkShapers::unicode::BidiRunIterator(
                unicode2.get(), utf8Text, textByteLen, kBidiLevelLTR);
#endif
        if (!bidi) {
            bidi = std::make_unique<SkShaper::TrivialBiDiRunIterator>(kBidiLevelLTR, textByteLen);
        }
        SkASSERT(bidi);

        std::unique_ptr<SkShaper::LanguageRunIterator> language =
                SkShaper::MakeStdLanguageRunIterator(utf8Text, textByteLen);
        SkASSERT(language);

        std::unique_ptr<SkShaper::ScriptRunIterator> script =
                SkShapers::HB::ScriptRunIterator(utf8Text, textByteLen);
        SkASSERT(script);

        std::unique_ptr<SkShaper::FontRunIterator> fontRuns =
                SkShaper::MakeFontMgrRunIterator(utf8Text, textByteLen, font, fontMgr);
        SkASSERT(fontRuns);

        shaper->shape(utf8Text,
                      textByteLen,
                      *fontRuns,
                      *bidi,
                      *script,
                      *language,
                      nullptr,
                      0,
                      width,
                      &runHandler);
        if (runHandler.lineEndOffsets().size() > 1) {
            result.lineBreakOffsets = runHandler.lineEndOffsets();
            SkASSERT(result.lineBreakOffsets.size() > 0);
            result.lineBreakOffsets.pop_back();
        }
        height = std::max(height, runHandler.endPoint().y());
        result.blob = runHandler.makeBlob();
    }
    result.glyphBounds.push_back(runHandler.finalRect(font));
    result.verticalAdvance = (int)ceilf(height);
    result.wordBreaks = GetUtf8WordBoundaries(utf8Text, textByteLen, locale);
    return result;
}
