/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGImage_DEFINED
#define SkSGImage_DEFINED

#include "SkSGRenderNode.h"

#include "SkFilterQuality.h"

class SkImage;

namespace sksg {

/**
 * Concrete rendering node, wrapping an SkImage.
 *
 */
class Image final : public RenderNode {
public:
    static sk_sp<Image> Make(sk_sp<SkImage> image) {
        return image ? sk_sp<Image>(new Image(std::move(image))) : nullptr;
    }

    SG_ATTRIBUTE(Quality  , SkFilterQuality, fQuality  )
    SG_ATTRIBUTE(AntiAlias, bool           , fAntiAlias)

protected:
    explicit Image(sk_sp<SkImage>);

    void onRender(SkCanvas*) const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    const sk_sp<SkImage> fImage;
    SkFilterQuality      fQuality   = kLow_SkFilterQuality;
    bool                 fAntiAlias = true;

    typedef RenderNode INHERITED;
};

} // namespace sksg

#endif // SkSGImage_DEFINED
