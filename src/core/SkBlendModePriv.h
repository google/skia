/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlendModePriv_DEFINED
#define SkBlendModePriv_DEFINED

#include "SkBlendMode.h"
#include "SkPM4f.h"

class SkRasterPipeline;

bool SkBlendMode_SupportsCoverageAsAlpha(SkBlendMode);

/**
 *  If this returns true, the blendmode can produce values > 1, so the call may need to append
 *  a clamp_1 or clamp_a stage afterwards. clamp_a is never wrong (for pre-mul colors) but it
 *  is slightly slower than clamp_1.
 */
bool SkBlendMode_CanOverflow(SkBlendMode);

/**
 *  Append the corresponding blend stage to the pipeline.
 */
void SkBlendMode_AppendStages(SkBlendMode, SkRasterPipeline*);

#if SK_SUPPORT_GPU
#include "GrXferProcessor.h"
const GrXPFactory* SkBlendMode_AsXPFactory(SkBlendMode);
#endif

enum class SkBlendModeCoeff {
    kZero, /** 0 */
    kOne,  /** 1 */
    kSC,   /** src color */
    kISC,  /** inverse src color (i.e. 1 - sc) */
    kDC,   /** dst color */
    kIDC,  /** inverse dst color (i.e. 1 - dc) */
    kSA,   /** src alpha */
    kISA,  /** inverse src alpha (i.e. 1 - sa) */
    kDA,   /** dst alpha */
    kIDA,  /** inverse dst alpha (i.e. 1 - da) */

    kCoeffCount
};

bool SkBlendMode_AsCoeff(SkBlendMode mode, SkBlendModeCoeff* src, SkBlendModeCoeff* dst);

SkPM4f SkBlendMode_Apply(SkBlendMode, SkPM4f src, SkPM4f dst);

#endif
