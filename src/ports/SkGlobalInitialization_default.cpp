/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/Sk1DPathEffect.h"
#include "include/effects/Sk2DPathEffect.h"
#include "include/effects/SkCornerPathEffect.h"
#include "include/effects/SkDiscretePathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkHighContrastFilter.h"
#include "include/effects/SkLayerDrawLooper.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkOverdrawColorFilter.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/effects/SkShaderMaskFilter.h"
#include "include/effects/SkTableColorFilter.h"
#include "include/effects/SkToSRGBColorFilter.h"
#include "src/core/SkColorMatrixFilterRowMajor255.h"
#include "src/core/SkNormalSource.h"
#include "src/effects/SkDashImpl.h"
#include "src/effects/SkEmbossMaskFilter.h"
#include "src/effects/SkOpPE.h"
#include "src/effects/SkTrimPE.h"
#include "src/shaders/SkLightingShader.h"

/*
 *  None of these are strictly "required" for Skia to operate.
 *
 *  These are the bulk of our "effects" -- subclasses of various effects on SkPaint.
 *
 *  Clients should feel free to dup this file and modify it as needed. This function "InitEffects"
 *  will automatically be called before any of skia's effects are asked to be deserialized.
 */
void SkFlattenable::PrivateInitializer::InitEffects() {
    // MaskFilter
    SkMaskFilter::InitializeFlattenables();
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkEmbossMaskFilter)
    SkShaderMaskFilter::InitializeFlattenables();

    // DrawLooper
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLayerDrawLooper)

    // ColorFilter
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorMatrixFilterRowMajor255)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLumaColorFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkToSRGBColorFilter)
    SkTableColorFilter::InitializeFlattenables();
    SkOverdrawColorFilter::InitializeFlattenables();
    SkHighContrastFilter::InitializeFlattenables();

    // Shader
    SkPerlinNoiseShader::InitializeFlattenables();
    SkGradientShader::InitializeFlattenables();
    SkLightingShader::InitializeFlattenables();
    SkNormalSource::InitializeFlattenables();

    // PathEffect
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkCornerPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDashImpl)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDiscretePathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPath1DPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLine2DPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPath2DPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkTrimPE)

    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkOpPE)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMatrixPE)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkStrokePE)
}
