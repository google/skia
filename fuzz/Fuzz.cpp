/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "Fuzz.h"
#include "FuzzCommon.h"

// UBSAN reminds us that bool can only legally hold 0 or 1.
void Fuzz::next(bool* b) {
  uint8_t n;
  this->next(&n);
  *b = (n & 1) == 1;
}

void Fuzz::next(SkImageFilter::CropRect* cropRect) {
    SkRect rect;
    uint8_t flags;
    this->next(&rect);
    this->nextRange(&flags, 0, 0xF);
    *cropRect = SkImageFilter::CropRect(rect, flags);
}

void Fuzz::next(SkRegion* region) {
    // See FuzzCommon.h
    FuzzNiceRegion(this, region, 10);
}

void Fuzz::nextRange(float* f, float min, float max) {
    this->next(f);
    if (!std::isnormal(*f) && *f != 0.0f) {
        // Don't deal with infinity or other strange floats.
        *f = max;
    }
    *f = min + std::fmod(std::abs(*f), (max - min + 1));
}
