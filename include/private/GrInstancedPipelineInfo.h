/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGrInstancedPipelineInfo_DEFINED
#define GrGrInstancedPipelineInfo_DEFINED

#include "GrRenderTargetProxy.h"

/**
 * Provides info about the pipeline that GrInstancedRendering needs in order to select appropriate
 * drawing algorithms.
 */
struct GrInstancedPipelineInfo {
    GrInstancedPipelineInfo(const GrRenderTargetProxy* rtp)
            : fIsMultisampled(GrFSAAType::kNone != rtp->fsaaType())
            , fIsMixedSampled(GrFSAAType::kMixedSamples == rtp->fsaaType())
            , fIsRenderingToFloat(GrPixelConfigIsFloatingPoint(rtp->config())) {}

    bool canUseCoverageAA() const { return !fIsMultisampled || fIsMixedSampled; }

    bool fIsMultisampled         : 1;
    bool fIsMixedSampled         : 1;
    bool fIsRenderingToFloat     : 1;
};

#endif
