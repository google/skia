/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkString.h"
#include "SkStringUtils.h"

void SkAddFlagToString(SkString* string, bool flag, const char* flagStr, bool* needSeparator) {
    if (flag) {
        if (*needSeparator) {
            string->append("|");
        }
        string->append(flagStr);
        *needSeparator = true;
    }
}

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
