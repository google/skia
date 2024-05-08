/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GpuTools_DEFINED
#define GpuTools_DEFINED

#include "include/core/SkSurface.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#endif

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/Surface_Graphite.h"
#endif

namespace skgpu {
// Flush any surface, even if we don't know what GPU backend it is for. This keeps the
// comparisons between Ganesh and Graphite more fair as the latter can do more batching
// unless we explicitly perform flushes.
inline void Flush(SkSurface* surface) {
#if defined(SK_GANESH)
    skgpu::ganesh::Flush(surface);
#endif
#if defined(SK_GRAPHITE)
    skgpu::graphite::Flush(surface);
#endif
}

inline void FlushAndSubmit(SkSurface* surface) {
#if defined(SK_GANESH)
    skgpu::ganesh::FlushAndSubmit(surface);
#endif
#if defined(SK_GRAPHITE)
    // Graphite doesn't have a "flush and submit" equivalent
    skgpu::graphite::Flush(surface);
#endif
}
}

#endif
