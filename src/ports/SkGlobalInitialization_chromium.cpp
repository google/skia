/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcShader.h"
#include "SkBlurImageFilter.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilter.h"
#include "SkCornerPathEffect.h"
#include "SkDashPathEffect.h"
#include "SkGradientShader.h"
#include "SkLayerDrawLooper.h"
#include "SkMallocPixelRef.h"
#include "SkXfermode.h"

void SkFlattenable::InitializeFlattenables() {
    SkBitmapProcShader::Init();
    SkBlurImageFilter::Init();
    SkBlurMaskFilter::Init();
    SkColorFilter::Init();
    SkCornerPathEffect::Init();
    SkDashPathEffect::Init();
    SkGradientShader::Init();
    SkLayerDrawLooper::Init();
    SkXfermode::Init();
}

void SkPixelRef::InitializeFlattenables() {
    SkMallocPixelRef::Init();
}
