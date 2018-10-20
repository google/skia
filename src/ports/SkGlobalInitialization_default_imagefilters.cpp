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
    SkAlphaThresholdFilter::InitializeFlattenables();
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
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMatrixImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkOffsetImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposeImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMergeImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorFilterImageFilter)
    SkLightingImageFilter::InitializeFlattenables();
}
