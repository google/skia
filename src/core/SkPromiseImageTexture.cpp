/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPromiseImageTexture.h"

#if defined(SK_GANESH)

SkPromiseImageTexture::SkPromiseImageTexture(const GrBackendTexture& backendTexture) {
    SkASSERT(backendTexture.isValid());
    fBackendTexture = backendTexture;
}

SkPromiseImageTexture::~SkPromiseImageTexture() {}

#endif // defined(SK_GANESH)
