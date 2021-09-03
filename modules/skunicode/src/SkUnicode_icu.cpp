/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "include/core/SkString.h"
#include "include/private/SkMutex.h"
#include "include/private/SkOnce.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkTHash.h"
#include "include/private/SkTemplates.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/src/SkUnicode_icu.h"
#include "src/utils/SkUTF.h"
#include <vector>
#include <functional>

#if defined(SK_USING_THIRD_PARTY_ICU)
#include "SkLoadICU.h"
#endif

static const SkICULib* ICULib() {
    static const auto gICU = SkLoadICULib();

    return gICU.get();
}

// sk_* wrappers for ICU funcs
#define SKICU_FUNC(funcname)                                                                \
    template <typename... Args>                                                             \
    auto sk_##funcname(Args&&... args) -> decltype(funcname(std::forward<Args>(args)...)) { \
        return ICULib()->f_##funcname(std::forward<Args>(args)...);                         \
    }                                                                                       \

SKICU_EMIT_FUNCS
#undef SKICU_FUNC

static inline UBreakIterator* sk_ubrk_clone(const UBreakIterator* bi, UErrorCode* status) {
    const auto* icu = ICULib();
    SkASSERT(icu->f_ubrk_clone_ || icu->f_ubrk_safeClone_);
    return icu->f_ubrk_clone_
        ? icu->f_ubrk_clone_(bi, status)
        : icu->f_ubrk_safeClone_(bi, nullptr, nullptr, status);
}

static void ubidi_close_wrapper(UBiDi* bidi) {
    sk_ubidi_close(bidi);
}
static UText* utext_close_wrapper(UText* ut) {
    return sk_utext_close(ut);
}
static void ubrk_close_wrapper(UBreakIterator* bi) {
    sk_ubrk_close(bi);
}

using SkUnicodeBidi = std::unique_ptr<UBiDi, SkFunctionWrapper<decltype(ubidi_close),
                                                               ubidi_close_wrapper>>;
using ICUUText = std::unique_ptr<UText, SkFunctionWrapper<decltype(utext_close),
                                                         utext_close_wrapper>>;
using ICUBreakIterator = std::unique_ptr<UBreakIterator, SkFunctionWrapper<decltype(ubrk_close),
                                                                           ubrk_close_wrapper>>;
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

class SkBidiIterator_icu : public SkBidiIterator {
    SkUnicodeBidi fBidi;
public:
    explicit SkBidiIterator_icu(SkUnicodeBidi bidi) : fBidi(std::move(bidi)) {}
    Position getLength() override { return sk_ubidi_getLength(fBidi.get()); }
    Level getLevelAt(Position pos) override { return sk_ubidi_getLevelAt(fBidi.get(), pos); }

    static std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t utf16[], int utf16Units, Direction dir) {
        UErrorCode status = U_ZERO_ERROR;
        SkUnicodeBidi bidi(sk_ubidi_openSized(utf16Units, 0, &status));
        if (U_FAILURE(status)) {
            SkDEBUGF("Bidi error: %s", sk_u_errorName(status));
            return nullptr;
        }
        SkASSERT(bidi);
        uint8_t bidiLevel = (dir == SkBidiIterator::kLTR) ? UBIDI_LTR : UBIDI_RTL;
        // The required lifetime of utf16 isn't well documented.
        // It appears it isn't used after ubidi_setPara except through ubidi_getText.
        sk_ubidi_setPara(bidi.get(), (const UChar*)utf16, utf16Units, bidiLevel, nullptr, &status);
        if (U_FAILURE(status)) {
            SkDEBUGF("Bidi error: %s", sk_u_errorName(status));
            return nullptr;
        }
        return std::unique_ptr<SkBidiIterator>(new SkBidiIterator_icu(std::move(bidi)));
    }

    // ICU bidi iterator works with utf16 but clients (Flutter for instance) may work with utf8
    // This method allows the clients not to think about all these details
    static std::unique_ptr<SkBidiIterator> makeBidiIterator(const char utf8[], int utf8Units, Direction dir) {
        // Convert utf8 into utf16 since ubidi only accepts utf16
        if (!SkTFitsIn<int32_t>(utf8Units)) {
            SkDEBUGF("Bidi error: text too long");
            return nullptr;
        }

        // Getting the length like this seems to always set U_BUFFER_OVERFLOW_ERROR
        int utf16Units = SkUTF::UTF8ToUTF16(nullptr, 0, utf8, utf8Units);
        if (utf16Units < 0) {
            SkDEBUGF("Bidi error: Invalid utf8 input");
            return nullptr;
        }
        std::unique_ptr<uint16_t[]> utf16(new uint16_t[utf16Units]);
        SkDEBUGCODE(int dstLen =) SkUTF::UTF8ToUTF16(utf16.get(), utf16Units, utf8, utf8Units);
        SkASSERT(dstLen == utf16Units);

        return makeBidiIterator(utf16.get(), utf16Units, dir);
    }

    // This method returns the final results only: a list of bidi regions
    // (this is all SkParagraph really needs; SkShaper however uses the iterator itself)
    static std::vector<Region> getBidiRegions(const char utf8[], int utf8Units, Direction dir) {

        auto bidiIterator = makeBidiIterator(utf8, utf8Units, dir);
        std::vector<Region> bidiRegions;
        const char* start8 = utf8;
        const char* end8 = utf8 + utf8Units;
        SkBidiIterator::Level currentLevel = 0;

        Position pos8 = 0;
        Position pos16 = 0;
        Position end16 = bidiIterator->getLength();
        while (pos16 < end16) {
            auto level = bidiIterator->getLevelAt(pos16);
            if (pos16 == 0) {
                currentLevel = level;
            } else if (level != currentLevel) {
                auto end = SkTo<Position>(start8 - utf8);
                bidiRegions.emplace_back(pos8, end, currentLevel);
                currentLevel = level;
                pos8 = end;
            }
            SkUnichar u = utf8_next(&start8, end8);
            pos16 += SkUTF::ToUTF16(u);
        }
        auto end = start8 - utf8;
        if (end != pos8) {
            bidiRegions.emplace_back(pos8, end, currentLevel);
        }
        return bidiRegions;
    }
};

