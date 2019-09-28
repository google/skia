/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PathOpsDebug_DEFINED
#define PathOpsDebug_DEFINED

#include <stdio.h>

class PathOpsDebug {
public:
    static bool gJson;
    static bool gMarkJsonFlaky;
    static bool gOutFirst;
    static bool gCheckForDuplicateNames;
    static bool gOutputSVG;
    static FILE* gOut;
};

#endif
