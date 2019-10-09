/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/GrOpsRenderPass.h"

#include "include/core/SkRect.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrPrimitiveProcessor.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrTexturePriv.h"

void GrOpsRenderPass::clear(const GrFixedClip& clip, const SkPMColor4f& color) {
    SkASSERT(fRenderTarget);
    // A clear at this level will always be a true clear, so make sure clears were not supposed to
    // be redirected to draws instead
    SkASSERT(!this->gpu()->caps()->performColorClearsAsDraws());
    SkASSERT(!clip.scissorEnabled() || !this->gpu()->caps()->performPartialClearsAsDraws());
    this->onClear(clip, color);
}

void GrOpsRenderPass::clearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
    // As above, make sure the stencil clear wasn't supposed to be a draw rect with stencil settings
    SkASSERT(!this->gpu()->caps()->performStencilClearsAsDraws());
    this->onClearStencilClip(clip, insideStencilMask);
}

bool GrOpsRenderPass::draw(const GrProgramInfo& programInfo,
                           const GrMesh meshes[], int meshCount, const SkRect& bounds) {
    if (!meshCount) {
        return true;
    }

#ifdef SK_DEBUG
    SkASSERT(!programInfo.primProc().hasInstanceAttributes() ||
             this->gpu()->caps()->instanceAttribSupport());

    for (int i = 0; i < meshCount; ++i) {
        SkASSERT(programInfo.primProc().hasVertexAttributes() == meshes[i].hasVertexData());
        SkASSERT(programInfo.primProc().hasInstanceAttributes() == meshes[i].hasInstanceData());
    }

    programInfo.checkAllInstantiated(meshCount);
    programInfo.checkMSAAAndMIPSAreResolved(meshCount);
#endif

    if (programInfo.primProc().numVertexAttributes() > this->gpu()->caps()->maxVertexAttributes()) {
        this->gpu()->stats()->incNumFailedDraws();
        return false;
    }
    this->onDraw(programInfo, meshes, meshCount, bounds);

#ifdef SK_DEBUG
    GrProcessor::CustomFeatures processorFeatures = programInfo.requestedFeatures();
    if (GrProcessor::CustomFeatures::kSampleLocations & processorFeatures) {
        // Verify we always have the same sample pattern key, regardless of graphics state.
        SkASSERT(this->gpu()->findOrAssignSamplePatternKey(fRenderTarget)
                         == fRenderTarget->renderTargetPriv().getSamplePatternKey());
    }
#endif
    return true;
}