void SkBidiIterator::ReorderVisual(const Level runLevels[], int levelsCount,
                                   int32_t logicalFromVisual[]) {
    sk_ubidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
}

class SkBreakIterator_icu : public SkBreakIterator {
    ICUBreakIterator fBreakIterator;
    Position fLastResult;
 public:
    explicit SkBreakIterator_icu(ICUBreakIterator iter)
        : fBreakIterator(std::move(iter)), fLastResult(0) {}
    Position first() override
      { return fLastResult = sk_ubrk_first(fBreakIterator.get()); }
    Position current() override
      { return fLastResult = sk_ubrk_current(fBreakIterator.get()); }
    Position next() override
      { return fLastResult = sk_ubrk_next(fBreakIterator.get()); }
    Position preceding(Position offset) override
        { return fLastResult = sk_ubrk_preceding(fBreakIterator.get(), offset); }
    Position following(Position offset) override
        { return fLastResult = sk_ubrk_following(fBreakIterator.get(), offset);}
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
    SkTHashMap<SkUnicode::BreakType, ICUBreakIterator> fBreakCache;
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

class SkScriptIterator_icu : public SkScriptIterator {
 public:
   bool getScript(SkUnichar u, ScriptID* script) override {
        UErrorCode status = U_ZERO_ERROR;
        UScriptCode scriptCode = sk_uscript_getScript(u, &status);
        if (U_FAILURE (status)) {
            return false;
        }
        if (script) {
            *script = (ScriptID)scriptCode;
        }
        return true;
   }

   static std::unique_ptr<SkScriptIterator> makeScriptIterator() {
        return std::unique_ptr<SkScriptIterator>(new SkScriptIterator_icu());
   }
};

class SkUnicode_icu : public SkUnicode {
    static bool extractBidi(const char utf8[],
                            int utf8Units,
                            TextDirection dir,
                            std::vector<BidiRegion>* bidiRegions) {

        // Convert to UTF16 since for now bidi iterator only operates on utf16
        std::unique_ptr<uint16_t[]> utf16;
        auto utf16Units = utf8ToUtf16(utf8, utf8Units, &utf16);
        if (utf16Units < 0) {
            return false;
        }

        // Create bidi iterator
        UErrorCode status = U_ZERO_ERROR;
        SkUnicodeBidi bidi(sk_ubidi_openSized(utf16Units, 0, &status));
        if (U_FAILURE(status)) {
            SkDEBUGF("Bidi error: %s", sk_u_errorName(status));
            return false;
        }
        SkASSERT(bidi);
        uint8_t bidiLevel = (dir == TextDirection::kLTR) ? UBIDI_LTR : UBIDI_RTL;
        // The required lifetime of utf16 isn't well documented.
        // It appears it isn't used after ubidi_setPara except through ubidi_getText.
        sk_ubidi_setPara(bidi.get(), (const UChar*)utf16.get(), utf16Units, bidiLevel, nullptr,
                         &status);
        if (U_FAILURE(status)) {
            SkDEBUGF("Bidi error: %s", sk_u_errorName(status));
            return false;
        }

        // Iterate through bidi regions and the result positions into utf8
        const char* start8 = utf8;
        const char* end8 = utf8 + utf8Units;
        BidiLevel currentLevel = 0;

        Position pos8 = 0;
        Position pos16 = 0;
        Position end16 = sk_ubidi_getLength(bidi.get());
        while (pos16 < end16) {
            auto level = sk_ubidi_getLevelAt(bidi.get(), pos16);
            if (pos16 == 0) {
                currentLevel = level;
            } else if (level != currentLevel) {
                Position end = start8 - utf8;
                bidiRegions->emplace_back(pos8, end, currentLevel);
                currentLevel = level;
                pos8 = end;
            }
            SkUnichar u = utf8_next(&start8, end8);
            pos16 += SkUTF::ToUTF16(u);
        }
        Position end = start8 - utf8;
        if (end != pos8) {
            bidiRegions->emplace_back(pos8, end, currentLevel);
        }
        return true;
    }

