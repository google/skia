/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPath_DEFINED
#define GrPath_DEFINED

#include "GrResource.h"
#include "SkRect.h"
#include "SkStrokeRec.h"

class GrPath : public GrResource {
public:
    SK_DECLARE_INST_COUNT(GrPath);

    GrPath(GrGpu* gpu, bool isWrapped, const SkStrokeRec& stroke)
        : INHERITED(gpu, isWrapped),
          fStroke(stroke) {
    }

    const SkRect& getBounds() const { return fBounds; }

    const SkStrokeRec& getStroke() const { return fStroke; }

protected:
    SkRect fBounds;
    SkStrokeRec fStroke;

private:
    typedef GrResource INHERITED;
};

#endif
