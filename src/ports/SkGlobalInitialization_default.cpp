/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#include "SkBitmapProcShader.h"
#include "SkMallocPixelRef.h"
#include "SkPathEffect.h"
#include "SkPixelRef.h"
#include "SkXfermode.h"

#include "Sk1DPathEffect.h"
#include "Sk2DPathEffect.h"
#include "SkAnnotation.h"
#include "SkArithmeticMode.h"
#include "SkAvoidXfermode.h"
#include "SkBicubicImageFilter.h"
#include "SkBitmapSource.h"
#include "SkBlurDrawLooper.h"
#include "SkBlurImageFilter.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkColorShader.h"
#include "SkColorTable.h"
#include "SkComposeImageFilter.h"
#include "SkComposeShader.h"
#include "SkCornerPathEffect.h"
#include "SkDashPathEffect.h"
#include "SkData.h"
#include "SkDataSet.h"
#include "SkDiscretePathEffect.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
#include "SkEmptyShader.h"
#include "SkEmbossMaskFilter.h"
#include "SkFlattenable.h"
#include "SkGradientShader.h"
#include "SkImages.h"
#include "SkLayerDrawLooper.h"
#include "SkLayerRasterizer.h"
#include "SkLerpXfermode.h"
#include "SkLightingImageFilter.h"
#include "SkMagnifierImageFilter.h"
#include "SkMatrixConvolutionImageFilter.h"
#include "SkMergeImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkPerlinNoiseShader.h"
#include "SkPixelXorXfermode.h"
#include "SkStippleMaskFilter.h"
#include "SkTableColorFilter.h"
#include "SkTestImageFilters.h"
#include "SkXfermodeImageFilter.h"

void SkFlattenable::InitializeFlattenables() {

    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkAnnotation)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkAvoidXfermode)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBicubicImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBitmapProcShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBitmapSource)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBlurDrawLooper)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBlurImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorMatrixFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorTable)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposePathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposeShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkCornerPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDashPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkData)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDataSet)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDilateImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDiscretePathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDisplacementMapEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDropShadowImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkEmbossMaskFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkEmptyShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkErodeImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLayerDrawLooper)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLayerRasterizer)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLerpXfermode)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPath1DPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(Sk2DPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLine2DPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPath2DPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPerlinNoiseShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPixelXorXfermode)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkStippleMaskFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSumPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkXfermodeImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMagnifierImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMatrixConvolutionImageFilter)

    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkOffsetImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposeImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMergeImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorFilterImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDownSampleImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMallocPixelRef)

    SkArithmeticMode::InitializeFlattenables();
    SkBlurMaskFilter::InitializeFlattenables();
    SkColorFilter::InitializeFlattenables();
    SkGradientShader::InitializeFlattenables();
    SkImages::InitializeFlattenables();
    SkLightingImageFilter::InitializeFlattenables();
    SkTableColorFilter::InitializeFlattenables();
    SkXfermode::InitializeFlattenables();
}
