/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTextAnimator_DEFINED
#define SkottieTextAnimator_DEFINED

#include "include/core/SkRefCnt.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGScene.h"

#include <memory>
#include <vector>

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
                    fill_color,
                    stroke_color;
        ScalarValue opacity  = 100,
                    scale    = 100,
                    rotation = 0,
                    tracking = 0;
    };

    struct ResolvedProps {
        SkPoint   position = { 0, 0 };
        float      opacity = 1,
                     scale = 1,
                  rotation = 0,
                  tracking = 0;
        SkColor fill_color = SK_ColorTRANSPARENT,
              stroke_color = SK_ColorTRANSPARENT;
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
        size_t fOffset, fCount;
    };
    using DomainMap = std::vector<DomainSpan>;

    struct DomainMaps {
        DomainMap fNonWhitespaceMap,
                  fWordsMap,
                  fLinesMap;
    };

    void modulateProps(const DomainMaps&, ModulatorBuffer&) const;

private:
    TextAnimator(std::vector<sk_sp<RangeSelector>>&&,
                 const skjson::ObjectValue&,
                 const AnimationBuilder*,
                 AnimatablePropertyContainer*);

    ResolvedProps modulateProps(const ResolvedProps&, float amount) const;

    const std::vector<sk_sp<RangeSelector>> fSelectors;

    AnimatedProps fTextProps;
    bool          fHasFillColor   : 1,
                  fHasStrokeColor : 1;
};

} // namespace internal
} // namespace skottie

#endif // SkottieTextAnimator_DEFINED
