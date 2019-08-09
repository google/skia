/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "include/core/SkColor.h"
#include "include/core/SkStream.h"
#include "src/core/SkRemoteGlyphCache.h"
#include "src/core/SkScalerContext.h"

int main(int argc, char** argv) {
    std::unordered_map<uint64_t, uint32_t> counts;

    std::vector<std::string> filenames;
    for (int i = 1; i < argc; i++) {
        filenames.emplace_back(argv[i]);
    }

    for (const auto& filename : filenames) {
        SkFILEStream in{filename.c_str()};
        auto trace = SkStrikeServer::CreateBlobTrace(&in);
        for (const auto& record : trace) {
            const SkPaint paint = record.paint;
            bool fastByPass = paint.getStyle() == SkPaint::kFill_Style
                    && paint.getPathEffect() == nullptr
                    && paint.getMaskFilter() == nullptr;
            if (fastByPass) {
                uint64_t blobID = record.origUniqueID;
                SkPoint offset = record.offset;
                SkColor c = SkMaskGamma::CanonicalColor(paint.getColor());
                uint32_t colorBits =
                        (SkColorGetR(c) >> 5u) << 6u
                      | (SkColorGetG(c) >> 5u) << 3u
                      | SkColorGetB(c) >> 5u;

                SkFixed fx = (SkScalarToFixed(offset.x()) >> 13) & 7;
                SkFixed fy = (SkScalarToFixed(offset.y()) >> 13) & 7;
                uint32_t posBits = (fx << 3 | fy) << 12;

                uint64_t blobKey = blobID << 32u | posBits | colorBits;
                auto lookup = counts.find(blobKey);
                if (lookup == counts.end()) {
                    bool ok;
                    std::tie(lookup, ok) = counts.insert({blobKey, 0});
                    SkASSERT(ok);
                }
                lookup->second += 1;
                std::cout << std::hex << blobKey << "\n";
            }
        }
    }

    int oneCount = 0;
    int moreCounts = 0;
    for (const auto& t : counts) {
        uint64_t id; uint32_t count;
        std::tie(id, count) = t;
        if (count == 1) {
            oneCount += 1;
        } else {
            moreCounts += count;
        }
    }

    std::cerr << "ones: " << oneCount << " more: " << moreCounts << "\n";
    return 0;
}
