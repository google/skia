/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGrInstancedPipelineInfo_DEFINED
#define GrGrInstancedPipelineInfo_DEFINED

#include "GrRenderTarget.h"

/**
 * Provides info about the pipeline that GrInstancedRendering needs in order to select appropriate
 * drawing algorithms.
 */
struct GrInstancedPipelineInfo {
    GrInstancedPipelineInfo(const GrRenderTarget* rt)
        : fIsMultisampled(rt->isStencilBufferMultisampled()),
          fIsMixedSampled(rt->isMixedSampled()),
          fIsRenderingToFloat(GrPixelConfigIsFloatingPoint(rt->desc().fConfig)),
          fColorDisabled(false),
          fDrawingShapeToStencil(false),
          fCanDiscard(false) {
    }

    bool canUseCoverageAA() const {
        return !fIsMultisampled || (fIsMixedSampled && !fDrawingShapeToStencil);
    }

    bool fIsMultisampled         : 1;
    bool fIsMixedSampled         : 1;
    bool fIsRenderingToFloat     : 1;
    bool fColorDisabled          : 1;
    /**
     * Indicates that the instanced renderer should take extra precautions to ensure the shape gets
     * drawn correctly to the stencil buffer (e.g. no coverage AA). NOTE: this does not mean a
     * stencil test is or is not active.
     */
    bool fDrawingShapeToStencil  : 1;
    /**
     * Indicates that the instanced renderer can use processors with discard instructions. This
     * should not be set if the shader will use derivatives, automatic mipmap LOD, or other features
     * that depend on neighboring pixels. Some draws will fail to create if this is not set.
     */
    bool fCanDiscard             : 1;
};

#endif
