/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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
#include "SkNormalSource.h"
#include "SkOverdrawColorFilter.h"
#include "SkPathEffect.h"
#include "SkPerlinNoiseShader.h"
#include "SkPictureShader.h"
#include "SkRecordedDrawable.h"
#include "SkShaderBase.h"
#include "SkShaderMaskFilter.h"
#include "SkTableColorFilter.h"
#include "SkToSRGBColorFilter.h"

/*
 *  Register most effects for deserialization.
 *
 *  None of these are strictly required for Skia to operate,
 *  so if you're not using deserialization yourself, you can
 *  build and link SkGlobalInitialization_none.cpp instead,
 *  or modify/replace this file as needed.
 */
void SkFlattenable::PrivateInitializer::InitEffects() {
    // Shaders.
    SK_REGISTER_FLATTENABLE(SkColor4Shader)
    SK_REGISTER_FLATTENABLE(SkColorFilterShader)
    SK_REGISTER_FLATTENABLE(SkColorShader)
    SK_REGISTER_FLATTENABLE(SkComposeShader)
    SK_REGISTER_FLATTENABLE(SkEmptyShader)
    SK_REGISTER_FLATTENABLE(SkLocalMatrixShader)
    SK_REGISTER_FLATTENABLE(SkPictureShader)
    SkGradientShader::RegisterFlattenables();
    SkLightingShader::RegisterFlattenables();
    SkPerlinNoiseShader::RegisterFlattenables();
    SkShaderBase::RegisterFlattenables();

    // Color filters.
    SK_REGISTER_FLATTENABLE(SkColorMatrixFilterRowMajor255)
    SK_REGISTER_FLATTENABLE(SkLumaColorFilter)
    SK_REGISTER_FLATTENABLE(SkToSRGBColorFilter)
    SkColorFilter::RegisterFlattenables();
    SkHighContrastFilter::RegisterFlattenables();
    SkOverdrawColorFilter::RegisterFlattenables();
    SkTableColorFilter::RegisterFlattenables();

    // Mask filters.
    SK_REGISTER_FLATTENABLE(SkEmbossMaskFilter)
    SkMaskFilter::RegisterFlattenables();
    SkShaderMaskFilter::RegisterFlattenables();

    // Path effects.
    SK_REGISTER_FLATTENABLE(SkCornerPathEffect)
    SK_REGISTER_FLATTENABLE(SkDashImpl)
    SK_REGISTER_FLATTENABLE(SkDiscretePathEffect)
    SK_REGISTER_FLATTENABLE(SkLine2DPathEffect)
    SK_REGISTER_FLATTENABLE(SkMatrixPE)
    SK_REGISTER_FLATTENABLE(SkOpPE)
    SK_REGISTER_FLATTENABLE(SkPath1DPathEffect)
    SK_REGISTER_FLATTENABLE(SkPath2DPathEffect)
    SK_REGISTER_FLATTENABLE(SkStrokePE)
    SK_REGISTER_FLATTENABLE(SkTrimPE)
    SkPathEffect::RegisterFlattenables();

    // Misc.
    SK_REGISTER_FLATTENABLE(SkLayerDrawLooper)
    SK_REGISTER_FLATTENABLE(SkRecordedDrawable)
    SkNormalSource::RegisterFlattenables();
}
