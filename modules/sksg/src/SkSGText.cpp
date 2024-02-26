/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGText.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"

#include <utility>

class SkMatrix;

namespace sksg {

sk_sp<Text> Text::Make(sk_sp<SkTypeface> tf, const SkString& text) {
    return sk_sp<Text>(new Text(std::move(tf), text));
}

Text::Text(sk_sp<SkTypeface> tf, const SkString& text)
    : fTypeface(std::move(tf))
    , fText(text) {}

Text::~Text() = default;

SkPoint Text::alignedPosition(SkScalar advance) const {
    auto aligned = fPosition;

    switch (fAlign) {
    case SkTextUtils::kLeft_Align:
        break;
    case SkTextUtils::kCenter_Align:
        aligned.offset(-advance / 2, 0);
        break;
    case SkTextUtils::kRight_Align:
        aligned.offset(-advance, 0);
        break;
    }

    return aligned;
}

SkRect Text::onRevalidate(InvalidationController*, const SkMatrix&) {
    // TODO: we could potentially track invals which don't require rebuilding the blob.

    SkFont font;
    font.setTypeface(fTypeface);
    font.setSize(fSize);
    font.setScaleX(fScaleX);
    font.setSkewX(fSkewX);
    font.setEdging(fEdging);
    font.setHinting(fHinting);

    // N.B.: fAlign is applied externally (in alignedPosition()), because
    //  1) SkTextBlob has some trouble computing accurate bounds with alignment.
    //  2) SkPaint::Align is slated for deprecation.

    fBlob = SkTextBlob::MakeFromText(fText.c_str(), fText.size(), font, SkTextEncoding::kUTF8);
    if (!fBlob) {
        return SkRect::MakeEmpty();
    }

    const auto& bounds = fBlob->bounds();
    const auto aligned_pos = this->alignedPosition(bounds.width());

    return bounds.makeOffset(aligned_pos.x(), aligned_pos.y());
}

void Text::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    const auto aligned_pos = this->alignedPosition(this->bounds().width());
    canvas->drawTextBlob(fBlob, aligned_pos.x(), aligned_pos.y(), paint);
}

bool Text::onContains(const SkPoint& p) const {
    return this->asPath().contains(p.x(), p.y());
}

SkPath Text::onAsPath() const {
    // TODO
    return SkPath();
}

void Text::onClip(SkCanvas* canvas, bool antiAlias) const {
    canvas->clipPath(this->asPath(), antiAlias);
}

} // namespace sksg
