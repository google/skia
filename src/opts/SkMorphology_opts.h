/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMorphology_opts_DEFINED
#define SkMorphology_opts_DEFINED

#include <SkMorphologyImageFilter.h>

enum SkMorphologyProcType {
    kDilateX_SkMorphologyProcType,
    kDilateY_SkMorphologyProcType,
    kErodeX_SkMorphologyProcType,
    kErodeY_SkMorphologyProcType
};

SkMorphologyImageFilter::Proc SkMorphologyGetPlatformProc(SkMorphologyProcType type);

#endif
