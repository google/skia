// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/editor/utf8_tools.h"

#if SK_EDITOR_USE_ICU

#include "src/utils/SkUTF.h"
#include "third_party/externals/icu/source/common/unicode/uchar.h"
#include <cctype>

#ifdef SK_USING_THIRD_PARTY_ICU
#include "SkLoadICU.h"
#endif

bool is_utf8_whitespace(const char* utf8, const char* end) {
    SkUnichar character = SkUTF::NextUTF8(&utf8, end);
    #ifdef SK_USING_THIRD_PARTY_ICU
    if (!SkLoadICU()) {
        return ::isspace(character);
    }
    #endif
    return character >= 0 ? (bool)u_isUWhiteSpace((UChar32)character) : false;
}

#else  // fallback if you are missing ICU.


bool is_utf8_whitespace(const char* utf8, const char* end) {
    return utf8 < end ? ::isspace(*(const unsigned char*)utf8) : false;
}

#endif

