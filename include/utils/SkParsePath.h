
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkParsePath_DEFINED
#define SkParsePath_DEFINED

#include "include/core/SkPath.h"

class SkString;

class SkParsePath {
public:
    static bool FromSVGString(const char str[], SkPath*);
    static void ToSVGString(const SkPath&, SkString*);
};

#endif
