/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTextFormatParams_DEFINES
#define SkTextFormatParams_DEFINES

#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"

// The fraction of text size to embolden fake bold text scales with text size.
// At 9 points or below, the stroke width is increased by text size / 24.
// At 36 points and above, it is increased by text size / 32.  In between,
// it is interpolated between those values.
static const SkScalar kStdFakeBoldInterpKeys[] = {
    SK_Scalar1*9,
    SK_Scalar1*36,
};
static const SkScalar kStdFakeBoldInterpValues[] = {
    SK_Scalar1/24,
    SK_Scalar1/32,
};
static_assert(SK_ARRAY_COUNT(kStdFakeBoldInterpKeys) == SK_ARRAY_COUNT(kStdFakeBoldInterpValues),
              "mismatched_array_size");
static const int kStdFakeBoldInterpLength = SK_ARRAY_COUNT(kStdFakeBoldInterpKeys);

#endif  //SkTextFormatParams_DEFINES
