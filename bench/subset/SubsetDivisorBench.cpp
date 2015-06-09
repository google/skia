/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SubsetDivisorBench.h"
#include "SubsetBenchPriv.h"
#include "SkData.h"
#include "SkCodec.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkStream.h"

/*
 *
 * This benchmark is designed to test the performance of subset decoding.
 * It uses a divisor to decode the entire image in a grid of divisor x divisor blocks.
 *
 */

SubsetDivisorBench::SubsetDivisorBench(const SkString& path,
                                       SkColorType colorType,
                                       uint32_t divisor,
                                       bool useCodec)
    : fColorType(colorType)
    , fDivisor(divisor)
    , fUseCodec(useCodec)
{
    // Parse the filename
    SkString baseName = SkOSPath::Basename(path.c_str());

    // Choose an informative color name
    const char* colorName = get_color_name(fColorType);

    fName.printf("%sSubsetDivisor_%dx%d_%s_%s", fUseCodec ? "Codec" : "Image", fDivisor, fDivisor,
            baseName.c_str(), colorName);

    // Perform the decode setup
    SkAutoTUnref<SkData> encoded(SkData::NewFromFileName(path.c_str()));
    fStream.reset(new SkMemoryStream(encoded));
}

const char* SubsetDivisorBench::onGetName() {
    return fName.c_str();
}

bool SubsetDivisorBench::isSuitableFor(Backend backend) {
    return kNonRendering_Backend == backend;
}

void SubsetDivisorBench::onDraw(const int n, SkCanvas* canvas) {
    // When the color type is kIndex8, we will need to store the color table.  If it is
    // used, it will be initialized by the codec.
    int colorCount;
    SkPMColor colors[256];
    if (fUseCodec) {
        for (int count = 0; count < n; count++) {
            SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(fStream->duplicate()));
            const SkImageInfo info = codec->getInfo().makeColorType(fColorType);
            SkAutoTDeleteArray<uint8_t> row(SkNEW_ARRAY(uint8_t, info.minRowBytes()));
            SkScanlineDecoder* scanlineDecoder = codec->getScanlineDecoder(
                    info, NULL, colors, &colorCount);

            const uint32_t subsetWidth = info.width() / fDivisor;
            const uint32_t subsetHeight = info.height() / fDivisor;
            const uint32_t maxSubsetWidth = subsetWidth + info.width() % fDivisor;
            const uint32_t maxSubsetHeight = subsetHeight + info.height() % fDivisor;
            SkBitmap bitmap;
            // Note that we use the same bitmap for all of the subsets.
            // It might be slightly larger than necessary for some of the subsets.
            bitmap.allocPixels(info.makeWH(maxSubsetWidth, maxSubsetHeight));

            for (uint32_t blockX = 0; blockX < fDivisor; blockX++) {
                for (uint32_t blockY = 0; blockY < fDivisor; blockY++) {
                    scanlineDecoder->skipScanlines(blockY * subsetHeight);
                    const uint32_t currSubsetWidth =
                            (blockX == fDivisor - 1) ? maxSubsetWidth : subsetWidth;
                    const uint32_t currSubsetHeight =
                            (blockY == fDivisor - 1) ? maxSubsetHeight : subsetHeight;
                    const uint32_t bpp = info.bytesPerPixel();
                    for (uint32_t y = 0; y < currSubsetHeight; y++) {
                        scanlineDecoder->getScanlines(row.get(), 1, 0);
                        memcpy(bitmap.getAddr(0, y), row.get() + blockX * subsetWidth * bpp,
                                currSubsetWidth * bpp);
                    }
                }
            }
        }
    } else {
        // We create a color table here to satisfy allocPixels() when the output
        // type is kIndex8.  It's okay that this is uninitialized since we never
        // use it.
        SkColorTable* colorTable = SkNEW_ARGS(SkColorTable, (colors, 0));
        for (int count = 0; count < n; count++) {
            int width, height;
            SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(fStream));
            decoder->buildTileIndex(fStream->duplicate(), &width, &height);

            const uint32_t subsetWidth = width / fDivisor;
            const uint32_t subsetHeight = height / fDivisor;
            const uint32_t maxSubsetWidth = subsetWidth + width % fDivisor;
            const uint32_t maxSubsetHeight = subsetHeight + height % fDivisor;
            SkBitmap bitmap;
            // Note that we use the same bitmap for all of the subsets.
            // It might be slightly larger than necessary for some of the subsets.
            // If we do not include this step, decodeSubset() would allocate space
            // for the pixels automatically, but this would not allow us to reuse the
            // same bitmap as the other subsets.  We want to reuse the same bitmap
            // because it gives a more fair comparison with SkCodec and is a common
            // use case of BitmapRegionDecoder.
            bitmap.allocPixels(SkImageInfo::Make(maxSubsetWidth, maxSubsetHeight,
                    fColorType, kOpaque_SkAlphaType), NULL, colorTable);

            for (uint32_t blockX = 0; blockX < fDivisor; blockX++) {
                for (uint32_t blockY = 0; blockY < fDivisor; blockY++) {
                    const uint32_t currSubsetWidth =
                            (blockX == fDivisor - 1) ? maxSubsetWidth : subsetWidth;
                    const uint32_t currSubsetHeight =
                            (blockY == fDivisor - 1) ? maxSubsetHeight : subsetHeight;
                    SkIRect rect = SkIRect::MakeXYWH(blockX * subsetWidth,
                            blockY * subsetHeight, currSubsetWidth, currSubsetHeight);
                    decoder->decodeSubset(&bitmap, rect, fColorType);
                }
            }
        }
    }
}
