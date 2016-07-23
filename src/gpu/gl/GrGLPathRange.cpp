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

GrGLPathRange::GrGLPathRange(GrGLGpu* gpu, PathGenerator* pathGenerator, const GrStyle& style)
    : INHERITED(gpu, pathGenerator),
      fStyle(style),
      fBasePathID(gpu->glPathRendering()->genPaths(this->getNumPaths())),
      fGpuMemorySize(0) {
    this->init();
    this->registerWithCache(SkBudgeted::kYes);
}

GrGLPathRange::GrGLPathRange(GrGLGpu* gpu,
                             GrGLuint basePathID,
                             int numPaths,
                             size_t gpuMemorySize,
                             const GrStyle& style)
    : INHERITED(gpu, numPaths),
      fStyle(style),
      fBasePathID(basePathID),
      fGpuMemorySize(gpuMemorySize) {
    this->init();
    this->registerWithCache(SkBudgeted::kYes);
}

void GrGLPathRange::init() {
    const SkStrokeRec& stroke = fStyle.strokeRec();
    // Must force fill:
    // * dashing: NVPR stroke dashing is different to Skia.
    // * end caps: NVPR stroking degenerate contours with end caps is different to Skia.
    bool forceFill = fStyle.pathEffect() ||
            (stroke.needToApply() && stroke.getCap() != SkPaint::kButt_Cap);

    if (forceFill) {
        fShouldStroke = false;
        fShouldFill = true;
    } else {
        fShouldStroke = stroke.needToApply();
        fShouldFill = stroke.isFillStyle() ||
                stroke.getStyle() == SkStrokeRec::kStrokeAndFill_Style;
    }
}

void GrGLPathRange::onInitPath(int index, const SkPath& origSkPath) const {
    GrGLGpu* gpu = static_cast<GrGLGpu*>(this->getGpu());
    if (nullptr == gpu) {
        return;
    }
    // Make sure the path at this index hasn't been initted already.
    SkDEBUGCODE(
        GrGLboolean isPath;
        GR_GL_CALL_RET(gpu->glInterface(), isPath, IsPath(fBasePathID + index)));
    SkASSERT(GR_GL_FALSE == isPath);

    if (origSkPath.isEmpty()) {
        GrGLPath::InitPathObjectEmptyPath(gpu, fBasePathID + index);
    } else if (fShouldStroke) {
        GrGLPath::InitPathObjectPathData(gpu, fBasePathID + index, origSkPath);
        GrGLPath::InitPathObjectStroke(gpu, fBasePathID + index, fStyle.strokeRec());
    } else {
        const SkPath* skPath = &origSkPath;
        SkTLazy<SkPath> tmpPath;
        if (!fStyle.isSimpleFill()) {
            SkStrokeRec::InitStyle fill;
            // The path effect must be applied to the path. However, if a path effect is present,
            // we must convert all the paths to fills. The path effect application may leave
            // simple paths as strokes but converts other paths to fills.
            // Thus we must stroke the strokes here, so that all paths in the
            // path range are using the same style.
            if (!fStyle.applyToPath(tmpPath.init(), &fill, *skPath, SK_Scalar1)) {
                return;
            }
            // We shouldn't have allowed hairlines or arbitrary path effect styles to get here
            // so after application we better have a filled path.
            SkASSERT(SkStrokeRec::kFill_InitStyle == fill);
            skPath = tmpPath.get();

        }
        GrGLPath::InitPathObjectPathData(gpu, fBasePathID + index, *skPath);
    }
    // TODO: Use a better approximation for the individual path sizes.
    fGpuMemorySize += 100;
}

void GrGLPathRange::onRelease() {
    SkASSERT(this->getGpu());

    if (0 != fBasePathID) {
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
