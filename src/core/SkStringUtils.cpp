/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "src/core/SkStringUtils.h"
#include "src/utils/SkUTF.h"

void SkAppendScalar(SkString* str, SkScalar value, SkScalarAsStringType asType) {
    switch (asType) {
        case kHex_SkScalarAsStringType:
            str->appendf("SkBits2Float(0x%08x)", SkFloat2Bits(value));
            break;
        case kDec_SkScalarAsStringType: {
            SkString tmp;
            tmp.printf("%g", value);
            if (tmp.contains('.')) {
                tmp.appendUnichar('f');
            }
            str->append(tmp);
            break;
        }
    }
}

SkString SkTabString(const SkString& string, int tabCnt) {
    if (tabCnt <= 0) {
        return string;
    }
    SkString tabs;
    for (int i = 0; i < tabCnt; ++i) {
        tabs.append("\t");
    }
    SkString result;
    static const char newline[] = "\n";
    const char* input = string.c_str();
    int nextNL = SkStrFind(input, newline);
    while (nextNL >= 0) {
        if (nextNL > 0) {
            result.append(tabs);
        }
        result.append(input, nextNL + 1);
        input += nextNL + 1;
        nextNL = SkStrFind(input, newline);
    }
    if (*input != '\0') {
        result.append(tabs);
        result.append(input);
    }
    return result;
}

SkString SkStringFromUTF16(const uint16_t* src, size_t count) {
    SkString ret;
    const uint16_t* stop = src + count;
    if (count > 0) {
        SkASSERT(src);
        size_t n = 0;
        const uint16_t* end = src + count;
        for (const uint16_t* ptr = src; ptr < end;) {
            const uint16_t* last = ptr;
            SkUnichar u = SkUTF::NextUTF16(&ptr, stop);
            size_t s = SkUTF::ToUTF8(u);
            if (n > UINT32_MAX - s) {
                end = last;  // truncate input string
                break;
            }
            n += s;
        }
        ret = SkString(n);
        char* out = ret.writable_str();
        for (const uint16_t* ptr = src; ptr < end;) {
            out += SkUTF::ToUTF8(SkUTF::NextUTF16(&ptr, stop), out);
        }
        SkASSERT(out == ret.writable_str() + n);
    }
    return ret;
}
