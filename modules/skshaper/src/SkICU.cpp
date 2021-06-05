/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skshaper/include/SkICU.h"

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

bool SkICU::ComputeProperties(SkSpan<const int32_t> unichars,
                              uint32_t requestedProperties,
                              uint32_t outProperties[]) {
    for (int i = 0; i < unichars.size(); ++i) {
        const int32_t uni = unichars.data()[i];
        uint32_t outProp = 0;

        if (requestedProperties & kWhiteSpaceBreak) {
            outProp |= bool2bit(u_isWhitespace(uni), kWhiteSpaceBreak);
        }
        if (requestedProperties & kIsControl) {
            outProp |= bool2bit(u_iscntrl(uni), kIsControl);
        }
        if (requestedProperties & kIsSpace) {
            outProp |= bool2bit(u_isspace(uni), kIsSpace);
        }

        outProperties[i] = outProp;
    }
    return true;
}

