// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/core/SkTypes.h"
#include "include/private/SkTemplates.h"
#include "modules/skplaintexteditor/src/word_boundaries.h"

#include <unicode/ubrk.h>
#include <unicode/utext.h>
#include <unicode/utypes.h>
#include <memory>


namespace {
template <typename T,typename P,P* p> using resource = std::unique_ptr<T, SkFunctionWrapper<P, p>>;
using ICUBrk   = resource<UBreakIterator, decltype(ubrk_close)       , ubrk_close       >;
using ICUUText = resource<UText         , decltype(utext_close)      , utext_close      >;
}  // namespace

std::vector<bool> GetUtf8WordBoundaries(const char* begin, size_t byteCount, const char* locale) {
    std::vector<bool> result;
    if (0 == byteCount) {
        return result;
    }
    result.resize(byteCount);

    UErrorCode status = U_ZERO_ERROR;
    UText sUtf8UText = UTEXT_INITIALIZER;
    ICUUText utf8UText(utext_openUTF8(&sUtf8UText, begin, byteCount, &status));
    if (U_FAILURE(status)) {
        SkDebugf("Could not create utf8UText: %s", u_errorName(status));
        return result;
    }

    ICUBrk wordBreakIterator(ubrk_open(UBRK_WORD, locale, nullptr, 0, &status));
    if (!wordBreakIterator || U_FAILURE(status)) {
        SkDEBUGF("Could not create line break iterator: %s", u_errorName(status));
        return result;
    }

    ubrk_setUText(&*wordBreakIterator, utf8UText.get(), &status);
    if (U_FAILURE(status)) {
        SkDebugf("Could not setText on break iterator: %s", u_errorName(status));
        return result;
    }

    int32_t pos = ubrk_first(&*wordBreakIterator);
    while (pos != UBRK_DONE) {
        if ((size_t)pos < byteCount) {
            result[pos] = true;
        }
        pos = ubrk_next(&*wordBreakIterator);
    }
    return result;
}
