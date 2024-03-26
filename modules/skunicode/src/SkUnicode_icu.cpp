/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "modules/skunicode/include/SkUnicode_icu.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/src/SkBidiFactory_icu_full.h"
#include "modules/skunicode/src/SkUnicode_icu_bidi.h"
#include "modules/skunicode/src/SkUnicode_icupriv.h"
#include "src/base/SkBitmaskEnum.h"
#include "src/base/SkUTF.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkTHash.h"

#include <unicode/ubrk.h>
#include <unicode/uchar.h>
#include <unicode/uloc.h>
#include <unicode/umachine.h>
#include <unicode/utext.h>
#include <unicode/utypes.h>

#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#if defined(SK_USING_THIRD_PARTY_ICU) && defined(SK_BUILD_FOR_WIN)
#include "SkLoadICU.h"
#include "include/private/base/SkOnce.h"
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
        case SkUnicode::BreakType::kSentences:
            return UBRK_SENTENCE;
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

class SkIcuBreakIteratorCache final {
    struct Request final {
        Request(SkUnicode::BreakType type, const char* icuLocale)
            : fType(type)
            , fIcuLocale(icuLocale)
            , hash(SkGoodHash()(type) ^ SkGoodHash()(fIcuLocale))
        {}
        const SkUnicode::BreakType fType;
        const SkString fIcuLocale;
        const uint32_t hash;
        struct Hash {
            uint32_t operator()(const Request& key) const {
                return key.hash;
            }
        };
        bool operator==(const Request& that) const {
            return this->fType == that.fType && this->fIcuLocale == that.fIcuLocale;
        }
    };
    /* Every holder of this class is referencing the same (logical) break iterator.
     * Due to caching, the actual break iterator may come and go.
     */
    class BreakIteratorRef final {
    public:
        BreakIteratorRef(ICUBreakIterator iter) : breakIterator(iter.release()), fRefCnt(1) {
            ++Instances;
        }
        BreakIteratorRef(SkRefCntBase&&) = delete;
        BreakIteratorRef(const SkRefCntBase&) = delete;
        BreakIteratorRef& operator=(SkRefCntBase&&) = delete;
        BreakIteratorRef& operator=(const SkRefCntBase&) = delete;
        ~BreakIteratorRef() {
            if (breakIterator) {
                ubrk_close_wrapper(breakIterator);
            }
        }

        void ref() const {
            SkASSERT(fRefCnt > 0);
            ++fRefCnt;
        }
        void unref() const {
            SkASSERT(fRefCnt > 0);
            if (1 == fRefCnt--) {
                delete this;
                --Instances;
            }
        }

        UBreakIterator* breakIterator;
        static int32_t GetInstanceCount() { return Instances; }
    private:
        mutable int32_t fRefCnt;
        static int32_t Instances;
    };
    THashMap<Request, sk_sp<BreakIteratorRef>, Request::Hash> fRequestCache;
    SkMutex fCacheMutex;

    void purgeIfNeeded() {
        // If there are too many requests remove some (oldest first?)
        // This may free some break iterators
        if (fRequestCache.count() > 100) {
            // remove the oldest requests
            fRequestCache.reset();
        }
        // If there are still too many break iterators remove some (oldest first?)
        if (BreakIteratorRef::GetInstanceCount() > 4) {
            // delete the oldest break iterators and set the references to nullptr
            for (auto&& [key, value] : fRequestCache) {
                if (value->breakIterator) {
                    sk_ubrk_close(value->breakIterator);
                    value->breakIterator = nullptr;
                }
            }
        }
    }

 public:
    static SkIcuBreakIteratorCache& get() {
        static SkIcuBreakIteratorCache instance;
        return instance;
    }

