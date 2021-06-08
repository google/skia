/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkMalloc.h"
#include "modules/skshaper/include/SkUnicodeProperties.h"
#include "src/utils/SkUTF.h"

#include <unicode/ubidi.h>
#include <unicode/ubrk.h>
#include <unicode/uscript.h>
#include <unicode/ustring.h>
#include <unicode/utext.h>
#include <unicode/utypes.h>

#if defined(SK_USING_THIRD_PARTY_ICU)
#include "SkLoadICU.h"
#endif

static inline uint32_t bool2bit(bool pred, uint32_t bit) {
    return pred ? bit : 0;
}

static void handle_br_grapheme(unsigned request, int32_t status, uint32_t* outProp) {
    *outProp |= SkUnicodeProperties::kGraphemeBreak;
}

static void handle_br_word(unsigned request, int32_t status, uint32_t* outProp) {
    // TODO: how to I distinguish Word from IntraWord ?
    *outProp |= SkUnicodeProperties::kWordBreak;
}

static void handle_br_line(unsigned request, int32_t status, uint32_t* outProp) {
    *outProp |= (status >= UBRK_LINE_HARD) ? SkUnicodeProperties::kHardLineBreak
                                           : SkUnicodeProperties::kSoftLineBreak;
}

bool SkUnicodeProperties::ComputeProperties(SkSpan<const uint16_t> text16,
                                            const char bcp47[],
                                            uint32_t requests,
                                            uint32_t properties[]) {
    const char* locale = bcp47 ? bcp47 : uloc_getDefault();

    sk_bzero(properties, text16.size() * sizeof(uint32_t));

    if (requests & (kIsControl_Request | kIsSpace_Request | kIsWhiteSpace_Request)) {
        const uint16_t* uPtr = text16.data();
        const uint16_t* ePtr = uPtr + text16.size();

        while (uPtr < ePtr) {
            const size_t index = uPtr - text16.data();
            const int32_t uni = SkUTF::NextUTF16(&uPtr, ePtr);
            uint32_t outProp = 0;

            if (requests & kIsControl_Request) {
                outProp |= bool2bit(u_iscntrl(uni), kIsControl);
            }
            if (requests & kIsSpace_Request) {
                outProp |= bool2bit(u_isspace(uni), kIsSpace);
            }
            if (requests & kIsWhiteSpace_Request) {
                outProp |= bool2bit(u_isWhitespace(uni), kIsWhiteSpace);
            }

            properties[index] = outProp;
        }
    }

    const struct {
        uint32_t           request;   // our request bit(s)
        UBreakIteratorType icbr;
        void (*handler)(unsigned request, int32_t status, uint32_t* outProp);
    } breakRecs [] = {
        { kGraphemeBreak_Request, UBRK_CHARACTER, handle_br_grapheme },
        { kWordBreak_Request,     UBRK_WORD     , handle_br_word     },
        { kLineBreak_Request,     UBRK_LINE     , handle_br_line     },
    };
    for (auto rec : breakRecs) {
        if (requests & rec.request) {
            UErrorCode status = U_ZERO_ERROR;
            auto iter = ubrk_open(rec.icbr, locale,
                                  (const char16_t*)text16.data(), text16.size(),
                                  &status);
            if (U_FAILURE(status)) {
                return false;
            }

            ubrk_first(iter);
            int32_t pos = ubrk_first(iter);
            while (pos != UBRK_DONE) {
                // sometimes pos == text16.size(), but we don't report those
                if ((size_t)pos < text16.size()) {
                    rec.handler(rec.request, ubrk_getRuleStatus(iter), &properties[pos]);
                }
                pos = ubrk_next(iter);
            }
            ubrk_close(iter);
        }
    }

    return true;
}

