/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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
 *  Register SkImageFilters for deserialization.
 *
 *  None of these are strictly required for Skia to operate,
 *  so if you're not using deserialization yourself, you can
 *  build and link SkGlobalInitialization_none_imagefilters.cpp instead,
 *  or modify/replace this file as needed.
 */
void SkFlattenable::PrivateInitializer::InitImageFilters() {
    SkAlphaThresholdFilter::RegisterFlattenables();
    SkImageFilter::RegisterFlattenables();
    SkArithmeticImageFilter::RegisterFlattenables();
    SkXfermodeImageFilter::RegisterFlattenables();
    SK_REGISTER_FLATTENABLE(SkDilateImageFilter)
    SK_REGISTER_FLATTENABLE(SkDisplacementMapEffect)
    SK_REGISTER_FLATTENABLE(SkDropShadowImageFilter)
    SK_REGISTER_FLATTENABLE(SkErodeImageFilter)
    SK_REGISTER_FLATTENABLE(SkImageSource)
    SK_REGISTER_FLATTENABLE(SkLocalMatrixImageFilter)
    SK_REGISTER_FLATTENABLE(SkPaintImageFilter)
    SK_REGISTER_FLATTENABLE(SkPictureImageFilter)
    SK_REGISTER_FLATTENABLE(SkTileImageFilter)
    SK_REGISTER_FLATTENABLE(SkMagnifierImageFilter)
    SK_REGISTER_FLATTENABLE(SkMatrixConvolutionImageFilter)
    SK_REGISTER_FLATTENABLE(SkMatrixImageFilter)
    SK_REGISTER_FLATTENABLE(SkOffsetImageFilter)
    SK_REGISTER_FLATTENABLE(SkComposeImageFilter)
    SK_REGISTER_FLATTENABLE(SkMergeImageFilter)
    SK_REGISTER_FLATTENABLE(SkColorFilterImageFilter)
    SkLightingImageFilter::RegisterFlattenables();
}
