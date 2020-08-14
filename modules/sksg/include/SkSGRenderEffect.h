/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGRenderEffect_DEFINED
#define SkSGRenderEffect_DEFINED

#include "modules/sksg/include/SkSGEffectNode.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/effects/SkImageFilters.h"

#include <memory>
#include <vector>

// TODO: merge EffectNode.h with this header

class SkImageFilter;
class SkMaskFilter;
class SkShader;

namespace sksg {

/**
 * Shader base class.
 */
class Shader : public Node {
public:
    ~Shader() override;

    const sk_sp<SkShader>& getShader() const {
        SkASSERT(!this->hasInval());
        return fShader;
    }

protected:
    Shader();

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) final;

    virtual sk_sp<SkShader> onRevalidateShader() = 0;

private:
    sk_sp<SkShader> fShader;

    using INHERITED = Node;
};

/**
 * Attaches a shader to the render DAG.
 */
class ShaderEffect final : public EffectNode {
public:
    ~ShaderEffect() override;

    static sk_sp<ShaderEffect> Make(sk_sp<RenderNode> child, sk_sp<Shader> shader = nullptr);

    void setShader(sk_sp<Shader>);

protected:
    void onRender(SkCanvas*, const RenderContext*) const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    ShaderEffect(sk_sp<RenderNode> child, sk_sp<Shader> shader);

    sk_sp<Shader> fShader;

    using INHERITED = EffectNode;
};

/**
 * Attaches a mask shader to the render DAG.
 */
class MaskShaderEffect final : public EffectNode {
public:
    static sk_sp<MaskShaderEffect> Make(sk_sp<RenderNode>, sk_sp<SkShader> = nullptr);

    SG_ATTRIBUTE(Shader, sk_sp<SkShader>, fShader)

protected:
    void onRender(SkCanvas*, const RenderContext*) const override;

private:
    MaskShaderEffect(sk_sp<RenderNode>, sk_sp<SkShader>);

    sk_sp<SkShader> fShader;

    using INHERITED = EffectNode;
};

/**
 * ImageFilter base class.
 */
class ImageFilter : public Node {
public:
    ~ImageFilter() override;

    const sk_sp<SkImageFilter>& getFilter() const {
        SkASSERT(!this->hasInval());
        return fFilter;
    }

protected:
    explicit ImageFilter(sk_sp<ImageFilter> input = nullptr);

    using InputsT = std::vector<sk_sp<ImageFilter>>;
    explicit ImageFilter(std::unique_ptr<InputsT> inputs);

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) final;

    virtual sk_sp<SkImageFilter> onRevalidateFilter() = 0;

    sk_sp<SkImageFilter> refInput(size_t) const;

private:
    const std::unique_ptr<InputsT> fInputs;

    sk_sp<SkImageFilter>           fFilter;

    using INHERITED = Node;
};

/**
 * Attaches an ImageFilter (chain) to the render DAG.
 */
class ImageFilterEffect final : public EffectNode {
public:
    ~ImageFilterEffect() override;

    static sk_sp<RenderNode> Make(sk_sp<RenderNode> child, sk_sp<ImageFilter> filter);

protected:
    void onRender(SkCanvas*, const RenderContext*) const override;
    const RenderNode* onNodeAt(const SkPoint&)     const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    ImageFilterEffect(sk_sp<RenderNode> child, sk_sp<ImageFilter> filter);

    sk_sp<ImageFilter> fImageFilter;

    using INHERITED = EffectNode;
};

/**
 * Wrapper for externally-managed SkImageFilters.
 */
class ExternalImageFilter final : public ImageFilter {
public:
    ~ExternalImageFilter() override;

    static sk_sp<ExternalImageFilter> Make() {
        return sk_sp<ExternalImageFilter>(new ExternalImageFilter());
    }

    SG_ATTRIBUTE(ImageFilter, sk_sp<SkImageFilter>, fImageFilter)

private:
    ExternalImageFilter();

    sk_sp<SkImageFilter> onRevalidateFilter() override { return fImageFilter; }

    sk_sp<SkImageFilter> fImageFilter;
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

    using INHERITED = ImageFilter;
};

/**
 * SkBlurImageFilter node.
 */
class BlurImageFilter final : public ImageFilter {
public:
    ~BlurImageFilter() override;

    static sk_sp<BlurImageFilter> Make(sk_sp<ImageFilter> input = nullptr);

    SG_ATTRIBUTE(Sigma   , SkVector  , fSigma   )
    SG_ATTRIBUTE(TileMode, SkTileMode, fTileMode)

protected:
    sk_sp<SkImageFilter> onRevalidateFilter() override;

private:
    explicit BlurImageFilter(sk_sp<ImageFilter> input);

    SkVector   fSigma    = { 0, 0 };
    SkTileMode fTileMode = SkTileMode::kClamp;

    using INHERITED = ImageFilter;
};

/**
 * Applies a SkBlendMode to descendant render nodes.
 */
class BlendModeEffect final : public EffectNode {
public:
    ~BlendModeEffect() override;

    static sk_sp<BlendModeEffect> Make(sk_sp<RenderNode> child,
                                       SkBlendMode = SkBlendMode::kSrcOver);

    SG_ATTRIBUTE(Mode, SkBlendMode, fMode)

protected:
    void onRender(SkCanvas*, const RenderContext*) const override;
    const RenderNode* onNodeAt(const SkPoint&)     const override;

private:
    BlendModeEffect(sk_sp<RenderNode>, SkBlendMode);

    SkBlendMode fMode;

    using INHERITED = EffectNode;
};

class LayerEffect final : public EffectNode {
public:
    ~LayerEffect() override;

    static sk_sp<LayerEffect> Make(sk_sp<RenderNode> child,
                                   SkBlendMode mode = SkBlendMode::kSrcOver);

    SG_ATTRIBUTE(Mode, SkBlendMode, fMode)

private:
    LayerEffect(sk_sp<RenderNode> child, SkBlendMode mode);

    void onRender(SkCanvas*, const RenderContext*) const override;

    SkBlendMode fMode;

    using INHERITED = EffectNode;
};

} // namespace sksg

#endif // SkSGRenderEffect_DEFINED
