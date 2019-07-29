// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/editor/utf8_tools.h"

#include "src/utils/SkUTF.h"

#include <cctype>

#ifdef SK_EDITOR_USE_ICU
#include "third_party/icu/SkLoadICU.h"
#include "unicode/uchar.h"
#endif

bool is_utf8_whitespace(const char* utf8, const char* end) {
    SkUnichar c = SkUTF::NextUTF8(&utf8, end);
#if SK_EDITOR_USE_ICU
    if (SkLoadICU()) {
        return c >= 0 ? (bool)u_isUWhiteSpace((UChar32)c) : false;
    }
#endif
    return (unsigned)c <= 0xFF ? ::isspace((int)c) : false;
}
