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

    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBitmapProcShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBlurImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkCornerPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDashPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLayerDrawLooper)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMallocPixelRef)

    SkBlurMaskFilter::InitializeFlattenables();
    SkColorFilter::InitializeFlattenables();
    SkGradientShader::InitializeFlattenables();
    SkXfermode::InitializeFlattenables();
}
