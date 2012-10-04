/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFlattenable.h"
#include "SkFlipPixelRef.h"
#include "SkImageRef_GlobalPool.h"
#include "SkImages.h"

void SkImages::InitializeFlattenables() {
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkFlipPixelRef)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkImageRef_GlobalPool)
}
