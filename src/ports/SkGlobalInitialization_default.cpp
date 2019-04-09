/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFlattenable.h"

#if defined(SK_DISABLE_EFFECT_DESERIALIZATION)

    void SkFlattenable::PrivateInitializer::InitEffects() {}
    void SkFlattenable::PrivateInitializer::InitImageFilters() {}

#else

    #include "../../src/effects/SkDashImpl.h"
    #include "../../src/effects/SkEmbossMaskFilter.h"
    #include "../../src/effects/SkOpPE.h"
    #include "../../src/effects/SkTrimPE.h"
    #include "Sk1DPathEffect.h"
    #include "Sk2DPathEffect.h"
    #include "SkBitmapProcShader.h"
    #include "SkColorFilter.h"
    #include "SkColorFilterShader.h"
    #include "SkColorMatrixFilterRowMajor255.h"
    #include "SkColorShader.h"
    #include "SkComposeShader.h"
    #include "SkCornerPathEffect.h"
    #include "SkDiscretePathEffect.h"
    #include "SkEmptyShader.h"
    #include "SkGradientShader.h"
    #include "SkHighContrastFilter.h"
    #include "SkImageShader.h"
    #include "SkLayerDrawLooper.h"
    #include "SkLightingShader.h"
    #include "SkLocalMatrixShader.h"
    #include "SkLumaColorFilter.h"
    #include "SkMixerBase.h"
    #include "SkMixerShader.h"
    #include "SkNormalSource.h"
    #include "SkOverdrawColorFilter.h"
    #include "SkPathEffect.h"
    #include "SkPerlinNoiseShader.h"
    #include "SkPictureShader.h"
    #include "SkRecordedDrawable.h"
    #include "SkShaderBase.h"
    #include "SkShaderMaskFilter.h"
    #include "SkTableColorFilter.h"

    #include "SkAlphaThresholdFilter.h"
    #include "SkBlurImageFilter.h"
    #include "SkColorFilterImageFilter.h"
    #include "SkComposeImageFilter.h"
    #include "SkDisplacementMapEffect.h"
    #include "SkDropShadowImageFilter.h"
    #include "SkImageSource.h"
    #include "SkLightingImageFilter.h"
    #include "SkLocalMatrixImageFilter.h"
    #include "SkMagnifierImageFilter.h"
    #include "SkMatrixConvolutionImageFilter.h"
    #include "SkMatrixImageFilter.h"
    #include "SkMergeImageFilter.h"
    #include "SkMorphologyImageFilter.h"
    #include "SkOffsetImageFilter.h"
    #include "SkPaintImageFilter.h"
    #include "SkPictureImageFilter.h"
    #include "SkTileImageFilter.h"
    #include "SkXfermodeImageFilter.h"

    /*
     *  Register most effects for deserialization.
     *
     *  None of these are strictly required for Skia to operate, so if you're
     *  not using deserialization yourself, you can define
     *  SK_DISABLE_EFFECT_SERIALIZATION, or modify/replace this file as needed.
     */
    void SkFlattenable::PrivateInitializer::InitEffects() {
        // Shaders.
        SK_REGISTER_FLATTENABLE(SkColor4Shader);
        SK_REGISTER_FLATTENABLE(SkColorFilterShader);
        SK_REGISTER_FLATTENABLE(SkColorShader);
        SK_REGISTER_FLATTENABLE(SkShader_Blend);
        SK_REGISTER_FLATTENABLE(SkShader_Lerp);
        SK_REGISTER_FLATTENABLE(SkShader_LerpRed);
        SK_REGISTER_FLATTENABLE(SkEmptyShader);
        SK_REGISTER_FLATTENABLE(SkLocalMatrixShader);
        SK_REGISTER_FLATTENABLE(SkPictureShader);
        SK_REGISTER_FLATTENABLE(SkShader_Mixer);
        SkGradientShader::RegisterFlattenables();
        SkLightingShader::RegisterFlattenables();
        SkPerlinNoiseShader::RegisterFlattenables();
        SkShaderBase::RegisterFlattenables();

        // Color filters.
        SK_REGISTER_FLATTENABLE(SkColorMatrixFilterRowMajor255);
        SK_REGISTER_FLATTENABLE(SkLumaColorFilter);
        SkColorFilter::RegisterFlattenables();
        SkHighContrastFilter::RegisterFlattenables();
        SkOverdrawColorFilter::RegisterFlattenables();
        SkTableColorFilter::RegisterFlattenables();

        // Mask filters.
        SK_REGISTER_FLATTENABLE(SkEmbossMaskFilter);
        SkMaskFilter::RegisterFlattenables();
        SkShaderMaskFilter::RegisterFlattenables();

        // Path effects.
        SK_REGISTER_FLATTENABLE(SkCornerPathEffect);
        SK_REGISTER_FLATTENABLE(SkDashImpl);
        SK_REGISTER_FLATTENABLE(SkDiscretePathEffect);
        SK_REGISTER_FLATTENABLE(SkLine2DPathEffect);
        SK_REGISTER_FLATTENABLE(SkMatrixPE);
        SK_REGISTER_FLATTENABLE(SkOpPE);
        SK_REGISTER_FLATTENABLE(SkPath1DPathEffect);
        SK_REGISTER_FLATTENABLE(SkPath2DPathEffect);
        SK_REGISTER_FLATTENABLE(SkStrokePE);
        SK_REGISTER_FLATTENABLE(SkTrimPE);
        SkPathEffect::RegisterFlattenables();

        // Misc.
        SK_REGISTER_FLATTENABLE(SkLayerDrawLooper);
        SK_REGISTER_FLATTENABLE(SkRecordedDrawable);
        SkNormalSource::RegisterFlattenables();
        SkMixerBase::RegisterFlattenables();
    }

    /*
     *  Register SkImageFilters for deserialization.
     *
     *  None of these are strictly required for Skia to operate, so if you're
     *  not using deserialization yourself, you can define
     *  SK_DISABLE_EFFECT_SERIALIZATION, or modify/replace this file as needed.
     */
    void SkFlattenable::PrivateInitializer::InitImageFilters() {
        SkAlphaThresholdFilter::RegisterFlattenables();
        SkImageFilter::RegisterFlattenables();
        SkArithmeticImageFilter::RegisterFlattenables();
        SkXfermodeImageFilter::RegisterFlattenables();
        SK_REGISTER_FLATTENABLE(SkDilateImageFilter);
        SK_REGISTER_FLATTENABLE(SkDisplacementMapEffect);
        SK_REGISTER_FLATTENABLE(SkDropShadowImageFilter);
        SK_REGISTER_FLATTENABLE(SkErodeImageFilter);
        SK_REGISTER_FLATTENABLE(SkImageSource);
        SK_REGISTER_FLATTENABLE(SkLocalMatrixImageFilter);
        SK_REGISTER_FLATTENABLE(SkPaintImageFilter);
        SK_REGISTER_FLATTENABLE(SkPictureImageFilter);
        SK_REGISTER_FLATTENABLE(SkTileImageFilter);
        SK_REGISTER_FLATTENABLE(SkMagnifierImageFilter);
        SK_REGISTER_FLATTENABLE(SkMatrixConvolutionImageFilter);
        SK_REGISTER_FLATTENABLE(SkMatrixImageFilter);
        SK_REGISTER_FLATTENABLE(SkOffsetImageFilter);
        SK_REGISTER_FLATTENABLE(SkComposeImageFilter);
        SK_REGISTER_FLATTENABLE(SkMergeImageFilter);
        SK_REGISTER_FLATTENABLE(SkColorFilterImageFilter);
        SkLightingImageFilter::RegisterFlattenables();
    }

#endif
