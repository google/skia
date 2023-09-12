/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skunicode/src/SkUnicode_icu.h"

#include <unicode/ubrk.h>
#include <unicode/uloc.h>
#include <unicode/utypes.h>

#include <memory>
#include <type_traits>
#include <utility>

namespace {

// ubrk_clone added as draft in ICU69 and Android API 31 (first ICU NDK).
// ubrk_safeClone deprecated in ICU69 and not exposed by Android.
template<typename T, typename = void>
struct SkUbrkClone {
    static UBreakIterator* clone(T bi, UErrorCode* status) {
        return ubrk_safeClone(bi, nullptr, nullptr, status);
    }
};
template<typename T>
struct SkUbrkClone<T, std::void_t<decltype(ubrk_clone(std::declval<T>(), nullptr))>> {
    static UBreakIterator* clone(T bi, UErrorCode* status) {
        return ubrk_clone(bi, status);
    }
};

// ubrk_getLocaleByType has been in ICU since version 2.8
// However, it was not included in the Android NDK
template<typename T, typename = void>
struct SkUbrkGetLocaleByType {
    static const char* getLocaleByType(T bi, ULocDataLocaleType type, UErrorCode* status) {
        *status = U_UNSUPPORTED_ERROR;
        return nullptr;
    }
};
template<typename T>
struct SkUbrkGetLocaleByType<
    T,
    std::void_t<decltype(ubrk_getLocaleByType(std::declval<T>(),
                                              std::declval<ULocDataLocaleType>(),
                                              nullptr))>>
{
    static const char* getLocaleByType(T bi, ULocDataLocaleType type, UErrorCode* status) {
        return ubrk_getLocaleByType(bi, type, status);
    }
};

}  // namespace

#define SKICU_FUNC(funcname) funcname,
std::unique_ptr<SkICULib> SkLoadICULib() {

    return std::make_unique<SkICULib>(SkICULib{
        SKICU_EMIT_FUNCS

        &SkUbrkClone<const UBreakIterator*>::clone,
        nullptr,
        &SkUbrkGetLocaleByType<const UBreakIterator*>::getLocaleByType,
    });
}
