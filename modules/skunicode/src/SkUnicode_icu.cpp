/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkOnce.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/src/SkUnicode_icu.h"
#include "modules/skunicode/src/SkUnicode_icu_bidi.h"
#include "src/base/SkBitmaskEnum.h"
#include "src/base/SkUTF.h"
#include "src/core/SkTHash.h"
#include <unicode/umachine.h>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#if defined(SK_USING_THIRD_PARTY_ICU)
#include "SkLoadICU.h"
#endif

using namespace skia_private;

const SkICULib* SkGetICULib() {
    static const auto gICU = SkLoadICULib();

    return gICU.get();
}

// sk_* wrappers for ICU funcs
#define SKICU_FUNC(funcname)                                                                \
    template <typename... Args>                                                             \
    auto sk_##funcname(Args&&... args) -> decltype(funcname(std::forward<Args>(args)...)) { \
        return SkGetICULib()->f_##funcname(std::forward<Args>(args)...);                    \
    }                                                                                       \

SKICU_EMIT_FUNCS
#undef SKICU_FUNC

static inline UBreakIterator* sk_ubrk_clone(const UBreakIterator* bi, UErrorCode* status) {
    const auto* icu = SkGetICULib();
    SkASSERT(icu->f_ubrk_clone_ || icu->f_ubrk_safeClone_);
    return icu->f_ubrk_clone_
        ? icu->f_ubrk_clone_(bi, status)
        : icu->f_ubrk_safeClone_(bi, nullptr, nullptr, status);
}

static UText* utext_close_wrapper(UText* ut) {
    return sk_utext_close(ut);
}
static void ubrk_close_wrapper(UBreakIterator* bi) {
    sk_ubrk_close(bi);
}

using ICUUText = std::unique_ptr<UText, SkFunctionObject<utext_close_wrapper>>;
using ICUBreakIterator = std::unique_ptr<UBreakIterator, SkFunctionObject<ubrk_close_wrapper>>;
/** Replaces invalid utf-8 sequences with REPLACEMENT CHARACTER U+FFFD. */
static inline SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}

static UBreakIteratorType convertType(SkUnicode::BreakType type) {
    switch (type) {
        case SkUnicode::BreakType::kLines: return UBRK_LINE;
        case SkUnicode::BreakType::kGraphemes: return UBRK_CHARACTER;
        case SkUnicode::BreakType::kWords: return UBRK_WORD;
        default:
            return UBRK_CHARACTER;
    }
}

class SkBreakIterator_icu : public SkBreakIterator {
    ICUBreakIterator fBreakIterator;
    Position fLastResult;
 public:
    explicit SkBreakIterator_icu(ICUBreakIterator iter)
            : fBreakIterator(std::move(iter))
            , fLastResult(0) {}
    Position first() override { return fLastResult = sk_ubrk_first(fBreakIterator.get()); }
    Position current() override { return fLastResult = sk_ubrk_current(fBreakIterator.get()); }
    Position next() override { return fLastResult = sk_ubrk_next(fBreakIterator.get()); }
    Status status() override { return sk_ubrk_getRuleStatus(fBreakIterator.get()); }
    bool isDone() override { return fLastResult == UBRK_DONE; }

    bool setText(const char utftext8[], int utf8Units) override {
        UErrorCode status = U_ZERO_ERROR;
        ICUUText text(sk_utext_openUTF8(nullptr, &utftext8[0], utf8Units, &status));

        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", sk_u_errorName(status));
            return false;
        }
        SkASSERT(text);
        sk_ubrk_setUText(fBreakIterator.get(), text.get(), &status);
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", sk_u_errorName(status));
            return false;
        }
        fLastResult = 0;
        return true;
    }
    bool setText(const char16_t utftext16[], int utf16Units) override {
        UErrorCode status = U_ZERO_ERROR;
        ICUUText text(sk_utext_openUChars(nullptr, reinterpret_cast<const UChar*>(&utftext16[0]),
                                          utf16Units, &status));

        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", sk_u_errorName(status));
            return false;
        }
        SkASSERT(text);
        sk_ubrk_setUText(fBreakIterator.get(), text.get(), &status);
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", sk_u_errorName(status));
            return false;
        }
        fLastResult = 0;
        return true;
    }
};

