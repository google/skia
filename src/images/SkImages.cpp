/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFlattenable.h"
#include "SkImageRef_GlobalPool.h"
#include "SkImages.h"

#ifdef SK_BUILD_FOR_ANDROID
#include "SkImageRef_ashmem.h"
#endif

void SkImages::InitializeFlattenables() {
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkImageRef_GlobalPool)
#ifdef SK_BUILD_FOR_ANDROID
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkImageRef_ashmem)
#endif
}
