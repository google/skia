/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPromiseImageTexture_DEFINED
#define SkPromiseImageTexture_DEFINED

#include "include/core/SkTypes.h"

// TODO(kjlubick) remove this shim header after clients are migrated
#if defined(SK_GANESH)
#include "include/private/chromium/GrPromiseImageTexture.h"

typedef GrPromiseImageTexture SkPromiseImageTexture;
#endif

#endif // SkPromiseImageTexture_DEFINED
