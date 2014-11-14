/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPath_DEFINED
#define GrPath_DEFINED

#include "GrGpuResource.h"
#include "GrResourceCache.h"
#include "SkPath.h"
#include "SkRect.h"
#include "SkStrokeRec.h"

class GrPath : public GrGpuResource {
public:
    SK_DECLARE_INST_COUNT(GrPath);

    /**
     * Initialize to a path with a fixed stroke. Stroke must not be hairline.
     */
    GrPath(GrGpu* gpu, bool isWrapped, const SkPath& skPath, const SkStrokeRec& stroke)
        : INHERITED(gpu, isWrapped),
          fSkPath(skPath),
          fStroke(stroke),
          fBounds(skPath.getBounds()) {
    }

    static GrResourceKey ComputeKey(const SkPath& path, const SkStrokeRec& stroke);
    static uint64_t ComputeStrokeKey(const SkStrokeRec&);

    bool isEqualTo(const SkPath& path, const SkStrokeRec& stroke) {
        return fSkPath == path && fStroke == stroke;
    }

    const SkRect& getBounds() const { return fBounds; }

    const SkStrokeRec& getStroke() const { return fStroke; }

protected:
    SkPath fSkPath;
    SkStrokeRec fStroke;
    SkRect fBounds;

private:
    typedef GrGpuResource INHERITED;
};

#endif
