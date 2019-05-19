/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBlurDrawLooper_DEFINED
#define SkBlurDrawLooper_DEFINED

#include "include/core/SkDrawLooper.h"

/**
 *  Draws a shadow of the object (possibly offset), and then draws the original object in
 *  its original position.
 */
namespace SkBlurDrawLooper {
    sk_sp<SkDrawLooper> SK_API Make(SkColor4f color, SkColorSpace* cs,
            SkScalar sigma, SkScalar dx, SkScalar dy);
    sk_sp<SkDrawLooper> SK_API Make(SkColor color, SkScalar sigma, SkScalar dx, SkScalar dy);
};

#endif
