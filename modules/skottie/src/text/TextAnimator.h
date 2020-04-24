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
#include "modules/sksg/include/SkSGScene.h"

#include <memory>
#include <vector>

namespace skottie {
namespace internal {

class AnimationBuilder;
class RangeSelector;

class TextAnimator final : public SkNVRefCnt<TextAnimator> {
public:
    static sk_sp<TextAnimator> Make(const skjson::ObjectValue*,
                                    const AnimationBuilder*);

    struct AnimatedProps {
        SkPoint   position = { 0, 0 };
        float      opacity = 1,
                     scale = 1,
                  rotation = 0,
                  tracking = 0;
        SkColor fill_color = SK_ColorTRANSPARENT,
              stroke_color = SK_ColorTRANSPARENT;
    };

    struct AnimatedPropsModulator {
        AnimatedProps props;     // accumulates properties across *all* animators
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
    TextAnimator(std::vector<sk_sp<RangeSelector>>&& selectors,
                 const skjson::ObjectValue& jprops,
                 const AnimationBuilder* abuilder);

    AnimatedProps modulateProps(const AnimatedProps&, float amount) const;

    const std::vector<sk_sp<RangeSelector>> fSelectors;

    AnimatedProps fTextProps;
    bool          fHasFillColor   : 1,
                  fHasStrokeColor : 1;
};

} // namespace internal
} // namespace skottie

#endif // SkottieTextAnimator_DEFINED
