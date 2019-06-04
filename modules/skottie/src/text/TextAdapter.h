/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTextAdapter_DEFINED
#define SkottieTextAdapter_DEFINED

#include "modules/skottie/src/SkottieAdapter.h"
#include "modules/skottie/src/text/SkottieShaper.h"
#include "modules/skottie/src/text/TextAnimator.h"
#include "modules/skottie/src/text/TextValue.h"

#include <vector>

namespace sksg {
class Group;
} // namespace sksg

namespace skottie {
namespace internal {

class TextAdapter final : public SkNVRefCnt<TextAdapter> {
public:
    TextAdapter(sk_sp<sksg::Group> root, bool hasAnimators);
    ~TextAdapter();

    ADAPTER_PROPERTY(Text, TextValue, TextValue())

    const sk_sp<sksg::Group>& root() const { return fRoot; }

    void applyAnimators(const std::vector<sk_sp<TextAnimator>>&);

private:
    struct FragmentRec;

    void addFragment(const skottie::Shaper::Fragment&);

    void apply();

    void pushPropsToFragment(const TextAnimator::AnimatedProps&, const FragmentRec&) const;

    sk_sp<sksg::Group>       fRoot;
    std::vector<FragmentRec> fFragments;

    const bool               fHasAnimators;
};

} // namespace internal
} // namespace skottie

#endif // SkottieTextAdapter_DEFINED
