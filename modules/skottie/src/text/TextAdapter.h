/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTextAdapter_DEFINED
#define SkottieTextAdapter_DEFINED

#include "modules/skottie/include/TextShaper.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/text/Font.h"
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
    static sk_sp<TextAdapter> Make(const skjson::ObjectValue&,
                                   const AnimationBuilder*,
                                   sk_sp<SkFontMgr>,
                                   sk_sp<CustomFont::GlyphCompMapper>,
                                   sk_sp<Logger>);

    ~TextAdapter() override;

    const sk_sp<sksg::Group>& node() const { return fRoot; }

    const TextValue& getText() const { return fText.fCurrentValue; }
    void setText(const TextValue&);

protected:
    void onSync() override;

private:
    class GlyphDecoratorNode;

    enum class AnchorPointGrouping : uint8_t {
        kCharacter,
        kWord,
        kLine,
        kAll,
    };

    TextAdapter(sk_sp<SkFontMgr>,
                sk_sp<CustomFont::GlyphCompMapper>,
                sk_sp<Logger>,
                AnchorPointGrouping);

    struct FragmentRec {
        SkPoint                      fOrigin; // fragment position

        const Shaper::ShapedGlyphs*  fGlyphs = nullptr;
        sk_sp<sksg::Matrix<SkM44>>   fMatrixNode;
        sk_sp<sksg::Color>           fFillColorNode,
                                     fStrokeColorNode;
        sk_sp<sksg::BlurImageFilter> fBlur;

        float                        fAdvance, // used for transform anchor point calculations
                                     fAscent;  // ^
    };

    void reshape();
    void addFragment(Shaper::Fragment&, sksg::Group* container);
    void buildDomainMaps(const Shaper::Result&);
    std::vector<sk_sp<sksg::RenderNode>> buildGlyphCompNodes(Shaper::ShapedGlyphs&) const;

    void pushPropsToFragment(const TextAnimator::ResolvedProps&, const FragmentRec&,
                             const SkV2& frag_offset, const SkV2& grouping_alignment,
                             const TextAnimator::DomainSpan*) const;

    SkV2 fragmentAnchorPoint(const FragmentRec&, const SkV2&,
                             const TextAnimator::DomainSpan*) const;
    uint32_t shaperFlags() const;

    SkM44 fragmentMatrix(const TextAnimator::ResolvedProps&, const FragmentRec&, const SkV2&) const;

    const sk_sp<sksg::Group>                 fRoot;
    const sk_sp<SkFontMgr>                   fFontMgr;
    const sk_sp<CustomFont::GlyphCompMapper> fCustomGlyphMapper;
    sk_sp<Logger>                            fLogger;
    const AnchorPointGrouping                fAnchorPointGrouping;

    std::vector<sk_sp<TextAnimator>>         fAnimators;
    std::vector<FragmentRec>                 fFragments;
    TextAnimator::DomainMaps                 fMaps;

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
    float                     fTextShapingScale  = 1;     // size adjustment from auto-scaling

    // Optional text path.
    struct PathInfo;
    std::unique_ptr<PathInfo> fPathInfo;

    bool                      fHasBlurAnimator         : 1,
                              fRequiresAnchorPoint     : 1,
                              fRequiresLineAdjustments : 1;
};

} // namespace internal
} // namespace skottie

#endif // SkottieTextAdapter_DEFINED
