/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_StripGenerator_DEFINED
#define skgpu_graphite_sparse_strips_StripGenerator_DEFINED

#include "include/core/SkRect.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

#include <cstdint>

class SkMatrix;
class SkPath;

namespace skgpu::graphite {

class Recorder;

class StripGenerator {
public:
    StripGenerator(int width,
                   int height,
                   const SparseStripConfig::LUTArray& maskLUT,
                   Recorder* recorder);
    ~StripGenerator();

    bool processGeometry(const SkPath& path, const SkMatrix& ctm);
    bool processGeometry(const SkPath& path, const SkMatrix& ctm, const SkIRect& clip);

    const EndCaps& ends() const { return fEnds; }
    const WideTiles& wides() const { return fWides; }

    bool hasNullCaps() const { return fEnds.hasNullCaps(); }
    bool cleanupNullCaps();

private:
    int fWidth;
    int fHeight;
    const SparseStripConfig::LUTArray& fMaskLUT;
    Recorder* fRecorder;
    EndCaps fEnds;
    WideTiles fWides;
    WindingHistogram fWindingHistogram;
    bool fCulled;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_StripGenerator_DEFINED
