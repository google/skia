/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFPArgs_DEFINED
#define GrFPArgs_DEFINED

#include "include/core/SkMatrix.h"
#include "src/shaders/SkShaderBase.h"

class GrColorInfo;
class GrRecordingContext;
class SkSurfaceProps;

struct GrFPArgs {
    enum class Scope {
        kDefault,
        kRuntimeEffect,
    };

    GrFPArgs(GrRecordingContext* context,
             const GrColorInfo* dstColorInfo,
             const SkSurfaceProps& surfaceProps,
             Scope scope)
            : fContext(context)
            , fDstColorInfo(dstColorInfo)
            , fSurfaceProps(surfaceProps)
            , fScope(scope) {
        SkASSERT(fContext);
    }

    GrRecordingContext* fContext;

    const GrColorInfo* fDstColorInfo;

    const SkSurfaceProps& fSurfaceProps;

    Scope fScope;
};

#endif
