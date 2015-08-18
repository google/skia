/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStringUtils_DEFINED
#define SkStringUtils_DEFINED

class SkString;

/**
 * Add 'flagStr' to 'string' and set 'needSeparator' to true only if 'flag' is
 * true. If 'needSeparator' is true append a '|' before 'flagStr'. This method
 * is used to streamline the creation of ASCII flag strings within the toString
 * methods.
 */
void SkAddFlagToString(SkString* string, bool flag,
                       const char* flagStr, bool* needSeparator);


enum SkScalarAsStringType {
    kDec_SkScalarAsStringType,
    kHex_SkScalarAsStringType,
};

void SkAppendScalar(SkString*, SkScalar, SkScalarAsStringType);

static inline void SkAppendScalarDec(SkString* str, SkScalar value) {
    SkAppendScalar(str, value, kDec_SkScalarAsStringType);
}

static inline void SkAppendScalarHex(SkString* str, SkScalar value) {
    SkAppendScalar(str, value, kHex_SkScalarAsStringType);
}

/** Indents every non-empty line of the string by tabCnt tabs */
SkString SkTabString(const SkString& string, int tabCnt);

#endif
