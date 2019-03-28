/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieShaper.h"

#include "SkShaper.h"
#include "SkTextBlob.h"
#include "SkTextBlobPriv.h"
#include "SkUTF.h"

namespace skottie {
namespace {

// Helper for interfacing with SkShaper: buffers shaper-fed runs and performs
// per-line position adjustments (for external line breaking, horizontal alignment, etc).
class BlobMaker final : public SkShaper::RunHandler {
public:
    BlobMaker(const Shaper::TextDesc& desc, const SkRect& box)
        : fDesc(desc)
        , fBox(box)
        , fAlignFactor(AlignFactor(fDesc.fAlign))
        , fFont(fDesc.fTypeface, fDesc.fTextSize)
        , fShaper(SkShaper::Make())
        , fCurrentOffset({fBox.x(), fBox.y()}) {
        fFont.setHinting(kNo_SkFontHinting);
        fFont.setSubpixel(true);
        fFont.setEdging(SkFont::Edging::kAntiAlias);
    }

    Buffer newRunBuffer(const RunInfo& info, const SkFont& font, size_t glyphCount,
                        Range) override {
        fPendingLineAdvance += info.fAdvance;

        if (!SkTFitsIn<int>(glyphCount)) {
            glyphCount = INT_MAX;
        }
        auto& run = fPendingLineRuns.emplace_back(font, info, glyphCount);

        return {
            run.fGlyphs   .data(),
            run.fPositions.data(),
            nullptr,
        };
    }

    void commitRun() override { }

    void commitLine() override {
        for (const auto& run : fPendingLineRuns) {
            const auto runSize = run.size();
            const auto& blobBuffer = fBuilder.allocRunPos(run.fFont, SkToInt(runSize));

            sk_careful_memcpy(blobBuffer.glyphs,
                              run.fGlyphs.data(),
                              runSize * sizeof(SkGlyphID));

            // Horizontal alignment.
            const auto h_adjust = fAlignFactor * (fPendingLineAdvance.x() - fBox.width());

            // When in point mode, the given position represents the baseline
            //   => we adjust for SkShaper which treats it as (baseline - ascent).
            const auto v_adjust = fBox.isEmpty() ? run.fInfo.fAscent : 0;

            for (size_t i = 0; i < runSize; ++i) {
                 blobBuffer.points()[i] = run.fPositions[SkToInt(i)]
                                        + SkVector::Make(h_adjust, v_adjust);
            }
        }

        fPendingLineRuns.reset();
        fPendingLineAdvance  = { 0, 0 };
    }

    sk_sp<SkTextBlob> makeBlob() {
        return fBuilder.make();
    }

    void shapeLine(const char* start, const char* end) {
        if (!fShaper) {
            return;
        }

        // When no text box is present, text is laid out on a single infinite line
        // (modulo explicit line breaks).
        const auto shape_width = fBox.isEmpty() ? SK_ScalarMax
                                                : fBox.width();

        fCurrentOffset = fShaper->shape(this, fFont, start, SkToSizeT(end - start),
                                        true, fCurrentOffset, shape_width);
    }

private:
    static float AlignFactor(SkTextUtils::Align align) {
        switch (align) {
        case SkTextUtils::kLeft_Align:   return  0.0f;
        case SkTextUtils::kCenter_Align: return -0.5f;
        case SkTextUtils::kRight_Align:  return -1.0f;
        }
        return 0.0f; // go home, msvc...
    }

    struct Run {
        SkFont                          fFont;
        SkShaper::RunHandler::RunInfo   fInfo;
        SkSTArray<128, SkGlyphID, true> fGlyphs;
        SkSTArray<128, SkPoint  , true> fPositions;

        Run(const SkFont& font, const SkShaper::RunHandler::RunInfo& info, int count)
            : fFont(font)
            , fInfo(info)
            , fGlyphs   (count)
            , fPositions(count) {
            fGlyphs   .push_back_n(count);
            fPositions.push_back_n(count);
        }

        size_t size() const {
            SkASSERT(fGlyphs.size() == fPositions.size());
            return fGlyphs.size();
        }
    };

    const Shaper::TextDesc&   fDesc;
    const SkRect&             fBox;
    const float               fAlignFactor;

    SkFont                    fFont;
    SkTextBlobBuilder         fBuilder;
    std::unique_ptr<SkShaper> fShaper;

    SkPoint                   fCurrentOffset;
    SkSTArray<2, Run, false>  fPendingLineRuns;
    SkVector                  fPendingLineAdvance = { 0, 0 };
};

Shaper::Result ShapeImpl(const SkString& txt, const Shaper::TextDesc& desc, const SkRect& box) {
    const auto& is_line_break = [](SkUnichar uch) {
        // TODO: other explicit breaks?
        return uch == '\r';
    };

    const char* ptr        = txt.c_str();
    const char* line_start = ptr;
    const char* end        = ptr + txt.size();

    BlobMaker blobMaker(desc, box);
    while (ptr < end) {
        if (is_line_break(SkUTF::NextUTF8(&ptr, end))) {
            blobMaker.shapeLine(line_start, ptr - 1);
            line_start = ptr;
        }
    }
    blobMaker.shapeLine(line_start, ptr);

    return {
        blobMaker.makeBlob(),
        { 0, 0 }               // the box offset is currently/temporarily baked into the blob.
    };
}

} // namespace

Shaper::Result Shaper::Shape(const SkString& txt, const TextDesc& desc, const SkPoint& point) {
    return ShapeImpl(txt, desc, SkRect::MakeEmpty().makeOffset(point.x(), point.y()));
}

Shaper::Result Shaper::Shape(const SkString& txt, const TextDesc& desc, const SkRect& box) {
    return ShapeImpl(txt, desc, box);
}

SkRect Shaper::Result::computeBounds() const {
    auto bounds = SkRect::MakeEmpty();

    if (!fBlob) {
        return bounds;
    }

    SkAutoSTArray<16, SkRect> glyphBounds;

    SkTextBlobRunIterator it(fBlob.get());

    for (SkTextBlobRunIterator it(fBlob.get()); !it.done(); it.next()) {
        glyphBounds.reset(SkToInt(it.glyphCount()));
        it.font().getBounds(it.glyphs(), it.glyphCount(), glyphBounds.get(), nullptr);

        SkASSERT(it.positioning() == SkTextBlobRunIterator::kFull_Positioning);
        for (uint32_t i = 0; i < it.glyphCount(); ++i) {
            bounds.join(glyphBounds[i].makeOffset(it.pos()[i * 2    ],
                                                  it.pos()[i * 2 + 1]));
        }
    }

    return bounds;
}

} // namespace skottie
