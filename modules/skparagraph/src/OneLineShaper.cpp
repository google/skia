// Copyright 2019 Google LLC.

#include "modules/skparagraph/src/Iterators.h"
#include "modules/skparagraph/src/OneLineShaper.h"
#include "src/base/SkUTF.h"

#include <algorithm>
#include <cstdint>
#include <unordered_set>

using namespace skia_private;

namespace skia {
namespace textlayout {

void OneLineShaper::commitRunBuffer(const RunInfo&) {

    fCurrentRun->commit();

    auto oldUnresolvedCount = fUnresolvedBlocks.size();
/*
    SkDebugf("Run [%zu:%zu)\n", fCurrentRun->fTextRange.start, fCurrentRun->fTextRange.end);
    for (size_t i = 0; i < fCurrentRun->size(); ++i) {
        SkDebugf("[%zu] %hu %u %f\n", i, fCurrentRun->fGlyphs[i], fCurrentRun->fClusterIndexes[i], fCurrentRun->fPositions[i].fX);
    }
*/
    // Find all unresolved blocks
    sortOutGlyphs([&](GlyphRange block){
        if (block.width() == 0) {
            return;
        }
        addUnresolvedWithRun(block);
    });

    // Fill all the gaps between unresolved blocks with resolved ones
    if (oldUnresolvedCount == fUnresolvedBlocks.size()) {
        // No unresolved blocks added - we resolved the block with one run entirely
        addFullyResolved();
        return;
    } else if (oldUnresolvedCount == fUnresolvedBlocks.size() - 1) {
        auto& unresolved = fUnresolvedBlocks.back();
        if (fCurrentRun->textRange() == unresolved.fText) {
            // Nothing was resolved; preserve the initial run if it makes sense
            auto& front = fUnresolvedBlocks.front();
            if (front.fRun != nullptr) {
               unresolved.fRun = front.fRun;
               unresolved.fGlyphs = front.fGlyphs;
            }
            return;
        }
    }

    fillGaps(oldUnresolvedCount);
}

#ifdef SK_DEBUG
void OneLineShaper::printState() {
    SkDebugf("Resolved: %zu\n", fResolvedBlocks.size());
    for (auto& resolved : fResolvedBlocks) {
        if (resolved.fRun ==  nullptr) {
            SkDebugf("[%zu:%zu) unresolved\n",
                    resolved.fText.start, resolved.fText.end);
            continue;
        }
        SkString name("???");
        if (resolved.fRun->fFont.getTypeface() != nullptr) {
            resolved.fRun->fFont.getTypeface()->getFamilyName(&name);
        }
        SkDebugf("[%zu:%zu) ", resolved.fGlyphs.start, resolved.fGlyphs.end);
        SkDebugf("[%zu:%zu) with %s\n",
                resolved.fText.start, resolved.fText.end,
                name.c_str());
    }

    auto size = fUnresolvedBlocks.size();
    SkDebugf("Unresolved: %zu\n", size);
    for (const auto& unresolved : fUnresolvedBlocks) {
        SkDebugf("[%zu:%zu)\n", unresolved.fText.start, unresolved.fText.end);
    }
}
#endif

void OneLineShaper::fillGaps(size_t startingCount) {
    // Fill out gaps between all unresolved blocks
    TextRange resolvedTextLimits = fCurrentRun->fTextRange;
    if (!fCurrentRun->leftToRight()) {
        std::swap(resolvedTextLimits.start, resolvedTextLimits.end);
    }
    TextIndex resolvedTextStart = resolvedTextLimits.start;
    GlyphIndex resolvedGlyphsStart = 0;

    auto begin = fUnresolvedBlocks.begin();
    auto end = fUnresolvedBlocks.end();
    begin += startingCount; // Skip the old ones, do the new ones
    TextRange prevText = EMPTY_TEXT;
    for (; begin != end; ++begin) {
        auto& unresolved = *begin;

        if (unresolved.fText == prevText) {
            // Clean up repetitive blocks that appear inside the same grapheme block
            unresolved.fText = EMPTY_TEXT;
            continue;
        } else {
            prevText = unresolved.fText;
        }

        TextRange resolvedText(resolvedTextStart, fCurrentRun->leftToRight() ? unresolved.fText.start : unresolved.fText.end);
        if (resolvedText.width() > 0) {
            if (!fCurrentRun->leftToRight()) {
                std::swap(resolvedText.start, resolvedText.end);
            }

            GlyphRange resolvedGlyphs(resolvedGlyphsStart, unresolved.fGlyphs.start);
            RunBlock resolved(fCurrentRun, resolvedText, resolvedGlyphs, resolvedGlyphs.width());

            if (resolvedGlyphs.width() == 0) {
                // Extend the unresolved block with an empty resolved
                if (unresolved.fText.end <= resolved.fText.start) {
                    unresolved.fText.end = resolved.fText.end;
                }
                if (unresolved.fText.start >= resolved.fText.end) {
                    unresolved.fText.start = resolved.fText.start;
                }
            } else {
                fResolvedBlocks.emplace_back(resolved);
            }
        }
        resolvedGlyphsStart = unresolved.fGlyphs.end;
        resolvedTextStart =  fCurrentRun->leftToRight()
                                ? unresolved.fText.end
                                : unresolved.fText.start;
    }

    TextRange resolvedText(resolvedTextStart,resolvedTextLimits.end);
    if (resolvedText.width() > 0) {
        if (!fCurrentRun->leftToRight()) {
            std::swap(resolvedText.start, resolvedText.end);
        }

        GlyphRange resolvedGlyphs(resolvedGlyphsStart, fCurrentRun->size());
        RunBlock resolved(fCurrentRun, resolvedText, resolvedGlyphs, resolvedGlyphs.width());
        fResolvedBlocks.emplace_back(resolved);
    }
}

void OneLineShaper::finish(const Block& block, SkScalar height, SkScalar& advanceX) {
    auto blockText = block.fRange;

    // Add all unresolved blocks to resolved blocks
    while (!fUnresolvedBlocks.empty()) {
        auto unresolved = fUnresolvedBlocks.front();
        fUnresolvedBlocks.pop_front();
        if (unresolved.fText.width() == 0) {
            continue;
        }
        fResolvedBlocks.emplace_back(unresolved);
        fUnresolvedGlyphs += unresolved.fGlyphs.width();
        fParagraph->addUnresolvedCodepoints(unresolved.fText);
    }

    // Sort all pieces by text
    std::sort(fResolvedBlocks.begin(), fResolvedBlocks.end(),
              [](const RunBlock& a, const RunBlock& b) {
                return a.fText.start < b.fText.start;
              });

    // Go through all of them
    size_t lastTextEnd = blockText.start;
    for (auto& resolvedBlock : fResolvedBlocks) {

        if (resolvedBlock.fText.end <= blockText.start) {
            continue;
        }

        if (resolvedBlock.fRun != nullptr) {
            fParagraph->fFontSwitches.emplace_back(resolvedBlock.fText.start, resolvedBlock.fRun->fFont);
        }

        auto run = resolvedBlock.fRun;
        auto glyphs = resolvedBlock.fGlyphs;
        auto text = resolvedBlock.fText;
        if (lastTextEnd != text.start) {
            SkDEBUGF("Text ranges mismatch: ...:%zu] - [%zu:%zu] (%zu-%zu)\n",
                     lastTextEnd, text.start, text.end,  glyphs.start, glyphs.end);
            SkASSERT(false);
        }
        lastTextEnd = text.end;

        if (resolvedBlock.isFullyResolved()) {
            // Just move the entire run
            resolvedBlock.fRun->fIndex = this->fParagraph->fRuns.size();
            this->fParagraph->fRuns.emplace_back(*resolvedBlock.fRun);
            resolvedBlock.fRun.reset();
            continue;
        } else if (run == nullptr) {
            continue;
        }

        auto runAdvance = SkVector::Make(run->posX(glyphs.end) - run->posX(glyphs.start), run->fAdvance.fY);
        const SkShaper::RunHandler::RunInfo info = {
                run->fFont,
                run->fBidiLevel,
                runAdvance,
                glyphs.width(),
                SkShaper::RunHandler::Range(text.start - run->fClusterStart, text.width())
        };
        this->fParagraph->fRuns.emplace_back(
                    this->fParagraph,
                    info,
                    run->fClusterStart,
                    height,
                    block.fStyle.getHalfLeading(),
                    block.fStyle.getBaselineShift(),
                    this->fParagraph->fRuns.size(),
                    advanceX
                );
        auto piece = &this->fParagraph->fRuns.back();

        // TODO: Optimize copying
        SkPoint zero = {run->fPositions[glyphs.start].fX, 0};
        for (size_t i = glyphs.start; i <= glyphs.end; ++i) {

            auto index = i - glyphs.start;
            if (i < glyphs.end) {
                piece->fGlyphs[index] = run->fGlyphs[i];
            }
            piece->fClusterIndexes[index] = run->fClusterIndexes[i];
            piece->fPositions[index] = run->fPositions[i] - zero;
            piece->fOffsets[index] = run->fOffsets[i];
            piece->addX(index, advanceX);
        }

        // Carve out the line text out of the entire run text
        fAdvance.fX += runAdvance.fX;
        fAdvance.fY = std::max(fAdvance.fY, runAdvance.fY);
    }

    advanceX = fAdvance.fX;
    if (lastTextEnd != blockText.end) {
        SkDEBUGF("Last range mismatch: %zu - %zu\n", lastTextEnd, blockText.end);
        SkASSERT(false);
    }
}

// Make it [left:right) regardless of a text direction
TextRange OneLineShaper::normalizeTextRange(GlyphRange glyphRange) {

    if (fCurrentRun->leftToRight()) {
        return TextRange(clusterIndex(glyphRange.start), clusterIndex(glyphRange.end));
    } else {
        return TextRange(clusterIndex(glyphRange.end - 1),
                glyphRange.start > 0
                ? clusterIndex(glyphRange.start - 1)
                : fCurrentRun->fTextRange.end);
    }
}

void OneLineShaper::addFullyResolved() {
    if (this->fCurrentRun->size() == 0) {
        return;
    }
    RunBlock resolved(fCurrentRun,
                      this->fCurrentRun->fTextRange,
                      GlyphRange(0, this->fCurrentRun->size()),
                      this->fCurrentRun->size());
    fResolvedBlocks.emplace_back(resolved);
}

void OneLineShaper::addUnresolvedWithRun(GlyphRange glyphRange) {
    auto extendedText = this->clusteredText(glyphRange); // It also modifies glyphRange if needed
    RunBlock unresolved(fCurrentRun, extendedText, glyphRange, 0);
    if (unresolved.fGlyphs.width() == fCurrentRun->size()) {
        SkASSERT(unresolved.fText.width() == fCurrentRun->fTextRange.width());
    } else if (fUnresolvedBlocks.size() > 0) {
        auto& lastUnresolved = fUnresolvedBlocks.back();
        if (lastUnresolved.fRun != nullptr &&
            lastUnresolved.fRun->fIndex == fCurrentRun->fIndex) {

            if (lastUnresolved.fText.end == unresolved.fText.start) {
              // Two pieces next to each other - can join them
              lastUnresolved.fText.end = unresolved.fText.end;
              lastUnresolved.fGlyphs.end = glyphRange.end;
              return;
            } else if(lastUnresolved.fText == unresolved.fText) {
                // Nothing was resolved; ignore it
                return;
            } else if (lastUnresolved.fText.contains(unresolved.fText)) {
                // We get here for the very first unresolved piece
                return;
            } else if (lastUnresolved.fText.intersects(unresolved.fText)) {
                // Few pieces of the same unresolved text block can ignore the second one
                lastUnresolved.fGlyphs.start = std::min(lastUnresolved.fGlyphs.start, glyphRange.start);
                lastUnresolved.fGlyphs.end = std::max(lastUnresolved.fGlyphs.end, glyphRange.end);
                lastUnresolved.fText = this->clusteredText(lastUnresolved.fGlyphs);
                return;
            }
        }
    }
    fUnresolvedBlocks.emplace_back(unresolved);
}

// Glue whitespaces to the next/prev unresolved blocks
// (so we don't have chinese text with english whitespaces broken into millions of tiny runs)
void OneLineShaper::sortOutGlyphs(std::function<void(GlyphRange)>&& sortOutUnresolvedBLock) {

    GlyphRange block = EMPTY_RANGE;
    bool graphemeResolved = false;
    TextIndex graphemeStart = EMPTY_INDEX;
    for (size_t i = 0; i < fCurrentRun->size(); ++i) {

        ClusterIndex ci = clusterIndex(i);
        // Removing all pretty optimizations for whitespaces
        // because they get in a way of grapheme rounding
        // Inspect the glyph
        auto glyph = fCurrentRun->fGlyphs[i];

        GraphemeIndex gi = fParagraph->findPreviousGraphemeBoundary(ci);
        if ((fCurrentRun->leftToRight() ? gi > graphemeStart : gi < graphemeStart) || graphemeStart == EMPTY_INDEX) {
            // This is the Flutter change
            // Do not count control codepoints as unresolved
            bool isControl8 = fParagraph->codeUnitHasProperty(ci,
                                                              SkUnicode::CodeUnitFlags::kControl);
            // We only count glyph resolved if all the glyphs in its grapheme are resolved
            graphemeResolved = glyph != 0 || isControl8;
            graphemeStart = gi;
        } else if (glyph == 0) {
            // Found unresolved glyph - the entire grapheme is unresolved now
            graphemeResolved = false;
        }

        if (!graphemeResolved) { // Unresolved glyph and not control codepoint
            if (block.start == EMPTY_INDEX) {
                // Start new unresolved block
                block.start = i;
                block.end = EMPTY_INDEX;
            } else {
                // Keep skipping unresolved block
            }
        } else { // Resolved glyph or control codepoint
            if (block.start == EMPTY_INDEX) {
                // Keep skipping resolved code points
            } else {
                // This is the end of unresolved block
                block.end = i;
                sortOutUnresolvedBLock(block);
                block = EMPTY_RANGE;
            }
        }
    }

    // One last block could have been left
    if (block.start != EMPTY_INDEX) {
        block.end = fCurrentRun->size();
        sortOutUnresolvedBLock(block);
    }
}

void OneLineShaper::iterateThroughFontStyles(TextRange textRange,
                                             SkSpan<Block> styleSpan,
                                             const ShapeSingleFontVisitor& visitor) {
    Block combinedBlock;
    TArray<SkShaper::Feature> features;

    auto addFeatures = [&features](const Block& block) {
        for (auto& ff : block.fStyle.getFontFeatures()) {
            if (ff.fName.size() != 4) {
                SkDEBUGF("Incorrect font feature: %s=%d\n", ff.fName.c_str(), ff.fValue);
                continue;
            }
            SkShaper::Feature feature = {
                SkSetFourByteTag(ff.fName[0], ff.fName[1], ff.fName[2], ff.fName[3]),
                SkToU32(ff.fValue),
                block.fRange.start,
                block.fRange.end
            };
            features.emplace_back(feature);
        }
        // Disable ligatures if letter spacing is enabled.
        if (block.fStyle.getLetterSpacing() > 0) {
            features.emplace_back(SkShaper::Feature{
                SkSetFourByteTag('l', 'i', 'g', 'a'), 0, block.fRange.start, block.fRange.end
            });
        }
    };

    for (auto& block : styleSpan) {
        BlockRange blockRange(std::max(block.fRange.start, textRange.start), std::min(block.fRange.end, textRange.end));
        if (blockRange.empty()) {
            continue;
        }
        SkASSERT(combinedBlock.fRange.width() == 0 || combinedBlock.fRange.end == block.fRange.start);

        if (!combinedBlock.fRange.empty()) {
            if (block.fStyle.matchOneAttribute(StyleType::kFont, combinedBlock.fStyle)) {
                combinedBlock.add(blockRange);
                addFeatures(block);
                continue;
            }
            // Resolve all characters in the block for this style
            visitor(combinedBlock, features);
        }

        combinedBlock.fRange = blockRange;
        combinedBlock.fStyle = block.fStyle;
        features.clear();
        addFeatures(block);
    }

    visitor(combinedBlock, features);
#ifdef SK_DEBUG
    //printState();
#endif
}

void OneLineShaper::matchResolvedFonts(const TextStyle& textStyle,
                                       const TypefaceVisitor& visitor) {
    std::vector<sk_sp<SkTypeface>> typefaces = fParagraph->fFontCollection->findTypefaces(textStyle.getFontFamilies(), textStyle.getFontStyle(), textStyle.getFontArguments());

    for (const auto& typeface : typefaces) {
        if (visitor(typeface) == Resolved::Everything) {
            // Resolved everything
            return;
        }
    }

    if (fParagraph->fFontCollection->fontFallbackEnabled()) {
        // Give fallback a clue
        // Some unresolved subblocks might be resolved with different fallback fonts
        std::vector<RunBlock> hopelessBlocks;
        while (!fUnresolvedBlocks.empty()) {
            auto unresolvedRange = fUnresolvedBlocks.front().fText;
            auto unresolvedText = fParagraph->text(unresolvedRange);
            const char* ch = unresolvedText.begin();
            // We have the global cache for all already found typefaces for SkUnichar
            // but we still need to keep track of all SkUnichars used in this unresolved block
            THashSet<SkUnichar> alreadyTriedCodepoints;
            THashSet<SkTypefaceID> alreadyTriedTypefaces;
            while (true) {
                if (ch == unresolvedText.end()) {
                    // Not a single codepoint could be resolved but we finished the block
                    hopelessBlocks.push_back(fUnresolvedBlocks.front());
                    fUnresolvedBlocks.pop_front();
                    break;
                }

                // See if we can switch to the next DIFFERENT codepoint/emoji
                SkUnichar codepoint = -1;
                SkUnichar emojiStart = -1;
                // We may loop until we find a new codepoint/emoji run
                while (ch != unresolvedText.end()) {
                  emojiStart = OneLineShaper::getEmojiSequenceStart(
                                                fParagraph->fUnicode.get(),
                                                &ch,
                                                unresolvedText.end());
                    if (emojiStart != -1) {
                        // We do not keep a cache of emoji runs, but we need to move the cursor
                        break;
                    } else {
                        codepoint = SkUTF::NextUTF8WithReplacement(&ch, unresolvedText.end());
                        if (!alreadyTriedCodepoints.contains(codepoint)) {
                            alreadyTriedCodepoints.add(codepoint);
                            break;
                        }
                    }
                }

                SkASSERT(codepoint != -1 || emojiStart != -1);

                sk_sp<SkTypeface> typeface = nullptr;
                if (emojiStart == -1) {
                    // First try to find in in a cache
                    FontKey fontKey(codepoint, textStyle.getFontStyle(), textStyle.getLocale());
                    auto found = fFallbackFonts.find(fontKey);
                    if (found != nullptr) {
                        typeface = *found;
                    }
                    if (typeface == nullptr) {
                        typeface = fParagraph->fFontCollection->defaultFallback(
                                                    codepoint,
                                                    textStyle.getFontStyle(),
                                                    textStyle.getLocale());
                        if (typeface != nullptr) {
                            fFallbackFonts.set(fontKey, typeface);
                        }
                    }
                } else {
                    typeface = fParagraph->fFontCollection->defaultEmojiFallback(
                                                emojiStart,
                                                textStyle.getFontStyle(),
                                                textStyle.getLocale());
                }

                if (typeface == nullptr) {
                    // There is no fallback font for this character,
                    // so move on to the next character.
                    continue;
                }

                // Check if we already tried this font on this text range
                if (!alreadyTriedTypefaces.contains(typeface->uniqueID())) {
                    alreadyTriedTypefaces.add(typeface->uniqueID());
                } else {
                    continue;
                }

                auto resolvedBlocksBefore = fResolvedBlocks.size();
                auto resolved = visitor(typeface);
                if (resolved == Resolved::Everything) {
                    if (hopelessBlocks.empty()) {
                        // Resolved everything, no need to try another font
                        return;
                    } else if (resolvedBlocksBefore < fResolvedBlocks.size()) {
                        // There are some resolved blocks
                        resolved = Resolved::Something;
                    } else {
                        // All blocks are hopeless
                        resolved = Resolved::Nothing;
                    }
                }

                if (resolved == Resolved::Something) {
                    // Resolved something, no need to try another codepoint
                    break;
                }
            }
        }

        // Return hopeless blocks back
        for (auto& block : hopelessBlocks) {
            fUnresolvedBlocks.emplace_front(block);
        }
    }
}

bool OneLineShaper::iterateThroughShapingRegions(const ShapeVisitor& shape) {

    size_t bidiIndex = 0;

    SkScalar advanceX = 0;
    for (auto& placeholder : fParagraph->fPlaceholders) {

        if (placeholder.fTextBefore.width() > 0) {
            // Shape the text by bidi regions
            while (bidiIndex < fParagraph->fBidiRegions.size()) {
                SkUnicode::BidiRegion& bidiRegion = fParagraph->fBidiRegions[bidiIndex];
                auto start = std::max(bidiRegion.start, placeholder.fTextBefore.start);
                auto end = std::min(bidiRegion.end, placeholder.fTextBefore.end);

                // Set up the iterators (the style iterator points to a bigger region that it could
                TextRange textRange(start, end);
                auto blockRange = fParagraph->findAllBlocks(textRange);
                if (!blockRange.empty()) {
                    SkSpan<Block> styleSpan(fParagraph->blocks(blockRange));

                    // Shape the text between placeholders
                    if (!shape(textRange, styleSpan, advanceX, start, bidiRegion.level)) {
                        return false;
                    }
                }

                if (end == bidiRegion.end) {
                    ++bidiIndex;
                } else /*if (end == placeholder.fTextBefore.end)*/ {
                    break;
                }
            }
        }

        if (placeholder.fRange.width() == 0) {
            continue;
        }

        // Get the placeholder font
        std::vector<sk_sp<SkTypeface>> typefaces = fParagraph->fFontCollection->findTypefaces(
            placeholder.fTextStyle.getFontFamilies(),
            placeholder.fTextStyle.getFontStyle(),
            placeholder.fTextStyle.getFontArguments());
        sk_sp<SkTypeface> typeface = typefaces.size() ? typefaces.front() : nullptr;
        SkFont font(typeface, placeholder.fTextStyle.getFontSize());

        // "Shape" the placeholder
        uint8_t bidiLevel = (bidiIndex < fParagraph->fBidiRegions.size())
            ? fParagraph->fBidiRegions[bidiIndex].level
            : 2;
        const SkShaper::RunHandler::RunInfo runInfo = {
            font,
            bidiLevel,
            SkPoint::Make(placeholder.fStyle.fWidth, placeholder.fStyle.fHeight),
            1,
            SkShaper::RunHandler::Range(0, placeholder.fRange.width())
        };
        auto& run = fParagraph->fRuns.emplace_back(this->fParagraph,
                                       runInfo,
                                       placeholder.fRange.start,
                                       0.0f,
                                       0.0f,
                                       false,
                                       fParagraph->fRuns.size(),
                                       advanceX);

        run.fPositions[0] = { advanceX, 0 };
        run.fOffsets[0] = {0, 0};
        run.fClusterIndexes[0] = 0;
        run.fPlaceholderIndex = &placeholder - fParagraph->fPlaceholders.begin();
        advanceX += placeholder.fStyle.fWidth;
    }
    return true;
}

bool OneLineShaper::shape() {

    // The text can be broken into many shaping sequences
    // (by place holders, possibly, by hard line breaks or tabs, too)
    auto limitlessWidth = std::numeric_limits<SkScalar>::max();

    auto result = iterateThroughShapingRegions(
            [this, limitlessWidth]
            (TextRange textRange, SkSpan<Block> styleSpan, SkScalar& advanceX, TextIndex textStart, uint8_t defaultBidiLevel) {

        // Set up the shaper and shape the next
        auto shaper = SkShaper::MakeShapeDontWrapOrReorder(fParagraph->fUnicode->copy(),
                                                           SkFontMgr::RefEmpty()); // no fallback
        if (shaper == nullptr) {
            // For instance, loadICU does not work. We have to stop the process
            return false;
        }

        iterateThroughFontStyles(textRange, styleSpan,
                [this, &shaper, defaultBidiLevel, limitlessWidth, &advanceX]
                (Block block, TArray<SkShaper::Feature> features) {
            auto blockSpan = SkSpan<Block>(&block, 1);

            // Start from the beginning (hoping that it's a simple case one block - one run)
            fHeight = block.fStyle.getHeightOverride() ? block.fStyle.getHeight() : 0;
            fUseHalfLeading = block.fStyle.getHalfLeading();
            fBaselineShift = block.fStyle.getBaselineShift();
            fAdvance = SkVector::Make(advanceX, 0);
            fCurrentText = block.fRange;
            fUnresolvedBlocks.emplace_back(RunBlock(block.fRange));

            this->matchResolvedFonts(block.fStyle, [&](sk_sp<SkTypeface> typeface) {

                // Create one more font to try
                SkFont font(std::move(typeface), block.fStyle.getFontSize());
                font.setEdging(SkFont::Edging::kAntiAlias);
                font.setHinting(SkFontHinting::kSlight);
                font.setSubpixel(true);

                // Apply fake bold and/or italic settings to the font if the
                // typeface's attributes do not match the intended font style.
                int wantedWeight = block.fStyle.getFontStyle().weight();
                bool fakeBold =
                    wantedWeight >= SkFontStyle::kSemiBold_Weight &&
                    wantedWeight - font.getTypeface()->fontStyle().weight() >= 200;
                bool fakeItalic =
                    block.fStyle.getFontStyle().slant() == SkFontStyle::kItalic_Slant &&
                    font.getTypeface()->fontStyle().slant() != SkFontStyle::kItalic_Slant;
                font.setEmbolden(fakeBold);
                font.setSkewX(fakeItalic ? -SK_Scalar1 / 4 : 0);

                // Walk through all the currently unresolved blocks
                // (ignoring those that appear later)
                auto resolvedCount = fResolvedBlocks.size();
                auto unresolvedCount = fUnresolvedBlocks.size();
                while (unresolvedCount-- > 0) {
                    auto unresolvedRange = fUnresolvedBlocks.front().fText;
                    if (unresolvedRange == EMPTY_TEXT) {
                        // Duplicate blocks should be ignored
                        fUnresolvedBlocks.pop_front();
                        continue;
                    }
                    auto unresolvedText = fParagraph->text(unresolvedRange);

                    SkShaper::TrivialFontRunIterator fontIter(font, unresolvedText.size());
                    LangIterator langIter(unresolvedText, blockSpan,
                                      fParagraph->paragraphStyle().getTextStyle());
                    SkShaper::TrivialBiDiRunIterator bidiIter(defaultBidiLevel, unresolvedText.size());
                    auto scriptIter = SkShaper::MakeSkUnicodeHbScriptRunIterator(
                            unresolvedText.begin(), unresolvedText.size());
                    fCurrentText = unresolvedRange;

                    // Map the block's features to subranges within the unresolved range.
                    TArray<SkShaper::Feature> adjustedFeatures(features.size());
                    for (const SkShaper::Feature& feature : features) {
                        SkRange<size_t> featureRange(feature.start, feature.end);
                        if (unresolvedRange.intersects(featureRange)) {
                            SkRange<size_t> adjustedRange = unresolvedRange.intersection(featureRange);
                            adjustedRange.Shift(-static_cast<std::make_signed_t<size_t>>(unresolvedRange.start));
                            adjustedFeatures.push_back({feature.tag, feature.value, adjustedRange.start, adjustedRange.end});
                        }
                    }

                    shaper->shape(unresolvedText.begin(), unresolvedText.size(),
                            fontIter, bidiIter,*scriptIter, langIter,
                            adjustedFeatures.data(), adjustedFeatures.size(),
                            limitlessWidth, this);

                    // Take off the queue the block we tried to resolved -
                    // whatever happened, we have now smaller pieces of it to deal with
                    fUnresolvedBlocks.pop_front();
                }

                if (fUnresolvedBlocks.empty()) {
                    // In some cases it does not mean everything
                    // (when we excluded some hopeless blocks from the list)
                    return Resolved::Everything;
                } else if (resolvedCount < fResolvedBlocks.size()) {
                    return Resolved::Something;
                } else {
                    return Resolved::Nothing;
                }
            });

            this->finish(block, fHeight, advanceX);
        });

        return true;
    });

    return result;
}

// When we extend TextRange to the grapheme edges, we also extend glyphs range
TextRange OneLineShaper::clusteredText(GlyphRange& glyphs) {

    enum class Dir { left, right };
    enum class Pos { inclusive, exclusive };

    // [left: right)
    auto findBaseChar = [&](TextIndex index, Dir dir) -> TextIndex {

        if (dir == Dir::right) {
            while (index < fCurrentRun->fTextRange.end) {
                if (this->fParagraph->codeUnitHasProperty(index,
                                                      SkUnicode::CodeUnitFlags::kGraphemeStart)) {
                    return index;
                }
                ++index;
            }
            return fCurrentRun->fTextRange.end;
        } else {
            while (index > fCurrentRun->fTextRange.start) {
                if (this->fParagraph->codeUnitHasProperty(index,
                                                      SkUnicode::CodeUnitFlags::kGraphemeStart)) {
                    return index;
                }
                --index;
            }
            return fCurrentRun->fTextRange.start;
        }
    };

    TextRange textRange(normalizeTextRange(glyphs));
    textRange.start = findBaseChar(textRange.start, Dir::left);
    textRange.end = findBaseChar(textRange.end, Dir::right);

    // Correct the glyphRange in case we extended the text to the grapheme edges
    // TODO: code it without if (as a part of LTR/RTL refactoring)
    if (fCurrentRun->leftToRight()) {
        while (glyphs.start > 0 && clusterIndex(glyphs.start) > textRange.start) {
          glyphs.start--;
        }
        while (glyphs.end < fCurrentRun->size() && clusterIndex(glyphs.end) < textRange.end) {
          glyphs.end++;
        }
    } else {
        while (glyphs.start > 0 && clusterIndex(glyphs.start - 1) < textRange.end) {
          glyphs.start--;
        }
        while (glyphs.end < fCurrentRun->size() && clusterIndex(glyphs.end) > textRange.start) {
          glyphs.end++;
        }
    }

    return { textRange.start, textRange.end };
}

bool OneLineShaper::FontKey::operator==(const OneLineShaper::FontKey& other) const {
    return fUnicode == other.fUnicode && fFontStyle == other.fFontStyle && fLocale == other.fLocale;
}

uint32_t OneLineShaper::FontKey::Hasher::operator()(const OneLineShaper::FontKey& key) const {
    return SkGoodHash()(key.fUnicode) ^
           SkGoodHash()(key.fFontStyle) ^
           SkGoodHash()(key.fLocale);
}


// By definition any emoji_sequence starts from a codepoint that has
// UCHAR_EMOJI property.
// If the first codepoint does not have UCHAR_EMOJI_COMPONENT property,
// we have an emoji sequence right away.
// In two (and only two) cases an emoji sequence starts with a codepoint
// that also has UCHAR_EMOJI_COMPONENT property.
// emoji_flag_sequence   := regional_indicator regional_indicator
// emoji_keycap_sequence := [0-9#*] \x{FE0F 20E3}
// These two cases require additional checks of the next codepoint(s).
SkUnichar OneLineShaper::getEmojiSequenceStart(SkUnicode* unicode, const char** begin, const char* end) {
    const char* next = *begin;
    auto codepoint1 = SkUTF::NextUTF8WithReplacement(&next, end);

    if (!unicode->isEmoji(codepoint1)) {
        // This is not a basic emoji nor it an emoji sequence
        return -1;
    }

    if (!unicode->isEmojiComponent(codepoint1)) {
        // This is an emoji sequence start
        *begin = next;
        return codepoint1;
    }

    // Now we need to look at the next codepoint to see what is going on
    const char* last = next;
    auto codepoint2 = SkUTF::NextUTF8WithReplacement(&last, end);

    // emoji_flag_sequence
    if (unicode->isRegionalIndicator(codepoint2)) {
        // We expect a second regional indicator here
        if (unicode->isRegionalIndicator(codepoint2)) {
            *begin = next;
            return codepoint1;
        } else {
            // That really should not happen assuming correct UTF8 text
            return -1;
        }
    }

    // emoji_keycap_sequence
    if (codepoint2 == 0xFE0F) {
        auto codepoint3 = SkUTF::NextUTF8WithReplacement(&last, end);
        if (codepoint3 == 0x20E3) {
            *begin = next;
            return codepoint1;
        }
    }

    return -1;
}

}  // namespace textlayout
}  // namespace skia
