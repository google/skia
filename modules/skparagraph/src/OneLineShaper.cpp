// Copyright 2019 Google LLC.

#include "modules/skparagraph/src/Iterators.h"
#include "modules/skparagraph/src/OneLineShaper.h"
#include <unicode/uchar.h>
#include <algorithm>
#include <unordered_set>
#include "src/utils/SkUTF.h"

namespace skia {
namespace textlayout {

namespace {

SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}

}

void OneLineShaper::commitRunBuffer(const RunInfo&) {

    fCurrentRun->commit();

    auto oldUnresolvedCount = fUnresolvedBlocks.size();

    // Find all unresolved blocks
    bool nothingWasUnresolved = true;
    sortOutGlyphs([&](GlyphRange block){
        if (block.width() == 0) {
            return;
        }
        nothingWasUnresolved = false;
        addUnresolvedWithRun(block);
    });

    // Fill all the gaps between unresolved blocks with resolved ones
    if (nothingWasUnresolved) {
        // No unresolved blocks added - we resolved the block with one run entirely
        addFullyResolved();
        return;
    }

    auto& front = fUnresolvedBlocks.front();    // The one we need to resolve
    auto& back = fUnresolvedBlocks.back();      // The one we have from shaper
    if (fUnresolvedBlocks.size() == oldUnresolvedCount + 1 &&
        front.fText == back.fText) {
        // The entire block remains unresolved!
        if (front.fRun != nullptr) {
            back.fRun = front.fRun;
        }
    } else {
        fillGaps(oldUnresolvedCount);
    }
}

#ifdef SK_DEBUG
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
        SkDebugf("[%d:%d) ", resolved.fGlyphs.start, resolved.fGlyphs.end);
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
#endif

void OneLineShaper::dropUnresolved() {
    SkASSERT(!fUnresolvedBlocks.empty());
    fUnresolvedBlocks.pop();
}

void OneLineShaper::fillGaps(size_t startingCount) {
    // Fill out gaps between all unresolved blocks
    TextRange resolvedTextLimits = fCurrentRun->fTextRange;
    if (!fCurrentRun->leftToRight()) {
        std::swap(resolvedTextLimits.start, resolvedTextLimits.end);
    }
    TextIndex resolvedTextStart = resolvedTextLimits.start;
    GlyphIndex resolvedGlyphsStart = 0;

    auto count = fUnresolvedBlocks.size();
    for (size_t i = 0; i < count; ++i) {
        auto unresolved = fUnresolvedBlocks.front();
        fUnresolvedBlocks.pop();
        fUnresolvedBlocks.push(unresolved);
        if (i < startingCount) {
            // Skip the first ones
            continue;
        }

        TextRange resolvedText(resolvedTextStart, fCurrentRun->leftToRight() ? unresolved.fText.start : unresolved.fText.end);
        if (resolvedText.width() > 0) {
            if (!fCurrentRun->leftToRight()) {
                std::swap(resolvedText.start, resolvedText.end);
            }

            GlyphRange resolvedGlyphs(resolvedGlyphsStart, unresolved.fGlyphs.start);
            RunBlock resolved(fCurrentRun, resolvedText, resolvedGlyphs, resolvedGlyphs.width());

            fResolvedBlocks.emplace_back(resolved);
        }
        resolvedGlyphsStart = unresolved.fGlyphs.end;
        resolvedTextStart =  fCurrentRun->leftToRight()
                                ? unresolved.fText.end
                                : unresolved.fText.start;
    }

    TextRange resolvedText(resolvedTextStart, resolvedTextLimits.end);
    if (resolvedText.width() > 0) {
        if (!fCurrentRun->leftToRight()) {
            std::swap(resolvedText.start, resolvedText.end);
        }

        GlyphRange resolvedGlyphs(resolvedGlyphsStart, fCurrentRun->size());
        RunBlock resolved(fCurrentRun, resolvedText, resolvedGlyphs, resolvedGlyphs.width());

        fResolvedBlocks.emplace_back(resolved);
    }
}

void OneLineShaper::finish(TextRange blockText, SkScalar height, SkScalar& advanceX) {

    // Add all unresolved blocks to resolved blocks
    while (!fUnresolvedBlocks.empty()) {
        auto unresolved = fUnresolvedBlocks.front();
        fUnresolvedBlocks.pop();
        if (unresolved.fText.width() == 0) {
            continue;
        }
        fResolvedBlocks.emplace_back(unresolved);
        fUnresolvedGlyphs += unresolved.fGlyphs.width();
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

        if (block.fRun != nullptr) {
            fParagraph->fFontSwitches.emplace_back(block.fText.start, block.fRun->fFont);
        }

        auto run = block.fRun;
        auto glyphs = block.fGlyphs;
        auto text = block.fText;
        if (lastTextEnd != text.start) {
            SkDEBUGF("Text ranges mismatch: ...:%d] - [%d:%d] (%d-%d)\n", lastTextEnd, text.start, text.end,  glyphs.start, glyphs.end);
            SkASSERT(false);
        }
        lastTextEnd = text.end;

        if (block.isFullyResolved()) {
            // Just move the entire run
            block.fRun->fIndex = this->fParagraph->fRuns.size();
            this->fParagraph->fRuns.emplace_back(*block.fRun.get());
            block.fRun.reset();
            continue;
        } else if (run == nullptr) {
            continue;
        }

        auto runAdvance = SkVector::Make(run->posX(glyphs.end) - run->posX(glyphs.start), run->fAdvance.fY);
        const SkShaper::RunHandler::RunInfo info = {
                run->fFont, run->fBidiLevel, runAdvance, glyphs.width(),
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
        auto zero = run->fPositions[glyphs.start];
        for (size_t i = glyphs.start; i <= glyphs.end; ++i) {

            auto index = i - glyphs.start;
            if (i < glyphs.end) {
                piece->fGlyphs[index] = run->fGlyphs[i];
                piece->fBounds[index] = run->fBounds[i];
            }
            piece->fClusterIndexes[index] = run->fClusterIndexes[i];
            piece->fOffsets[index] = run->fOffsets[i];
            piece->fPositions[index] = run->fPositions[i] - zero;
            piece->addX(index, advanceX);
        }

        // Carve out the line text out of the entire run text
        fAdvance.fX += runAdvance.fX;
        fAdvance.fY = std::max(fAdvance.fY, runAdvance.fY);
    }

    advanceX = fAdvance.fX;
    if (lastTextEnd != blockText.end) {
        SkDEBUGF("Last range mismatch: %d - %d\n", lastTextEnd, blockText.end);
        SkASSERT(false);
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

    if (fCurrentRun->leftToRight()) {
        return TextRange(clusterIndex(glyphRange.start), clusterIndex(glyphRange.end));
    } else {
        return TextRange(clusterIndex(glyphRange.end - 1),
                glyphRange.start > 0
                ? clusterIndex(glyphRange.start - 1)
                : fCurrentRun->fClusterStart + fCurrentRun->fTextRange.width());
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
    RunBlock unresolved(fCurrentRun, clusteredText(glyphRange), glyphRange, 0);
    if (unresolved.fGlyphs.width() == fCurrentRun->size()) {
        SkASSERT(unresolved.fText.width() == fCurrentRun->fTextRange.width());
    } else if (fUnresolvedBlocks.size() > 1) {
        auto& lastUnresolved = fUnresolvedBlocks.back();
        if (lastUnresolved.fRun != nullptr &&
            lastUnresolved.fRun->fIndex == fCurrentRun->fIndex) {

            if (lastUnresolved.fText.end == unresolved.fText.start) {
                // Two pieces next to each other - can join them
                lastUnresolved.fText.end = unresolved.fText.end;
                lastUnresolved.fGlyphs.end = glyphRange.end;
                return;
            } else if (lastUnresolved.fText.intersects(unresolved.fText)) {
                // Few pieces of the same unresolved text block can ignore the second one
                lastUnresolved.fGlyphs.start =
                        std::min(lastUnresolved.fGlyphs.start, glyphRange.start);
                lastUnresolved.fGlyphs.end = std::max(lastUnresolved.fGlyphs.end, glyphRange.end);
                lastUnresolved.fText = clusteredText(lastUnresolved.fGlyphs);
                return;
            }
        }
    }
    fUnresolvedBlocks.emplace(unresolved);
}

void OneLineShaper::sortOutGlyphs(std::function<void(GlyphRange)>&& sortOutUnresolvedBLock) {

    auto text = fCurrentRun->fMaster->text();
    size_t unresolvedGlyphs = 0;

    GlyphRange block = EMPTY_RANGE;
    for (size_t i = 0; i < fCurrentRun->size(); ++i) {

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
            const char* cluster = text.begin() + clusterIndex(i);
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

void OneLineShaper::iterateThroughFontStyles(TextRange textRange,
                                             SkSpan<Block> styleSpan,
                                             const ShapeSingleFontVisitor& visitor) {
    Block combinedBlock;
    SkTArray<SkShaper::Feature> features;

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
        features.reset();
        addFeatures(block);
    }

    visitor(combinedBlock, features);
#ifdef SK_DEBUG
    //printState();
#endif
}

void OneLineShaper::matchResolvedFonts(const TextStyle& textStyle,
                                       const TypefaceVisitor& visitor) {
    std::vector<sk_sp<SkTypeface>> typefaces = fParagraph->fFontCollection->findTypefaces(textStyle.getFontFamilies(), textStyle.getFontStyle());

    for (const auto& typeface : typefaces) {
        if (visitor(typeface) == Resolved::Everything) {
            // Resolved everything
            return;
        }
    }

    if (fParagraph->fFontCollection->fontFallbackEnabled()) {
        // Give fallback a clue
        // Some unresolved subblocks might be resolved with different fallback fonts
        while (!fUnresolvedBlocks.empty()) {
            auto unresolvedRange = fUnresolvedBlocks.front().fText;
            auto unresolvedText = fParagraph->text(unresolvedRange);
            const char* ch = unresolvedText.begin();
            // We have the global cache for all already found typefaces for SkUnichar
            // but we still need to keep track of all SkUnichars used in this unresolved block
            SkTHashSet<SkUnichar> alreadyTried;
            SkUnichar unicode = utf8_next(&ch, unresolvedText.end());
            while (true) {

                sk_sp<SkTypeface> typeface;

                // First try to find in in a cache
                FontKey fontKey(unicode, textStyle.getFontStyle(), textStyle.getLocale());
                auto found = fFallbackFonts.find(fontKey);
                if (found != nullptr) {
                    typeface = *found;
                } else {
                    typeface = fParagraph->fFontCollection->defaultFallback(
                            unicode, textStyle.getFontStyle(), textStyle.getLocale());

                    if (typeface == nullptr) {
                        return;
                    }
                    fFallbackFonts.set(fontKey, typeface);
                }

                auto resolved = visitor(typeface);
                if (resolved == Resolved::Everything) {
                    // Resolved everything, no need to try another font
                    return;
                }

                if (resolved == Resolved::Something) {
                    // Resolved something, no need to try another codepoint
                    break;
                }

                if (ch == unresolvedText.end()) {
                    // Not a single codepoint could be resolved but we finished the block
                    break;
                }

                // We can stop here or we can switch to another DIFFERENT codepoint
                while (ch != unresolvedText.end()) {
                    unicode = utf8_next(&ch, unresolvedText.end());
                    auto found = alreadyTried.find(unicode);
                    if (found == nullptr) {
                        alreadyTried.add(unicode);
                        break;
                    }
                }
            }

        }
    }
}

bool OneLineShaper::iterateThroughShapingRegions(const ShapeVisitor& shape) {

    SkTArray<BidiRegion> bidiRegions;
    if (!fParagraph->calculateBidiRegions(&bidiRegions)) {
        return false;
    }

    size_t bidiIndex = 0;

    SkScalar advanceX = 0;
    for (auto& placeholder : fParagraph->fPlaceholders) {

        if (placeholder.fTextBefore.width() > 0) {
            // Shape the text by bidi regions
            while (bidiIndex < bidiRegions.size()) {
                BidiRegion& bidiRegion = bidiRegions[bidiIndex];
                auto start = std::max(bidiRegion.text.start, placeholder.fTextBefore.start);
                auto end = std::min(bidiRegion.text.end, placeholder.fTextBefore.end);

                // Set up the iterators (the style iterator points to a bigger region that it could
                TextRange textRange(start, end);
                auto blockRange = fParagraph->findAllBlocks(textRange);
                SkSpan<Block> styleSpan(fParagraph->blocks(blockRange));

                // Shape the text between placeholders
                if (!shape(textRange, styleSpan, advanceX, start, bidiRegion.direction)) {
                    return false;
                }

                if (end == bidiRegion.text.end) {
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
            placeholder.fTextStyle.getFontStyle());
        sk_sp<SkTypeface> typeface = typefaces.size() ? typefaces.front() : nullptr;
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
                                       0.0f,
                                       fParagraph->fRuns.count(),
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
        auto shaper = SkShaper::MakeShapeDontWrapOrReorder();
        if (shaper == nullptr) {
            // For instance, loadICU does not work. We have to stop the process
            return false;
        }

        iterateThroughFontStyles(textRange, styleSpan,
                [this, &shaper, defaultBidiLevel, limitlessWidth, &advanceX]
                (Block block, SkTArray<SkShaper::Feature> features) {
            auto blockSpan = SkSpan<Block>(&block, 1);

            // Start from the beginning (hoping that it's a simple case one block - one run)
            fHeight = block.fStyle.getHeightOverride() ? block.fStyle.getHeight() : 0;
            fAdvance = SkVector::Make(advanceX, 0);
            fCurrentText = block.fRange;
            fUnresolvedBlocks.emplace(RunBlock(block.fRange));

            matchResolvedFonts(block.fStyle, [&](sk_sp<SkTypeface> typeface) {

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
                    auto unresolvedText = fParagraph->text(unresolvedRange);

                    SkShaper::TrivialFontRunIterator fontIter(font, unresolvedText.size());
                    LangIterator langIter(unresolvedText, blockSpan,
                                      fParagraph->paragraphStyle().getTextStyle());
                    SkShaper::TrivialBiDiRunIterator bidiIter(defaultBidiLevel, unresolvedText.size());
                    auto scriptIter = SkShaper::MakeHbIcuScriptRunIterator
                                     (unresolvedText.begin(), unresolvedText.size());
                    fCurrentText = unresolvedRange;
                    shaper->shape(unresolvedText.begin(), unresolvedText.size(),
                            fontIter, bidiIter,*scriptIter, langIter,
                            features.data(), features.size(),
                            limitlessWidth, this);

                    // Take off the queue the block we tried to resolved -
                    // whatever happened, we have now smaller pieces of it to deal with
                    this->dropUnresolved();
                }

                if (fUnresolvedBlocks.empty()) {
                    return Resolved::Everything;
                } else if (resolvedCount < fResolvedBlocks.size()) {
                    return Resolved::Something;
                } else {
                    return Resolved::Nothing;
                }
            });

            this->finish(block.fRange, fHeight, advanceX);
        });

        return true;
    });

    return result;
}

TextRange OneLineShaper::clusteredText(GlyphRange glyphs) {

    enum class Dir { left, right };
    enum class Pos { inclusive, exclusive };

    // [left: right)
    auto findBaseChar = [&](TextIndex index, Dir dir) -> TextIndex {

        if (dir == Dir::right) {
            while (index < fCurrentRun->fTextRange.end) {
                if (this->fParagraph->fGraphemes.contains(index)) {
                    return index;
                }
                ++index;
            }
            return fCurrentRun->fTextRange.end;
        } else {
            while (index >= fCurrentRun->fTextRange.start) {
                if (this->fParagraph->fGraphemes.contains(index)) {
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

    return { textRange.start, textRange.end };
}

bool OneLineShaper::FontKey::operator==(const OneLineShaper::FontKey& other) const {
    return fUnicode == other.fUnicode && fFontStyle == other.fFontStyle && fLocale == other.fLocale;
}

size_t OneLineShaper::FontKey::Hasher::operator()(const OneLineShaper::FontKey& key) const {

    return SkGoodHash()(key.fUnicode) ^
           SkGoodHash()(key.fFontStyle) ^
           SkGoodHash()(key.fLocale.c_str());
}

}
}
