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
#include "SkArithmeticMode.h"
#include "SkArcToPathEffect.h"
#include "SkBitmapSource.h"
#include "SkBlurDrawLooper.h"
#include "SkBlurImageFilter.h"
#include "SkBlurMaskFilter.h"
#include "SkColorCubeFilter.h"
#include "SkColorFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkColorShader.h"
#include "SkComposeImageFilter.h"
#include "SkComposeShader.h"
#include "SkCornerPathEffect.h"
#include "SkDashPathEffect.h"
#include "SkDiscretePathEffect.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
#include "SkEmptyShader.h"
#include "SkEmbossMaskFilter.h"
#include "SkFlattenable.h"
#include "SkGradientShader.h"
#include "SkLayerDrawLooper.h"
#include "SkLayerRasterizer.h"
#include "SkLerpXfermode.h"
#include "SkLightingImageFilter.h"
#include "../effects/SkLightingShader.h"
#include "SkLocalMatrixShader.h"
#include "SkLumaColorFilter.h"
#include "SkMagnifierImageFilter.h"
#include "SkMatrixConvolutionImageFilter.h"
#include "SkMergeImageFilter.h"
#include "SkModeColorFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkOnce.h"
#include "SkPerlinNoiseShader.h"
#include "SkPictureImageFilter.h"
#include "SkPictureShader.h"
#include "SkPixelXorXfermode.h"
#include "SkRectShaderImageFilter.h"
#include "SkTableColorFilter.h"
#include "SkTestImageFilters.h"
#include "SkTileImageFilter.h"
#include "SkMatrixImageFilter.h"
#include "SkXfermodeImageFilter.h"

//  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
//  Adding new classes to Init() below has security consequences in Chrome.
//
//  In particular, it is important that we don't create code paths that
//  deserialize untrusted data as SkImageFilters; SkImageFilters are sent from
//  Chrome renderers (untrusted) to the main (trusted) process.
//
//  If you add a new SkImageFilter here _or_ other effect that can be part of
//  an SkImageFilter, it's a good idea to have chrome-security@google.com sign
//  off on the CL, and at minimum extend SampleFilterFuzz.cpp to fuzz it.
//
//  SkPictures are untrusted data.  Please be extremely careful not to allow
//  SkPictures created in a Chrome renderer to be deserialized in the main process.
//
//  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class SkPrivateEffectInitializer {
public:
    static void Init() {
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkArcToPathEffect)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBitmapProcShader)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBitmapSource)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBlurDrawLooper)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBlurImageFilter)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorCubeFilter)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorMatrixFilter)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorShader)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposePathEffect)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposeShader)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkCornerPathEffect)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDashPathEffect)
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
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLocalMatrixShader)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLumaColorFilter)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPath1DPathEffect)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLine2DPathEffect)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkModeColorFilter)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPath2DPathEffect)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPerlinNoiseShader)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPictureImageFilter)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPictureShader)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPixelXorXfermode)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkRectShaderImageFilter)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSumPathEffect)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkTileImageFilter)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMatrixImageFilter)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkXfermodeImageFilter)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMagnifierImageFilter)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMatrixConvolutionImageFilter)

        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkOffsetImageFilter)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposeImageFilter)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMergeImageFilter)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorFilterImageFilter)
        SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDownSampleImageFilter)

        SkArithmeticMode::InitializeFlattenables();
        SkBlurMaskFilter::InitializeFlattenables();
        SkColorFilter::InitializeFlattenables();
        SkGradientShader::InitializeFlattenables();
        SkLightingImageFilter::InitializeFlattenables();
        SkLightingShader::InitializeFlattenables();
        SkTableColorFilter::InitializeFlattenables();
        SkXfermode::InitializeFlattenables();
    }
};

SK_DECLARE_STATIC_ONCE(once);
void SkFlattenable::InitializeFlattenablesIfNeeded() {
    SkOnce(&once, SkPrivateEffectInitializer::Init);
}
