/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CodecBenchPriv.h"
#include "SubsetTranslateBench.h"
#include "SubsetBenchPriv.h"
#include "SkData.h"
#include "SkCodec.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkStream.h"

/*
 *
 * This benchmark is designed to test the performance of subset decoding.
 * It uses input dimensions to decode the entire image where each block is susbetW x subsetH.
 *
 */

SubsetTranslateBench::SubsetTranslateBench(const SkString& path,
                                           SkColorType colorType,
                                           uint32_t subsetWidth,
                                           uint32_t subsetHeight)
    : fColorType(colorType)
    , fSubsetWidth(subsetWidth)
    , fSubsetHeight(subsetHeight)
{
    // Parse the filename
    SkString baseName = SkOSPath::Basename(path.c_str());

    // Choose an informative color name
    const char* colorName = color_type_to_str(fColorType);

    fName.printf("CodecSubsetTranslate_%dx%d_%s_%s", fSubsetWidth,
            fSubsetHeight, baseName.c_str(), colorName);
    
    // Perform the decode setup
    SkAutoTUnref<SkData> encoded(SkData::NewFromFileName(path.c_str()));
    fStream.reset(new SkMemoryStream(encoded));
}

const char* SubsetTranslateBench::onGetName() {
    return fName.c_str();
}

bool SubsetTranslateBench::isSuitableFor(Backend backend) {
    return kNonRendering_Backend == backend;
}

// Allows allocating the bitmap first, and then writing to them later (in startScanlineDecode)
static SkPMColor* get_colors(SkBitmap* bm) {
    SkColorTable* ct = bm->getColorTable();
    if (!ct) {
        return nullptr;
    }

    return const_cast<SkPMColor*>(ct->readColors());
}

void SubsetTranslateBench::onDraw(int n, SkCanvas* canvas) {
    // When the color type is kIndex8, we will need to store the color table.  If it is
    // used, it will be initialized by the codec.
    int colorCount = 256;
    SkPMColor colors[256];
    for (int count = 0; count < n; count++) {
        SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(fStream->duplicate()));
        SkASSERT(SkCodec::kOutOfOrder_SkScanlineOrder != codec->getScanlineOrder());
        const SkImageInfo info = codec->getInfo().makeColorType(fColorType);

        SkBitmap bitmap;
        // Note that we use the same bitmap for all of the subsets.
        // It might be larger than necessary for the end subsets.
        SkImageInfo subsetInfo = info.makeWH(fSubsetWidth, fSubsetHeight);
        alloc_pixels(&bitmap, subsetInfo, colors, colorCount);

        for (int x = 0; x < info.width(); x += fSubsetWidth) {
            for (int y = 0; y < info.height(); y += fSubsetHeight) {
                const uint32_t currSubsetWidth =
                        x + (int) fSubsetWidth > info.width() ?
                        info.width() - x : fSubsetWidth;
                const uint32_t currSubsetHeight =
                        y + (int) fSubsetHeight > info.height() ?
                        info.height() - y : fSubsetHeight;

                // The scanline decoder will handle subsetting in the x-dimension.
                SkIRect subset = SkIRect::MakeXYWH(x, 0, currSubsetWidth,
                        codec->getInfo().height());
                SkCodec::Options options;
                options.fSubset = &subset;

                SkDEBUGCODE(SkCodec::Result result =)
                codec->startScanlineDecode(info, &options, get_colors(&bitmap), &colorCount);
                SkASSERT(SkCodec::kSuccess == result);

                SkDEBUGCODE(bool success =) codec->skipScanlines(y);
                SkASSERT(success);

                SkDEBUGCODE(uint32_t lines =) codec->getScanlines(bitmap.getPixels(),
                        currSubsetHeight, bitmap.rowBytes());
                SkASSERT(currSubsetHeight == lines);
            }
        }
    }
}
