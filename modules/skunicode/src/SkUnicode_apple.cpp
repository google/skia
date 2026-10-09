/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkAssert.h"
#include "include/private/SkTArray.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/include/SkUnicode_apple.h"
#include "modules/skunicode/src/SkBidiFactory_icu_subset.h"
#include "modules/skunicode/src/SkUnicode_hardcoded.h"
#include "modules/skunicode/src/SkUnicode_icu_bidi.h"
#include "src/core/SkBitmaskEnum.h"
#include "src/core/SkUTF.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#include <CoreFoundation/CoreFoundation.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace skia_private;

namespace {

template <typename Callback>
bool Tokenize(CFStringRef str,
              CFIndex len16,
              CFOptionFlags unit,
              const char* locale,
              Callback&& callback) {
    if (len16 <= 0 || !str) {
        return true;
    }
    SkUniqueCFRef<CFLocaleRef> cfLoc;
    if (locale != nullptr && locale[0] != '\0') {
        SkUniqueCFRef<CFStringRef> locStr(
                CFStringCreateWithCString(kCFAllocatorDefault, locale, kCFStringEncodingUTF8));
        if (locStr) {
            cfLoc.reset(CFLocaleCreate(kCFAllocatorDefault, locStr.get()));
        }
    }

    SkUniqueCFRef<CFStringTokenizerRef> tokenizer(CFStringTokenizerCreate(
            kCFAllocatorDefault, str, CFRangeMake(0, len16), unit, cfLoc.get()));
    if (!tokenizer) {
        return false;
    }

    CFStringTokenizerTokenType tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer.get());
    CFIndex guard = 0;
    const CFIndex maxSteps = 2 * len16 + 2;
    while (tokenType != kCFStringTokenizerTokenNone && ++guard <= maxSteps) {
        CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer.get());
        callback(r);
        tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer.get());
    }

    return true;
}

class SkBreakIterator_apple : public SkBreakIterator {
public:
    SkBreakIterator_apple(SkUnicode::BreakType type, const char* locale)
            : fType(type), fLocale(locale ? locale : "") {}

    ~SkBreakIterator_apple() override = default;

    Position first() override { return fBreaks.empty() ? 0 : fBreaks[(fIndex = 0)]; }
    Position current() override { return fBreaks.empty() ? 0 : fBreaks[fIndex]; }
    Position next() override {
        if (fIndex + 1 < SkTo<Position>(fBreaks.size())) {
            return fBreaks[++fIndex];
        }
        fIndex = fBreaks.size();
        return -1;
    }
    Status status() override {
        if (fIndex < SkTo<Position>(fStatuses.size())) {
            return fStatuses[fIndex];
        }
        return 0;
    }
    bool isDone() override { return fIndex >= SkTo<Position>(fBreaks.size()); }

