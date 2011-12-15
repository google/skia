/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#include "SkTypes.h"

#if !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

#include "SkBitmapProcShader.h"
#include "SkEffects.h"
#include "SkFlipPixelRef.h"
#include "SkImageRef_ashmem.h"
#include "SkImageRef_GlobalPool.h"
#include "SkMallocPixelRef.h"
#include "SkPathEffect.h"
#include "SkPixelRef.h"
#include "SkShape.h"
#include "SkXfermode.h"

void SkFlattenable::InitializeFlattenables() {
    SkBitmapProcShader::Init();
    SkEffects::Init();
    SkPathEffect::Init();
    SkShape::Init();
    SkXfermode::Init();
}

void SkPixelRef::InitializeFlattenables() {
    SkFlipPixelRef::Init();
    SkImageRef_GlobalPool::Init();
    SkMallocPixelRef::Init();
}

#endif
