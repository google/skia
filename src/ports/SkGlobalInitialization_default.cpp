/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sk1DPathEffect.h"
#include "Sk2DPathEffect.h"
#include "SkAlphaThresholdFilter.h"
#include "SkArcToPathEffect.h"
#include "SkBitmapSourceDeserializer.h"
#include "SkBlurImageFilter.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilterRowMajor255.h"
#include "SkComposeImageFilter.h"
#include "SkCornerPathEffect.h"
#include "../../src/effects/SkDashImpl.h"
#include "SkDiscretePathEffect.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
#include "../../src/effects/SkEmbossMaskFilter.h"
#include "SkGradientShader.h"
#include "SkHighContrastFilter.h"
#include "SkImageSource.h"
#include "SkLayerDrawLooper.h"
#include "SkLayerRasterizer.h"
#include "SkLightingImageFilter.h"
#include "SkLightingShader.h"
#include "SkLocalMatrixImageFilter.h"
#include "SkLumaColorFilter.h"
#include "SkMagnifierImageFilter.h"
#include "SkMatrixConvolutionImageFilter.h"
#include "SkMergeImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkNormalSource.h"
#include "SkOffsetImageFilter.h"
#include "../../src/effects/SkOverdrawColorFilter.h"
#include "SkPaintImageFilter.h"
#include "SkPerlinNoiseShader.h"
#include "SkPictureImageFilter.h"
#include "SkRRectsGaussianEdgeMaskFilter.h"
#include "SkTableColorFilter.h"
#include "SkTileImageFilter.h"
#include "SkXfermodeImageFilter.h"

// Security note:
//
// As new subclasses are added here, they should be reviewed by chrome security before they
// support deserializing cross-process: chrome-security@google.com. SampleFilterFuzz.cpp should
// also be amended to exercise the new subclass.
//
// See SkReadBuffer::isCrossProcess() and SkPicture::PictureIOSecurityPrecautionsEnabled()
//

/*
 *  None of these are strictly "required" for Skia to operate.
 *
 *  These are the bulk of our "effects" -- subclasses of various effects on SkPaint.
 *
 *  Clients should feel free to dup this file and modify it as needed. This function "InitEffects"
 *  will automatically be called before any of skia's effects are asked to be deserialized.
 */
void SkFlattenable::PrivateInitializer::InitEffects() {
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBitmapSourceDeserializer)

    // MaskFilter
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkEmbossMaskFilter)
    SkBlurMaskFilter::InitializeFlattenables();
    SkRRectsGaussianEdgeMaskFilter::InitializeFlattenables();

    // DrawLooper
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLayerDrawLooper)

    // Rasterizer
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLayerRasterizer)

    // ColorFilter
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorMatrixFilterRowMajor255)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLumaColorFilter)
    SkAlphaThresholdFilter::InitializeFlattenables();
    SkTableColorFilter::InitializeFlattenables();
    SkOverdrawColorFilter::InitializeFlattenables();
    SkHighContrastFilter::InitializeFlattenables();

    // Shader
    SkPerlinNoiseShader::InitializeFlattenables();
    SkGradientShader::InitializeFlattenables();
    SkLightingShader::InitializeFlattenables();
    SkNormalSource::InitializeFlattenables();

    // PathEffect
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkArcToPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkCornerPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDashImpl)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDiscretePathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPath1DPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLine2DPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPath2DPathEffect)

    // ImageFilter
    SkImageFilter::InitializeFlattenables();
    SkArithmeticImageFilter::InitializeFlattenables();
    SkXfermodeImageFilter::InitializeFlattenables();
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDilateImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDisplacementMapEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDropShadowImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkErodeImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkImageSource)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLocalMatrixImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPaintImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPictureImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkTileImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMagnifierImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMatrixConvolutionImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkOffsetImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposeImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMergeImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorFilterImageFilter)
    SkLightingImageFilter::InitializeFlattenables();
}
