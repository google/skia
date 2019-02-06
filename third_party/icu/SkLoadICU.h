/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef load_icu_DEFINED
#define load_icu_DEFINED

#include "SkTypes.h"

#if defined(SK_BUILD_FOR_WIN) && defined(SK_USING_THIRD_PARTY_ICU)
bool SkLoadICU();
#else
inline bool SkLoadICU() { return true; }
#endif  // SK_BUILD_FOR_WIN
#endif  // load_icu_DEFINED

