/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skunicode/src/SkUnicode_icu.h"

#define SKICU_FUNC(funcname) funcname,

// ubrk_clone added as draft in ICU69 and Android API 31 (first ICU NDK).
// ubrk_safeClone deprecated in ICU69 and not exposed by Android.
template<typename...> using void_t = void;
template<typename T, typename = void>
struct SkUbrkClone {
    static UBreakIterator* clone(T bi, UErrorCode* status) {
        return ubrk_safeClone(bi, nullptr, nullptr, status);
    }
};
template<typename T>
struct SkUbrkClone<T, void_t<decltype(ubrk_clone(std::declval<T>(), nullptr))>> {
    static UBreakIterator* clone(T bi, UErrorCode* status) {
        return ubrk_clone(bi, status);
    }
};

std::unique_ptr<SkICULib> SkLoadICULib() {

    return std::make_unique<SkICULib>(SkICULib{
        SKICU_EMIT_FUNCS

        &SkUbrkClone<const UBreakIterator*>::clone,
        nullptr,
    });
}
