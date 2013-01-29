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
