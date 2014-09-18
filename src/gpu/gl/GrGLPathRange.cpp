
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLPathRange.h"
#include "GrGLPath.h"
#include "GrGLPathRendering.h"
#include "GrGpuGL.h"

GrGLPathRange::GrGLPathRange(GrGpuGL* gpu, PathGenerator* pathGenerator, const SkStrokeRec& stroke)
    : INHERITED(gpu, pathGenerator, stroke),
      fBasePathID(gpu->glPathRendering()->genPaths(this->getNumPaths())),
      fGpuMemorySize(0) {
    this->registerWithCache();
}

GrGLPathRange::GrGLPathRange(GrGpuGL* gpu,
                             GrGLuint basePathID,
                             int numPaths,
                             size_t gpuMemorySize,
                             const SkStrokeRec& stroke)
    : INHERITED(gpu, numPaths, stroke),
      fBasePathID(basePathID),
      fGpuMemorySize(gpuMemorySize) {
    this->registerWithCache();
}

GrGLPathRange::~GrGLPathRange() {
    this->release();
}

void GrGLPathRange::onInitPath(int index, const SkPath& skPath) const {
    GrGpuGL* gpu = static_cast<GrGpuGL*>(this->getGpu());
    if (NULL == gpu) {
        return;
    }

    // Make sure the path at this index hasn't been initted already.
    SkDEBUGCODE(
        GrGLboolean isPath;
        GR_GL_CALL_RET(gpu->glInterface(), isPath, IsPath(fBasePathID + index)));
    SkASSERT(GR_GL_FALSE == isPath);

    GrGLPath::InitPathObject(gpu, fBasePathID + index, skPath, this->getStroke());

    // TODO: Use a better approximation for the individual path sizes.
    fGpuMemorySize += 100;
}

void GrGLPathRange::onRelease() {
    SkASSERT(this->getGpu());

    if (0 != fBasePathID && !this->isWrapped()) {
        static_cast<GrGpuGL*>(this->getGpu())->glPathRendering()->deletePaths(fBasePathID,
                                                                              this->getNumPaths());
        fBasePathID = 0;
    }

    INHERITED::onRelease();
}

void GrGLPathRange::onAbandon() {
    fBasePathID = 0;

    INHERITED::onAbandon();
}
