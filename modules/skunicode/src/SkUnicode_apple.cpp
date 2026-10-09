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
    CFLocaleRef cfLoc = nullptr;
    if (locale != nullptr && locale[0] != '\0') {
        CFStringRef locStr =
                CFStringCreateWithCString(kCFAllocatorDefault, locale, kCFStringEncodingUTF8);
        if (locStr) {
            cfLoc = CFLocaleCreate(kCFAllocatorDefault, locStr);
            CFRelease(locStr);
        }
    }

    CFStringTokenizerRef tokenizer =
            CFStringTokenizerCreate(kCFAllocatorDefault, str, CFRangeMake(0, len16), unit, cfLoc);
    if (cfLoc) {
        CFRelease(cfLoc);
    }
    if (!tokenizer) {
        return false;
    }

    CFStringTokenizerTokenType tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer);
    CFIndex guard = 0;
    const CFIndex maxSteps = 2 * len16 + 2;
    while (tokenType != kCFStringTokenizerTokenNone && ++guard <= maxSteps) {
        CFRange r = CFStringTokenizerGetCurrentTokenRange(tokenizer);
        callback(r);
        tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer);
    }

    CFRelease(tokenizer);
    return true;
}

class SkBreakIterator_apple : public SkBreakIterator {
public:
    SkBreakIterator_apple(SkUnicode::BreakType type, const char* locale)
            : fType(type), fLocale(locale ? locale : "") {}

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
        if (fType == SkUnicode::BreakType::kLines) {
            fBreaks.push_back(0);
            fStatuses.push_back(SkUnicode::CodeUnitFlags::kSoftLineBreakBefore);
            Tokenize(str, len16, kCFStringTokenizerUnitLineBreak, loc, [&](CFRange r) {
                fBreaks.push_back(r.location + r.length);
                fStatuses.push_back(SkUnicode::CodeUnitFlags::kSoftLineBreakBefore);
            });

            const char* current = utftext8;
            const char* end = utftext8 + utf8Units;
            std::vector<size_t> u8ToU16;
            SkUnicode::extractUtfConversionMapping(
                    SkSpan<const char>(utftext8, utf8Units),
                    [&](size_t u8) {},
                    [&](size_t u16) { u8ToU16.push_back(u16); });
            while (current < end) {
                SkUnichar unichar = SkUTF::NextUTF8(&current, end);
                if (unichar == '\n' || unichar == u'\u2028') {
                    auto after = current - utftext8;
                    if (SkToSizeT(after) < u8ToU16.size()) {
                        Position pos16 = u8ToU16[after];
                        auto it = std::find(fBreaks.begin(), fBreaks.end(), pos16);
                        if (it != fBreaks.end()) {
                            auto idx = std::distance(fBreaks.begin(), it);
                            fStatuses[idx] |= SkUnicode::CodeUnitFlags::kHardLineBreakBefore;
                        } else {
                            fBreaks.push_back(pos16);
                            fStatuses.push_back(SkUnicode::CodeUnitFlags::kHardLineBreakBefore |
                                                SkUnicode::CodeUnitFlags::kSoftLineBreakBefore);
                        }
                    }
                }
            }
        } else if (fType == SkUnicode::BreakType::kGraphemes) {
            CFIndex idx = 0;
            while (idx < len16) {
                fBreaks.push_back(idx);
                fStatuses.push_back(0);
                CFRange r = CFStringGetRangeOfComposedCharactersAtIndex(str, idx);
                CFIndex advance = std::max<CFIndex>(r.length, 1);
                idx = std::max(idx + 1, r.location + advance);
            }
            fBreaks.push_back(len16);
            fStatuses.push_back(0);
        } else {
            CFOptionFlags unit = (fType == SkUnicode::BreakType::kWords)
                                         ? kCFStringTokenizerUnitWordBoundary
                                         : kCFStringTokenizerUnitSentence;
            Tokenize(str, len16, unit, loc, [&](CFRange r) {
                fBreaks.push_back(r.location);
                fStatuses.push_back(0);
            });
            if (!fBreaks.empty()) {
                fBreaks.push_back(len16);
                fStatuses.push_back(0);
            }
        }

        CFRelease(str);
        return true;
    }

    bool setText(const char16_t utftext16[], int utf16Units) override {
        SkString utf8 = SkUnicode::convertUtf16ToUtf8(utftext16, utf16Units);
        return this->setText(utf8.c_str(), utf8.size());
    }

