/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMorphology_opts.h"

SkMorphologyProc SkMorphologyGetPlatformProc(SkMorphologyProcType) {
    return NULL;
}
