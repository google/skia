/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPath_DEFINED
#define GrPath_DEFINED

#include "GrGpuResource.h"
#include "GrPathRendering.h"
#include "GrStyle.h"
#include "SkPath.h"
#include "SkRect.h"

class GrShape;

class GrPath : public GrGpuResource {
public:
    /**
     * Initialize to a path with a fixed stroke. Stroke must not be hairline.
     */
    GrPath(GrGpu* gpu, const SkPath& skPath, const GrStyle& style)
        : INHERITED(gpu)
        , fBounds(SkRect::MakeEmpty())
        , fFillType(GrPathRendering::kWinding_FillType)
#ifdef SK_DEBUG
        , fSkPath(skPath)
        , fStyle(style)
#endif
    {
    }

    static void ComputeKey(const GrShape&, GrUniqueKey* key, bool* outIsVolatile);

    const SkRect& getBounds() const { return fBounds; }

    GrPathRendering::FillType getFillType() const { return fFillType; }
#ifdef SK_DEBUG
    bool isEqualTo(const SkPath& path, const GrStyle& style) const;
#endif

protected:
    // Subclass should init these.
    SkRect fBounds;
    GrPathRendering::FillType fFillType;
#ifdef SK_DEBUG
    SkPath fSkPath;
    GrStyle fStyle;
#endif

private:
    typedef GrGpuResource INHERITED;
};

#endif