private:
    SkUnicode::BreakType fType;
    std::string fLocale;
    std::vector<Position> fBreaks;
    std::vector<Status> fStatuses;
    Position fIndex = 0;
};

}  // namespace

class SkUnicode_apple : public SkUnicodeHardCodedCharProperties {
public:
    SkUnicode_apple() = default;
    ~SkUnicode_apple() override = default;

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

        std::vector<size_t> u16ToU8;
        extractUtfConversionMapping(
                SkSpan<const char>(utf8, utf8Units),
                [&](size_t u8) { u16ToU8.push_back(u8); },
                [&](size_t u16) {});

        results->reserve(u16Words.size());
        for (auto w16 : u16Words) {
            if (SkToSizeT(w16) < u16ToU8.size()) {
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
        std::vector<size_t> u16ToU8;
        extractUtfConversionMapping(
                SkSpan<const char>(utf8, utf8Units),
                [&](size_t u8) { u16ToU8.push_back(u8); },
                [&](size_t u16) {});

        bool ok = Tokenize(str, len16, kCFStringTokenizerUnitSentence, locale, [&](CFRange r) {
            if (SkToSizeT(r.location) < u16ToU8.size()) {
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
        std::vector<size_t> u16ToU8;
        extractUtfConversionMapping(
                SkSpan<const char>(utf8, utf8Units),
                [&](size_t u8) { u16ToU8.push_back(u8); },
                [&](size_t u16) {});

        CFIndex idx = 0;
        while (idx < len16) {
            CFRange r = CFStringGetRangeOfComposedCharactersAtIndex(str, idx);
            if (SkToSizeT(r.location) < u16ToU8.size()) {
                (*results)[u16ToU8[r.location]] |= CodeUnitFlags::kGraphemeStart;
            }
            CFIndex advance = std::max<CFIndex>(r.length, 1);
            idx = std::max(idx + 1, r.location + advance);
        }
        (*results)[utf8Units] |= CodeUnitFlags::kGraphemeStart;

        (*results)[0] |= CodeUnitFlags::kSoftLineBreakBefore;
        Tokenize(str, len16, kCFStringTokenizerUnitLineBreak, nullptr, [&](CFRange r) {
            CFIndex boundary16 = r.location + r.length;
            if (SkToSizeT(boundary16) < u16ToU8.size()) {
                (*results)[u16ToU8[boundary16]] |= CodeUnitFlags::kSoftLineBreakBefore;
            }
        });
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

            for (auto i = before; i < after; ++i) {
                if (this->isSpace(unichar)) {
                    results->at(i) |= CodeUnitFlags::kPartOfIntraWordBreak;
                }
                if (this->isWhitespace(unichar)) {
                    results->at(i) |= CodeUnitFlags::kPartOfWhiteSpaceBreak;
                }
                if (this->isControl(unichar)) {
                    results->at(i) |= CodeUnitFlags::kControl;
                }
                if (this->isIdeographic(unichar)) {
                    results->at(i) |= CodeUnitFlags::kIdeographic;
                }
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
        Tokenize(str, utf16Units, kCFStringTokenizerUnitLineBreak, nullptr, [&](CFRange r) {
            (*results)[r.location + r.length] |= CodeUnitFlags::kSoftLineBreakBefore;
        });
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
                                   for (auto i = start; i < end; ++i) {
                                       if (replaceTabs && this->isTabulation(unichar)) {
                                           results->at(i) |= CodeUnitFlags::kTabulation;
                                           unichar = ' ';
                                           utf16[start] = ' ';
                                       }
                                       if (this->isSpace(unichar)) {
                                           results->at(i) |= CodeUnitFlags::kPartOfIntraWordBreak;
                                       }
                                       if (this->isWhitespace(unichar)) {
                                           results->at(i) |= CodeUnitFlags::kPartOfWhiteSpaceBreak;
                                       }
                                       if (this->isControl(unichar)) {
                                           results->at(i) |= CodeUnitFlags::kControl;
                                       }
                                       if (this->isIdeographic(unichar)) {
                                           results->at(i) |= CodeUnitFlags::kIdeographic;
                                       }
                                   }
                               });

        return true;
    }

private:
    sk_sp<SkBidiFactory> fBidiFact = sk_make_sp<SkBidiSubsetFactory>();
};

namespace SkUnicodes::Apple {
sk_sp<SkUnicode> Make() { return sk_make_sp<SkUnicode_apple>(); }
}  // namespace SkUnicodes::Apple
