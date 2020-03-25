/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkShader.h"
#include "include/effects/SkGradientShader.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "src/utils/SkJSON.h"

#include <cmath>

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
        // Re-record the layer picture if needed.
        if (!fLayerPicture || this->hasChildrenInval()) {
            SkASSERT(this->children().size() == 1ul);
            const auto& layer = this->children()[0];

            layer->revalidate(ic, ctm);

            SkPictureRecorder recorder;
            layer->render(recorder.beginRecording(fLayerSize.width(), fLayerSize.height()));
            fLayerPicture = recorder.finishRecordingAsPicture();
        }

        // tileW and tileH use layer size percentage units.
        const auto tileW = SkTPin(fTileW, 0.0f, 100.0f) * 0.01f * fLayerSize.width(),
                   tileH = SkTPin(fTileH, 0.0f, 100.0f) * 0.01f * fLayerSize.height();
        const auto tile_size = SkSize::Make(std::max(tileW, 1.0f),
                                            std::max(tileH, 1.0f));
        const auto tile  = SkRect::MakeXYWH(fTileCenter.fX - 0.5f * tile_size.width(),
                                            fTileCenter.fY - 0.5f * tile_size.height(),
                                            tile_size.width(),
                                            tile_size.height());

        const auto layerShaderMatrix = SkMatrix::MakeRectToRect(
                    SkRect::MakeWH(fLayerSize.width(), fLayerSize.height()),
                    tile, SkMatrix::kFill_ScaleToFit);

        const auto tm = fMirrorEdges ? SkTileMode::kMirror : SkTileMode::kRepeat;
        auto layer_shader = fLayerPicture->makeShader(tm, tm, &layerShaderMatrix);

        if (fPhase) {
            // To implement AE phase semantics, we construct a mask shader for the pass-through
            // rows/columns.  We then draw the layer content through this mask, and then again
            // through the inverse mask with a phase shift.
            const auto phase_vec = fHorizontalPhase
                    ? SkVector::Make(tile.width(), 0)
                    : SkVector::Make(0, tile.height());
            const auto phase_shift = SkVector::Make(phase_vec.fX / layerShaderMatrix.getScaleX(),
                                                    phase_vec.fY / layerShaderMatrix.getScaleY())
                                     * std::fmod(fPhase * (1/360.0f), 1);
            const auto phase_shader_matrix = SkMatrix::MakeTrans(phase_shift.x(), phase_shift.y());

            // The mask is generated using a step gradient shader, spanning 2 x tile width/height,
            // and perpendicular to the phase vector.
            static constexpr SkColor colors[] = { 0xffffffff, 0x00000000 };
            static constexpr SkScalar   pos[] = {       0.5f,       0.5f };

            const SkPoint pts[] = {{ tile.x(), tile.y() },
                                   { tile.x() + 2 * (tile.width()  - phase_vec.fX),
                                     tile.y() + 2 * (tile.height() - phase_vec.fY) }};

            auto mask_shader = SkGradientShader::MakeLinear(pts, colors, pos,
                                                            SK_ARRAY_COUNT(colors),
                                                            SkTileMode::kRepeat);

            // First drawing pass: in-place masked layer content.
            fMainPassShader  = SkShaders::Blend(SkBlendMode::kSrcIn , mask_shader, layer_shader);
            // Second pass: phased-shifted layer content, with an inverse mask.
            fPhasePassShader = SkShaders::Blend(SkBlendMode::kSrcOut, mask_shader, layer_shader)
                               ->makeWithLocalMatrix(phase_shader_matrix);
        } else {
            fMainPassShader  = std::move(layer_shader);
            fPhasePassShader = nullptr;
        }

        // outputW and outputH also use layer size percentage units.
        const auto outputW = fOutputW * 0.01f * fLayerSize.width(),
                   outputH = fOutputH * 0.01f * fLayerSize.height();

        return SkRect::MakeXYWH((fLayerSize.width()  - outputW) * 0.5f,
                                (fLayerSize.height() - outputH) * 0.5f,
                                outputW, outputH);
    }

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
        // AE allow one of the tile dimensions to collapse, but not both.
        if (this->bounds().isEmpty() || (fTileW <= 0 && fTileH <= 0)) {
            return;
        }

        SkPaint paint;
        paint.setAntiAlias(true);

        paint.setShader(fMainPassShader);
        canvas->drawRect(this->bounds(), paint);

        if (fPhasePassShader) {
            paint.setShader(fPhasePassShader);
            canvas->drawRect(this->bounds(), paint);
        }
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

    // These are computed/cached on revalidation.
    sk_sp<SkPicture> fLayerPicture;      // cached picture for layer content
    sk_sp<SkShader>  fMainPassShader,    // shader for the main tile(s)
                     fPhasePassShader;   // shader for the phased tile(s)

    using INHERITED = sksg::CustomRenderNode;
};

class MotionTileAdapter final : public DiscardableAdapterBase<MotionTileAdapter, TileRenderNode> {
public:
    MotionTileAdapter(const skjson::ArrayValue& jprops,
                      sk_sp<sksg::RenderNode> layer,
                      const AnimationBuilder& abuilder,
                      const SkSize& layer_size)
        : INHERITED(sk_make_sp<TileRenderNode>(layer_size, std::move(layer))) {

        enum : size_t {
                      kTileCenter_Index = 0,
                       kTileWidth_Index = 1,
                      kTileHeight_Index = 2,
                     kOutputWidth_Index = 3,
                    kOutputHeight_Index = 4,
                     kMirrorEdges_Index = 5,
                           kPhase_Index = 6,
            kHorizontalPhaseShift_Index = 7,
        };

        EffectBinder(jprops, abuilder, this)
            .bind(          kTileCenter_Index, fTileCenter     )
            .bind(           kTileWidth_Index, fTileW          )
            .bind(          kTileHeight_Index, fTileH          )
            .bind(         kOutputWidth_Index, fOutputW        )
            .bind(        kOutputHeight_Index, fOutputH        )
            .bind(         kMirrorEdges_Index, fMirrorEdges    )
            .bind(               kPhase_Index, fPhase          )
            .bind(kHorizontalPhaseShift_Index, fHorizontalPhase);
    }

private:
    void onSync() override {
        const auto& tiler = this->node();

        tiler->setTileCenter({fTileCenter.x, fTileCenter.y});
        tiler->setTileWidth (fTileW);
        tiler->setTileHeight(fTileH);
        tiler->setOutputWidth (fOutputW);
        tiler->setOutputHeight(fOutputH);
        tiler->setPhase(fPhase);
        tiler->setMirrorEdges(SkToBool(fMirrorEdges));
        tiler->setHorizontalPhase(SkToBool(fHorizontalPhase));
    }

    Vec2Value   fTileCenter      = {0,0};
    ScalarValue fTileW           = 1,
                fTileH           = 1,
                fOutputW         = 1,
                fOutputH         = 1,
                fMirrorEdges     = 0,
                fPhase           = 0,
                fHorizontalPhase = 0;

    using INHERITED = DiscardableAdapterBase<MotionTileAdapter, TileRenderNode>;
};

} // anonymous ns

sk_sp<sksg::RenderNode> EffectBuilder::attachMotionTileEffect(const skjson::ArrayValue& jprops,
                                                              sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<MotionTileAdapter>(jprops,
                                                                 std::move(layer),
                                                                 *fBuilder,
                                                                 fLayerSize);
}

} // namespace internal
} // namespace skottie
