/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRive_DEFINED
#define SkRive_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkM44.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "modules/sksg/include/SkSGGroup.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <memory>
#include <vector>

class SkStreamAsset;

namespace skrive {

class Node : public sksg::Group {
public:
    SG_ATTRIBUTE(Name               , SkString , fName               )
    SG_ATTRIBUTE(Translation        , SkV2     , fTranslation        )
    SG_ATTRIBUTE(Scale              , SkV2     , fScale              )
    SG_ATTRIBUTE(Rotation           , float    , fRotation           )
    SG_ATTRIBUTE(Opacity            , float    , fOpacity            )
    SG_ATTRIBUTE(CollapsedVisibility, bool     , fCollapsedVisibility)

    Node() : Node(Type::kNode) {}

protected:
    enum class Type : uint32_t {
        kNode, // base group node
    };

    explicit Node(Type t) : fType(t) {}

    SkRect onRevalidate(sksg::InvalidationController*, const SkMatrix&) override;

    Type type() const { return fType; }

private:
    const Type fType;

    SkString  fName;
    SkV2      fTranslation         = {0, 0},
              fScale               = {1, 1};
    float     fRotation            = 0,
              fOpacity             = 1;
    bool      fCollapsedVisibility = false;

    using INHERITED = sksg::Group;
};

class Drawable : public Node {
public:
    SG_ATTRIBUTE(DrawOrder, size_t     , fDrawOrder)
    SG_ATTRIBUTE(BlendMode, SkBlendMode, fBlendMode)
    SG_ATTRIBUTE(IsHidden , bool       , fIsHidden )

private:
    size_t      fDrawOrder = 0;
    SkBlendMode fBlendMode = SkBlendMode::kSrcOver;
    bool        fIsHidden  = false;

    using INHERITED = Node;
};

class Shape final : public Drawable {
public:
    SG_ATTRIBUTE(TransformAffectsStroke, bool, fTransformAffectsStroke)

private:
    bool fTransformAffectsStroke = true;

    using INHERITED = Drawable;
};

class Artboard final : public sksg::RenderNode {
public:
    SG_ATTRIBUTE(Name        , SkString , fName        ) // TODO: non-invalidating attributes?
    SG_ATTRIBUTE(Color       , SkColor4f, fColor       )
    SG_ATTRIBUTE(Size        , SkV2     , fSize        )
    SG_ATTRIBUTE(Origin      , SkV2     , fOrigin      )
    SG_ATTRIBUTE(Translation , SkV2     , fTranslation )
    SG_ATTRIBUTE(ClipContents, bool     , fClipContents)

private:
    SkRect onRevalidate(sksg::InvalidationController*, const SkMatrix&) override;
    void onRender(SkCanvas*, const RenderContext*) const override;
    const RenderNode* onNodeAt(const SkPoint&) const override;

    SkString  fName;
    SkColor4f fColor        = {0, 0, 0, 1};
    SkV2      fSize         = {0, 0},
              fOrigin       = {0, 0},
              fTranslation  = {0, 0};
    bool      fClipContents = false;

    using INHERITED = RenderNode;
};

class SK_API SkRive final : public SkNVRefCnt<SkRive> {
public:
    class Builder final {
    public:
        sk_sp<SkRive> make(std::unique_ptr<SkStreamAsset>);
    };

    const std::vector<sk_sp<Artboard>>& artboards() const { return fArtboards; }
          std::vector<sk_sp<Artboard>>& artboards()       { return fArtboards; }

private:
    std::vector<sk_sp<Artboard>> fArtboards;
};

} // skrive

#endif // SkRive_DEFINED
