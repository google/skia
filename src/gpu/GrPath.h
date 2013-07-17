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

class GrPath : public GrResource {
public:
    SK_DECLARE_INST_COUNT(GrPath);

    GrPath(GrGpu* gpu, bool isWrapped) : INHERITED(gpu, isWrapped) {}

    const SkRect& getBounds() const { return fBounds; }

protected:
    SkRect fBounds;

private:
    typedef GrResource INHERITED;
};

#endif
