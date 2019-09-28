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

class SkFontMgr;

namespace skottie {
namespace internal {

class TextAdapter final : public DiscardableAdaptorBase {
public:
    static sk_sp<TextAdapter> Make(const skjson::ObjectValue&, const AnimationBuilder*,
                                   sk_sp<SkFontMgr>, sk_sp<Logger>);

    ~TextAdapter() override;

    const sk_sp<sksg::Group>& renderNode() const { return fRoot; }

    const TextValue& getText() const { return fText; }
    void setText(const TextValue& t) { fText = t; }

protected:
    void onSync() override;

private:
    TextAdapter(sk_sp<SkFontMgr>, sk_sp<Logger>, std::vector<sk_sp<TextAnimator>>&&);

    struct FragmentRec {
        SkPoint                       fOrigin; // fragment position

        sk_sp<sksg::Matrix<SkMatrix>> fMatrixNode;
        sk_sp<sksg::Color>            fFillColorNode,
                                      fStrokeColorNode;
    };

    void addFragment(const Shaper::Fragment&);
    void buildDomainMaps(const Shaper::Result&);

    void pushPropsToFragment(const TextAnimator::AnimatedProps&, const FragmentRec&) const;

    void adjustLineTracking(const TextAnimator::ModulatorBuffer&,
                            const TextAnimator::DomainSpan&,
                            float line_tracking) const;

    const sk_sp<sksg::Group>               fRoot;
    const sk_sp<SkFontMgr>                 fFontMgr;
    const std::vector<sk_sp<TextAnimator>> fAnimators;
    sk_sp<Logger>                          fLogger;

    std::vector<FragmentRec> fFragments;
    TextAnimator::DomainMaps fMaps;

    TextValue                fText;
};

} // namespace internal
} // namespace skottie

#endif // SkottieTextAdapter_DEFINED
