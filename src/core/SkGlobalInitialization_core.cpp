/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkPathEffect.h"
#include "include/private/SkOnce.h"
#include "src/core/SkFlattenablePriv.h"
#include "src/core/SkMatrixImageFilter.h"
#include "src/core/SkRecordedDrawable.h"
#include "src/shaders/SkBitmapProcShader.h"
#include "src/shaders/SkColorFilterShader.h"
#include "src/shaders/SkColorShader.h"
#include "src/shaders/SkComposeShader.h"
#include "src/shaders/SkEmptyShader.h"
#include "src/shaders/SkImageShader.h"
#include "src/shaders/SkLocalMatrixShader.h"
#include "src/shaders/SkPictureShader.h"
#include "src/shaders/SkShaderBase.h"

/*
 *  Registers all of the required effects subclasses for picture deserialization.
 *
 *  Optional subclasses (e.g. Blur) should be registered in the ports/ version of this file,
 *  inside the InitEffects() method.
 */
void SkFlattenable::PrivateInitializer::InitCore() {
    // Shader
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorFilterShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColor4Shader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposeShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkEmptyShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLocalMatrixShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPictureShader)


    // ImageFilter
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMatrixImageFilter)

    SkColorFilter::InitializeFlattenables();
    SkPathEffect::InitializeFlattenables();
    SkShaderBase::InitializeFlattenables();

    // Drawable
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkRecordedDrawable)

    // Now initialize any optional/additional effects (implemented in src/ports)
    InitEffects();
    InitImageFilters();

    // Finalize flattenable initialization.
    SkFlattenable::Finalize();
};

void SkFlattenable::InitializeFlattenablesIfNeeded() {
    static SkOnce once;
    once(SkFlattenable::PrivateInitializer::InitCore);
}
