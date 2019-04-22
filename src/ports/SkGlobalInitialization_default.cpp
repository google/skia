/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../../include/core/SkFlattenable.h"

#if defined(SK_DISABLE_EFFECT_DESERIALIZATION)

    void SkFlattenable::PrivateInitializer::InitEffects() {}
    void SkFlattenable::PrivateInitializer::InitImageFilters() {}

#else

#include "../effects/SkDashImpl.h"
#include "../effects/SkEmbossMaskFilter.h"
#include "../effects/SkOpPE.h"
#include "../effects/SkTrimPE.h"
#include "../../include/effects/Sk1DPathEffect.h"
#include "../../include/effects/Sk2DPathEffect.h"
#include "../shaders/SkBitmapProcShader.h"
#include "../../include/core/SkColorFilter.h"
#include "../shaders/SkColorFilterShader.h"
#include "../core/SkColorMatrixFilterRowMajor255.h"
#include "../shaders/SkColorShader.h"
#include "../shaders/SkComposeShader.h"
#include "../../include/effects/SkCornerPathEffect.h"
#include "../../include/effects/SkDiscretePathEffect.h"
#include "../shaders/SkEmptyShader.h"
#include "../../include/effects/SkGradientShader.h"
#include "../../include/effects/SkHighContrastFilter.h"
#include "../shaders/SkImageShader.h"
#include "../../include/effects/SkLayerDrawLooper.h"
#include "../shaders/SkLightingShader.h"
#include "../shaders/SkLocalMatrixShader.h"
#include "../../include/effects/SkLumaColorFilter.h"
#include "../core/SkNormalSource.h"
#include "../../include/effects/SkOverdrawColorFilter.h"
#include "../../include/core/SkPathEffect.h"
#include "../../include/effects/SkPerlinNoiseShader.h"
#include "../shaders/SkPictureShader.h"
#include "../core/SkRecordedDrawable.h"
#include "../shaders/SkShaderBase.h"
#include "../../include/effects/SkShaderMaskFilter.h"
#include "../../include/effects/SkTableColorFilter.h"

#include "../../include/effects/SkAlphaThresholdFilter.h"
#include "../../include/effects/SkBlurImageFilter.h"
#include "../../include/effects/SkColorFilterImageFilter.h"
#include "../../include/effects/SkComposeImageFilter.h"
#include "../../include/effects/SkDisplacementMapEffect.h"
#include "../../include/effects/SkDropShadowImageFilter.h"
#include "../../include/effects/SkImageSource.h"
#include "../../include/effects/SkLightingImageFilter.h"
#include "../core/SkLocalMatrixImageFilter.h"
#include "../../include/effects/SkMagnifierImageFilter.h"
#include "../../include/effects/SkMatrixConvolutionImageFilter.h"
#include "../core/SkMatrixImageFilter.h"
#include "../../include/effects/SkMergeImageFilter.h"
#include "../../include/effects/SkMorphologyImageFilter.h"
#include "../../include/effects/SkOffsetImageFilter.h"
#include "../../include/effects/SkPaintImageFilter.h"
#include "../../include/effects/SkPictureImageFilter.h"
#include "../../include/effects/SkTileImageFilter.h"
#include "../../include/effects/SkXfermodeImageFilter.h"

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
