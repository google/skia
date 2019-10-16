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

static int utf8_byte_type(uint8_t c) {
    if (c < 0x80) {
        return 1;
    } else if (c < 0xC0) {
        return 0;
    } else if (c >= 0xF5 || (c & 0xFE) == 0xC0) { // "octet values c0, c1, f5 to ff never appear"
        return -1;
    } else {
        int value = (((0xe5 << 24) >> ((unsigned)c >> 4 << 1)) & 3) + 1;
        return value;
    }
}

SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}

SkUnichar utf8_prev(const char** ptr, const char* begin, const char* end) {

    for (auto bytes = 1; bytes <= SkTMin(4l, *ptr - begin); ++bytes) {
        auto prev = *ptr - bytes;
        auto byte = *(const uint8_t*)(prev);
        auto type = utf8_byte_type(byte);
        if (type <= 0) {
            // Invalid or continuation
            continue;
        }
        if (type == bytes) {
            // We read exactly as many bytes as the code point takes
            auto dummy = &prev;
            auto result = SkUTF::NextUTF8(dummy, end);
            *ptr = prev;
            return result;
        }
    }
    return -1;
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

        // Here comes our unresolved block
        if (addUnresolvedWithRun(block)) {
            // Some text (left of our unresolved block) was resolved
            auto last = fUnresolvedBlocks.back().fGlyphs;
            if (firstResolvedGlyph < last.start) {
                addResolved(GlyphRange(firstResolvedGlyph, last.start));
            }
            firstResolvedGlyph = last.end;
        } else {
            auto last = fUnresolvedBlocks.back().fGlyphs;
            firstResolvedGlyph = last.end;
        }
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

    //printState();

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
            //auto clusterIndex = block.fRun->fClusterIndexes.front();
            //SkDebugf("Finish1 [%d:%d) @%d + %d\n", text.start, text.end, block.fRun->fClusterStart, clusterIndex);
            block.fRun->fIndex = this->fParagraph->fRuns.size();
            this->fParagraph->fRuns.emplace_back(std::move(*block.fRun));
            continue;
        } else if (run == nullptr) {
            //SkDebugf("Finish0 [%d:%d)\n", text.start, text.end);
            continue;
        }

        auto runAdvance = SkVector::Make(
                run->fPositions[glyphs.end].fX - run->fPositions[glyphs.start].fX,
                run->fAdvance.fY);
        const SkShaper::RunHandler::RunInfo info = {
                run->fFont, run->fBidiLevel, runAdvance, glyphs.width(),
                // TODO: Correct it by first char index
                SkShaper::RunHandler::Range(text.start - run->fClusterStart, text.width())};
        this->fParagraph->fRuns.emplace_back(
                    this->fParagraph,
                    info,
                    run->fClusterStart,
                    height,
                    this->fParagraph->fRuns.count(),
                    advanceX
                );
        auto piece = &this->fParagraph->fRuns.back();

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
        //auto clusterIndex = piece->fClusterIndexes.front();
        //SkDebugf("Finish2 [%d:%d) @%d + %d\n", text.start, text.end, piece->fClusterStart, clusterIndex);

        // Carve out the line text out of the entire run text
        fAdvance.fX += runAdvance.fX;
        fAdvance.fY = SkMaxScalar(fAdvance.fY, runAdvance.fY);
    };

    advanceX = fAdvance.fX;
    if (lastTextEnd != blockText.end) {
        SkDebugf("Last range mismatch: %d - %d\n", lastTextEnd, blockText.end);
    }
}

void OneLineShaper::increment(TextIndex& index) {
    auto text = fCurrentRun->fMaster->text();
    auto cluster = text.begin() + index;

    if (cluster < text.end()) {
        utf8_next(&cluster, text.end());
        index = cluster - text.begin();
    }
}

// Make it [left:right) regardless of a text direction
TextRange OneLineShaper::normalizeTextRange(GlyphRange glyphRange) {
    TextRange textRange(fTextStart + fCurrentRun->fClusterIndexes[glyphRange.start],
                        fTextStart + fCurrentRun->fClusterIndexes[glyphRange.end]);
    if (!fCurrentRun->leftToRight()) {
        std::swap(textRange.start, textRange.end);
        if (textRange.end != fCurrentRun->fTextRange.end) {
            increment(textRange.end);
        }
        if (textRange.start != fCurrentRun->fTextRange.start) {
            increment(textRange.start);
        }
    }

    return textRange;
}

void OneLineShaper::addResolved(GlyphRange glyphRange) {
    if (glyphRange.width() == 0) {
        return;
    }
    ClusterRange clusterRange(normalizeTextRange(glyphRange));
    RunBlock resolved(fCurrentRun, clusterRange, glyphRange);
    fResolvedBlocks.emplace_back(resolved);
    //SkDebugf("addResolved: [%d:%d) -> [%d:%d)\n", glyphRange.start, glyphRange.end, clusterRange.start, clusterRange.end);
}

