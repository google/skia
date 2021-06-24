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

template <typename T, typename... Args>
std::tuple<sk_sp<Component>, size_t> make_from_stream(StreamReader* sr, Args&&... args) {
    auto node = sk_make_sp<T>(std::forward<Args>(args)...);

    const auto parent_id = parse_node<T>(sr, node.get());

    return std::make_tuple(std::move(node), parent_id);
}

std::tuple<sk_sp<Component>, size_t> parse_component(StreamReader* sr) {
    StreamReader::AutoBlock block(sr);
    switch (block.type()) {
        case StreamReader::BlockType::kActorNode:
            return make_from_stream<Node >(sr);
        case StreamReader::BlockType::kActorShape:
            return make_from_stream<Shape>(sr);
        case StreamReader::BlockType::kColorFill:
            return make_from_stream<ColorPaint>(sr, SkPaint::kFill_Style);
        case StreamReader::BlockType::kColorStroke:
            return make_from_stream<ColorPaint>(sr, SkPaint::kStroke_Style);
        case StreamReader::BlockType::kActorEllipse:
            return make_from_stream<Ellipse>(sr);
        case StreamReader::BlockType::kActorRectangle:
            return make_from_stream<Rectangle>(sr);
        default:
            break;
    }

    SkDebugf("!! unsupported node type: %d\n", (int)block.type());
    return {nullptr, 0};
}

sk_sp<Node> parse_components(StreamReader* sr) {
    const auto count = sr->readLength16();

    std::vector<sk_sp<Component>> components;
    components.reserve(count);

    for (size_t i = 0; i < count; ++i) {
        auto [ component, parent_id ] = parse_component(sr);

        // parent IDs are kinda-sorta one-based
        if (parent_id > 0) {
            parent_id -= 1;
        }

        if (component && parent_id < i && components[parent_id]) {
            if (Node* node = *components[parent_id]) {
                node->addChild(component);
            }
        }

        components.push_back(std::move(component));
    }

    SkDebugf(".. parsed %zu components\n", components.size());

    // hmm...
    for (const auto& comp : components) {
        if (comp && comp->is<Node>()) {
            return sk_ref_sp(static_cast<Node*>(*comp));
        }
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
            ab->setRoot(parse_components(sr));
            break;
        default:
            SkDebugf("!! Unsupported block type: %d\n", (int)block.type());
            break;
        }
    }

    SkDebugf(".. parsed artboard \"%s\" [%f x %f]\n",
             ab->getName().c_str(), ab->getSize().x, ab->getSize().y);

    return ab;
}

} // namespace internal

void Artboard::render(SkCanvas* canvas) const {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->translate(fTranslation.x, fTranslation.y);

    SkPaint paint;
    paint.setColor4f(fColor);

    canvas->drawRect(SkRect::MakeWH(fSize.x, fSize.y), paint);

    if (fRoot) {
        fRoot->revalidate();
        fRoot->render(canvas);
    }
}

} // namespace skrive
