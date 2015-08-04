/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SubsetZoomBench.h"
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
 * Choose subsets to mimic a user zooming in or out on a photo.
 *
 */

SubsetZoomBench::SubsetZoomBench(const SkString& path,
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

    fName.printf("%sSubsetZoom_%dx%d_%s_%s", fUseCodec ? "Codec" : "Image", fSubsetWidth,
            fSubsetHeight, baseName.c_str(), colorName);
    
    // Perform the decode setup
    SkAutoTUnref<SkData> encoded(SkData::NewFromFileName(path.c_str()));
    fStream.reset(new SkMemoryStream(encoded));
}

const char* SubsetZoomBench::onGetName() {
    return fName.c_str();
}

bool SubsetZoomBench::isSuitableFor(Backend backend) {
    return kNonRendering_Backend == backend;
}

void SubsetZoomBench::onDraw(const int n, SkCanvas* canvas) {
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

            const int centerX = info.width() / 2;
            const int centerY = info.height() / 2;
            int w = fSubsetWidth;
            int h = fSubsetHeight;
            do {
                const int subsetStartX = SkTMax(0, centerX - w / 2);
                const int subsetStartY = SkTMax(0, centerY - h / 2);
                const int subsetWidth = SkTMin(w, info.width() - subsetStartX);
                const int subsetHeight = SkTMin(h, info.height() - subsetStartY);
                // Note that if we subsetted and scaled in a single step, we could use the
                // same bitmap - as is often done in actual use cases.
                SkBitmap bitmap;
                SkImageInfo subsetInfo = info.makeWH(subsetWidth, subsetHeight);
                alloc_pixels(&bitmap, subsetInfo, colors, colorCount);

                uint32_t bpp = info.bytesPerPixel();
                scanlineDecoder->skipScanlines(subsetStartY);
                for (int y = 0; y < subsetHeight; y++) {
                    scanlineDecoder->getScanlines(row.get(), 1, 0);
                    memcpy(bitmap.getAddr(0, y), row.get() + subsetStartX * bpp,
                            subsetWidth * bpp);
                }
                w <<= 1;
                h <<= 1;
            } while (w < 2 * info.width() || h < 2 * info.height());
        }
    } else {
        for (int count = 0; count < n; count++) {
            int width, height;
            SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(fStream));
            decoder->buildTileIndex(fStream->duplicate(), &width, &height);

            const int centerX = width / 2;
            const int centerY = height / 2;
            int w = fSubsetWidth;
            int h = fSubsetHeight;
            do {
                const int subsetStartX = SkTMax(0, centerX - w / 2);
                const int subsetStartY = SkTMax(0, centerY - h / 2);
                const int subsetWidth = SkTMin(w, width - subsetStartX);
                const int subsetHeight = SkTMin(h, height - subsetStartY);
                SkBitmap bitmap;
                SkIRect rect = SkIRect::MakeXYWH(subsetStartX, subsetStartY, subsetWidth,
                        subsetHeight);
                decoder->decodeSubset(&bitmap, rect, fColorType);
                w <<= 1;
                h <<= 1;
            } while (w < 2 * width || h < 2 * height);
        }
    }
}
