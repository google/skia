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
#include "modules/skottie/src/text/TextValue.h"

#include <vector>

namespace sksg {
class Group;
} // namespace sksg

namespace skottie {

class TextAdapter final : public SkNVRefCnt<TextAdapter> {
public:
    TextAdapter(sk_sp<sksg::Group> root, bool hasAnimators);
    ~TextAdapter();

    ADAPTER_PROPERTY(Text, TextValue, TextValue())

    const sk_sp<sksg::Group>& root() const { return fRoot; }

    struct AnimatedProps {
        SkPoint   position = { 0, 0 };
        SkColor fill_color = SK_ColorTRANSPARENT,
              stroke_color = SK_ColorTRANSPARENT;
    };

    void applyAnimatedProps(const AnimatedProps&);

private:
    struct FragmentRec;

    void addFragment(const skottie::Shaper::Fragment&);

    void apply();

    sk_sp<sksg::Group>       fRoot;
    std::vector<FragmentRec> fFragments;

    const bool               fHasAnimators;
};

} // namespace skottie

#endif // SkottieTextAdapter_DEFINED
