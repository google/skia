/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRive_DEFINED
#define SkRive_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <memory>
#include <vector>

class SkStreamAsset;

namespace skrive {

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
