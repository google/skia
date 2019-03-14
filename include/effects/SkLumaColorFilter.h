/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLumaColorFilter_DEFINED
#define SkLumaColorFilter_DEFINED

#include "SkColorFilter.h"
#include "SkRefCnt.h"

class SkRasterPipeline;

/**
 *  Luminance-to-alpha color filter, as defined in
 *  http://www.w3.org/TR/SVG/masking.html#Masking
 *  http://www.w3.org/TR/css-masking/#MaskValues
 *
 *  The resulting color is black with alpha equal to the
 *  luma (yes luma, not luminance, despite the name) value
 *  modulated by previous alpha:
 *
 *    C  = [ r, g, b, a ]
 *    C' = [ 0, 0, 0, a * Luma(r,g,b) ]
 *
 */

class SK_API SkLumaColorFilter {
public:
    static sk_sp<SkColorFilter> Make();
};

#endif
