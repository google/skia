/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

#include "Sk1DPathEffect.h"
#include "Sk2DPathEffect.h"
#include "SkAvoidXfermode.h"
#include "SkBlurDrawLooper.h"
#include "SkBlurImageFilter.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkCornerPathEffect.h"
#include "SkDashPathEffect.h"
#include "SkDiscretePathEffect.h"
#include "SkEffects.h"
#include "SkFlattenable.h"
#include "SkGradientShader.h"
#include "SkGroupShape.h"
#include "SkLayerDrawLooper.h"
#include "SkLayerRasterizer.h"
#include "SkPathEffect.h"
#include "SkPixelXorXfermode.h"
#include "SkRectShape.h"

void SkEffects::Init() {
    SkAvoidXfermode::Init();
    SkBlurDrawLooper::Init();
    SkBlurImageFilter::Init();
    SkBlurMaskFilter::Init();
    SkColorFilter::Init();
    SkColorMatrixFilter::Init();
    SkCornerPathEffect::Init();
    SkDashPathEffect::Init();
    SkDiscretePathEffect::Init();
    SkGradientShader::Init();
    SkGroupShape::Init();
    SkLayerDrawLooper::Init();
    SkLayerRasterizer::Init();
    SkPath1DPathEffect::Init();
    SkPath2DPathEffect::Init();
    SkPixelXorXfermode::Init();
    SkRectShape::Init();
}

#endif
