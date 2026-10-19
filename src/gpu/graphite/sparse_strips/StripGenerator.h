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

#if defined(GPU_TEST_UTILS)
#include "include/core/SkSize.h"

#include <functional>
#endif

class SkMatrix;
class SkPath;

#if defined(GPU_TEST_UTILS)
class SparseStripsDebugSlide;
#endif

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
#if defined(GPU_TEST_UTILS)
    friend class ::SparseStripsDebugSlide;

    // Debug-only hook which observes the exact inputs of every processGeometry() call, so that
    // viewer's SparseStripsDebug slide can capture a frame for offline repro. Not thread safe;
    // install and remove it from the thread that records draws. Pass an empty function to
    // uninstall.
    using DebugCaptureFn = std::function<void(const SkPath& path,
                                              const SkMatrix& ctm,
                                              const SkIRect& clip,
                                              SkISize targetSize)>;
    static void SetDebugCaptureCallback(DebugCaptureFn fn);
    // Returns the installed hook, or an empty function if none is installed.
    static const DebugCaptureFn& GetDebugCaptureCallback();
    static DebugCaptureFn& DebugCaptureCallbackStorage();
#endif

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
