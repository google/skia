/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTextAdapter_DEFINED
#define SkottieTextAdapter_DEFINED

#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/text/SkottieShaper.h"
#include "modules/skottie/src/text/TextAnimator.h"
#include "modules/skottie/src/text/TextValue.h"

#include <vector>

class SkFontMgr;

namespace sksg {
class BlurImageFilter;
class Group;
template <typename T>
class Matrix;
} // namespace sksg

namespace skottie {
namespace internal {

class TextAdapter final : public AnimatablePropertyContainer {
public:
    static sk_sp<TextAdapter> Make(const skjson::ObjectValue&, const AnimationBuilder*,
                                   sk_sp<SkFontMgr>, sk_sp<Logger>);

    ~TextAdapter() override;

    const sk_sp<sksg::Group>& node() const { return fRoot; }

    const TextValue& getText() const { return fText.fCurrentValue; }
    void setText(const TextValue&);

protected:
    void onSync() override;

private:
    enum class AnchorPointGrouping : uint8_t {
        kCharacter,
        kWord,
        kLine,
        kAll,
    };

    TextAdapter(sk_sp<SkFontMgr>, sk_sp<Logger>, AnchorPointGrouping);

    struct FragmentRec {
        SkPoint                      fOrigin; // fragment position

        sk_sp<sksg::Matrix<SkM44>>   fMatrixNode;
        sk_sp<sksg::Color>           fFillColorNode,
                                     fStrokeColorNode;
        sk_sp<sksg::BlurImageFilter> fBlur;

        float                        fAdvance, // used for transform anchor point calculations
                                     fAscent;  // ^
    };

    void reshape();
    void addFragment(const Shaper::Fragment&);
    void buildDomainMaps(const Shaper::Result&);

    void pushPropsToFragment(const TextAnimator::ResolvedProps&, const FragmentRec&,
                             const SkV2&, const TextAnimator::DomainSpan*) const;

    void adjustLineProps(const TextAnimator::ModulatorBuffer&,
                         const TextAnimator::DomainSpan&,
                         const SkV2& line_offset,
                         float line_tracking) const;

    SkV2 fragmentAnchorPoint(const FragmentRec&, const SkV2&,
                             const TextAnimator::DomainSpan*) const;
    uint32_t shaperFlags() const;

    SkM44 fragmentMatrix(const TextAnimator::ResolvedProps&, const FragmentRec&, const SkV2&) const;

    const sk_sp<sksg::Group>         fRoot;
    const sk_sp<SkFontMgr>           fFontMgr;
    sk_sp<Logger>                    fLogger;
    const AnchorPointGrouping        fAnchorPointGrouping;

    std::vector<sk_sp<TextAnimator>> fAnimators;
    std::vector<FragmentRec>         fFragments;
    TextAnimator::DomainMaps         fMaps;

    // Helps detect external value changes.
    struct TextValueTracker {
        TextValue fCurrentValue;

        bool hasChanged() const {
            if (fCurrentValue != fPrevValue) {
                fPrevValue = fCurrentValue;
                return true;
            }
            return false;
        }

        const TextValue* operator->() const { return &fCurrentValue; }

    private:
        mutable TextValue fPrevValue;
    };

    TextValueTracker          fText;
    Vec2Value                 fGroupingAlignment = {0,0};

    // Optional text path.
    struct PathInfo;
    std::unique_ptr<PathInfo> fPathInfo;

    bool                      fHasBlurAnimator     : 1,
                              fRequiresAnchorPoint : 1;
};

} // namespace internal
} // namespace skottie

#endif // SkottieTextAdapter_DEFINED
