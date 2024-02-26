/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/effects/Effects.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "modules/sksg/include/SkSGTransform.h"

#include <array>
#include <cstddef>
#include <utility>

namespace skjson {
class ArrayValue;
}

namespace skottie::internal {

namespace  {

class CornerPinAdapter final : public AnimatablePropertyContainer {
public:
    static sk_sp<CornerPinAdapter> Make(const skjson::ArrayValue& jprops,
                                        const AnimationBuilder& abuilder,
                                        const SkSize& layer_size) {
        return sk_sp<CornerPinAdapter>(new CornerPinAdapter(jprops, abuilder, layer_size));
    }

    auto& node() const { return fMatrixNode; }

private:
    CornerPinAdapter(const skjson::ArrayValue& jprops,
                     const AnimationBuilder& abuilder,
                     const SkSize& layer_size)
        : fMatrixNode(sksg::Matrix<SkMatrix>::Make(SkMatrix::I()))
        , fLayerSize(layer_size) {
        enum : size_t {
             kUpperLeft_Index = 0,
            kUpperRight_Index = 1,
             kLowerLeft_Index = 2,
            kLowerRight_Index = 3,
        };

        EffectBinder(jprops, abuilder, this)
            .bind( kUpperLeft_Index, fUL)
            .bind(kUpperRight_Index, fUR)
            .bind( kLowerLeft_Index, fLL)
            .bind(kLowerRight_Index, fLR);
    }

    void onSync() override {
        const SkPoint src[] = {{                 0,                   0},
                               {fLayerSize.width(),                   0},
                               {fLayerSize.width(), fLayerSize.height()},
                               {                 0, fLayerSize.height()}},

                      dst[] = {{ fUL.x, fUL.y},
                               { fUR.x, fUR.y},
                               { fLR.x, fLR.y},
                               { fLL.x, fLL.y}};
        static_assert(std::size(src) == std::size(dst));

        SkMatrix m;
        if (m.setPolyToPoly(src, dst, std::size(src))) {
            fMatrixNode->setMatrix(m);
        }
    }

    const sk_sp<sksg::Matrix<SkMatrix>> fMatrixNode;
    const SkSize                        fLayerSize;

    Vec2Value fUL,
              fLL,
              fUR,
              fLR;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachCornerPinEffect(const skjson::ArrayValue& jprops,
                                                             sk_sp<sksg::RenderNode> layer) const {
    sk_sp<sksg::Matrix<SkMatrix>> matrix_node =
            fBuilder->attachDiscardableAdapter<CornerPinAdapter>(jprops, *fBuilder, fLayerSize);

    return sksg::TransformEffect::Make(std::move(layer), std::move(matrix_node));
}

} // namespace skottie::internal
