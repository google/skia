
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkParsePaint_DEFINED
#define SkParsePaint_DEFINED

#include "SkPaint.h"
#include "SkDOM.h"

/** "color"             color
    "opacity"           scalar  [0..1]
    "stroke-width"      scalar  (0...inf)
    "text-size"         scalar  (0..inf)
    "is-stroke"         bool
    "is-antialias"      bool
    "is-lineartext"     bool
*/
void SkPaint_Inflate(SkPaint*, const SkDOM&, const SkDOM::Node*);

#endif

