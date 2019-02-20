/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGRenderEffect_DEFINED
#define SkSGRenderEffect_DEFINED

#include "SkSGEffectNode.h"

#include "SkBlendMode.h"
#include "SkColor.h"

#include <memory>
#include <vector>

// TODO: merge EffectNode.h with this header

class SkColorFilter;
class SkImageFilter;

namespace sksg {

class Color;

namespace internal {

// Base class for filter nodes (image filters, color filters)
template <typename T>
class Filter : public Node {
public:
    const sk_sp<T>& getFilter() const {
        SkASSERT(!this->hasInval());
        return fCachedFilter;
    }

protected:
    using InputsT = std::vector<sk_sp<Filter>>;

    explicit Filter(std::unique_ptr<InputsT> inputs)
        : INHERITED(kBubbleDamage_Trait)
        , fInputs(std::move(inputs)) {
        if (fInputs) {
            for (const auto& input : *fInputs) {
                this->observeInval(input);
            }
        }
    }

    explicit Filter(sk_sp<Filter> input = 0)
        : Filter(input ? std::unique_ptr<InputsT>(new InputsT(1, std::move(input))) : nullptr) {}

    ~Filter() override {
        if (fInputs) {
            for (const auto& input : *fInputs) {
                this->unobserveInval(input);
            }
        }
    }

    sk_sp<T> refInput(size_t i) const {
        if (!fInputs || i >= fInputs->size()) {
            return nullptr;
        }
        return (*fInputs)[i]->getFilter();
    }

    virtual sk_sp<T> onRevalidateFilter() = 0;

    SkRect onRevalidate(InvalidationController* ic, const SkMatrix& ctm) final {
        SkASSERT(this->hasInval());
        if (fInputs) {
            for (const auto& input : *fInputs) {
                input->revalidate(ic, ctm);
            }
        }

        fCachedFilter = this->onRevalidateFilter();
        return SkRect::MakeEmpty();
    }

private:
    const std::unique_ptr<InputsT> fInputs;
    sk_sp<T>                       fCachedFilter;

    using INHERITED = Node;
};

// Render node attaching a filter (DAG) to the render DAG.
template <typename T>
class FilterEffect : public EffectNode {
protected:
    FilterEffect(sk_sp<RenderNode> child, sk_sp<Filter<T>> filter)
        // filters always override descendent damage
        : INHERITED(std::move(child), kOverrideDamage_Trait)
        , fFilter(std::move(filter)) {
        SkASSERT(fFilter);
        this->observeInval(fFilter);
    }

    ~FilterEffect() override {
        this->unobserveInval(fFilter);
    }

    virtual SkRect onRevalidateFilterEffect(const SkRect& content_bounds) = 0;

    SkRect onRevalidate(InvalidationController* ic, const SkMatrix& ctm) final {
        fFilter->revalidate(ic, ctm);
        return this->onRevalidateFilterEffect(this->INHERITED::onRevalidate(ic, ctm));
    }

    const sk_sp<T>& getFilter() const { return fFilter->getFilter(); }

private:
    sk_sp<Filter<T>> fFilter;

    using INHERITED = EffectNode;
};

} // namespace internal

using ColorFilter = internal::Filter<SkColorFilter>;
using ImageFilter = internal::Filter<SkImageFilter>;

/**
 * Attaches a ColorFilter (DAG) to the render DAG.
 */

class ColorFilterEffect final : public internal::FilterEffect<SkColorFilter> {
public:
    ~ColorFilterEffect() override;

    static sk_sp<RenderNode> Make(sk_sp<RenderNode> child, sk_sp<ColorFilter> filter);

protected:
    void onRender(SkCanvas*, const RenderContext*) const override;
    const RenderNode* onNodeAt(const SkPoint&)     const override;

    SkRect onRevalidateFilterEffect(const SkRect&) override;

private:
    ColorFilterEffect(sk_sp<RenderNode>, sk_sp<ColorFilter>);

    using INHERITED = internal::FilterEffect<SkColorFilter>;
};

/**
 *
 */
class ColorModeFilter final : public ColorFilter {
public:
    ~ColorModeFilter() override;

    static sk_sp<ColorModeFilter> Make(sk_sp<Color> color, SkBlendMode mode);

    SG_ATTRIBUTE(Mode , SkBlendMode, fMode)

protected:
    sk_sp<SkColorFilter> onRevalidateFilter() override;

private:
    ColorModeFilter(sk_sp<Color>, SkBlendMode);

    sk_sp<Color> fColor;
    SkBlendMode  fMode;

    using INHERITED = ColorFilter;
};

/**
 * Attaches an ImageFilter (DAG) to the render DAG.
 */
class ImageFilterEffect final : public internal::FilterEffect<SkImageFilter> {
public:
    static sk_sp<RenderNode> Make(sk_sp<RenderNode> child, sk_sp<ImageFilter> filter);

protected:
    void onRender(SkCanvas*, const RenderContext*) const override;
    const RenderNode* onNodeAt(const SkPoint&)     const override;

    SkRect onRevalidateFilterEffect(const SkRect&) override;

private:
    ImageFilterEffect(sk_sp<RenderNode>, sk_sp<ImageFilter>);

    using INHERITED = internal::FilterEffect<SkImageFilter>;
};

/**
 * SkDropShadowImageFilter node.
 */
class DropShadowImageFilter final : public ImageFilter {
public:
    ~DropShadowImageFilter() override;

    static sk_sp<DropShadowImageFilter> Make(sk_sp<ImageFilter> input = nullptr);

    enum class Mode { kShadowAndForeground, kShadowOnly };

    SG_ATTRIBUTE(Offset, SkVector, fOffset)
    SG_ATTRIBUTE(Sigma , SkVector, fSigma )
    SG_ATTRIBUTE(Color , SkColor , fColor )
    SG_ATTRIBUTE(Mode  , Mode    , fMode  )

protected:
    sk_sp<SkImageFilter> onRevalidateFilter() override;

private:
    explicit DropShadowImageFilter(sk_sp<ImageFilter> input);

    SkVector             fOffset = { 0, 0 },
                         fSigma  = { 0, 0 };
    SkColor              fColor  = SK_ColorBLACK;
    Mode                 fMode   = Mode::kShadowAndForeground;

    using INHERITED = internal::Filter<SkImageFilter>;
};

} // namespace sksg

#endif // SkSGRenderEffect_DEFINED
