/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieProperty_DEFINED
#define SkottieProperty_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypeface.h"
#include "include/utils/SkTextUtils.h"
#include "modules/skottie/include/TextShaper.h"

#include <functional>
#include <vector>

class SkCanvas;

namespace sksg {

class Color;
class OpacityEffect;

} // namespace sksg

namespace skottie {

using ColorPropertyValue   = SkColor;
using OpacityPropertyValue = float;

enum class TextPaintOrder : uint8_t {
    kFillStroke,
    kStrokeFill,
};

// Optional callback invoked when drawing text layers.
// Allows clients to render custom text decorations.
class GlyphDecorator : public SkRefCnt {
public:
    struct GlyphInfo {
        SkRect   fBounds;  // visual glyph bounds
        SkMatrix fMatrix;  // glyph matrix
        size_t   fCluster; // cluster index in the original text string
        float    fAdvance; // horizontal glyph advance
    };

    struct TextInfo {
        SkSpan<const GlyphInfo> fGlyphs;
        float                   fScale;  // Additional font scale applied by auto-sizing.
    };

    virtual void onDecorate(SkCanvas*, const TextInfo&) = 0;
};

struct TextPropertyValue {
    sk_sp<SkTypeface>       fTypeface;
    SkString                fText;
    float                   fTextSize       = 0,
                            fMinTextSize    = 0,                                 // when auto-sizing
                            fMaxTextSize    = std::numeric_limits<float>::max(), // when auto-sizing
                            fStrokeWidth    = 0,
                            fLineHeight     = 0,
                            fLineShift      = 0,
                            fAscent         = 0;
    size_t                  fMaxLines       = 0;                                 // when auto-sizing
    SkTextUtils::Align      fHAlign         = SkTextUtils::kLeft_Align;
    Shaper::VAlign          fVAlign         = Shaper::VAlign::kTop;
    Shaper::ResizePolicy    fResize         = Shaper::ResizePolicy::kNone;
    Shaper::LinebreakPolicy fLineBreak      = Shaper::LinebreakPolicy::kExplicit;
    Shaper::Direction       fDirection      = Shaper::Direction::kLTR;
    Shaper::Capitalization  fCapitalization = Shaper::Capitalization::kNone;
    SkRect                  fBox            = SkRect::MakeEmpty();
    SkColor                 fFillColor      = SK_ColorTRANSPARENT,
                            fStrokeColor    = SK_ColorTRANSPARENT;
    TextPaintOrder          fPaintOrder     = TextPaintOrder::kFillStroke;
    SkPaint::Join           fStrokeJoin     = SkPaint::Join::kMiter_Join;
    bool                    fHasFill        = false,
                            fHasStroke      = false;
    sk_sp<GlyphDecorator>   fDecorator;
                            // The locale to be used for text shaping, in BCP47 form.  This includes
                            // support for RFC6067 extensions, so one can e.g. select strict line
                            // breaking rules for certain scripts: ja-u-lb-strict.
                            // Pass an empty string to use the system locale.
    SkString                fLocale;

    bool operator==(const TextPropertyValue& other) const;
    bool operator!=(const TextPropertyValue& other) const;
};

struct TransformPropertyValue {
    SkPoint  fAnchorPoint,
             fPosition;
    SkVector fScale;
    SkScalar fRotation,
             fSkew,
             fSkewAxis;

    bool operator==(const TransformPropertyValue& other) const;
    bool operator!=(const TransformPropertyValue& other) const;
};

namespace internal { class SceneGraphRevalidator; }

/**
 * Property handles are adapters between user-facing AE model/values
 * and the internal scene-graph representation.
 */
template <typename ValueT, typename NodeT>
class SK_API PropertyHandle final {
public:
    explicit PropertyHandle(sk_sp<NodeT>);
    PropertyHandle(sk_sp<NodeT> node, sk_sp<internal::SceneGraphRevalidator> revalidator)
        : fNode(std::move(node))
        , fRevalidator(std::move(revalidator)) {}
    ~PropertyHandle();

    PropertyHandle(const PropertyHandle&);

    ValueT get() const;
    void set(const ValueT&);

private:
    const sk_sp<NodeT>                           fNode;
    const sk_sp<internal::SceneGraphRevalidator> fRevalidator;
};

namespace internal {

class TextAdapter;
class TransformAdapter2D;

} // namespace internal

using ColorPropertyHandle     = PropertyHandle<ColorPropertyValue,
                                               sksg::Color>;
using OpacityPropertyHandle   = PropertyHandle<OpacityPropertyValue,
                                               sksg::OpacityEffect>;
using TextPropertyHandle      = PropertyHandle<TextPropertyValue,
                                               internal::TextAdapter>;
using TransformPropertyHandle = PropertyHandle<TransformPropertyValue,
                                               internal::TransformAdapter2D>;

/**
 * A PropertyObserver can be used to track and manipulate certain properties of "interesting"
 * Lottie nodes.
 *
 * When registered with an animation builder, PropertyObserver receives notifications for
 * various properties of layer and shape nodes.  The |node_name| argument corresponds to the
 * name ("nm") node property.
 */
class SK_API PropertyObserver : public SkRefCnt {
public:
    enum class NodeType {COMPOSITION, LAYER, EFFECT, OTHER};

    template <typename T>
    using LazyHandle = std::function<std::unique_ptr<T>()>;

    virtual void onColorProperty    (const char node_name[],
                                     const LazyHandle<ColorPropertyHandle>&);
    virtual void onOpacityProperty  (const char node_name[],
                                     const LazyHandle<OpacityPropertyHandle>&);
    virtual void onTextProperty     (const char node_name[],
                                     const LazyHandle<TextPropertyHandle>&);
    virtual void onTransformProperty(const char node_name[],
                                     const LazyHandle<TransformPropertyHandle>&);
    virtual void onEnterNode(const char node_name[], NodeType node_type);
    virtual void onLeavingNode(const char node_name[], NodeType node_type);
};

} // namespace skottie

#endif // SkottieProperty_DEFINED
