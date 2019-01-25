/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkLoadICU_DEFINED
#define SkLoadICU_DEFINED

#ifdef _WIN32
void SkLoadICU();
#else
static inline void SkLoadICU() {}
#endif  // _WIN32

#endif  // SkLoadICU_DEFINED
