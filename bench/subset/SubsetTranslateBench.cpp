/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SubsetTranslateBench.h"
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
 * It uses input dimensions to decode the entire image where each block is susbetW x subsetH.
 *
 */

SubsetTranslateBench::SubsetTranslateBench(const SkString& path,
                                           SkColorType colorType,
                                           uint32_t subsetWidth,
                                           uint32_t subsetHeight,
                                           bool useCodec)
    : fColorType(colorType)
    , fSubsetWidth(subsetWidth)
    , fSubsetHeight(subsetHeight)
    , fUseCodec(useCodec)
{
    // Parse the filename
    SkString baseName = SkOSPath::Basename(path.c_str());

    // Choose an informative color name
    const char* colorName = get_color_name(fColorType);

    fName.printf("%sSubsetTranslate_%dx%d_%s_%s", fUseCodec ? "Codec" : "Image", fSubsetWidth,
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

void SubsetTranslateBench::onDraw(const int n, SkCanvas* canvas) {
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
            // Note that we use the same bitmap for all of the subsets.
            // It might be larger than necessary for the end subsets.
            SkImageInfo subsetInfo = info.makeWH(fSubsetWidth, fSubsetHeight);
            alloc_pixels(&bitmap, subsetInfo, colors, colorCount);

            for (int x = 0; x < info.width(); x += fSubsetWidth) {
                for (int y = 0; y < info.height(); y += fSubsetHeight) {
                    scanlineDecoder->skipScanlines(y);
                    const uint32_t currSubsetWidth =
                            x + (int) fSubsetWidth > info.width() ?
                            info.width() - x : fSubsetWidth;
                    const uint32_t currSubsetHeight =
                            y + (int) fSubsetHeight > info.height() ?
                            info.height() - y : fSubsetHeight;
                    const uint32_t bpp = info.bytesPerPixel();
                    for (uint32_t y = 0; y < currSubsetHeight; y++) {
                        scanlineDecoder->getScanlines(row.get(), 1, 0);
                        memcpy(bitmap.getAddr(0, y), row.get() + x * bpp,
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
            SkBitmap bitmap;
            // Note that we use the same bitmap for all of the subsets.
            // It might be larger than necessary for the end subsets.
            // If we do not include this step, decodeSubset() would allocate space
            // for the pixels automatically, but this would not allow us to reuse the
            // same bitmap as the other subsets.  We want to reuse the same bitmap
            // because it gives a more fair comparison with SkCodec and is a common
            // use case of BitmapRegionDecoder.
            bitmap.allocPixels(SkImageInfo::Make(fSubsetWidth, fSubsetHeight,
                    fColorType, kOpaque_SkAlphaType), NULL, colorTable);

            for (int x = 0; x < width; x += fSubsetWidth) {
                for (int y = 0; y < height; y += fSubsetHeight) {
                    const uint32_t currSubsetWidth = x + (int) fSubsetWidth > width ?
                            width - x : fSubsetWidth;
                    const uint32_t currSubsetHeight = y + (int) fSubsetHeight > height ?
                            height - y : fSubsetHeight;
                    SkIRect rect = SkIRect::MakeXYWH(x, y, currSubsetWidth,
                            currSubsetHeight);
                    decoder->decodeSubset(&bitmap, rect, fColorType);
                }
            }
        }
    }
}
