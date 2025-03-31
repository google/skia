/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFPArgs_DEFINED
#define GrFPArgs_DEFINED

#include "include/private/base/SkAssert.h"

class GrColorInfo;
class SkSurfaceProps;
namespace skgpu::ganesh { class SurfaceDrawContext; }

struct GrFPArgs {
    enum class Scope {
        kDefault,
        kRuntimeEffect,
    };

    GrFPArgs(skgpu::ganesh::SurfaceDrawContext* sdc,
             const GrColorInfo* dstColorInfo,
             const SkSurfaceProps& surfaceProps,
             Scope scope)
            : fSurfaceDrawContext(sdc)
            , fDstColorInfo(dstColorInfo)
            , fSurfaceProps(surfaceProps)
            , fScope(scope) {
        SkASSERT(fSurfaceDrawContext);
    }

    skgpu::ganesh::SurfaceDrawContext* fSurfaceDrawContext;

    const GrColorInfo* fDstColorInfo;

    const SkSurfaceProps& fSurfaceProps;

    Scope fScope;
};

#endif
