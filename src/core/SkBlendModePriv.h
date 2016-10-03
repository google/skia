/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlendModePriv_DEFINED
#define SkBlendModePriv_DEFINED

#include "SkBlendMode.h"

bool SkBlendMode_SupportsCoverageAsAlpha(SkBlendMode);

#if SK_SUPPORT_GPU
sk_sp<GrXPFactory> SkBlendMode_AsXPFactory(SkBlendMode);
#endif

#endif
