/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPath_DEFINED
#define GrPath_DEFINED

#include "GrGpuResource.h"
#include "GrStrokeInfo.h"
#include "SkPath.h"
#include "SkRect.h"

class GrPath : public GrGpuResource {
public:
    

    /**
     * Initialize to a path with a fixed stroke. Stroke must not be hairline.
     */
    GrPath(GrGpu* gpu, const SkPath& skPath, const GrStrokeInfo& stroke)
        : INHERITED(gpu, kCached_LifeCycle)
        , fBounds(skPath.getBounds())
#ifdef SK_DEBUG
        , fSkPath(skPath)
        , fStroke(stroke)
#endif
    {
    }

    static void ComputeKey(const SkPath& path, const GrStrokeInfo& stroke, GrUniqueKey* key,
                           bool* outIsVolatile);

    const SkRect& getBounds() const { return fBounds; }

#ifdef SK_DEBUG
    bool isEqualTo(const SkPath& path, const GrStrokeInfo& stroke) {
        return fSkPath == path && fStroke.hasEqualEffect(stroke);
    }
#endif

protected:
    SkRect fBounds;
#ifdef SK_DEBUG
    SkPath fSkPath;
    GrStrokeInfo fStroke;
#endif

private:
    typedef GrGpuResource INHERITED;
};

#endif
