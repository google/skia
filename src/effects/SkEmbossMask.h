/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkEmbossMask_DEFINED
#define SkEmbossMask_DEFINED

#include "src/effects/SkEmbossMaskFilter.h"

struct SkMaskBuilder;

class SkEmbossMask {
public:
    static void Emboss(SkMaskBuilder* mask, const SkEmbossMaskFilter::Light&);
};

#endif
