/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SubsetSingleBench.h"
#include "SubsetBenchPriv.h"
#include "SkData.h"
#include "SkCodec.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkScanlineDecoder.h"
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
                                     uint32_t offsetTop,
                                     bool useCodec)
    : fColorType(colorType)
    , fSubsetWidth(subsetWidth)
    , fSubsetHeight(subsetHeight)
    , fOffsetLeft(offsetLeft)
    , fOffsetTop(offsetTop)
    , fUseCodec(useCodec)
{
    // Parse the filename
    SkString baseName = SkOSPath::Basename(path.c_str());

    // Choose an informative color name
    const char* colorName = get_color_name(fColorType);

    fName.printf("%sSubsetSingle_%dx%d +%d_+%d_%s_%s", fUseCodec ? "Codec" : "Image", fSubsetWidth,
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

void SubsetSingleBench::onDraw(const int n, SkCanvas* canvas) {
    // When the color type is kIndex8, we will need to store the color table.  If it is
    // used, it will be initialized by the codec.
    int colorCount;
    SkPMColor colors[256];
    if (fUseCodec) {
        for (int count = 0; count < n; count++) {
            SkAutoTDelete<SkScanlineDecoder> scanlineDecoder(
                    SkScanlineDecoder::NewFromStream(fStream->duplicate()));
            const SkImageInfo info = scanlineDecoder->getInfo().makeColorType(fColorType);
            SkAutoTDeleteArray<uint8_t> row(SkNEW_ARRAY(uint8_t, info.minRowBytes()));
            scanlineDecoder->start(info, NULL, colors, &colorCount);

            SkBitmap bitmap;
            SkImageInfo subsetInfo = info.makeWH(fSubsetWidth, fSubsetHeight);
            alloc_pixels(&bitmap, subsetInfo, colors, colorCount);

            scanlineDecoder->skipScanlines(fOffsetTop);
            uint32_t bpp = info.bytesPerPixel();
            for (uint32_t y = 0; y < fSubsetHeight; y++) {
                scanlineDecoder->getScanlines(row.get(), 1, 0);
                memcpy(bitmap.getAddr(0, y), row.get() + fOffsetLeft * bpp,
                        fSubsetWidth * bpp);
            }
        }
    } else {
        for (int count = 0; count < n; count++) {
            SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(fStream));
            int width, height;
            decoder->buildTileIndex(fStream->duplicate(), &width, &height);
            SkBitmap bitmap;
            SkIRect rect = SkIRect::MakeXYWH(fOffsetLeft, fOffsetTop, fSubsetWidth,
                    fSubsetHeight);
            decoder->decodeSubset(&bitmap, rect, fColorType);
        }
    }
}