    bool setText(const char utftext8[], int utf8Units) override {
        fBreaks.clear();
        fStatuses.clear();
        fIndex = 0;
        if (utf8Units <= 0) {
            return true;
        }

        CFStringRef str = CFStringCreateWithBytesNoCopy(kCFAllocatorDefault,
                                                        (const UInt8*)utftext8,
                                                        utf8Units,
                                                        kCFStringEncodingUTF8,
                                                        false,
                                                        kCFAllocatorNull);
        if (!str) {
            return false;
        }

        CFIndex len16 = CFStringGetLength(str);
        const char* loc = fLocale.empty() ? nullptr : fLocale.c_str();

        STArray<256, size_t> u16ToU8;
        SkUnicode::extractUtfConversionMapping(
                SkSpan<const char>(utftext8, utf8Units),
                [&](size_t u8) { u16ToU8.push_back(u8); },
                [&](size_t u16) {});

        if (fType == SkUnicode::BreakType::kLines) {
            fBreaks.push_back(0);
            fStatuses.push_back(SkUnicode::CodeUnitFlags::kSoftLineBreakBefore);
            this->tokenize(str, len16, kCFStringTokenizerUnitLineBreak, loc, [&](CFRange r) {
                CFIndex boundary16 = r.location + r.length;
                if (SkToInt(boundary16) < u16ToU8.size()) {
                    fBreaks.push_back(u16ToU8[boundary16]);
                    fStatuses.push_back(SkUnicode::CodeUnitFlags::kSoftLineBreakBefore);
                }
            });

            const char* current = utftext8;
            const char* end = utftext8 + utf8Units;
            while (current < end) {
                SkUnichar unichar = SkUTF::NextUTF8(&current, end);
                if (unichar == '\n' || unichar == u'\u2028') {
                    Position after = SkTo<Position>(current - utftext8);
                    auto it = std::lower_bound(fBreaks.begin(), fBreaks.end(), after);
                    if (it != fBreaks.end() && *it == after) {
                        auto idx = std::distance(fBreaks.begin(), it);
                        fStatuses[idx] |= SkUnicode::CodeUnitFlags::kHardLineBreakBefore;
                    } else {
                        auto idx = std::distance(fBreaks.begin(), it);
                        fBreaks.insert(it, after);
                        fStatuses.insert(fStatuses.begin() + idx,
                                         SkUnicode::CodeUnitFlags::kHardLineBreakBefore |
                                                 SkUnicode::CodeUnitFlags::kSoftLineBreakBefore);
                    }
                }
            }
        } else if (fType == SkUnicode::BreakType::kGraphemes) {
            CFIndex idx = 0;
            while (idx < len16) {
                if (idx < u16ToU8.size()) {
                    fBreaks.push_back(u16ToU8[idx]);
                    fStatuses.push_back(0);
                }
                CFRange r = CFStringGetRangeOfComposedCharactersAtIndex(str, idx);
                CFIndex advance = std::max<CFIndex>(r.length, 1);
                idx = std::max(idx + 1, r.location + advance);
            }
            fBreaks.push_back(utf8Units);
            fStatuses.push_back(0);
        } else {
            CFOptionFlags unit = (fType == SkUnicode::BreakType::kWords)
                                         ? kCFStringTokenizerUnitWordBoundary
                                         : kCFStringTokenizerUnitSentence;
            this->tokenize(str, len16, unit, loc, [&](CFRange r) {
                if (SkToInt(r.location) < u16ToU8.size()) {
                    fBreaks.push_back(u16ToU8[r.location]);
                    fStatuses.push_back(0);
                }
            });
            if (!fBreaks.empty()) {
                fBreaks.push_back(utf8Units);
                fStatuses.push_back(0);
            }
        }

        CFRelease(str);
        return true;
    }

