/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_SubRunData_DEFINED
#define skgpu_graphite_geom_SubRunData_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkM44.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/text/gpu/SubRunContainer.h"

#include <utility>

namespace skgpu::graphite {

class Recorder;

/**
 * SubRunData represents an AtlasSubRun subspan for which per-pixel coverage data comes from a
 * persistent glyph atlas texture.
 *
 * Like CoverageMaskShape, this is defined relative to an intermediate mask space that is specified
 * by a matrix stored on each SubRunData.
 */
class SubRunData {
public:
    SubRunData() = delete;
    SubRunData(const SubRunData& subRun) = default;
    SubRunData(SubRunData&&) = delete;

    SubRunData(const sktext::gpu::AtlasSubRun* subRun,
               sk_sp<SkRefCnt> supportDataKeepAlive,
               Rect maskBounds,
               const SkM44& maskToDevice,
               int startGlyphIndex,
               int glyphCount,
               SkColor luminanceColor,
               bool useGammaCorrectDistanceTable,
               SkPixelGeometry pixelGeometry,
               Recorder* recorder,
               sktext::gpu::RendererData rendererData)
        : fSubRun(subRun)
        , fSupportDataKeepAlive(std::move(supportDataKeepAlive))
        , fBounds(maskBounds)
        , fMaskToDevice(maskToDevice)
        , fStartGlyphIndex(startGlyphIndex)
        , fGlyphCount(glyphCount)
        , fLuminanceColor(luminanceColor)
        , fUseGammaCorrectDistanceTable(useGammaCorrectDistanceTable)
        , fPixelGeometry(pixelGeometry)
        , fRecorder(recorder)
        , fRendererData(rendererData) {}

    ~SubRunData() = default;

    // NOTE: None of the geometry types benefit from move semantics, so we don't bother
    // defining a move assignment operator for SubRunData.
    SubRunData& operator=(SubRunData&&) = delete;
    SubRunData& operator=(const SubRunData& that) = default;

    // The bounding box of the originating AtlasSubRun in mask (texture) space
    Rect bounds() const { return fBounds; }

    // The transform from the mask texture to device coordinates.
    const SkM44& maskToDevice() const { return fMaskToDevice; }

    // Access the individual elements of the subrun data.
    const sktext::gpu::AtlasSubRun* subRun() const { return fSubRun; }
    int startGlyphIndex() const { return fStartGlyphIndex; }
    int glyphCount() const { return fGlyphCount; }
    SkColor luminanceColor() const { return fLuminanceColor; }
    bool useGammaCorrectDistanceTable() const { return fUseGammaCorrectDistanceTable; }
    SkPixelGeometry pixelGeometry() const { return fPixelGeometry; }
    Recorder* recorder() const { return fRecorder; }
    const sktext::gpu::RendererData& rendererData() const { return fRendererData; }

private:
    const sktext::gpu::AtlasSubRun* fSubRun;
    // Keep the TextBlob or Slug alive until we're done with the Geometry.
    sk_sp<SkRefCnt> fSupportDataKeepAlive;

    Rect fBounds;  // bounds of the data stored in the SubRun
    SkM44 fMaskToDevice;
    int fStartGlyphIndex;
    int fGlyphCount;
    SkColor fLuminanceColor;            // only used by SDFTextRenderStep
    bool fUseGammaCorrectDistanceTable; // only used by SDFTextRenderStep
    SkPixelGeometry fPixelGeometry;     // only used by SDFTextLCDRenderStep
    Recorder* fRecorder; // this SubRun can only be associated with this Recorder's atlas
    sktext::gpu::RendererData fRendererData;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_SubRunData_DEFINED
