/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/include/SkRive.h"

#include "experimental/skrive/src/reader/StreamReader.h"
#include "include/core/SkCanvas.h"

namespace skrive {

namespace internal {

sk_sp<Artboard> parse_artboard(StreamReader* sr) {
    auto ab = sk_make_sp<Artboard>();

    ab->setName        (sr->readString("name"        ));
    ab->setTranslation (sr->readV2    ("translation" ));
    ab->setSize       ({sr->readFloat ("width"       ),
                        sr->readFloat ("height"      )});
    ab->setOrigin      (sr->readV2    ("origin"      ));
    ab->setClipContents(sr->readBool  ("clipContents"));
    ab->setColor       (sr->readColor ("color"       ));

    SkDebugf(".. parsed artboard \"%s\" [%f x %f]\n",
             ab->getName().c_str(), ab->getSize().x, ab->getSize().y);

    return ab;
}

} // namespace internal

SkRect Artboard::onRevalidate(sksg::InvalidationController*, const SkMatrix&) {
    return SkRect::MakeXYWH(fTranslation.x, fTranslation.y, fSize.x, fSize.y);
}

const sksg::RenderNode* Artboard::onNodeAt(const SkPoint&) const {
    return this;
}

void Artboard::onRender(SkCanvas* canvas, const RenderContext*) const {
    SkPaint paint;
    paint.setColor4f(fColor);

    canvas->drawRect(this->bounds(), paint);
}

} // namespace skrive
