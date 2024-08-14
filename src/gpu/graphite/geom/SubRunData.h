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
 * The bounds() represent the bounds of the entire AtlasSubRun and does not directly map to the
 * local coordinates of this particular subspan. Rather, the dimensions and offset coordinates of a
 * subspan are defined in a coordinate space that is partially transformed by a decomposition of
 * the local-to-device matrix computed by the AtlasSubRun per instance. The transform of the draw is
 * the rest of the decomposed transform (often only a translation) that maps this intermediate space
 * to the device-space coordinates of the draw.
 *
 * The local coordinates used in shading are derived by transforming the final device coordinates
 * using the inverse of the local-to-device matrix.
 */
class SubRunData {
public:
    SubRunData() = delete;
    SubRunData(const SubRunData& subRun) = default;
    SubRunData(SubRunData&&) = delete;

    SubRunData(const sktext::gpu::AtlasSubRun* subRun,
               sk_sp<SkRefCnt> supportDataKeepAlive,
               Rect deviceBounds,
               const SkM44& deviceToLocal,
               int startGlyphIndex,
               int glyphCount,
               SkColor luminanceColor,
               bool useGammaCorrectDistanceTable,
               SkPixelGeometry pixelGeometry,
               Recorder* recorder,
               sktext::gpu::RendererData rendererData)
        : fSubRun(subRun)
        , fSupportDataKeepAlive(std::move(supportDataKeepAlive))
        , fBounds(deviceBounds)
        , fDeviceToLocal(deviceToLocal)
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

    // The bounding box of the originating AtlasSubRun.
    Rect bounds() const { return fBounds; }

    // The inverse local-to-device matrix.
    const SkM44& deviceToLocal() const { return fDeviceToLocal; }

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
    SkM44 fDeviceToLocal;
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
