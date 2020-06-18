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

template <typename T>
size_t parse_node(StreamReader*, T*);

template <typename T>
std::tuple<sk_sp<Node>, size_t> make_from_stream(StreamReader* sr) {
    auto node = sk_make_sp<T>();
    const auto parent_index = parse_node<T>(sr, node.get());

    return std::make_tuple(std::move(node), parent_index);
}

std::tuple<sk_sp<Node>, size_t> parse_component(StreamReader* sr) {
    StreamReader::AutoBlock block(sr);
    switch (block.type()) {
    case StreamReader::BlockType::kActorNode : return make_from_stream<Node >(sr);
    case StreamReader::BlockType::kActorShape: return make_from_stream<Shape>(sr);
    default:
        break;
    }

    SkDebugf("!! unsupported node type: %d\n", block.type());
    return {nullptr, 0};
}

sk_sp<Node> parse_components(StreamReader* sr) {
    const auto count = sr->readLength16();

    std::vector<sk_sp<Node>> nodes;
    nodes.reserve(count);

    for (size_t i = 0; i < count; ++i) {
        auto [ node, parent_index ] = parse_component(sr);

        if (node && parent_index < i && nodes[parent_index]) {
            nodes[parent_index]->addChild(node);
        }

        nodes.push_back(std::move(node));
    }

    SkDebugf(".. parsed %zu nodes\n", nodes.size());

    return count > 0
            ? std::move(nodes[0])
            : nullptr;
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
