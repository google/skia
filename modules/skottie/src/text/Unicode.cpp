/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>
#include <utility>

#include "include/core/SkTypes.h"
#include "include/private/base/SkAssert.h"
#include "modules/skottie/include/TextShaper.h"
#include "modules/skunicode/include/SkUnicode.h"

namespace {

// Performs iterator "intersection", i.e. it only returns break positions matching both iterators.
class IntersectingBreakIterator final : public SkBreakIterator {
public:
    IntersectingBreakIterator(std::unique_ptr<SkBreakIterator> a,
                              std::unique_ptr<SkBreakIterator> b)
        : fA(std::move(a))
        , fB(std::move(b))
    {
        SkASSERT(fA);
        SkASSERT(fB);
    }

    Position first() override {
        fCurrent = fA->first();
        SkAssertResult(fB->first() == fCurrent);
        fDone = false;
        return fCurrent;
    }

    Position current() override {
        return fCurrent;
    }

    bool isDone() override {
        SkASSERT((fA->isDone() || fB->isDone()) == fDone);
        return fDone;
    }

    Position next() override {
        SkASSERT(fDone || (fA->current() == fB->current()));
        Position pos_a = fCurrent,
                 pos_b = fCurrent;

        while (!fDone) {
            if (pos_a < pos_b) {
                pos_a = fA->next();
                fDone = fA->isDone();
            } else {
                pos_b = fB->next();
                fDone = fB->isDone();
            }

            if (pos_a == pos_b) {
                break;
            }
        }

        // At this point the positions are either valid and equal (advanced successfully),
        // or one of them is negative (for the done iterator);
        SkASSERT(pos_a == pos_b || fDone);
        SkASSERT((pos_a < 0 || pos_b < 0) == fDone);

        return fCurrent = std::min(pos_a, pos_b);
    }

    bool setText(const char utftext8[], int utf8Units) override {
        fDone = false;
        fCurrent = 0;
        return fA->setText(utftext8, utf8Units) &&
               fB->setText(utftext8, utf8Units);
    }

    bool setText(const char16_t utftext16[], int utf16Units) override {
        fDone = false;
        fCurrent = 0;
        return fA->setText(utftext16, utf16Units) &&
               fB->setText(utftext16, utf16Units);
    }

    Status status() override {
        SkUNREACHABLE;
    }

private:
    const std::unique_ptr<SkBreakIterator> fA, fB;
    Position                               fCurrent = 0;
    bool                                   fDone    = false;
};

// An SkUnicode wrapper which offers stricter line break semantics (it never breaks words).
class StrictLinebreakUnicode final : public SkUnicode {
public:
    explicit StrictLinebreakUnicode(sk_sp<SkUnicode> uc) : fUnicode(std::move(uc))
    {
        SkASSERT(fUnicode);
    }

    std::unique_ptr<SkBreakIterator> makeBreakIterator(const char locale[],
                                                       BreakType breakType) override {
        std::unique_ptr<SkBreakIterator> brk = fUnicode->makeBreakIterator(locale, breakType);
        if (!brk || breakType != BreakType::kLines) {
            return brk;
        }

        // When requested a line break iterator, return a composite iterator which prevents
        // breaking mid-word.
        std::unique_ptr<SkBreakIterator> wbrk =
            fUnicode->makeBreakIterator(locale, BreakType::kWords);

        return wbrk
            ? std::make_unique<IntersectingBreakIterator>(std::move(brk), std::move(wbrk))
            : std::move(brk);
    }

    // Proxy everything else.
    SkString toUpper(const SkString& str) override { return fUnicode->toUpper(str); }
    SkString toUpper(const SkString& str, const char* locale) override {
        return fUnicode->toUpper(str, locale);
    }
    bool isControl(SkUnichar utf8) override { return fUnicode->isControl(utf8); }
    bool isWhitespace(SkUnichar utf8) override { return fUnicode->isWhitespace(utf8); }
    bool isSpace(SkUnichar utf8) override { return fUnicode->isSpace(utf8); }
    bool isTabulation(SkUnichar utf8) override { return fUnicode->isTabulation(utf8); }
    bool isHardBreak(SkUnichar utf8) override { return fUnicode->isHardBreak(utf8); }
    bool isEmoji(SkUnichar utf8) override { return fUnicode->isEmoji(utf8); }
    bool isEmojiComponent(SkUnichar utf8) override { return fUnicode->isEmojiComponent(utf8); }
    bool isEmojiModifierBase(SkUnichar utf8) override {
        return fUnicode->isEmojiModifierBase(utf8);
    }
    bool isEmojiModifier(SkUnichar utf8) override { return fUnicode->isEmojiModifier(utf8); }
    bool isRegionalIndicator(SkUnichar utf8) override {
        return fUnicode->isRegionalIndicator(utf8);
    }
    bool isIdeographic(SkUnichar utf8) override { return fUnicode->isIdeographic(utf8); }
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override {
        return fUnicode->makeBidiIterator(text, count, dir);
    }
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override {
        return fUnicode->makeBidiIterator(text, count, dir);
    }
    std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType breakType) override {
        return fUnicode->makeBreakIterator(breakType);
    }
    bool getBidiRegions(const char utf8[],
                        int utf8Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) override {
        return fUnicode->getBidiRegions(utf8, utf8Units, dir, results);
    }
    bool getWords(const char utf8[], int utf8Units, const char* locale,
                  std::vector<Position>* results) override {
        return fUnicode->getWords(utf8, utf8Units, locale, results);
    }
    bool getUtf8Words(const char utf8[],
                      int utf8Units,
                      const char* locale,
                      std::vector<Position>* results) override {
        return fUnicode->getUtf8Words(utf8, utf8Units, locale, results);
    }
    bool getSentences(const char utf8[],
                      int utf8Units,
                      const char* locale,
                      std::vector<Position>* results) override {
        return fUnicode->getSentences(utf8, utf8Units, locale, results);
    }
    bool computeCodeUnitFlags(char utf8[], int utf8Units, bool replaceTabs,
            skia_private::TArray<SkUnicode::CodeUnitFlags, true>* results) override {
        return fUnicode->computeCodeUnitFlags(utf8, utf8Units, replaceTabs, results);
    }
    bool computeCodeUnitFlags(char16_t utf16[], int utf16Units, bool replaceTabs,
            skia_private::TArray<SkUnicode::CodeUnitFlags, true>* results) override {
        return fUnicode->computeCodeUnitFlags(utf16, utf16Units, replaceTabs, results);
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {
        return fUnicode->reorderVisual(runLevels, levelsCount, logicalFromVisual);
    }

private:
    const sk_sp<SkUnicode> fUnicode;
};

}  // namespace

namespace skottie {

sk_sp<SkUnicode> SK_API MakeStrictLinebreakUnicode(sk_sp<SkUnicode> uc) {
    return uc
        ? sk_make_sp<StrictLinebreakUnicode>(std::move(uc))
        : nullptr;
}

std::unique_ptr<SkBreakIterator> MakeIntersectingBreakIteratorForTesting(
        std::unique_ptr<SkBreakIterator> a, std::unique_ptr<SkBreakIterator> b) {
    return std::make_unique<IntersectingBreakIterator>(std::move(a), std::move(b));
}

}  // namespace skottie
