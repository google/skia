/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFDot6Constants_DEFINED
#define SkFDot6Constants_DEFINED

#include "SkTypes.h"

static const int kInverseTableSize = 1024; // SK_FDot6One * 16

extern const int32_t gFDot6INVERSE[kInverseTableSize * 2];

#endif
