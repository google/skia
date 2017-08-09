/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorLookUpTable.h"
#include "SkColorSpaceXformPriv.h"
#include "SkFloatingPoint.h"

SkColorLookUpTable::SkColorLookUpTable(uint8_t inputChannels, const uint8_t limits[]) {
    fInputChannels = inputChannels;
    SkASSERT(inputChannels >= 1 && inputChannels <= kMaxColorChannels);
    memcpy(fLimits, limits, fInputChannels * sizeof(uint8_t));

    for (int i = 0; i < inputChannels; i++) {
        SkASSERT(fLimits[i] > 1);
    }
}
