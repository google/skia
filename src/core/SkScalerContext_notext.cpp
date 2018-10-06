/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkScalerContext.h"

size_t SkScalerContext::GetGammaLUTSize(SkScalar contrast, SkScalar paintGamma,
                                        SkScalar deviceGamma, int* width, int* height) {
    *width = *height = 0;
    return 0;
}
bool SkScalerContext::GetGammaLUTData(SkScalar contrast, SkScalar paintGamma, SkScalar deviceGamma,
                                      uint8_t* data) {
    return false;
}
