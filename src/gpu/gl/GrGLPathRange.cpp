
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

GrGLPathRange::GrGLPathRange(GrGpuGL* gpu, size_t size, const SkStrokeRec& stroke)
    : INHERITED(gpu, size, stroke),
      fBasePathID(gpu->glPathRendering()->genPaths(fSize)),
      fNumDefinedPaths(0) {
}

GrGLPathRange::~GrGLPathRange() {
    this->release();
}

void GrGLPathRange::initAt(size_t index, const SkPath& skPath) {
    GrGpuGL* gpu = static_cast<GrGpuGL*>(this->getGpu());
    if (NULL == gpu) {
        return;
    }

    // Make sure the path at this index hasn't been initted already.
    SkDEBUGCODE(
        GrGLboolean isPath;
        GR_GL_CALL_RET(gpu->glInterface(), isPath, IsPath(fBasePathID + index)));
    SkASSERT(GR_GL_FALSE == isPath);

    GrGLPath::InitPathObject(gpu, fBasePathID + index, skPath, fStroke);
    ++fNumDefinedPaths;
    this->didChangeGpuMemorySize();
}

void GrGLPathRange::onRelease() {
    SkASSERT(NULL != this->getGpu());

    if (0 != fBasePathID && !this->isWrapped()) {
        static_cast<GrGpuGL*>(this->getGpu())->glPathRendering()->deletePaths(fBasePathID, fSize);
        fBasePathID = 0;
    }

    INHERITED::onRelease();
}

void GrGLPathRange::onAbandon() {
    fBasePathID = 0;

    INHERITED::onAbandon();
}
