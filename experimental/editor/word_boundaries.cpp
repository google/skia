// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/editor/word_boundaries.h"

#include <unicode/brkiter.h>
#include <unicode/unistr.h>

#include <memory>

std::vector<bool> GetUtf8WordBoundaries(const char* begin, const char* end, const char* locale) {
    static constexpr UBreakIteratorType kIteratorType = UBRK_WORD;
    struct UTextCloser {
        void operator()(UText* p) { (void)utext_close(p); }
    };
    struct UBreakCloser {
        void operator()(UBreakIterator* p) { (void)ubrk_close(p); }
    };

    std::vector<bool> result;
    if (end <= begin) {
        return result;
    }
    size_t byteCount = end - begin;
    result.resize(byteCount);

    UText utf8UText = UTEXT_INITIALIZER;
    UErrorCode errorCode = U_ZERO_ERROR;
    (void)utext_openUTF8(&utf8UText, begin, byteCount, &errorCode);
    std::unique_ptr<UText, UTextCloser> autoclose1(&utf8UText);
    if (U_FAILURE(errorCode)) {
        return result;
    }
    UBreakIterator* iter = ubrk_open(kIteratorType, locale, nullptr, 0, &errorCode);
    std::unique_ptr<UBreakIterator, UBreakCloser> autoclose2(iter);
    if (U_FAILURE(errorCode)) {
        return result;
    }
    ubrk_setUText(iter, &utf8UText, &errorCode);
    if (U_FAILURE(errorCode)) {
        return result;
    }
    int pos = ubrk_first(iter);
    while (pos != icu::BreakIterator::DONE) {
        if ((unsigned)pos < (unsigned)byteCount) {
            result[pos] = true;
        }
        pos = ubrk_next(iter);
    }
    return result;
}
