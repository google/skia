/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSemaphore.h"
#include "gl/GrGLTypes.h"

GrGLExternalTextureData::GrGLExternalTextureData(const GrGLTextureInfo& info,
                                                 sk_sp<GrSemaphore> semaphore)
        : INHERITED(std::move(semaphore))
        , fInfo(info) {}
