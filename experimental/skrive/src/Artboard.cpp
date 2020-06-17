/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/include/SkRive.h"

#include "experimental/skrive/src/reader/StreamReader.h"
#include "include/core/SkCanvas.h"

#include <tuple>
#include <vector>

namespace skrive {

namespace internal {

extern std::tuple<sk_sp<Node>, size_t> parse_node(StreamReader*);

sk_sp<Node> parse_components(StreamReader* sr) {
    const auto count = sr->readLength16();

    std::vector<sk_sp<Node>> nodes;
    nodes.reserve(count);

    for (size_t i = 0; i < count; ++i) {
        auto [ node, parent_index ] = parse_node(sr);

        if (parent_index < i && nodes[parent_index]) {
            nodes[parent_index]->addChild(node);
        }

        nodes.push_back(std::move(node));
    }

    return nullptr;
}

sk_sp<Artboard> parse_artboard(StreamReader* sr) {
    auto ab = sk_make_sp<Artboard>();

    ab->setName        (sr->readString("name"        ));
    ab->setTranslation (sr->readV2    ("translation" ));
    ab->setSize       ({sr->readFloat ("width"       ),
                        sr->readFloat ("height"      )});
    ab->setOrigin      (sr->readV2    ("origin"      ));
    ab->setClipContents(sr->readBool  ("clipContents"));
    ab->setColor       (sr->readColor ("color"       ));

    for (;;) {
        StreamReader::AutoBlock block(sr);
        if (block.type() == StreamReader::BlockType::kEoB) {
            break;
        }

        switch (block.type()) {
        case StreamReader::BlockType::kComponents:
            parse_components(sr);
            break;
        default:
            SkDebugf("!! Unsupported block type: %d\n", block.type());
            break;
        }
    }

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
