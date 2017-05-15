/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrSKSLPrettyPrint_DEFINED
#define GrSKSLPrettyPrint_DEFINED

#include "SkString.h"

namespace GrSKSLPrettyPrint {
SkString PrettyPrint(const char** strings, int* lengths, int count, bool countlines);
};

#endif