bool OneLineShaper::addUnresolved(GlyphRange glyphRange) {
    if (glyphRange.width() == 0) {
        return false;
    }

    RunBlock unresolved(fCurrentRun, clusteredText(glyphRange));
    if (!fUnresolvedBlocks.empty()) {
        auto& lastUnresolved = fUnresolvedBlocks.back();
        if (lastUnresolved.fRun == nullptr &&
            lastUnresolved.fText.end == unresolved.fText.start) {
            // We can merge 2 unresolved items
            lastUnresolved.fText.end = unresolved.fText.end;
            return false;
        }
    }
    fUnresolvedBlocks.emplace(unresolved);
    return true;
}

bool OneLineShaper::addUnresolvedWithRun(GlyphRange glyphRange) {
    if (glyphRange.width() == 0) {
        return false;
    }

    RunBlock unresolved(fCurrentRun, clusteredText(glyphRange), glyphRange);
    if (!fUnresolvedBlocks.empty()) {
        auto& lastUnresolved = fUnresolvedBlocks.back();
        if (lastUnresolved.fRun != nullptr &&
            lastUnresolved.fRun->fIndex == fCurrentRun->fIndex &&
            lastUnresolved.fText.end == unresolved.fText.start) {
            // We can merge 2 unresolved items
            lastUnresolved.fText.end = unresolved.fText.end;
            lastUnresolved.fGlyphs.end = glyphRange.end;
            //SkDebugf("addUnresolvedWithRun: [%d:%d) +> [%d:%d)\n",
            //        lastUnresolved.fGlyphs.start, lastUnresolved.fGlyphs.end, lastUnresolved.fText.start, lastUnresolved.fText.end);
            return true;
        }
    }
    fUnresolvedBlocks.emplace(unresolved);
    //SkDebugf("addUnresolvedWithRun: [%d:%d) -> [%d:%d)\n", glyphRange.start, glyphRange.end, unresolved.fText.start, unresolved.fText.end);
    return true;
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

void OneLineShaper::iterateThroughFontStyles(SkSpan<Block> styleSpan,
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
            (SkSpan<const char> textSpan, SkSpan<Block> styleSpan, SkScalar& advanceX, TextIndex textStart) {

        // Set up the shaper and shape the next
        auto shaper = SkShaper::MakeShapeDontWrapOrReorder();
        SkASSERT_RELEASE(shaper != nullptr);

        iterateThroughFontStyles(styleSpan, [this, &shaper, textDirection, limitlessWidth,
                                             textStart, &advanceX](Block block) {
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
            fTextRange = block.fRange;
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

                    fTextStart = unresolvedRange.start;
                    shaper->shape(unresolvedText.begin(), unresolvedText.size(), fontIter, *bidi,
                                  *script, lang, limitlessWidth, this);

                    // Check if we actually resolved something
                    if (fUnresolvedBlocks.size() > count &&
                            fUnresolvedBlocks.front().fText.width() == unresolvedRange.width()) {
                        // The entire block remains unresolved!
                        if (fUnresolvedBlocks.front().fRun != nullptr) {
                            fUnresolvedBlocks.back().fRun = fUnresolvedBlocks.front().fRun;
                        }
                    }
                    this->dropUnresolved();
                }

                // Continue until we resolved all the code points
                return fUnresolvedBlocks.size() > 0;
            });

            this->finish(block.fRange, textStart, block.fStyle.getHeight(), advanceX);
        });

        return true;
    });

    return result;
}

TextRange OneLineShaper::clusteredText(GlyphRange glyphs) {

    enum class Dir { left, right };
    enum class Pos { inclusive, exclusive };

    TextRange text(fCurrentRun->clusterIndex(glyphs.start), fCurrentRun->clusterIndex(glyphs.end));

    // [left: right)
    auto findBaseChar = [&](TextIndex index, Dir dir) -> TextIndex {
        auto text = fParagraph->text(fCurrentRun->fTextRange);
        const char* cluster = fParagraph->text().begin() + index;
        if (dir == Dir::right) {
            while (cluster < text.end()) {
                auto result = cluster;
                auto codepoint = utf8_next(&cluster, text.end());
                if (is_base(codepoint)) {
                    return result - fParagraph->text().begin();
                }
            }
            return fCurrentRun->fTextRange.end;
        } else {
            const char* current = cluster;
            auto codepoint = utf8_next(&current, text.end());
            if (is_base(codepoint)) {
                return index;
            }
            while (cluster < text.end()) {
                codepoint = utf8_prev(&cluster, text.begin(), text.end());
                if (is_base(codepoint)) {
                    return cluster - fParagraph->text().begin();
                }
            }
            return fCurrentRun->fTextRange.start;
        }
    };

    TextRange textRange(normalizeTextRange(glyphs));
    textRange.start = findBaseChar(textRange.start, Dir::left);
    textRange.end = findBaseChar(textRange.end, Dir::right);

    //SkDebugf("ClusteredText([%d:%d))=[%d:%d)\n",
    //         glyphs.start, glyphs.end, textRange.start, textRange.end);
    return TextRange(textRange.start, textRange.end);
}
}
}