/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGImage_DEFINED
#define SkSGImage_DEFINED

#include "modules/sksg/include/SkSGRenderNode.h"

#include "include/core/SkSamplingOptions.h"

class SkImage;

namespace sksg {

/**
 * Concrete rendering node, wrapping an SkImage.
 *
 */
class Image final : public RenderNode {
public:
    static sk_sp<Image> Make(sk_sp<SkImage> image) {
        return sk_sp<Image>(new Image(std::move(image)));
    }

    SG_ATTRIBUTE(Image          , sk_sp<SkImage>   , fImage          )
    SG_ATTRIBUTE(SamplingOptions, SkSamplingOptions, fSamplingOptions)
    SG_ATTRIBUTE(AntiAlias      , bool             , fAntiAlias      )

protected:
    explicit Image(sk_sp<SkImage>);

    void onRender(SkCanvas*, const RenderContext*) const override;
    const RenderNode* onNodeAt(const SkPoint&)     const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    SkSamplingOptions fSamplingOptions;
    sk_sp<SkImage>    fImage;
    bool              fAntiAlias = true;

    using INHERITED = RenderNode;
};

} // namespace sksg

#endif // SkSGImage_DEFINED
