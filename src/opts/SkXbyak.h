/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXbyak_DEFINED
#define SkXbyak_DEFINED

#include "SkRasterPipeline.h"

std::function<void(size_t, size_t, size_t)>
sk_compile_pipeline_xbyak(const SkRasterPipeline::Stage*, int);

#endif//SkXbyak_DEFINED
