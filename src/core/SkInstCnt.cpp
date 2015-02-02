/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkInstCnt.h"

#if SK_ENABLE_INST_COUNT && !defined(SKIA_DLL)  // See SkInstCnt.h
    bool gPrintInstCount = false;
    void SkInstCountPrintLeaksOnExit() { gPrintInstCount = true; }
#else
    void SkInstCountPrintLeaksOnExit() {}
#endif
