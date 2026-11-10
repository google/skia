/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/codec/SkColorPalette.h"

#include "include/private/SkMalloc.h"

SkColorPalette::SkColorPalette(const SkPMColor colors[], int count)
        : fColors(count)
        , fCount(count) {
    SkASSERT(0 == count || colors);
    SkASSERT(count >= 0 && count <= 256);

    sk_careful_memcpy(fColors.get(), colors, count * sizeof(SkPMColor));
}

SkColorPalette::~SkColorPalette() = default;
