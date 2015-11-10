/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CodecBenchPriv.h"
#include "SubsetSingleBench.h"
#include "SubsetBenchPriv.h"
#include "SkData.h"
#include "SkCodec.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkStream.h"

/*
 *
 * This benchmark is designed to test the performance of subset decoding.
 * It uses an input width, height, left, and top to decode a single subset.
 *
 */

SubsetSingleBench::SubsetSingleBench(const SkString& path,
                                     SkColorType colorType,
                                     uint32_t subsetWidth,
                                     uint32_t subsetHeight,
                                     uint32_t offsetLeft,
                                     uint32_t offsetTop)
    : fColorType(colorType)
    , fSubsetWidth(subsetWidth)
    , fSubsetHeight(subsetHeight)
    , fOffsetLeft(offsetLeft)
    , fOffsetTop(offsetTop)
{
    // Parse the filename
    SkString baseName = SkOSPath::Basename(path.c_str());

    // Choose an informative color name
    const char* colorName = color_type_to_str(fColorType);

    fName.printf("CodecSubsetSingle_%dx%d +%d_+%d_%s_%s", fSubsetWidth,
            fSubsetHeight, fOffsetLeft, fOffsetTop, baseName.c_str(), colorName);

    // Perform the decode setup
    SkAutoTUnref<SkData> encoded(SkData::NewFromFileName(path.c_str()));
    fStream.reset(new SkMemoryStream(encoded));
}

const char* SubsetSingleBench::onGetName() {
    return fName.c_str();
}

bool SubsetSingleBench::isSuitableFor(Backend backend) {
    return kNonRendering_Backend == backend;
}

void SubsetSingleBench::onDraw(int n, SkCanvas* canvas) {
    // When the color type is kIndex8, we will need to store the color table.  If it is
    // used, it will be initialized by the codec.
    int colorCount;
    SkPMColor colors[256];
    for (int count = 0; count < n; count++) {
        SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(fStream->duplicate()));
        SkASSERT(SkCodec::kOutOfOrder_SkScanlineOrder != codec->getScanlineOrder());
        const SkImageInfo info = codec->getInfo().makeColorType(fColorType);

        // The scanline decoder will handle subsetting in the x-dimension.
        SkIRect subset = SkIRect::MakeXYWH(fOffsetLeft, 0, fSubsetWidth,
                codec->getInfo().height());
        SkCodec::Options options;
        options.fSubset = &subset;

        SkDEBUGCODE(SkCodec::Result result =)
        codec->startScanlineDecode(info, &options, colors, &colorCount);
        SkASSERT(result == SkCodec::kSuccess);

        SkBitmap bitmap;
        SkImageInfo subsetInfo = info.makeWH(fSubsetWidth, fSubsetHeight);
        alloc_pixels(&bitmap, subsetInfo, colors, colorCount);

        SkDEBUGCODE(bool success = ) codec->skipScanlines(fOffsetTop);
        SkASSERT(success);

        SkDEBUGCODE(uint32_t lines = ) codec->getScanlines(bitmap.getPixels(), fSubsetHeight,
                bitmap.rowBytes());
        SkASSERT(lines == fSubsetHeight);
    }
}