    bool setText(const char16_t utftext16[], int utf16Units) override {
        fBreaks.clear();
        fStatuses.clear();
        fIndex = 0;
        if (utf16Units <= 0) {
            return true;
        }

        CFStringRef str = CFStringCreateWithCharactersNoCopy(
                kCFAllocatorDefault, (const UniChar*)utftext16, utf16Units, kCFAllocatorNull);
        if (!str) {
            return false;
        }

        const char* loc = fLocale.empty() ? nullptr : fLocale.c_str();
        if (fType == SkUnicode::BreakType::kLines) {
            fBreaks.push_back(0);
            fStatuses.push_back(SkUnicode::CodeUnitFlags::kSoftLineBreakBefore);
            this->tokenize(str, utf16Units, kCFStringTokenizerUnitLineBreak, loc, [&](CFRange r) {
                fBreaks.push_back(r.location + r.length);
                fStatuses.push_back(SkUnicode::CodeUnitFlags::kSoftLineBreakBefore);
            });

            const auto* u16 = reinterpret_cast<const uint16_t*>(utftext16);
            const uint16_t* current = u16;
            const uint16_t* end = u16 + utf16Units;
            while (current < end) {
                SkUnichar unichar = SkUTF::NextUTF16(&current, end);
                if (unichar < 0) {
                    unichar = 0xFFFD;
                }
                if (unichar == '\n' || unichar == u'\u2028') {
                    Position after = SkTo<Position>(current - u16);
                    auto it = std::lower_bound(fBreaks.begin(), fBreaks.end(), after);
                    if (it != fBreaks.end() && *it == after) {
                        auto idx = std::distance(fBreaks.begin(), it);
                        fStatuses[idx] |= SkUnicode::CodeUnitFlags::kHardLineBreakBefore;
                    } else {
                        auto idx = std::distance(fBreaks.begin(), it);
                        fBreaks.insert(it, after);
                        fStatuses.insert(fStatuses.begin() + idx,
                                         SkUnicode::CodeUnitFlags::kHardLineBreakBefore |
                                                 SkUnicode::CodeUnitFlags::kSoftLineBreakBefore);
                    }
                }
            }
        } else if (fType == SkUnicode::BreakType::kGraphemes) {
            CFIndex idx = 0;
            while (idx < utf16Units) {
                fBreaks.push_back(idx);
                fStatuses.push_back(0);
                CFRange r = CFStringGetRangeOfComposedCharactersAtIndex(str, idx);
                CFIndex advance = std::max<CFIndex>(r.length, 1);
                idx = std::max(idx + 1, r.location + advance);
            }
            fBreaks.push_back(utf16Units);
            fStatuses.push_back(0);
        } else {
            CFOptionFlags unit = (fType == SkUnicode::BreakType::kWords)
                                         ? kCFStringTokenizerUnitWordBoundary
                                         : kCFStringTokenizerUnitSentence;
            this->tokenize(str, utf16Units, unit, loc, [&](CFRange r) {
                fBreaks.push_back(r.location);
                fStatuses.push_back(0);
            });
            if (!fBreaks.empty()) {
                fBreaks.push_back(utf16Units);
                fStatuses.push_back(0);
            }
        }

        CFRelease(str);
        return true;
    }

private:
    template <typename Callback>
    bool tokenize(CFStringRef str,
                  CFIndex len16,
                  CFOptionFlags unit,
                  const char* locale,
                  Callback&& callback) {
        if (len16 <= 0 || !str) {
            return true;
        }
        if (!fTokenizer) {
            SkUniqueCFRef<CFLocaleRef> cfLoc;
            if (locale != nullptr && locale[0] != '\0') {
                SkUniqueCFRef<CFStringRef> locStr(CFStringCreateWithCString(
                        kCFAllocatorDefault, locale, kCFStringEncodingUTF8));
                if (locStr) {
                    cfLoc.reset(CFLocaleCreate(kCFAllocatorDefault, locStr.get()));
                }
            }
            fTokenizer.reset(CFStringTokenizerCreate(
                    kCFAllocatorDefault, str, CFRangeMake(0, len16), unit, cfLoc.get()));
        } else {
            CFStringTokenizerSetString(fTokenizer.get(), str, CFRangeMake(0, len16));
        }
        if (!fTokenizer) {
            return false;
        }

        CFStringTokenizerTokenType tokenType =
                CFStringTokenizerAdvanceToNextToken(fTokenizer.get());
        CFIndex guard = 0;
        const CFIndex maxSteps = 2 * len16 + 2;
        while (tokenType != kCFStringTokenizerTokenNone && ++guard <= maxSteps) {
            CFRange r = CFStringTokenizerGetCurrentTokenRange(fTokenizer.get());
            callback(r);
            tokenType = CFStringTokenizerAdvanceToNextToken(fTokenizer.get());
        }

        // Unbind the temporary string to release non-owning byte references
        CFStringTokenizerSetString(fTokenizer.get(), CFSTR(""), CFRangeMake(0, 0));
        return true;
    }

    SkUnicode::BreakType fType;
    std::string fLocale;
    SkUniqueCFRef<CFStringTokenizerRef> fTokenizer;
    std::vector<Position> fBreaks;
    std::vector<Status> fStatuses;
    Position fIndex = 0;
};

}  // namespace

class SkUnicode_apple : public SkUnicodeHardCodedCharProperties {
public:
    SkUnicode_apple() = default;
    ~SkUnicode_apple() override = default;

    bool isControl(SkUnichar unichar) override {
        return CFCharacterSetIsLongCharacterMember(
                CFCharacterSetGetPredefined(kCFCharacterSetControl), unichar);
    }

    bool isWhitespace(SkUnichar unichar) override {
        // Non-breaking spaces and line controls must not be treated as soft break opportunities
        if (unichar == 0x00A0 || unichar == 0x2007 || unichar == 0x202F || unichar == 0x0085) {
            return false;
        }
        return CFCharacterSetIsLongCharacterMember(
                CFCharacterSetGetPredefined(kCFCharacterSetWhitespaceAndNewline), unichar);
    }

    bool isSpace(SkUnichar unichar) override {
        return CFCharacterSetIsLongCharacterMember(
                CFCharacterSetGetPredefined(kCFCharacterSetWhitespaceAndNewline), unichar);
    }

    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override {
        return fBidiFact->MakeIterator(text, count, dir);
    }

    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override {
        return fBidiFact->MakeIterator(text, count, dir);
    }

    std::unique_ptr<SkBreakIterator> makeBreakIterator(const char locale[],
                                                       BreakType breakType) override {
        return std::make_unique<SkBreakIterator_apple>(breakType, locale);
    }

