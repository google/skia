/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_VelloRenderer_DEFINED
#define skgpu_graphite_compute_VelloRenderer_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPath.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStrokeRec.h"
#include "src/gpu/graphite/compute/VelloComputeSteps.h"
#include "third_party/vello/cpp/vello.h"

#include <memory>

namespace skgpu::graphite {

class Caps;
class DispatchGroup;
class Recorder;
class TextureProxy;
class Transform;

// Encodes Bezier path fills, shapes, and clips. Once populated, this data structure can be used
// with the full compositing and coverage mask generating pipelines. The latter only uses the red
// color channel information.
//
// All color type parameters are expected to be unpremultiplied and in the sRGB color space.
class VelloScene final {
public:
    VelloScene();

    void reset();

    void solidFill(const SkPath&,
                   const SkColor4f&,
                   const SkPathFillType,
                   const Transform& transform);

    void solidStroke(const SkPath&,
                     const SkColor4f&,
                     const SkStrokeRec&,
                     const Transform& transform);

    void pushClipLayer(const SkPath& shape, const Transform& transform);
    void popClipLayer();

    void append(const VelloScene& other);

private:
    friend class VelloRenderer;

    // Disallow copy
    VelloScene(const VelloScene&) = delete;
    VelloScene& operator=(const VelloScene&) = delete;

    ::rust::Box<::vello_cpp::Encoding> fEncoding;
    SkDEBUGCODE(int fLayers = 0;)
};

enum class VelloAaConfig {
    kAnalyticArea,
    kMSAA16,
    kMSAA8,
};

// A VelloRenderer that is specialized for rendering coverage masks. The renderer only supports
// paths and clipping and omits the full vello imaging model (e.g. gradients and images).
//
// VelloRenderer requires `kAlpha_8_SkColorType` as the target color type on platforms that
// support the `R8Unorm` storage texture view format. Otherwise, the texture format must be
// `kRGBA_8888_SkColorType`.
class VelloRenderer final {
public:
    explicit VelloRenderer(const Caps*);
    ~VelloRenderer();

    struct RenderParams {
        // Dimensions of the fine rasterization target
        uint32_t fWidth;
        uint32_t fHeight;

        // The background color used during blending.
        SkColor4f fBaseColor;

        // The antialiasing method.
        VelloAaConfig fAaConfig;
    };

    // Run the full pipeline which supports compositing colors with different blend styles. Does
    // nothing if `scene` or target render dimensions are empty.
    //
    // The color type of `target` must be `kAlpha_8_SkColorType` on platforms that support R8Unorm
    // storage textures. Otherwise, it must be `kRGBA_8888_SkColorType`.
    std::unique_ptr<DispatchGroup> renderScene(const RenderParams&,
                                               const VelloScene&,
                                               sk_sp<TextureProxy> target,
                                               Recorder*) const;

private:
    // Pipelines
    VelloBackdropDynStep fBackdrop;
    VelloBboxClearStep fBboxClear;
    VelloBinningStep fBinning;
    VelloClipLeafStep fClipLeaf;
    VelloClipReduceStep fClipReduce;
    VelloCoarseStep fCoarse;
    VelloDrawLeafStep fDrawLeaf;
    VelloDrawReduceStep fDrawReduce;
    VelloFlattenStep fFlatten;
    VelloPathCountStep fPathCount;
    VelloPathCountSetupStep fPathCountSetup;
    VelloPathTilingStep fPathTiling;
    VelloPathTilingSetupStep fPathTilingSetup;
    VelloPathtagReduceStep fPathtagReduce;
    VelloPathtagReduce2Step fPathtagReduce2;
    VelloPathtagScan1Step fPathtagScan1;
    VelloPathtagScanLargeStep fPathtagScanLarge;
    VelloPathtagScanSmallStep fPathtagScanSmall;
    VelloTileAllocStep fTileAlloc;

    // Fine rasterization stage variants:
    std::unique_ptr<ComputeStep> fFineArea;
    std::unique_ptr<ComputeStep> fFineMsaa16;
    std::unique_ptr<ComputeStep> fFineMsaa8;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_compute_VelloRenderer_DEFINED
