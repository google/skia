/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTextAnimator_DEFINED
#define SkottieTextAnimator_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkM44.h"
#include "include/core/SkRefCnt.h"
#include "modules/skottie/src/SkottieValue.h"

#include <cstddef>
#include <vector>

namespace skjson {
class ObjectValue;
}

namespace skottie {
namespace internal {
class AnimatablePropertyContainer;
class AnimationBuilder;
class RangeSelector;

class TextAnimator final : public SkNVRefCnt<TextAnimator> {
public:
    static sk_sp<TextAnimator> Make(const skjson::ObjectValue*,
                                    const AnimationBuilder*,
                                    AnimatablePropertyContainer* acontainer);

    // Direct mapping of AE properties.
    struct AnimatedProps {
        VectorValue position,
                    scale          = { 100, 100, 100 };
        ColorValue  fill_color,
                    stroke_color;
        // unlike pos/scale which are animated vectors, rotation is separated in each dimension.
        SkV3        rotation       = { 0, 0, 0 };
        Vec2Value   blur           = { 0, 0 },
                line_spacing       = { 0, 0 };
        ScalarValue opacity        = 100,
                    fill_opacity   = 100,
                    stroke_opacity = 100,
                    tracking       = 0,
                    stroke_width   = 0;
    };

    struct ResolvedProps {
        SkV3      position = { 0, 0, 0 },
                     scale = { 1, 1, 1 },
                  rotation = { 0, 0, 0 };
        float      opacity = 1,
                  tracking = 0,
              stroke_width = 0;
        SkColor fill_color = SK_ColorTRANSPARENT,
              stroke_color = SK_ColorTRANSPARENT;
        SkV2          blur = { 0, 0 },
              line_spacing = { 0, 0 };
    };

    struct AnimatedPropsModulator {
        ResolvedProps props;     // accumulates properties across *all* animators
        float         coverage;  // accumulates range selector coverage for a given animator
    };
    using ModulatorBuffer = std::vector<AnimatedPropsModulator>;

    // Domain maps describe how a given index domain (words, lines, etc) relates
    // to the full fragment index range.
    //
    // Each domain[i] represents a [domain[i].fOffset.. domain[i].fOffset+domain[i].fCount-1]
    // fragment subset.
    struct DomainSpan {
        size_t fOffset,
               fCount;
        float  fAdvance, // cumulative advance for all fragments in span
               fAscent;  // max ascent for all fragments in span
    };
    using DomainMap = std::vector<DomainSpan>;

    struct DomainMaps {
        DomainMap fNonWhitespaceMap,
                  fWordsMap,
                  fLinesMap;
    };

    void modulateProps(const DomainMaps&, ModulatorBuffer&) const;

    bool hasBlur() const { return fHasBlur; }

    bool requiresAnchorPoint()     const { return fRequiresAnchorPoint;     }
    bool requiresLineAdjustments() const { return fRequiresLineAdjustments; }

private:
    TextAnimator(std::vector<sk_sp<RangeSelector>>&&,
                 const skjson::ObjectValue&,
                 const AnimationBuilder*,
                 AnimatablePropertyContainer*);

    ResolvedProps modulateProps(const ResolvedProps&, float amount) const;

    const std::vector<sk_sp<RangeSelector>> fSelectors;

    AnimatedProps fTextProps;
    bool          fHasFillColor            : 1,
                  fHasStrokeColor          : 1,
                  fHasFillOpacity          : 1,
                  fHasStrokeOpacity        : 1,
                  fHasOpacity              : 1,
                  fHasBlur                 : 1,
                  fRequiresAnchorPoint     : 1, // animator sensitive to transform origin?
                  fRequiresLineAdjustments : 1; // animator effects line-wide fragment adjustments
};

} // namespace internal
} // namespace skottie

#endif // SkottieTextAnimator_DEFINED
