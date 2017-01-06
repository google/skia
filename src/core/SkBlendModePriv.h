/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlendModePriv_DEFINED
#define SkBlendModePriv_DEFINED

#include "SkBlendMode.h"
#include "SkRasterPipeline.h"

bool SkBlendMode_SupportsCoverageAsAlpha(SkBlendMode);
bool SkBlendMode_CanOverflow(SkBlendMode);
bool SkBlendMode_AppendStages(SkBlendMode, SkRasterPipeline* = nullptr);

#if SK_SUPPORT_GPU
#include "GrXferProcessor.h"
const GrXPFactory* SkBlendMode_AsXPFactory(SkBlendMode);
#endif


#endif
