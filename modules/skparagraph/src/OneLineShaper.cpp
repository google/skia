// Copyright 2019 Google LLC.

#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/OneLineShaper.h"
#include "modules/skparagraph/src/Iterators.h"
#include <unicode/uchar.h>
#include <unicode/ustring.h>
#include <algorithm>
#include "src/utils/SkUTF.h"

namespace skia {
namespace textlayout {

namespace {

SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}

bool is_not_base(SkUnichar codepoint) {
    return u_hasBinaryProperty(codepoint, UCHAR_DIACRITIC) ||
           u_hasBinaryProperty(codepoint, UCHAR_EXTENDER);
}

bool is_base(SkUnichar codepoint) { return !is_not_base(codepoint); }

}

void OneLineShaper::commitRunBuffer(const RunInfo&) {
    GlyphIndex firstResolvedGlyph = 0;

    sortOutGlyphs([&](GlyphRange block){

        // Some text (left of our unresolved block) was resolved
        addResolved(GlyphRange(firstResolvedGlyph, block.start));
        // Here comes our unresolved block
        addUnresolvedWithRun(block);
        firstResolvedGlyph = block.end;
    });

    // Some text (right of the last unresolved block, but inside the run) was resolved
    addResolved(GlyphRange(firstResolvedGlyph, fCurrentRun->size()));
}

void OneLineShaper::printState() {
    SkDebugf("Resolved: %d\n", fResolvedBlocks.size());
    for (auto& resolved : fResolvedBlocks) {
        if (resolved.fRun ==  nullptr) {
            SkDebugf("[%d:%d) unresolved\n",
                    resolved.fText.start, resolved.fText.end);
            continue;
        }
        SkString name("???");
        if (resolved.fRun->fFont.getTypeface() != nullptr) {
            resolved.fRun->fFont.getTypeface()->getFamilyName(&name);
        }
        SkDebugf("[%d:%d) with %s\n",
                resolved.fText.start, resolved.fText.end,
                name.c_str());
    }

    auto size = fUnresolvedBlocks.size();
    SkDebugf("Unresolved: %d\n", size);
    for (size_t i = 0; i < size; ++i) {
        auto unresolved = fUnresolvedBlocks.front();
        fUnresolvedBlocks.pop();
        SkDebugf("[%d:%d)\n", unresolved.fText.start, unresolved.fText.end);
        fUnresolvedBlocks.emplace(unresolved);
    }
}

TextRange OneLineShaper::topUnresolved() {
    SkASSERT(!fUnresolvedBlocks.empty());
    return fUnresolvedBlocks.front().fText;
}

void OneLineShaper::dropUnresolved() {
    SkASSERT(!fUnresolvedBlocks.empty());
    fUnresolvedBlocks.pop();
}

void OneLineShaper::finish(TextRange blockText, size_t firstChar, SkScalar height, SkScalar& advanceX) {

    printState();

    // Add all unresolved blocks to resolved blocks
    while (!fUnresolvedBlocks.empty()) {
        auto unresolved = fUnresolvedBlocks.front();
        fUnresolvedBlocks.pop();
        fResolvedBlocks.emplace_back(unresolved);
    }

    // Sort all pieces by text
    std::sort(fResolvedBlocks.begin(), fResolvedBlocks.end(),
              [](const RunBlock& a, const RunBlock& b) {
                return a.fText.start < b.fText.start;
              });

    // Go through all of them
    size_t lastTextEnd = blockText.start;
    for (auto& block : fResolvedBlocks) {

        if (block.fText.end <= blockText.start) {
            continue;
        }

        auto run = block.fRun;
        auto glyphs = block.fGlyphs;
        auto text = block.fText;
        if (lastTextEnd != text.start) {
            SkDebugf("Text ranges mismatch: ...:%d] - [%d:%d] (%d-%d)\n", lastTextEnd, text.start, text.end,  glyphs.start, glyphs.end);
        }
        lastTextEnd = text.end;

        if (block.isFullyResolved()) {
            // Just move the entire run
            SkDebugf("Finish1 [%d:%d) @%d\n", text.start, text.end, block.fRun->fFirstChar);
            block.fRun->fIndex = this->fParagraph->fRuns.size();
            this->fParagraph->fRuns.emplace_back(std::move(*block.fRun));
            continue;
        } else if (run == nullptr) {
            SkDebugf("Finish0 [%d:%d)\n", text.start, text.end);
            continue;
        }

        auto runAdvance = SkVector::Make(
                run->fPositions[glyphs.end].fX - run->fPositions[glyphs.start].fX,
                run->fAdvance.fY);
        const SkShaper::RunHandler::RunInfo info = {
                run->fFont, run->fBidiLevel, runAdvance, glyphs.width(),
                // TODO: Correct it by first char index
                SkShaper::RunHandler::Range(text.start, text.width())};
        this->fParagraph->fRuns.emplace_back(
                    this->fParagraph,
                    info,
                    firstChar,
                    height,
                    this->fParagraph->fRuns.count(),
                    advanceX
                );
        auto piece = &this->fParagraph->fRuns.back();

        SkDebugf("Finish2 [%d:%d) @%d\n", text.start, text.end, piece->fFirstChar);
        // TODO: Optimize copying
        for (size_t i = glyphs.start; i <= glyphs.end; ++i) {

            auto index = i - glyphs.start;
            if (i < glyphs.end) {
                piece->fGlyphs[index] = run->fGlyphs[i];
            }
            piece->fClusterIndexes[index] = run->fClusterIndexes[i];
            auto position = run->fPositions[i];
            position.fX += advanceX;
            piece->fPositions[index] = position;
        }

        // Carve out the line text out of the entire run text
        fAdvance.fX += runAdvance.fX;
        fAdvance.fY = SkMaxScalar(fAdvance.fY, runAdvance.fY);
    };

    advanceX = fAdvance.fX;
    if (lastTextEnd != blockText.end) {
        SkDebugf("Last range mismatch: %d - %d\n", lastTextEnd, blockText.end);
    }
}

void OneLineShaper::addResolved(GlyphRange glyphRange) {
    if (glyphRange.width() == 0) {
        return;
    }
    RunBlock resolved(fCurrentRun, clusteredText(glyphRange), glyphRange);
    fResolvedBlocks.emplace_back(resolved);
}

void OneLineShaper::addUnresolved(GlyphRange glyphRange) {
    if (glyphRange.width() == 0) {
        return;
    }

    RunBlock unresolved(fCurrentRun, clusteredText(glyphRange));
    if (!fUnresolvedBlocks.empty()) {
        auto& lastUnresolved = fUnresolvedBlocks.back();
        if (lastUnresolved.fRun == nullptr &&
            lastUnresolved.fText.end == unresolved.fText.start) {
            // We can merge 2 unresolved items
            lastUnresolved.fText.end = unresolved.fText.end;
            return;
        }
    }
    fUnresolvedBlocks.emplace(unresolved);
}

void OneLineShaper::addUnresolvedWithRun(GlyphRange glyphRange) {
    if (glyphRange.width() == 0) {
        return;
    }

    RunBlock unresolved(fCurrentRun, clusteredText(glyphRange), glyphRange);
    if (!fUnresolvedBlocks.empty()) {
        auto& lastUnresolved = fUnresolvedBlocks.back();
        if (lastUnresolved.fRun != nullptr &&
            lastUnresolved.fRun->fIndex == fCurrentRun->fIndex &&
            lastUnresolved.fText.end == unresolved.fText.start) {
            // We can merge 2 unresolved items
            lastUnresolved.fText.end = unresolved.fText.end;
            return;
        }
    }
    fUnresolvedBlocks.emplace(unresolved);
}

void OneLineShaper::sortOutGlyphs(std::function<void(GlyphRange)>&& sortOutUnresolvedBLock) {

    auto text = fCurrentRun->fMaster->text();
    size_t unresolvedGlyphs = 0;

    GlyphRange block = EMPTY_RANGE;
    for (size_t i = 0; i < fCurrentRun->size(); ++i) {

        auto clusterIndex = fCurrentRun->fClusterIndexes[i];

        // Inspect the glyph
        auto glyph = fCurrentRun->fGlyphs[i];
        if (glyph != 0) {
            if (block.start == EMPTY_INDEX) {
                // Keep skipping resolved code points
                continue;
            }
            // This is the end of unresolved block
            block.end = i;
        } else {
            const char* cluster = text.begin() + clusterIndex;
            SkUnichar codepoint = utf8_next(&cluster, text.end());
            if (u_iscntrl(codepoint)) {
                // This codepoint does not have to be resolved; let's pretend it's resolved
                if (block.start == EMPTY_INDEX) {
                    // Keep skipping resolved code points
                    continue;
                }
                // This is the end of unresolved block
                block.end = i;
            } else {
                ++unresolvedGlyphs;
                if (block.start == EMPTY_INDEX) {
                    // Start new unresolved block
                    block.start = i;
                    block.end = EMPTY_INDEX;
                } else {
                    // Keep skipping unresolved block
                }
                continue;
            }
        }

        // Found an unresolved block
        sortOutUnresolvedBLock(block);
        block = EMPTY_RANGE;
    }

    // One last block could have been left
    if (block.start != EMPTY_INDEX) {
        block.end = fCurrentRun->size();
        sortOutUnresolvedBLock(block);
    }

}

void OneLineShaper::iterateThroughFonts(SkSpan<Block> styleSpan,
                                        ShapeSingleFontVisitor visitor) {

    Block combinedBlock;
    for (auto& block : styleSpan) {
        SkASSERT(combinedBlock.fRange.width() == 0 ||
                 combinedBlock.fRange.end == block.fRange.start);

        if (!combinedBlock.fRange.empty()) {
            if (block.fStyle.matchOneAttribute(StyleType::kFont, combinedBlock.fStyle)) {
                combinedBlock.add(block.fRange);
                continue;
            }
            // Resolve all characters in the block for this style
            visitor(combinedBlock);
        }

        combinedBlock.fRange = block.fRange;
        combinedBlock.fStyle = block.fStyle;
    }

    visitor(combinedBlock);
}

void OneLineShaper::matchResolvedFonts(const TextStyle& textStyle,
                                   SkUnichar unicode,
                                   TypefaceVisitor visitor) {
    for (auto& fontFamily : textStyle.getFontFamilies()) {
        auto typeface = fParagraph->fFontCollection->matchTypeface(
                fontFamily.c_str(), textStyle.getFontStyle(), textStyle.getLocale());
        if (typeface.get() == nullptr) {
            continue;
        }

        if (!visitor(typeface)) {
            return;
        }
    }

    auto typeface = fParagraph->fFontCollection->matchDefaultTypeface(textStyle.getFontStyle(),
                                                                      textStyle.getLocale());
    if (typeface != nullptr) {
        if (!visitor(typeface)) {
            return;
        }
    }

    if (fParagraph->fFontCollection->fontFallbackEnabled()) {
        typeface = fParagraph->fFontCollection->defaultFallback(
                unicode, textStyle.getFontStyle(), textStyle.getLocale());
    }
    if (typeface != nullptr) {
        if (!visitor(typeface)) {
            return;
        }
    }
}

bool OneLineShaper::iterateThroughShapingRegions(ShapeVisitor shape) {

    SkScalar advanceX = 0;
    for (auto& placeholder : fParagraph->fPlaceholders) {
        // Shape the text
        if (placeholder.fTextBefore.width() > 0) {
            // Set up the iterators
            SkSpan<const char> textSpan = fParagraph->text(placeholder.fTextBefore);
            SkSpan<Block> styleSpan(fParagraph->fTextStyles.begin() + placeholder.fBlocksBefore.start,
                                    placeholder.fBlocksBefore.width());

            if (!shape(textSpan, styleSpan, advanceX, placeholder.fTextBefore.start)) {
                return false;
            }
        }

        if (placeholder.fRange.width() == 0) {
            continue;
        }

        // Get the placeholder font
        sk_sp<SkTypeface> typeface = nullptr;
        for (auto& ff : placeholder.fTextStyle.getFontFamilies()) {
            typeface = fParagraph->fFontCollection->matchTypeface(ff.c_str(), placeholder.fTextStyle.getFontStyle(), placeholder.fTextStyle.getLocale());
            if (typeface != nullptr) {
                break;
            }
        }
        SkFont font(typeface, placeholder.fTextStyle.getFontSize());

        // "Shape" the placeholder
        const SkShaper::RunHandler::RunInfo runInfo = {
            font,
            (uint8_t)2,
            SkPoint::Make(placeholder.fStyle.fWidth, placeholder.fStyle.fHeight),
            1,
            SkShaper::RunHandler::Range(placeholder.fRange.start, placeholder.fRange.width())
        };
        auto& run = fParagraph->fRuns.emplace_back(this->fParagraph,
                                       runInfo,
                                       0,
                                       1.0f,
                                       fRuns.count(),
                                       advanceX);

        run.fPositions[0] = { advanceX, 0 };
        run.fClusterIndexes[0] = 0;
        run.fPlaceholder = &placeholder.fStyle;
        advanceX += placeholder.fStyle.fWidth;
    }
    return true;
}

bool OneLineShaper::shape() {

    // The text can be broken into many shaping sequences
    // (by place holders, possibly, by hard line breaks or tabs, too)
    uint8_t textDirection = fParagraph->fParagraphStyle.getTextDirection() == TextDirection::kLtr  ? 2 : 1;
    auto limitlessWidth = std::numeric_limits<SkScalar>::max();

    auto result = iterateThroughShapingRegions(
            [this, textDirection, limitlessWidth]
            (SkSpan<const char> textSpan, SkSpan<Block> styleSpan, SkScalar& advanceX, size_t start) {

        // Set up the shaper and shape the next
        auto shaper = SkShaper::MakeShapeDontWrapOrReorder();
        SkASSERT_RELEASE(shaper != nullptr);

        iterateThroughFonts(styleSpan, [this, &shaper, textDirection, limitlessWidth, start,
                                        &advanceX](Block block) {
            auto text = fParagraph->text(block.fRange);
            auto blockSpan = SkSpan<Block>(&block, 1);

            // In case we have fallback enabled give it a clue
            SkUnichar unicode = 0;
            if (fParagraph->fFontCollection->fontFallbackEnabled()) {
                const char* ch = text.begin();
                unicode = utf8_next(&ch, text.end());
            }

            // Start from the beginning (hoping that it's a simple case one block - one run)
            fHeight = block.fStyle.getHeight();
            fAdvance = SkVector::Make(advanceX, 0);
            fTextStart = block.fRange.start;
            fUnresolvedBlocks.emplace(RunBlock(block.fRange));

            matchResolvedFonts(block.fStyle, unicode, [&](sk_sp<SkTypeface> typeface) {
                // Create one more font to try
                SkFont font(typeface, block.fStyle.getFontSize());
                font.setEdging(SkFont::Edging::kAntiAlias);
                font.setHinting(SkFontHinting::kSlight);
                font.setSubpixel(true);

                // Walk through all the currently unresolved blocks
                // (ignoring those that appear later)
                auto count = fUnresolvedBlocks.size();
                while (count-- > 0) {
                    auto unresolvedRange = fUnresolvedBlocks.front().fText;
                    auto unresolvedText = fParagraph->text(unresolvedRange);

                    SingleFontIterator fontIter(unresolvedText, font);
                    LangIterator lang(unresolvedText, blockSpan,
                                      fParagraph->paragraphStyle().getTextStyle());
                    auto script = SkShaper::MakeHbIcuScriptRunIterator(unresolvedText.begin(),
                                                                       unresolvedText.size());
                    auto bidi = SkShaper::MakeIcuBiDiRunIterator(
                            unresolvedText.begin(), unresolvedText.size(), textDirection);
                    if (bidi == nullptr) {
                        return false;
                    }

                    SkString name;
                    typeface->getFamilyName(&name);
                    SkDebugf("Shape [%d:%d) with %s\n", unresolvedRange.start, unresolvedRange.end,
                             name.c_str());
                    fTextStart = unresolvedRange.start;
                    shaper->shape(unresolvedText.begin(), unresolvedText.size(), fontIter, *bidi,
                                  *script, lang, limitlessWidth, this);

                    this->dropUnresolved();
                }

                // Leave the iterator if we resolved all the codepoints
                return fUnresolvedBlocks.size() > 0;
            });

            this->finish(block.fRange, start, block.fStyle.getHeight(), advanceX);
        });

        return true;
    });

    return result;
}

TextRange OneLineShaper::clusteredText(GlyphRange glyphs) {

    auto text = fCurrentRun->fMaster->text();
    ClusterRange clusterRange;
    auto initial = glyphs;
    auto step = 1;
    GlyphRange limits(0, fCurrentRun->size());

    if (fCurrentRun->leftToRight()) {
        // Walk left until we find a base codepoint
        const char* cluster = text.begin();
        while (cluster < text.end()) {
            auto clusterIndex = fCurrentRun->clusterIndex(glyphs.start);
            cluster = text.begin() + clusterIndex;
            SkUnichar codepoint = utf8_next(&cluster, text.end());
            if (is_base(codepoint) || glyphs.start == limits.start) {
                break;
            }
            glyphs.start -= step;
        }

        // Find the first glyph in the left cluster
        clusterRange.start = fCurrentRun->clusterIndex(glyphs.start);
        while (glyphs.start != limits.start) {
             if (fCurrentRun->clusterIndex(glyphs.start) != clusterRange.start) {
                  glyphs.start += step;
                 break;
             }
            glyphs.start -= step;
        }

        // Walk right until we find a base codepoint
        cluster = text.begin();
        while (cluster < text.end()) {
            auto clusterIndex = fCurrentRun->clusterIndex(glyphs.end);
            cluster = text.begin() + clusterIndex;
            SkUnichar codepoint = utf8_next(&cluster, text.end());
            if (is_base(codepoint) || glyphs.end == limits.end) {
                break;
            }
            glyphs.end += step;
        };

        // Find the first glyph in the left cluster
        clusterRange.end = fCurrentRun->clusterIndex(glyphs.end);
        while (glyphs.end != limits.end) {
             if (fCurrentRun->clusterIndex(glyphs.end) != clusterRange.end) {
                 break;
             }
             glyphs.end += step;
        }
    } else {
        // Walk left until we find a base codepoint
        step = -1;
        std::swap(glyphs.start, glyphs.end);
        std::swap(limits.start, limits.end);
        const char* cluster = text.begin();
        glyphs.start += step;
        while (cluster < text.end()) {
            auto clusterIndex = fCurrentRun->clusterIndex(glyphs.start);
            cluster = text.begin() + clusterIndex;
            SkUnichar codepoint = utf8_next(&cluster, text.end());
            if (is_base(codepoint) || glyphs.start == limits.start) {
                break;
            }
            glyphs.start -= step;
        }

        // Find the first glyph in the left cluster
        clusterRange.start = fCurrentRun->clusterIndex(glyphs.start);
        while (glyphs.start != limits.start) {
            if (fCurrentRun->clusterIndex(glyphs.start) != clusterRange.start) {
                glyphs.start += step;
                break;
            }
            glyphs.start -= step;
        }

        // Walk right until we find a base codepoint
        cluster = text.begin();
        while (cluster < text.end()) {
            auto clusterIndex = fCurrentRun->clusterIndex(glyphs.end);
            cluster = text.begin() + clusterIndex;
            SkUnichar codepoint = utf8_next(&cluster, text.end());
            if (is_base(codepoint) || glyphs.end == limits.end) {
                break;
            }
            glyphs.end += step;
        }

        // Find the first glyph in the right cluster
        clusterRange.end = fCurrentRun->clusterIndex(glyphs.end == 0 ? fCurrentRun->size() : glyphs.end + step);
        while (glyphs.end != limits.end) {
            glyphs.end += step;
            if (fCurrentRun->clusterIndex(glyphs.end) != clusterRange.end) {
                glyphs.end -= step;
                break;
            }
        }
    }

    SkDebugf("ClusteredText([%d:%d))=[%d:%d)-[%d:%d)\n", initial.start, initial.end,
             glyphs.start, glyphs.end, fTextStart + clusterRange.start, fTextStart + clusterRange.end);
    return TextRange(fTextStart + clusterRange.start, fTextStart + clusterRange.end);
}
}
}