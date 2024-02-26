/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGRenderEffect_DEFINED
#define SkSGRenderEffect_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkPoint_impl.h"
#include "modules/sksg/include/SkSGEffectNode.h"
#include "modules/sksg/include/SkSGNode.h"

#include <optional>

class SkCanvas;
class SkMatrix;

// TODO: merge EffectNode.h with this header

namespace sksg {
class InvalidationController;
class RenderNode;

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

    SG_ATTRIBUTE(CropRect, SkImageFilters::CropRect, fCropRect)

protected:
    ImageFilter();

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) final;

    virtual sk_sp<SkImageFilter> onRevalidateFilter() = 0;

private:
    sk_sp<SkImageFilter>     fFilter;
    SkImageFilters::CropRect fCropRect = std::nullopt;

    using INHERITED = Node;
};

/**
 * Attaches an ImageFilter (chain) to the render DAG.
 */
class ImageFilterEffect final : public EffectNode {
public:
    ~ImageFilterEffect() override;

    static sk_sp<RenderNode> Make(sk_sp<RenderNode> child, sk_sp<ImageFilter> filter);

    enum class Cropping {
        kNone,    // Doesn't use a crop rect.
        kContent, // Uses the content bounding box as a crop rect.
    };

    SG_ATTRIBUTE(Cropping, Cropping, fCropping)

protected:
    void onRender(SkCanvas*, const RenderContext*) const override;
    const RenderNode* onNodeAt(const SkPoint&)     const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    ImageFilterEffect(sk_sp<RenderNode> child, sk_sp<ImageFilter> filter);

    sk_sp<ImageFilter> fImageFilter;
    Cropping           fCropping = Cropping::kNone;

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

    static sk_sp<DropShadowImageFilter> Make();

    enum class Mode { kShadowAndForeground, kShadowOnly };

    SG_ATTRIBUTE(Offset, SkVector, fOffset)
    SG_ATTRIBUTE(Sigma , SkVector, fSigma )
    SG_ATTRIBUTE(Color , SkColor , fColor )
    SG_ATTRIBUTE(Mode  , Mode    , fMode  )

protected:
    sk_sp<SkImageFilter> onRevalidateFilter() override;

private:
    explicit DropShadowImageFilter();

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

    static sk_sp<BlurImageFilter> Make();

    SG_ATTRIBUTE(Sigma   , SkVector  , fSigma   )
    SG_ATTRIBUTE(TileMode, SkTileMode, fTileMode)

protected:
    sk_sp<SkImageFilter> onRevalidateFilter() override;

private:
    explicit BlurImageFilter();

    SkVector   fSigma    = { 0, 0 };
    SkTileMode fTileMode = SkTileMode::kDecal;

    using INHERITED = ImageFilter;
};

/**
 * Applies an SkBlender to descendant render nodes.
 */
class BlenderEffect final : public EffectNode {
public:
    ~BlenderEffect() override;

    static sk_sp<BlenderEffect> Make(sk_sp<RenderNode> child, sk_sp<SkBlender> = nullptr);

    SG_ATTRIBUTE(Blender, sk_sp<SkBlender>, fBlender)

protected:
    void onRender(SkCanvas*, const RenderContext*) const override;
    const RenderNode* onNodeAt(const SkPoint&)     const override;

private:
    BlenderEffect(sk_sp<RenderNode>, sk_sp<SkBlender>);

    sk_sp<SkBlender> fBlender;

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