class SkIcuBreakIteratorCache {
    THashMap<SkUnicode::BreakType, ICUBreakIterator> fBreakCache;
    SkMutex fBreakCacheMutex;

 public:
    static SkIcuBreakIteratorCache& get() {
        static SkIcuBreakIteratorCache instance;
        return instance;
    }

    ICUBreakIterator makeBreakIterator(SkUnicode::BreakType type) {
        UErrorCode status = U_ZERO_ERROR;
        ICUBreakIterator* cachedIterator;
        {
            SkAutoMutexExclusive lock(fBreakCacheMutex);
            cachedIterator = fBreakCache.find(type);
            if (!cachedIterator) {
                ICUBreakIterator newIterator(sk_ubrk_open(convertType(type), sk_uloc_getDefault(),
                                                          nullptr, 0, &status));
                if (U_FAILURE(status)) {
                    SkDEBUGF("Break error: %s", sk_u_errorName(status));
                } else {
                    cachedIterator = fBreakCache.set(type, std::move(newIterator));
                }
            }
        }
        ICUBreakIterator iterator;
        if (cachedIterator) {
            iterator.reset(sk_ubrk_clone(cachedIterator->get(), &status));
            if (U_FAILURE(status)) {
                SkDEBUGF("Break error: %s", sk_u_errorName(status));
            }
        }
        return iterator;
    }
};

class SkUnicode_icu : public SkUnicode {

    std::unique_ptr<SkUnicode> copy() override {
        return std::make_unique<SkUnicode_icu>();
    }

    static bool extractWords(uint16_t utf16[], int utf16Units, const char* locale,  std::vector<Position>* words) {

        UErrorCode status = U_ZERO_ERROR;

        ICUBreakIterator iterator = SkIcuBreakIteratorCache::get().makeBreakIterator(BreakType::kWords);
        if (!iterator) {
            SkDEBUGF("Break error: %s", sk_u_errorName(status));
            return false;
        }
        SkASSERT(iterator);

        ICUUText utf16UText(sk_utext_openUChars(nullptr, (UChar*)utf16, utf16Units, &status));
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", sk_u_errorName(status));
            return false;
        }

        sk_ubrk_setUText(iterator.get(), utf16UText.get(), &status);
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", sk_u_errorName(status));
            return false;
        }

        // Get the words
        int32_t pos = sk_ubrk_first(iterator.get());
        while (pos != UBRK_DONE) {
            words->emplace_back(pos);
            pos = sk_ubrk_next(iterator.get());
        }

        return true;
    }

    static bool extractPositions
        (const char utf8[], int utf8Units, BreakType type, std::function<void(int, int)> setBreak) {

        UErrorCode status = U_ZERO_ERROR;
        ICUUText text(sk_utext_openUTF8(nullptr, &utf8[0], utf8Units, &status));

        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", sk_u_errorName(status));
            return false;
        }
        SkASSERT(text);

        ICUBreakIterator iterator = SkIcuBreakIteratorCache::get().makeBreakIterator(type);
        if (!iterator) {
            return false;
        }

        sk_ubrk_setUText(iterator.get(), text.get(), &status);
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", sk_u_errorName(status));
            return false;
        }

        auto iter = iterator.get();
        int32_t pos = sk_ubrk_first(iter);
        while (pos != UBRK_DONE) {
            int s = type == SkUnicode::BreakType::kLines
                        ? UBRK_LINE_SOFT
                        : sk_ubrk_getRuleStatus(iter);
            setBreak(pos, s);
            pos = sk_ubrk_next(iter);
        }

        if (type == SkUnicode::BreakType::kLines) {
            // This is a workaround for https://bugs.chromium.org/p/skia/issues/detail?id=10715
            // (ICU line break iterator does not work correctly on Thai text with new lines)
            // So, we only use the iterator to collect soft line breaks and
            // scan the text for all hard line breaks ourselves
            const char* end = utf8 + utf8Units;
            const char* ch = utf8;
            while (ch < end) {
                auto unichar = utf8_next(&ch, end);
                if (SkUnicode_icu::isHardLineBreak(unichar)) {
                    setBreak(ch - utf8, UBRK_LINE_HARD);
                }
            }
        }
        return true;
    }

    bool isControl(SkUnichar utf8) override {
        return sk_u_iscntrl(utf8);
    }

    bool isWhitespace(SkUnichar utf8) override {
        return sk_u_isWhitespace(utf8);
    }

    bool isSpace(SkUnichar utf8) override {
        return sk_u_isspace(utf8);
    }

    bool isHardBreak(SkUnichar utf8) override {
        return SkUnicode_icu::isHardLineBreak(utf8);
    }

    bool isEmoji(SkUnichar unichar) override {
        return sk_u_hasBinaryProperty(unichar, UCHAR_EMOJI) ||
               sk_u_hasBinaryProperty(unichar, UCHAR_EMOJI_COMPONENT);
    }

    bool isIdeographic(SkUnichar unichar) override {
        return sk_u_hasBinaryProperty(unichar, UCHAR_IDEOGRAPHIC);
    }

    bool isTabulation(SkUnichar utf8) override {
        return utf8 == '\t';
    }

    static bool isHardLineBreak(SkUnichar utf8) {
        auto property = sk_u_getIntPropertyValue(utf8, UCHAR_LINE_BREAK);
        return property == U_LB_LINE_FEED || property == U_LB_MANDATORY_BREAK;
    }

