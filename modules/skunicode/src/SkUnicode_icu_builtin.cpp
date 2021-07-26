/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skunicode/src/SkUnicode_icu.h"

#define SKICU_FUNC(funcname) funcname,

std::unique_ptr<SkICULib> SkLoadICULib() {

    return std::make_unique<SkICULib>(SkICULib{
        SKICU_EMIT_FUNCS

        // ubrk_clone added as draft in ICU69 and Android API 31 (first ICU NDK).
        // ubrk_safeClone deprecated in ICU69 and not exposed by Android.
#if U_ICU_VERSION_MAJOR_NUM >= 69
        ubrk_clone,
        nullptr,
#else
        nullptr,
        ubrk_safeClone,
#endif
    });
}