    std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType breakType) override {
        return this->makeBreakIterator(nullptr, breakType);
    }

    SkString toUpper(const SkString& str) override { return this->toUpper(str, nullptr); }

    SkString toUpper(const SkString& str, const char* locale) override {
        if (str.isEmpty()) {
            return SkString();
        }
        CFStringRef cfStr = CFStringCreateWithBytesNoCopy(kCFAllocatorDefault,
                                                          (const UInt8*)str.c_str(),
                                                          str.size(),
                                                          kCFStringEncodingUTF8,
                                                          false,
                                                          kCFAllocatorNull);
        if (!cfStr) {
            return str;
        }
        CFMutableStringRef mutStr = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, cfStr);
        CFRelease(cfStr);
        if (!mutStr) {
            return str;
        }
        CFLocaleRef cfLoc = nullptr;
        if (locale != nullptr && locale[0] != '\0') {
            CFStringRef locStr =
                    CFStringCreateWithCString(kCFAllocatorDefault, locale, kCFStringEncodingUTF8);
            if (locStr) {
                cfLoc = CFLocaleCreate(kCFAllocatorDefault, locStr);
                CFRelease(locStr);
            }
        }
        CFStringUppercase(mutStr, cfLoc);
        if (cfLoc) {
            CFRelease(cfLoc);
        }
        CFIndex len = CFStringGetLength(mutStr);
        CFIndex maxBytes = CFStringGetMaximumSizeForEncoding(len, kCFStringEncodingUTF8);
        SkString result;
        result.resize(maxBytes);
        CFIndex usedBytes = 0;
        CFStringGetBytes(mutStr,
                         CFRangeMake(0, len),
                         kCFStringEncodingUTF8,
                         0,
                         false,
                         (UInt8*)result.data(),
                         maxBytes,
                         &usedBytes);
        result.resize(usedBytes);
        CFRelease(mutStr);
        return result;
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {
        if (levelsCount <= 0) {
            return;
        }
        SkASSERT(runLevels != nullptr);
        SkASSERT(logicalFromVisual != nullptr);
        fBidiFact->bidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
    }

    bool getBidiRegions(const char utf8[],
                        int utf8Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) override {
        return fBidiFact->ExtractBidi(utf8, utf8Units, dir, results);
    }

    bool getWords(const char utf8[],
                  int utf8Units,
                  const char* locale,
                  std::vector<Position>* results) override {
        results->clear();
        if (utf8Units <= 0) {
            return true;
        }

        CFStringRef str = CFStringCreateWithBytesNoCopy(kCFAllocatorDefault,
                                                        (const UInt8*)utf8,
                                                        utf8Units,
                                                        kCFStringEncodingUTF8,
                                                        false,
                                                        kCFAllocatorNull);
        if (!str) {
            return false;
        }

        CFIndex len16 = CFStringGetLength(str);
        bool ok = Tokenize(str, len16, kCFStringTokenizerUnitWordBoundary, locale, [&](CFRange r) {
            results->push_back(r.location);
        });
        if (!results->empty()) {
            results->push_back(len16);
        }

        CFRelease(str);
        return ok;
    }

    bool getUtf8Words(const char utf8[],
                      int utf8Units,
                      const char* locale,
                      std::vector<Position>* results) override {
        results->clear();
        if (utf8Units <= 0) {
            return true;
        }

        std::vector<Position> u16Words;
        if (!this->getWords(utf8, utf8Units, locale, &u16Words)) {
            return false;
        }

        STArray<256, size_t> u16ToU8;
        extractUtfConversionMapping(
                SkSpan<const char>(utf8, utf8Units),
                [&](size_t u8) { u16ToU8.push_back(u8); },
                [&](size_t u16) {});

        results->reserve(u16Words.size());
        for (auto w16 : u16Words) {
            if (SkToInt(w16) < u16ToU8.size()) {
                results->push_back(u16ToU8[w16]);
            }
        }
        return true;
    }

    bool getSentences(const char utf8[],
                      int utf8Units,
                      const char* locale,
                      std::vector<Position>* results) override {
        results->clear();
        if (utf8Units <= 0) {
            return true;
        }

        CFStringRef str = CFStringCreateWithBytesNoCopy(kCFAllocatorDefault,
                                                        (const UInt8*)utf8,
                                                        utf8Units,
                                                        kCFStringEncodingUTF8,
                                                        false,
                                                        kCFAllocatorNull);
        if (!str) {
            return false;
        }

        CFIndex len16 = CFStringGetLength(str);
        STArray<256, size_t> u16ToU8;
        extractUtfConversionMapping(
                SkSpan<const char>(utf8, utf8Units),
                [&](size_t u8) { u16ToU8.push_back(u8); },
                [&](size_t u16) {});

        bool ok = Tokenize(str, len16, kCFStringTokenizerUnitSentence, locale, [&](CFRange r) {
            if (SkToInt(r.location) < u16ToU8.size()) {
                results->push_back(u16ToU8[r.location]);
            }
        });
        if (!results->empty()) {
            results->push_back(utf8Units);
        }

        CFRelease(str);
        return ok;
    }

    bool computeCodeUnitFlags(char utf8[],
                              int utf8Units,
                              bool replaceTabs,
                              TArray<CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf8Units + 1, CodeUnitFlags::kNoCodeUnitFlag);
        if (utf8Units == 0) {
            return true;
        }

        CFStringRef str = CFStringCreateWithBytesNoCopy(kCFAllocatorDefault,
                                                        (const UInt8*)utf8,
                                                        utf8Units,
                                                        kCFStringEncodingUTF8,
                                                        false,
                                                        kCFAllocatorNull);
        if (!str) {
            return false;
        }

        CFIndex len16 = CFStringGetLength(str);
        STArray<256, size_t> u16ToU8;
        extractUtfConversionMapping(
                SkSpan<const char>(utf8, utf8Units),
                [&](size_t u8) { u16ToU8.push_back(u8); },
                [&](size_t u16) {});

        CFIndex idx = 0;
        while (idx < len16) {
            CFRange r = CFStringGetRangeOfComposedCharactersAtIndex(str, idx);
            if (r.location < u16ToU8.size()) {
                (*results)[u16ToU8[r.location]] |= CodeUnitFlags::kGraphemeStart;
            }
            CFIndex advance = std::max<CFIndex>(r.length, 1);
            idx = std::max(idx + 1, r.location + advance);
        }
        (*results)[utf8Units] |= CodeUnitFlags::kGraphemeStart;

        (*results)[0] |= CodeUnitFlags::kSoftLineBreakBefore;
        if (!fLineTokenizer) {
            fLineTokenizer.reset(CFStringTokenizerCreate(kCFAllocatorDefault,
                                                         str,
                                                         CFRangeMake(0, len16),
                                                         kCFStringTokenizerUnitLineBreak,
                                                         nullptr));
        } else {
            CFStringTokenizerSetString(fLineTokenizer.get(), str, CFRangeMake(0, len16));
        }

        if (fLineTokenizer) {
            CFStringTokenizerTokenType tokenType =
                    CFStringTokenizerAdvanceToNextToken(fLineTokenizer.get());
            CFIndex guard = 0;
            const CFIndex maxSteps = 2 * len16 + 2;
            while (tokenType != kCFStringTokenizerTokenNone && ++guard <= maxSteps) {
                CFRange r = CFStringTokenizerGetCurrentTokenRange(fLineTokenizer.get());
                CFIndex boundary16 = r.location + r.length;
                if (SkToInt(boundary16) < u16ToU8.size()) {
                    (*results)[u16ToU8[boundary16]] |= CodeUnitFlags::kSoftLineBreakBefore;
                }
                tokenType = CFStringTokenizerAdvanceToNextToken(fLineTokenizer.get());
            }
            CFStringTokenizerSetString(fLineTokenizer.get(), CFSTR(""), CFRangeMake(0, 0));
        }
        (*results)[utf8Units] |= CodeUnitFlags::kSoftLineBreakBefore;

        CFRelease(str);

        const char* current = utf8;
        const char* end = utf8 + utf8Units;
        while (current < end) {
            auto before = current - utf8;
            SkUnichar unichar = SkUTF::NextUTF8(&current, end);
            auto after = current - utf8;

            if (this->isHardBreak(unichar)) {
                (*results)[after] |=
                        (CodeUnitFlags::kHardLineBreakBefore | CodeUnitFlags::kSoftLineBreakBefore);
            }

            if (replaceTabs && this->isTabulation(unichar)) {
                results->at(before) |= CodeUnitFlags::kTabulation;
                unichar = ' ';
                utf8[before] = ' ';
            }

            CodeUnitFlags flags = CodeUnitFlags::kNoCodeUnitFlag;
            if (this->isSpace(unichar)) {
                flags |= CodeUnitFlags::kPartOfIntraWordBreak;
            }
            if (this->isWhitespace(unichar)) {
                flags |= CodeUnitFlags::kPartOfWhiteSpaceBreak;
            }
            if (this->isControl(unichar)) {
                flags |= CodeUnitFlags::kControl;
            }
            if (this->isIdeographic(unichar)) {
                flags |= CodeUnitFlags::kIdeographic;
            }

            for (auto i = before; i < after; ++i) {
                results->at(i) |= flags;
            }
        }

        return true;
    }

    bool computeCodeUnitFlags(char16_t utf16[],
                              int utf16Units,
                              bool replaceTabs,
                              TArray<CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf16Units + 1, CodeUnitFlags::kNoCodeUnitFlag);
        if (utf16Units == 0) {
            return true;
        }

        CFStringRef str = CFStringCreateWithCharactersNoCopy(
                kCFAllocatorDefault, (const UniChar*)utf16, utf16Units, kCFAllocatorNull);
        if (!str) {
            return false;
        }

        CFIndex idx = 0;
        while (idx < utf16Units) {
            CFRange r = CFStringGetRangeOfComposedCharactersAtIndex(str, idx);
            (*results)[r.location] |= CodeUnitFlags::kGraphemeStart;
            CFIndex advance = std::max<CFIndex>(r.length, 1);
            idx = std::max(idx + 1, r.location + advance);
        }
        (*results)[utf16Units] |= CodeUnitFlags::kGraphemeStart;

        (*results)[0] |= CodeUnitFlags::kSoftLineBreakBefore;
        if (!fLineTokenizer) {
            fLineTokenizer.reset(CFStringTokenizerCreate(kCFAllocatorDefault,
                                                         str,
                                                         CFRangeMake(0, utf16Units),
                                                         kCFStringTokenizerUnitLineBreak,
                                                         nullptr));
        } else {
            CFStringTokenizerSetString(fLineTokenizer.get(), str, CFRangeMake(0, utf16Units));
        }

        if (fLineTokenizer) {
            CFStringTokenizerTokenType tokenType =
                    CFStringTokenizerAdvanceToNextToken(fLineTokenizer.get());
            CFIndex guard = 0;
            const CFIndex maxSteps = 2 * utf16Units + 2;
            while (tokenType != kCFStringTokenizerTokenNone && ++guard <= maxSteps) {
                CFRange r = CFStringTokenizerGetCurrentTokenRange(fLineTokenizer.get());
                (*results)[r.location + r.length] |= CodeUnitFlags::kSoftLineBreakBefore;
                tokenType = CFStringTokenizerAdvanceToNextToken(fLineTokenizer.get());
            }
            CFStringTokenizerSetString(fLineTokenizer.get(), CFSTR(""), CFRangeMake(0, 0));
        }
        (*results)[utf16Units] |= CodeUnitFlags::kSoftLineBreakBefore;

        CFRelease(str);

        this->forEachCodepoint((char16_t*)&utf16[0],
                               utf16Units,
                               [this, results, replaceTabs, &utf16](
                                       SkUnichar unichar, int32_t start, int32_t end) {
                                   if (this->isHardBreak(unichar)) {
                                       (*results)[end] |= (CodeUnitFlags::kHardLineBreakBefore |
                                                           CodeUnitFlags::kSoftLineBreakBefore);
                                   }
                                   if (replaceTabs && this->isTabulation(unichar)) {
                                       results->at(start) |= CodeUnitFlags::kTabulation;
                                       unichar = ' ';
                                       utf16[start] = ' ';
                                   }
                                   CodeUnitFlags flags = CodeUnitFlags::kNoCodeUnitFlag;
                                   if (this->isSpace(unichar)) {
                                       flags |= CodeUnitFlags::kPartOfIntraWordBreak;
                                   }
                                   if (this->isWhitespace(unichar)) {
                                       flags |= CodeUnitFlags::kPartOfWhiteSpaceBreak;
                                   }
                                   if (this->isControl(unichar)) {
                                       flags |= CodeUnitFlags::kControl;
                                   }
                                   if (this->isIdeographic(unichar)) {
                                       flags |= CodeUnitFlags::kIdeographic;
                                   }
                                   for (auto i = start; i < end; ++i) {
                                       results->at(i) |= flags;
                                   }
                               });

        return true;
    }

private:
    sk_sp<SkBidiFactory> fBidiFact = sk_make_sp<SkBidiSubsetFactory>();
    SkUniqueCFRef<CFStringTokenizerRef> fLineTokenizer;
};

namespace SkUnicodes::Apple {
sk_sp<SkUnicode> Make() { return sk_make_sp<SkUnicode_apple>(); }
}  // namespace SkUnicodes::Apple
