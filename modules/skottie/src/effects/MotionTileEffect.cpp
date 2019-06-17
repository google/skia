/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPictureRecorder.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "src/utils/SkJSON.h"

namespace skottie {
namespace internal {

namespace  {

// AE motion tile effect semantics
// (https://helpx.adobe.com/after-effects/using/stylize-effects.html#motion_tile_effect):
//
//   - the full content of the layer is mapped to a tile: tile_center, tile_width, tile_height
//
//   - tiles are repeated in both dimensions to fill the output area: output_width, output_height
//
//   - tiling mode is either kRepeat (default) or kMirror (when mirror_edges == true)
//
//   - for a non-zero phase, alternating vertical columns (every other column) are offset by
//     the specified amount
//
//   - when horizontal_phase is true, the phase is applied to horizontal rows instead of columns
//
class TileRenderNode final : public sksg::CustomRenderNode {
public:
    TileRenderNode(const SkSize& size, sk_sp<sksg::RenderNode> layer)
        : INHERITED({std::move(layer)})
        , fLayerSize(size) {}

    SG_ATTRIBUTE(TileCenter     , SkPoint , fTileCenter     )
    SG_ATTRIBUTE(TileWidth      , SkScalar, fTileW          )
    SG_ATTRIBUTE(TileHeight     , SkScalar, fTileH          )
    SG_ATTRIBUTE(OutputWidth    , SkScalar, fOutputW        )
    SG_ATTRIBUTE(OutputHeight   , SkScalar, fOutputH        )
    SG_ATTRIBUTE(Phase          , SkScalar, fPhase          )
    SG_ATTRIBUTE(MirrorEdges    , bool    , fMirrorEdges    )
    SG_ATTRIBUTE(HorizontalPhase, bool    , fHorizontalPhase)

protected:
    const RenderNode* onNodeAt(const SkPoint&) const override { return nullptr; } // no hit-testing

    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        SkASSERT(this->children().size() == 1ul);
        this->children()[0]->revalidate(ic, ctm);

        // outputW and outputH are layer size percentage units.
        const auto outputW = fOutputW * 0.01f * fLayerSize.width(),
                   outputH = fOutputH * 0.01f * fLayerSize.height();

        return SkRect::MakeXYWH((fLayerSize.width()  - outputW) * 0.5f,
                                (fLayerSize.height() - outputH) * 0.5f,
                                outputW, outputH);
    }

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
        // tileW and tileH are also layer size percentage units
        const auto tileW = SkTPin(fTileW, 0.0f, 100.0f) * 0.01f * fLayerSize.width(),
                   tileH = SkTPin(fTileH, 0.0f, 100.0f) * 0.01f * fLayerSize.height();

        // AE allow one of the tile dimensions to collapse, but not both.
        if (this->bounds().isEmpty() || (!tileW && !tileH)) {
            return;
        }

        const auto tile_size = SkSize::Make(std::max(tileW, 1.0f),
                                            std::max(tileH, 1.0f));
        const auto tile = SkRect::MakeXYWH(fTileCenter.fX - 0.5f * tile_size.width(),
                                           fTileCenter.fY - 0.5f * tile_size.height(),
                                           tile_size.width(),
                                           tile_size.height());

        SkASSERT(this->children().size() == 1ul);
        const auto& layer        = this->children()[0];
        const auto  layer_bounds = SkRect::MakeWH(fLayerSize.width(), fLayerSize.height());

        // TODO: phase

        SkPictureRecorder recorder;
        layer->render(recorder.beginRecording(layer_bounds));
        const auto layer_pic = recorder.finishRecordingAsPicture();

        const auto shader_matrix = SkMatrix::MakeRectToRect(layer_bounds, tile,
                                                            SkMatrix::kFill_ScaleToFit);

        const auto tm = fMirrorEdges ? SkTileMode::kMirror : SkTileMode::kRepeat;

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setShader(layer_pic->makeShader(tm, tm, &shader_matrix));

        canvas->drawRect(this->bounds(), paint);
    }

private:
    const SkSize fLayerSize;

    SkPoint  fTileCenter      = { 0, 0 };
    SkScalar fTileW           = 1,
             fTileH           = 1,
             fOutputW         = 1,
             fOutputH         = 1,
             fPhase           = 0;
    bool     fMirrorEdges     = false;
    bool     fHorizontalPhase = false;

    using INHERITED = sksg::CustomRenderNode;
};

} // anonymous ns

sk_sp<sksg::RenderNode> EffectBuilder::attachMotionTileEffect(const skjson::ArrayValue& jprops,
                                                              sk_sp<sksg::RenderNode> layer) const {
    enum : size_t {
        kTileCenter_Index           = 0,
        kTileWidth_Index            = 1,
        kTileHeight_Index           = 2,
        kOutputWidth_Index          = 3,
        kOutputHeight_Index         = 4,
        kMirrorEdges_Index          = 5,
        kPhase_Index                = 6,
        kHorizontalPhaseShift_Index = 7,
    };

    auto tiler = sk_make_sp<TileRenderNode>(fLayerSize, std::move(layer));

    fBuilder->bindProperty<VectorValue>(GetPropValue(jprops, kTileCenter_Index), fScope,
        [tiler](const VectorValue& tc) {
            tiler->setTileCenter(ValueTraits<VectorValue>::As<SkPoint>(tc));
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kTileWidth_Index), fScope,
        [tiler](const ScalarValue& tw) {
            tiler->setTileWidth(tw);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kTileHeight_Index), fScope,
        [tiler](const ScalarValue& th) {
            tiler->setTileHeight(th);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kOutputWidth_Index), fScope,
        [tiler](const ScalarValue& ow) {
            tiler->setOutputWidth(ow);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kOutputHeight_Index), fScope,
        [tiler](const ScalarValue& oh) {
            tiler->setOutputHeight(oh);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kMirrorEdges_Index), fScope,
        [tiler](const ScalarValue& me) {
            tiler->setMirrorEdges(SkScalarRoundToInt(me));
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kPhase_Index), fScope,
        [tiler](const ScalarValue& ph) {
            tiler->setPhase(ph);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kHorizontalPhaseShift_Index), fScope,
        [tiler](const ScalarValue& hp) {
            tiler->setHorizontalPhase(SkScalarRoundToInt(hp));
        });

    return std::move(tiler);
}

} // namespace internal
} // namespace skottie