    static bool extractWords(uint16_t utf16[], int utf16Units, std::vector<Position>* words) {

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
            auto status = type == SkUnicode::BreakType::kLines
                              ? UBRK_LINE_SOFT
                              : sk_ubrk_getRuleStatus(iter);
            setBreak(pos, status);
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
                if (isHardLineBreak(unichar)) {
                    setBreak(ch - utf8, UBRK_LINE_HARD);
                }
            }
        }
        return true;
    }

    static int utf8ToUtf16(const char* utf8, size_t utf8Units, std::unique_ptr<uint16_t[]>* utf16) {
        int utf16Units = SkUTF::UTF8ToUTF16(nullptr, 0, utf8, utf8Units);
        if (utf16Units < 0) {
            SkDEBUGF("Convert error: Invalid utf8 input");
            return utf16Units;
        }
        *utf16 = std::unique_ptr<uint16_t[]>(new uint16_t[utf16Units]);
        SkDEBUGCODE(int dstLen =) SkUTF::UTF8ToUTF16(utf16->get(), utf16Units, utf8, utf8Units);
        SkASSERT(dstLen == utf16Units);
        return utf16Units;
   }

    static int utf16ToUtf8(const uint16_t* utf16, size_t utf16Units, std::unique_ptr<char[]>* utf8) {
        int utf8Units = SkUTF::UTF16ToUTF8(nullptr, 0, utf16, utf16Units);
        if (utf8Units < 0) {
            SkDEBUGF("Convert error: Invalid utf16 input");
            return utf8Units;
        }
        *utf8 = std::unique_ptr<char[]>(new char[utf8Units]);
        SkDEBUGCODE(int dstLen =) SkUTF::UTF16ToUTF8(utf8->get(), utf8Units, utf16, utf16Units);
        SkASSERT(dstLen == utf8Units);
        return utf8Units;
   }

public:
    ~SkUnicode_icu() override { }
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[], int count,
                                                     SkBidiIterator::Direction dir) override {
        return SkBidiIterator_icu::makeBidiIterator(text, count, dir);
    }
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override {
        return SkBidiIterator_icu::makeBidiIterator(text, count, dir);
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
    std::unique_ptr<SkScriptIterator> makeScriptIterator() override {
        return SkScriptIterator_icu::makeScriptIterator();
    }

    // TODO: Use ICU data file to detect controls and whitespaces
    bool isControl(SkUnichar utf8) override {
        return sk_u_iscntrl(utf8);
    }

    bool isWhitespace(SkUnichar utf8) override {
        return sk_u_isWhitespace(utf8);
    }

    bool isSpace(SkUnichar utf8) override {
        return sk_u_isspace(utf8);
    }

    static bool isHardLineBreak(SkUnichar utf8) {
        auto property = sk_u_getIntPropertyValue(utf8, UCHAR_LINE_BREAK);
        return property == U_LB_LINE_FEED || property == U_LB_MANDATORY_BREAK;
    }

    SkString convertUtf16ToUtf8(const std::u16string& utf16) override {
        std::unique_ptr<char[]> utf8;
        auto utf8Units = SkUnicode_icu::utf16ToUtf8((uint16_t*)utf16.data(), utf16.size(), &utf8);
        if (utf8Units >= 0) {
            return SkString(utf8.get(), utf8Units);
        } else {
            return SkString();
        }
    }

    bool getBidiRegions(const char utf8[],
                        int utf8Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) override {
        return extractBidi(utf8, utf8Units, dir, results);
    }

    bool getLineBreaks(const char utf8[],
                       int utf8Units,
                       std::vector<LineBreakBefore>* results) override {

        return extractPositions(utf8, utf8Units, BreakType::kLines,
            [results](int pos, int status) {
                    results->emplace_back(pos, status == UBRK_LINE_HARD
                                                        ? LineBreakType::kHardLineBreak
                                                        : LineBreakType::kSoftLineBreak);
        });
    }

    bool getWords(const char utf8[], int utf8Units, std::vector<Position>* results) override {

        // Convert to UTF16 since we want the results in utf16
        std::unique_ptr<uint16_t[]> utf16;
        auto utf16Units = utf8ToUtf16(utf8, utf8Units, &utf16);
        if (utf16Units < 0) {
            return false;
        }

        return extractWords(utf16.get(), utf16Units, results);
    }

    bool getGraphemes(const char utf8[], int utf8Units, std::vector<Position>* results) override {

        return extractPositions(utf8, utf8Units, BreakType::kGraphemes,
            [results](int pos, int status) { results->emplace_back(pos);
        });
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {
        sk_ubidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
    }
};

std::unique_ptr<SkUnicode> SkUnicode::Make() {
    #if defined(SK_USING_THIRD_PARTY_ICU)
    if (!SkLoadICU()) {
        static SkOnce once;
        once([] { SkDEBUGF("SkLoadICU() failed!\n"); });
        return nullptr;
    }
    #endif

    return ICULib()
        ? std::make_unique<SkUnicode_icu>()
        : nullptr;
}
