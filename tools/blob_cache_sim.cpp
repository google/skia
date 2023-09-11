/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkTextBlobTrace.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

int main(int argc, char** argv) {
    std::unordered_map<uint64_t, uint32_t> counts;
    size_t total = 0;

    for (int i = 1; i < argc; i++) {
        const char* filename = argv[i];

        SkFILEStream in{filename};
        std::vector<SkTextBlobTrace::Record> trace = SkTextBlobTrace::CreateBlobTrace(&in);
        for (const SkTextBlobTrace::Record& record : trace) {
            total++;
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
                      |  SkColorGetB(c) >> 5u;

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
        std::cerr << "trace: " << filename
                  << " unique: " << counts.size()
                  << " all: " << total
                  << " ratio: " << (float)total/counts.size() << "\n";

        SkRect bounds = {0, 0, 0, 0};
        for (const SkTextBlobTrace::Record& record : trace) {
            bounds.join(record.blob->bounds().makeOffset(record.offset.x(), record.offset.y()));
        }
        SkIRect iBounds = bounds.roundOut();
        if (iBounds.size().isEmpty()) {
            continue;
        }
        static constexpr SkColor kBackground = SK_ColorWHITE;
        sk_sp<SkSurface> surf = SkSurfaces::Raster(
                SkImageInfo::MakeN32Premul(iBounds.width() + 16, iBounds.height() + 16));
        SkCanvas* canvas = surf->getCanvas();
        canvas->translate(8.0f - iBounds.x(), 8.0f - iBounds.y());
        canvas->clear(kBackground);

        for (const SkTextBlobTrace::Record& record : trace) {
            canvas->drawTextBlob(
                    record.blob.get(), record.offset.x(), record.offset.y(), record.paint);
        }

        sk_sp<SkImage> img(surf->makeImageSnapshot());
        if (img) {
            sk_sp<SkData> png = SkPngEncoder::Encode(nullptr, img.get(), {});
            if (png) {
                SkString path = SkStringPrintf("text_blob_trace_%04d.png", i);
                SkFILEWStream(path.c_str()).write(png->data(), png->size());
            }
        }
    }
    return 0;
}
