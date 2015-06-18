
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLPathRange.h"
#include "GrGLPath.h"
#include "GrGLPathRendering.h"
#include "GrGLGpu.h"

GrGLPathRange::GrGLPathRange(GrGLGpu* gpu, PathGenerator* pathGenerator, const GrStrokeInfo& stroke)
    : INHERITED(gpu, pathGenerator),
      fStroke(stroke),
      fBasePathID(gpu->glPathRendering()->genPaths(this->getNumPaths())),
      fGpuMemorySize(0) {
    this->init();
    this->registerWithCache();
}

GrGLPathRange::GrGLPathRange(GrGLGpu* gpu,
                             GrGLuint basePathID,
                             int numPaths,
                             size_t gpuMemorySize,
                             const GrStrokeInfo& stroke)
    : INHERITED(gpu, numPaths),
      fStroke(stroke),
      fBasePathID(basePathID),
      fGpuMemorySize(gpuMemorySize) {
    this->init();
    this->registerWithCache();
}

void GrGLPathRange::init() {
    if (fStroke.isDashed()) {
        fShouldStroke = false;
        fShouldFill = true;
    } else {
        fShouldStroke = fStroke.needToApply();
        fShouldFill = fStroke.isFillStyle() ||
                fStroke.getStyle() == SkStrokeRec::kStrokeAndFill_Style;
    }
}

void GrGLPathRange::onInitPath(int index, const SkPath& origSkPath) const {
    GrGLGpu* gpu = static_cast<GrGLGpu*>(this->getGpu());
    if (NULL == gpu) {
        return;
    }

    // Make sure the path at this index hasn't been initted already.
    SkDEBUGCODE(
        GrGLboolean isPath;
        GR_GL_CALL_RET(gpu->glInterface(), isPath, IsPath(fBasePathID + index)));
    SkASSERT(GR_GL_FALSE == isPath);

    const SkPath* skPath = &origSkPath;
    SkTLazy<SkPath> tmpPath;
    const GrStrokeInfo* stroke = &fStroke;
    GrStrokeInfo tmpStroke(SkStrokeRec::kFill_InitStyle);

    // Dashing must be applied to the path. However, if dashing is present,
    // we must convert all the paths to fills. The GrStrokeInfo::applyDash leaves
    // simple paths as strokes but converts other paths to fills.
    // Thus we must stroke the strokes here, so that all paths in the
    // path range are using the same style.
    if (fStroke.isDashed()) {
        if (!stroke->applyDashToPath(tmpPath.init(), &tmpStroke, *skPath)) {
            return;
        }
        skPath = tmpPath.get();
        stroke = &tmpStroke;
        if (tmpStroke.needToApply()) {
            if (!tmpStroke.applyToPath(tmpPath.get(), *tmpPath.get())) {
                return;
            }
            tmpStroke.setFillStyle();
        }
    }

    GrGLPath::InitPathObject(gpu, fBasePathID + index, *skPath, *stroke);

    // TODO: Use a better approximation for the individual path sizes.
    fGpuMemorySize += 100;
}

void GrGLPathRange::onRelease() {
    SkASSERT(this->getGpu());

    if (0 != fBasePathID && this->shouldFreeResources()) {
        static_cast<GrGLGpu*>(this->getGpu())->glPathRendering()->deletePaths(fBasePathID,
                                                                              this->getNumPaths());
        fBasePathID = 0;
    }

    INHERITED::onRelease();
}

void GrGLPathRange::onAbandon() {
    fBasePathID = 0;

    INHERITED::onAbandon();
}
