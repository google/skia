/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/include/SkRive.h"

#include "experimental/skrive/src/reader/StreamReader.h"

namespace skrive {

namespace internal {

template <typename T>
size_t parse_node(StreamReader*, T*);

template <>
size_t parse_node<Paint>(StreamReader* sr, Paint* node) {
    const auto parent_id = parse_node<Component>(sr, node);

    node->setOpacity(sr->readFloat("opacity"));

    return parent_id;
}

void parse_fill_stroke(StreamReader* sr, Paint* node) {
    if (node->style() == SkPaint::kFill_Style) {
        static constexpr SkPathFillType gFillTypeMap[] = {
            SkPathFillType::kWinding,  // 0
            SkPathFillType::kEvenOdd,  // 1
        };
        node->setFillRule(gFillTypeMap[std::min<size_t>(sr->readUInt8("fillRule"),
                                                        SK_ARRAY_COUNT(gFillTypeMap) - 1)]);
    } else {
        node->setStrokeWidth(sr->readFloat("width"));

        static constexpr SkPaint::Cap gCapMap[] = {
            SkPaint::kButt_Cap,   // 0
            SkPaint::kRound_Cap,  // 1
            SkPaint::kSquare_Cap, // 2
        };
        node->setStrokeCap(gCapMap[std::min<size_t>(sr->readUInt8("cap"),
                                                    SK_ARRAY_COUNT(gCapMap) - 1)]);

        static constexpr SkPaint::Join gJoinMap[] = {
            SkPaint::kMiter_Join,  // 0
            SkPaint::kRound_Join,  // 1
            SkPaint::kBevel_Join,  // 2
        };
        node->setStrokeJoin(gJoinMap[std::min<size_t>(sr->readUInt8("join"),
                                                      SK_ARRAY_COUNT(gJoinMap) - 1)]);

        static constexpr Paint::StrokeTrim gTrimMap[] = {
            Paint::StrokeTrim::kOff,         // 0
            Paint::StrokeTrim::kSequential,  // 1
            Paint::StrokeTrim::kSynced,      // 2
        };
        node->setStrokeTrim(gTrimMap[std::min<size_t>(sr->readUInt8("trim"),
                                                      SK_ARRAY_COUNT(gTrimMap) - 1)]);

        if (node->getStrokeTrim() != Paint::StrokeTrim::kOff) {
            node->setStrokeTrimStart (sr->readFloat("start" ));
            node->setStrokeTrimEnd   (sr->readFloat("end"   ));
            node->setStrokeTrimOffset(sr->readFloat("offset"));
        }
    }
}

} // namespace internal

void Paint::onApply(SkPaint* paint) const {
    paint->setAntiAlias(true);
    paint->setStyle(this->style());

    paint->setStrokeWidth(fStrokeWidth);
    paint->setStrokeCap  (fStrokeCap  );
    paint->setStrokeJoin (fStrokeJoin );
}

} // namespace skrive