    ICUBreakIterator makeBreakIterator(SkUnicode::BreakType type, const char* bcp47) {
        SkAutoMutexExclusive lock(fCacheMutex);
        UErrorCode status = U_ZERO_ERROR;

        // Get ICU locale for BCP47 langtag
        char localeIDStorage[ULOC_FULLNAME_CAPACITY];
        const char* localeID = nullptr;
        if (bcp47) {
            sk_uloc_forLanguageTag(bcp47, localeIDStorage, ULOC_FULLNAME_CAPACITY, nullptr, &status);
            if (U_FAILURE(status)) {
                SkDEBUGF("Break error could not get language tag: %s", sk_u_errorName(status));
            } else if (localeIDStorage[0]) {
                localeID = localeIDStorage;
            }
        }
        if (!localeID) {
            localeID = sk_uloc_getDefault();
        }

        auto make = [](const Request& request) -> UBreakIterator* {
            UErrorCode status = U_ZERO_ERROR;
            UBreakIterator* bi = sk_ubrk_open(convertType(request.fType),
                                              request.fIcuLocale.c_str(),
                                              nullptr, 0, &status);
            if (U_FAILURE(status)) {
                SkDEBUGF("Break error: %s", sk_u_errorName(status));
            }
            return bi;
        };

        auto clone = [](const UBreakIterator* existing) -> ICUBreakIterator {
            if (!existing) {
                return nullptr;
            }

            UErrorCode status = U_ZERO_ERROR;
            ICUBreakIterator clone(sk_ubrk_clone(existing, &status));
            if (U_FAILURE(status)) {
                SkDEBUGF("Break error: %s", sk_u_errorName(status));
            }
            return clone;
        };

        Request request(type, localeID);

        // See if this request is already in the cache
        const sk_sp<BreakIteratorRef>* ref = fRequestCache.find(request);
        if (ref) {
            // See if the breakIterator needs to be re-created
            if (!(*ref)->breakIterator) {
                (*ref)->breakIterator = make(request);
            }
            return clone((*ref)->breakIterator);
        }

        // This request was not in the cache, create an iterator.
        ICUBreakIterator newIter(make(request));
        if (!newIter) {
            return nullptr;
        }

        sk_sp<BreakIteratorRef> newRef;

        // Check if the new iterator is a duplicate
        // Android doesn't expose ubrk_getLocaleByType so there is no means of de-duplicating.
        // ubrk_getAvailable seems like it should work, but the implementation is just every locale.
        if (SkGetICULib()->f_ubrk_getLocaleByType) {
            const char* actualLocale = SkGetICULib()->f_ubrk_getLocaleByType(
                                           newIter.get(), ULOC_ACTUAL_LOCALE, &status);
            // Android doesn't expose ubrk_getLocaleByType so a wrapper may return an error.
            if (!U_FAILURE(status)) {
                if (!actualLocale) {
                    actualLocale = "";
                }
                // If the actual locale is the same as the requested locale we know there is no entry.
                if (strcmp(actualLocale, localeID) != 0) {
                    Request actualRequest(type, actualLocale);
                    const sk_sp<BreakIteratorRef>* actualRef = fRequestCache.find(actualRequest);
                    if (actualRef) {
                        if (!(*actualRef)->breakIterator) {
                            (*actualRef)->breakIterator = newIter.release();
                        }
                        actualRef = fRequestCache.set(request, *actualRef);
                        return clone((*actualRef)->breakIterator);
                    } else {
                        this->purgeIfNeeded();
                        newRef = sk_make_sp<BreakIteratorRef>(std::move(newIter));
                        fRequestCache.set(actualRequest, newRef);
                    }
                }
            }
        }

        if (!newRef) {
            this->purgeIfNeeded();
            newRef = sk_make_sp<BreakIteratorRef>(std::move(newIter));
        }
        fRequestCache.set(request, newRef);

        return clone(newRef->breakIterator);
    }
};
/*static*/ int32_t SkIcuBreakIteratorCache::BreakIteratorRef::Instances{0};

class SkUnicode_icu : public SkUnicode {

    static bool extractWords(uint16_t utf16[], int utf16Units, const char* locale,
                             std::vector<Position>* words) {

        UErrorCode status = U_ZERO_ERROR;

        const BreakType type = BreakType::kWords;
        ICUBreakIterator iterator = SkIcuBreakIteratorCache::get().makeBreakIterator(type, locale);
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

    static bool extractPositions(const char utf8[], int utf8Units,
                                 BreakType type, const char* locale,
                                 const std::function<void(int, int)>& setBreak) {

        UErrorCode status = U_ZERO_ERROR;
        ICUUText text(sk_utext_openUTF8(nullptr, &utf8[0], utf8Units, &status));
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", sk_u_errorName(status));
            return false;
        }
        SkASSERT(text);

        ICUBreakIterator iterator = SkIcuBreakIteratorCache::get().makeBreakIterator(type, locale);
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
        return sk_u_hasBinaryProperty(unichar, UCHAR_EMOJI);
    }

    bool isEmojiComponent(SkUnichar unichar) override {
        return sk_u_hasBinaryProperty(unichar, UCHAR_EMOJI_COMPONENT);
    }

    bool isEmojiModifierBase(SkUnichar unichar) override {
        return sk_u_hasBinaryProperty(unichar, UCHAR_EMOJI_MODIFIER_BASE);
    }

    bool isEmojiModifier(SkUnichar unichar) override {
        return sk_u_hasBinaryProperty(unichar, UCHAR_EMOJI_MODIFIER);
    }

