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


#endif