public:
    ~SkUnicode_icu() override { }
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[], int count,
                                                     SkBidiIterator::Direction dir) override {
        return SkUnicode_IcuBidi::MakeIterator(text, count, dir);
    }
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override {
        return SkUnicode_IcuBidi::MakeIterator(text, count, dir);
    }
    std::unique_ptr<SkBreakIterator> makeBreakIterator(const char locale[],
                                                       BreakType breakType) override {
        UErrorCode status = U_ZERO_ERROR;
        ICUBreakIterator iterator(sk_ubrk_open(convertType(breakType), locale, nullptr, 0,
                                               &status));
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", sk_u_errorName(status));
            return nullptr;
        }
        return std::unique_ptr<SkBreakIterator>(new SkBreakIterator_icu(std::move(iterator)));
    }
    std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType breakType) override {
        return makeBreakIterator(sk_uloc_getDefault(), breakType);
    }

    SkString toUpper(const SkString& str) override {
        // Convert to UTF16 since that's what ICU wants.
        auto str16 = SkUnicode::convertUtf8ToUtf16(str.c_str(), str.size());

        UErrorCode icu_err = U_ZERO_ERROR;
        const auto upper16len = sk_u_strToUpper(nullptr, 0, (UChar*)(str16.c_str()), str16.size(),
                                                nullptr, &icu_err);
        if (icu_err != U_BUFFER_OVERFLOW_ERROR || upper16len <= 0) {
            return SkString();
        }

        AutoSTArray<128, uint16_t> upper16(upper16len);
        icu_err = U_ZERO_ERROR;
        sk_u_strToUpper((UChar*)(upper16.get()), SkToS32(upper16.size()),
                        (UChar*)(str16.c_str()), str16.size(),
                        nullptr, &icu_err);
        SkASSERT(!U_FAILURE(icu_err));

        // ... and back to utf8 'cause that's what we want.
        return convertUtf16ToUtf8((char16_t*)upper16.get(), upper16.size());
    }

    bool getBidiRegions(const char utf8[],
                        int utf8Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) override {
        return SkUnicode_IcuBidi::ExtractBidi(utf8, utf8Units, dir, results);
    }

    bool getWords(const char utf8[], int utf8Units, const char* locale, std::vector<Position>* results) override {

        // Convert to UTF16 since we want the results in utf16
        auto utf16 = convertUtf8ToUtf16(utf8, utf8Units);
        return SkUnicode_icu::extractWords((uint16_t*)utf16.c_str(), utf16.size(), locale, results);
    }

    bool computeCodeUnitFlags(char utf8[], int utf8Units, bool replaceTabs,
                          TArray<SkUnicode::CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf8Units + 1, CodeUnitFlags::kNoCodeUnitFlag);

        SkUnicode_icu::extractPositions(utf8, utf8Units, BreakType::kLines, [&](int pos,
                                                                       int status) {
            (*results)[pos] |= status == UBRK_LINE_HARD
                                    ? CodeUnitFlags::kHardLineBreakBefore
                                    : CodeUnitFlags::kSoftLineBreakBefore;
        });

        SkUnicode_icu::extractPositions(utf8, utf8Units, BreakType::kGraphemes, [&](int pos,
                                                                       int status) {
            (*results)[pos] |= CodeUnitFlags::kGraphemeStart;
        });

        const char* current = utf8;
        const char* end = utf8 + utf8Units;
        while (current < end) {
            auto before = current - utf8;
            SkUnichar unichar = SkUTF::NextUTF8(&current, end);
            if (unichar < 0) unichar = 0xFFFD;
            auto after = current - utf8;
            if (replaceTabs && this->isTabulation(unichar)) {
                results->at(before) |= SkUnicode::kTabulation;
                if (replaceTabs) {
                    unichar = ' ';
                    utf8[before] = ' ';
                }
            }
            for (auto i = before; i < after; ++i) {
                if (this->isSpace(unichar)) {
                    results->at(i) |= SkUnicode::kPartOfIntraWordBreak;
                }
                if (this->isWhitespace(unichar)) {
                    results->at(i) |= SkUnicode::kPartOfWhiteSpaceBreak;
                }
                if (this->isControl(unichar)) {
                    results->at(i) |= SkUnicode::kControl;
                }
                if (this->isIdeographic(unichar)) {
                    results->at(i) |= SkUnicode::kIdeographic;
                }
            }
        }

        return true;
    }

    bool computeCodeUnitFlags(char16_t utf16[], int utf16Units, bool replaceTabs,
                          TArray<SkUnicode::CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf16Units + 1, CodeUnitFlags::kNoCodeUnitFlag);

        // Get white spaces
        this->forEachCodepoint((char16_t*)&utf16[0], utf16Units,
           [this, results, replaceTabs, &utf16](SkUnichar unichar, int32_t start, int32_t end) {
                for (auto i = start; i < end; ++i) {
                    if (replaceTabs && this->isTabulation(unichar)) {
                        results->at(i) |= SkUnicode::kTabulation;
                    if (replaceTabs) {
                            unichar = ' ';
                            utf16[start] = ' ';
                        }
                    }
                    if (this->isSpace(unichar)) {
                        results->at(i) |= SkUnicode::kPartOfIntraWordBreak;
                    }
                    if (this->isWhitespace(unichar)) {
                        results->at(i) |= SkUnicode::kPartOfWhiteSpaceBreak;
                    }
                    if (this->isControl(unichar)) {
                        results->at(i) |= SkUnicode::kControl;
                    }
                }
           });
        // Get graphemes
        this->forEachBreak((char16_t*)&utf16[0],
                           utf16Units,
                           SkUnicode::BreakType::kGraphemes,
                           [results](SkBreakIterator::Position pos, SkBreakIterator::Status) {
                               (*results)[pos] |= CodeUnitFlags::kGraphemeStart;
                           });
        // Get line breaks
        this->forEachBreak(
                (char16_t*)&utf16[0],
                utf16Units,
                SkUnicode::BreakType::kLines,
                [results](SkBreakIterator::Position pos, SkBreakIterator::Status status) {
                    if (status ==
                        (SkBreakIterator::Status)SkUnicode::LineBreakType::kHardLineBreak) {
                        // Hard line breaks clears off all the other flags
                        // TODO: Treat \n as a formatting mark and do not pass it to SkShaper
                        (*results)[pos-1] = CodeUnitFlags::kHardLineBreakBefore;
                    } else {
                        (*results)[pos] |= CodeUnitFlags::kSoftLineBreakBefore;
                    }
                });

        return true;
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {
        SkUnicode_IcuBidi::bidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
    }
};

std::unique_ptr<SkUnicode> SkUnicode::MakeIcuBasedUnicode() {
    #if defined(SK_USING_THIRD_PARTY_ICU)
    if (!SkLoadICU()) {
        static SkOnce once;
        once([] { SkDEBUGF("SkLoadICU() failed!\n"); });
        return nullptr;
    }
    #endif

    return SkGetICULib()
        ? std::make_unique<SkUnicode_icu>()
        : nullptr;
}
