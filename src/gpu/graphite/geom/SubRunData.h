/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_SubRunData_DEFINED
#define skgpu_graphite_geom_SubRunData_DEFINED

#include "src/gpu/graphite/geom/Rect.h"

namespace sktext::gpu { class AtlasSubRun; }

namespace skgpu::graphite {

class Recorder;

/**
 * SubRunData contains all the data we need to render AtlasSubRuns
 */
class SubRunData {
public:
    SubRunData() = delete;
    SubRunData(const SubRunData& subRun) = default;
    SubRunData(SubRunData&&) = delete;

    SubRunData(const sktext::gpu::AtlasSubRun* subRun,
               sk_sp<SkRefCnt> supportDataKeepAlive,
               Rect deviceBounds,
               int startGlyphIndex,
               int glyphCount,
               Recorder* recorder)
        : fSubRun(subRun)
        , fSupportDataKeepAlive(std::move(supportDataKeepAlive))
        , fBounds(deviceBounds)
        , fStartGlyphIndex(startGlyphIndex)
        , fGlyphCount(glyphCount)
        , fRecorder(recorder) {}

    ~SubRunData() = default;

    // NOTE: None of the geometry types benefit from move semantics, so we don't bother
    // defining a move assignment operator for SubRunData.
    SubRunData& operator=(SubRunData&&) = delete;
    SubRunData& operator=(const SubRunData& that) = default;

    // The bounding box of the subrun data.
    Rect bounds() const { return fBounds; }

    // Access the individual elements of the subrun data.
    const sktext::gpu::AtlasSubRun* subRun() const { return fSubRun; }
    int startGlyphIndex() const { return fStartGlyphIndex; }
    int glyphCount() const { return fGlyphCount; }
    Recorder* recorder() const { return fRecorder; }

private:
    const sktext::gpu::AtlasSubRun* fSubRun;
    // Keep the TextBlob or Slug alive until we're done with the Geometry.
    sk_sp<SkRefCnt> fSupportDataKeepAlive;

    Rect fBounds;  // bounds of the data stored in the SubRun
    int fStartGlyphIndex;
    int fGlyphCount;
    Recorder* fRecorder; // this SubRun can only be associated with this Recorder's atlas
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_SubRunData_DEFINED
