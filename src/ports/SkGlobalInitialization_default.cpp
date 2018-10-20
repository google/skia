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
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColor4Shader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorFilterShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposeShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkEmptyShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLocalMatrixShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPictureShader)
    SkGradientShader::InitializeFlattenables();
    SkLightingShader::InitializeFlattenables();
    SkPerlinNoiseShader::InitializeFlattenables();
    SkShaderBase::InitializeFlattenables();

    // Color filters.
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorMatrixFilterRowMajor255)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLumaColorFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkToSRGBColorFilter)
    SkColorFilter::InitializeFlattenables();
    SkHighContrastFilter::InitializeFlattenables();
    SkOverdrawColorFilter::InitializeFlattenables();
    SkTableColorFilter::InitializeFlattenables();

    // Mask filters.
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkEmbossMaskFilter)
    SkMaskFilter::InitializeFlattenables();
    SkShaderMaskFilter::InitializeFlattenables();

    // Path effects.
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkCornerPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDashImpl)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDiscretePathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLine2DPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMatrixPE)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkOpPE)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPath1DPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPath2DPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkStrokePE)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkTrimPE)
    SkPathEffect::InitializeFlattenables();

    // Misc.
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLayerDrawLooper)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkRecordedDrawable)
    SkNormalSource::InitializeFlattenables();
}
