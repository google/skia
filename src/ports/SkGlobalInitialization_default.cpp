/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFlattenable.h"

#if defined(SK_DISABLE_EFFECT_DESERIALIZATION)

    void SkFlattenable::PrivateInitializer::InitEffects() {}
    void SkFlattenable::PrivateInitializer::InitImageFilters() {}

#else

    #include "include/core/SkBBHFactory.h"
    #include "include/core/SkColorFilter.h"
    #include "include/core/SkPathEffect.h"
    #include "include/effects/Sk1DPathEffect.h"
    #include "include/effects/Sk2DPathEffect.h"
    #include "include/effects/SkCornerPathEffect.h"
    #include "include/effects/SkDiscretePathEffect.h"
    #include "include/effects/SkOverdrawColorFilter.h"
    #include "include/effects/SkPerlinNoiseShader.h"
    #include "include/effects/SkShaderMaskFilter.h"
    #include "src/core/SkBlendModeBlender.h"
    #include "src/core/SkImageFilter_Base.h"
    #include "src/core/SkRecordedDrawable.h"
    #include "src/effects/SkDashImpl.h"
    #include "src/effects/SkEmbossMaskFilter.h"
    #include "src/effects/SkOpPE.h"
    #include "src/effects/SkTrimPE.h"
    #include "src/shaders/SkBitmapProcShader.h"
    #include "src/shaders/SkColorFilterShader.h"
    #include "src/shaders/SkImageShader.h"
    #include "src/shaders/SkLocalMatrixShader.h"
    #include "src/shaders/SkPictureShader.h"
    #include "src/shaders/SkShaderBase.h"
    #include "src/shaders/gradients/SkGradientShaderBase.h"

    #include "include/effects/SkImageFilters.h"
    #include "src/core/SkLocalMatrixImageFilter.h"

#ifdef SK_ENABLE_SKSL
    #include "include/effects/SkRuntimeEffect.h"
#endif

#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
    #include "include/effects/SkLayerDrawLooper.h"
#endif

    /**
     *  Register most effects for deserialization.
     *
     *  None of these are strictly required for Skia to operate, so if you're
     *  not using deserialization yourself, you can define
     *  SK_DISABLE_EFFECT_SERIALIZATION, or modify/replace this file as needed.
     */
    void SkFlattenable::PrivateInitializer::InitEffects() {
        // Shaders.
        SkRegisterColor4ShaderFlattenable();
        SK_REGISTER_FLATTENABLE(SkColorFilterShader);
        SkRegisterColorShaderFlattenable();
        SkRegisterComposeShaderFlattenable();
        SkRegisterCoordClampShaderFlattenable();
        SkRegisterEmptyShaderFlattenable();
        SK_REGISTER_FLATTENABLE(SkLocalMatrixShader);
        SK_REGISTER_FLATTENABLE(SkPictureShader);
        SkRegisterLinearGradientShaderFlattenable();
        SkRegisterRadialGradientShaderFlattenable();
        SkRegisterSweepGradientShaderFlattenable();
        SkRegisterTwoPointConicalGradientShaderFlattenable();
        SkPerlinNoiseShader::RegisterFlattenables();
        SkShaderBase::RegisterFlattenables();

        // Color filters.
        SkRegisterMatrixColorFilterFlattenable();
        SkRegisterComposeColorFilterFlattenable();
        SkRegisterModeColorFilterFlattenable();
        SkRegisterColorSpaceXformColorFilterFlattenable();
        SkRegisterWorkingFormatColorFilterFlattenable();
        SkRegisterTableColorFilterFlattenable();

        // Blenders.
        SK_REGISTER_FLATTENABLE(SkBlendModeBlender);

#ifdef SK_ENABLE_SKSL
        // Runtime shaders, color filters, and blenders.
        SkRuntimeEffect::RegisterFlattenables();
#endif

        // Mask filters.
        SK_REGISTER_FLATTENABLE(SkEmbossMaskFilter);
        SkMaskFilter::RegisterFlattenables();
        SkShaderMaskFilter::RegisterFlattenables();

        // Path effects.
        SkCornerPathEffect::RegisterFlattenables();
        SK_REGISTER_FLATTENABLE(SkDashImpl);
        SkDiscretePathEffect::RegisterFlattenables();
        SkLine2DPathEffect::RegisterFlattenables();
        SkPath2DPathEffect::RegisterFlattenables();
        SK_REGISTER_FLATTENABLE(SkMatrixPE);
        SK_REGISTER_FLATTENABLE(SkOpPE);
        SkPath1DPathEffect::RegisterFlattenables();
        SK_REGISTER_FLATTENABLE(SkStrokePE);
        SK_REGISTER_FLATTENABLE(SkStrokeAndFillPE);
        SK_REGISTER_FLATTENABLE(SkTrimPE);
        SkPathEffectBase::RegisterFlattenables();

        // Misc.
#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
        SK_REGISTER_FLATTENABLE(SkLayerDrawLooper);
#endif
        SK_REGISTER_FLATTENABLE(SkRecordedDrawable);
    }

    /*
     *  Register SkImageFilters for deserialization.
     *
     *  None of these are strictly required for Skia to operate, so if you're
     *  not using deserialization yourself, you can define
     *  SK_DISABLE_EFFECT_SERIALIZATION, or modify/replace this file as needed.
     */
    void SkFlattenable::PrivateInitializer::InitImageFilters() {
        SkRegisterAlphaThresholdImageFilterFlattenable();
        SkRegisterArithmeticImageFilterFlattenable();
        SkRegisterBlendImageFilterFlattenable();
        SkRegisterBlurImageFilterFlattenable();
        SkRegisterColorFilterImageFilterFlattenable();
        SkRegisterComposeImageFilterFlattenable();
        SkRegisterCropImageFilterFlattenable();
        SkRegisterDisplacementMapImageFilterFlattenable();
        SkRegisterDropShadowImageFilterFlattenable();
        SkRegisterImageImageFilterFlattenable();
        SkRegisterLightingImageFilterFlattenables();
        SkRegisterMagnifierImageFilterFlattenable();
        SkRegisterMatrixConvolutionImageFilterFlattenable();
        SkRegisterMatrixTransformImageFilterFlattenable();
        SkRegisterMergeImageFilterFlattenable();
        SkRegisterMorphologyImageFilterFlattenables();
        SkRegisterPictureImageFilterFlattenable();
#ifdef SK_ENABLE_SKSL
        SkRegisterRuntimeImageFilterFlattenable();
#endif
        SkRegisterShaderImageFilterFlattenable();
        SkRegisterTileImageFilterFlattenable();
        SK_REGISTER_FLATTENABLE(SkLocalMatrixImageFilter);
    }

#endif
