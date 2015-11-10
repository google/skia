/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CodecBenchPriv.h"
#include "SubsetZoomBench.h"
#include "SubsetBenchPriv.h"
#include "SkData.h"
#include "SkCodec.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
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
                                 uint32_t subsetHeight)
    : fColorType(colorType)
    , fSubsetWidth(subsetWidth)
    , fSubsetHeight(subsetHeight)
{
    // Parse the filename
    SkString baseName = SkOSPath::Basename(path.c_str());

    // Choose an informative color name
    const char* colorName = color_type_to_str(fColorType);

    fName.printf("CodecSubsetZoom_%dx%d_%s_%s", fSubsetWidth,
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

void SubsetZoomBench::onDraw(int n, SkCanvas* canvas) {
    // When the color type is kIndex8, we will need to store the color table.  If it is
    // used, it will be initialized by the codec.
    int colorCount;
    SkPMColor colors[256];
    for (int count = 0; count < n; count++) {
        SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(fStream->duplicate()));
        SkASSERT(SkCodec::kOutOfOrder_SkScanlineOrder != codec->getScanlineOrder());
        const SkImageInfo info = codec->getInfo().makeColorType(fColorType);

        const int centerX = info.width() / 2;
        const int centerY = info.height() / 2;
        int w = fSubsetWidth;
        int h = fSubsetHeight;
        do {
            const int subsetStartX = SkTMax(0, centerX - w / 2);
            const int subsetStartY = SkTMax(0, centerY - h / 2);
            const int subsetWidth = SkTMin(w, info.width() - subsetStartX);
            const int subsetHeight = SkTMin(h, info.height() - subsetStartY);

            // The scanline decoder will handle subsetting in the x-dimension.
            SkIRect subset = SkIRect::MakeXYWH(subsetStartX, 0, subsetWidth,
                    codec->getInfo().height());
            SkCodec::Options options;
            options.fSubset = &subset;

            SkDEBUGCODE(SkCodec::Result result = )
            codec->startScanlineDecode(info, &options, colors, &colorCount);
            SkASSERT(SkCodec::kSuccess == result);

            // Note that if we subsetted and scaled in a single step, we could use the
            // same bitmap - as is often done in actual use cases.
            SkBitmap bitmap;
            SkImageInfo subsetInfo = info.makeWH(subsetWidth, subsetHeight);
            alloc_pixels(&bitmap, subsetInfo, colors, colorCount);

            SkDEBUGCODE(bool success = ) codec->skipScanlines(subsetStartY);
            SkASSERT(success);

            SkDEBUGCODE(int lines = ) codec->getScanlines(bitmap.getPixels(),
                    subsetHeight, bitmap.rowBytes());
            SkASSERT(subsetHeight == lines);

            w <<= 1;
            h <<= 1;
        } while (w < 2 * info.width() || h < 2 * info.height());
    }
}
