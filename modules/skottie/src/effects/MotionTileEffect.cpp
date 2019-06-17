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
        SkASSERT(this->children().size() == 1ul);
        this->children()[0]->revalidate(ic, ctm);

        // tileW and tileH use layer size percentage units.
        const auto tileW = SkTPin(fTileW, 0.0f, 100.0f) * 0.01f * fLayerSize.width(),
                   tileH = SkTPin(fTileH, 0.0f, 100.0f) * 0.01f * fLayerSize.height();
        const auto tile_size = SkSize::Make(std::max(tileW, 1.0f),
                                            std::max(tileH, 1.0f));
        const auto tile  = SkRect::MakeXYWH(fTileCenter.fX - 0.5f * tile_size.width(),
                                            fTileCenter.fY - 0.5f * tile_size.height(),
                                            tile_size.width(),
                                            tile_size.height());

        fLayerShaderMatrix = SkMatrix::MakeRectToRect(SkRect::MakeWH(fLayerSize.width(),
                                                                     fLayerSize.height()),
                                                      tile, SkMatrix::kFill_ScaleToFit);

        if (fPhase) {
            // To implement AE phase semantics, we construct a mask shader for the pass-through
            // rows/columns.  We then draw the layer content through this mask, and then again
            // through the inverse mask with a phase shift.
            const auto phase_vec = fHorizontalPhase
                    ? SkVector::Make(tile.width(), 0)
                    : SkVector::Make(0, tile.height());
            const auto phase_shift = SkVector::Make(phase_vec.fX / fLayerShaderMatrix.getScaleX(),
                                                    phase_vec.fY / fLayerShaderMatrix.getScaleY())
                                     * std::fmod(fPhase * (1/360.0f), 1);
            fPhaseShaderMatrix.setTranslate(phase_shift);

            // The mask is generated using a step gradient shader, spanning 2 x tile width/height,
            // and perpendicular to the phase vector.
            static constexpr SkColor colors[] = { 0xffffffff, 0x00000000 };
            static constexpr SkScalar   pos[] = {       0.5f,       0.5f };

            const SkPoint pts[] = {{ tile.x(), tile.y() },
                                   { tile.x() + 2 * (tile.width()  - phase_vec.fX),
                                     tile.y() + 2 * (tile.height() - phase_vec.fY) }};

            fMaskShader = SkGradientShader::MakeLinear(pts, colors, pos,
                                                       SK_ARRAY_COUNT(colors),
                                                       SkTileMode::kRepeat);
        } else {
            fMaskShader = nullptr;
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

        const auto tm = fMirrorEdges ? SkTileMode::kMirror : SkTileMode::kRepeat;

        SkPictureRecorder recorder;
        SkASSERT(this->children().size() == 1ul);
        this->children()[0]->render(recorder.beginRecording(fLayerSize.width(),
                                                            fLayerSize.height()));
        const auto layer_shader =
            recorder.finishRecordingAsPicture()->makeShader(tm, tm, &fLayerShaderMatrix);

        SkPaint paint;
        paint.setAntiAlias(true);

        // First drawing pass: pass-through layer content, with an optional phase mask.
        paint.setShader(fMaskShader
                ? SkShaders::Blend(SkBlendMode::kSrcIn, fMaskShader, layer_shader)
                : layer_shader);
        canvas->drawRect(this->bounds(), paint);

        if (fMaskShader) {
            // Second pass: phased/shifted layer rows/columns, with an inverse mask.

            // TODO: would be nice for SkShaders::* to take an optional local matrix.
            paint.setShader(
                SkShaders::Blend(SkBlendMode::kSrcOut,
                                 fMaskShader,
                                 layer_shader->makeWithLocalMatrix(fPhaseShaderMatrix)));
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
    SkMatrix        fLayerShaderMatrix, // matrix for the pass-through layer content shader
                    fPhaseShaderMatrix; // matrix for the phased (shifted) rows/columns
    sk_sp<SkShader> fMaskShader;        // cached mask shader for the pass-through rows/columns

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
