/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef load_icu_DEFINED
#define load_icu_DEFINED

#if defined(_WIN32) && defined(SK_USING_THIRD_PARTY_ICU)
bool SkLoadICU();
#else
static inline bool SkLoadICU() { return true; }
#endif  // defined(_WIN32) && defined(SK_USING_THIRD_PARTY_ICU)

#endif  // load_icu_DEFINED