    bool isRegionalIndicator(SkUnichar unichar) override {
        return sk_u_hasBinaryProperty(unichar, UCHAR_REGIONAL_INDICATOR);
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
        return fBidiFact->MakeIterator(text, count, dir);
    }
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override {
        return fBidiFact->MakeIterator(text, count, dir);
    }
    std::unique_ptr<SkBreakIterator> makeBreakIterator(const char locale[],
                                                       BreakType type) override {
        ICUBreakIterator iterator = SkIcuBreakIteratorCache::get().makeBreakIterator(type, locale);
        if (!iterator) {
            return nullptr;
        }
        return std::unique_ptr<SkBreakIterator>(new SkBreakIterator_icu(std::move(iterator)));
    }
    std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType type) override {
        return makeBreakIterator(sk_uloc_getDefault(), type);
    }

    SkString toUpper(const SkString& str) override {
        return this->toUpper(str, nullptr);
    }

    SkString toUpper(const SkString& str, const char* locale) override {
        // Convert to UTF16 since that's what ICU wants.
        auto str16 = SkUnicode::convertUtf8ToUtf16(str.c_str(), str.size());

        UErrorCode icu_err = U_ZERO_ERROR;
        const auto upper16len = sk_u_strToUpper(nullptr, 0, (UChar*)(str16.c_str()), str16.size(),
                                                locale, &icu_err);
        if (icu_err != U_BUFFER_OVERFLOW_ERROR || upper16len <= 0) {
            return SkString();
        }

        AutoSTArray<128, uint16_t> upper16(upper16len);
        icu_err = U_ZERO_ERROR;
        sk_u_strToUpper((UChar*)(upper16.get()), SkToS32(upper16.size()),
                        (UChar*)(str16.c_str()), str16.size(),
                        locale, &icu_err);
        SkASSERT(!U_FAILURE(icu_err));

        // ... and back to utf8 'cause that's what we want.
        return convertUtf16ToUtf8((char16_t*)upper16.get(), upper16.size());
    }

    bool getBidiRegions(const char utf8[],
                        int utf8Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) override {
        return fBidiFact->ExtractBidi(utf8, utf8Units, dir, results);
    }

    bool getWords(const char utf8[], int utf8Units, const char* locale,
                  std::vector<Position>* results) override {

        // Convert to UTF16 since we want the results in utf16
        auto utf16 = convertUtf8ToUtf16(utf8, utf8Units);
        return SkUnicode_icu::extractWords((uint16_t*)utf16.c_str(), utf16.size(), locale, results);
    }

    bool getUtf8Words(const char utf8[],
                      int utf8Units,
                      const char* locale,
                      std::vector<Position>* results) override {
        // Convert to UTF16 since we want the results in utf16
        auto utf16 = convertUtf8ToUtf16(utf8, utf8Units);
        std::vector<Position> utf16Results;
        if (!SkUnicode_icu::extractWords(
                    (uint16_t*)utf16.c_str(), utf16.size(), locale, &utf16Results)) {
            return false;
        }

        std::vector<Position> mapping;
        SkSpan<const char> text(utf8, utf8Units);
        SkUnicode::extractUtfConversionMapping(
                text, [&](size_t index) { mapping.emplace_back(index); }, [&](size_t index) {});

        for (auto i16 : utf16Results) {
            results->emplace_back(mapping[i16]);
        }
        return true;
    }

    bool getSentences(const char utf8[],
                      int utf8Units,
                      const char* locale,
                      std::vector<SkUnicode::Position>* results) override {
        SkUnicode_icu::extractPositions(
                utf8, utf8Units, BreakType::kSentences, nullptr,
                [&](int pos, int status) {
                    results->emplace_back(pos);
                });
        return true;
    }

    bool computeCodeUnitFlags(char utf8[], int utf8Units, bool replaceTabs,
                              TArray<SkUnicode::CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf8Units + 1, CodeUnitFlags::kNoCodeUnitFlag);

        SkUnicode_icu::extractPositions(utf8, utf8Units, BreakType::kLines, nullptr, // TODO: locale
                                        [&](int pos, int status) {
            (*results)[pos] |= status == UBRK_LINE_HARD
                                       ? CodeUnitFlags::kHardLineBreakBefore
                                       : CodeUnitFlags::kSoftLineBreakBefore;
        });

        SkUnicode_icu::extractPositions(utf8, utf8Units, BreakType::kGraphemes, nullptr, //TODO
                                        [&](int pos, int status) {
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
        fBidiFact->bidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
    }

private:
    sk_sp<SkBidiFactory> fBidiFact = sk_make_sp<SkBidiICUFactory>();
};

namespace SkUnicodes::ICU {
sk_sp<SkUnicode> Make() {
    // We haven't yet created a way to encode the ICU data for assembly on Windows,
    // so we use a helper library to load icudtl.dat from the harddrive.
#if defined(SK_USING_THIRD_PARTY_ICU) && defined(SK_BUILD_FOR_WIN)
    if (!SkLoadICU()) {
        static SkOnce once;
        once([] { SkDEBUGF("SkLoadICU() failed!\n"); });
        return nullptr;
    }
#endif
    if (SkGetICULib()) {
        return sk_make_sp<SkUnicode_icu>();
    }
    return nullptr;
}
}  // namespace SkUnicodes::ICU
