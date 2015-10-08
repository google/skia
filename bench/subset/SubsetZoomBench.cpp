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
    const char* colorName = color_type_to_str(fColorType);

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

void SubsetZoomBench::onDraw(int n, SkCanvas* canvas) {
    // When the color type is kIndex8, we will need to store the color table.  If it is
    // used, it will be initialized by the codec.
    int colorCount;
    SkPMColor colors[256];
    if (fUseCodec) {
        for (int count = 0; count < n; count++) {
            SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(fStream->duplicate()));
            const SkImageInfo info = codec->getInfo().makeColorType(fColorType);
            SkAutoTDeleteArray<uint8_t> row(nullptr);
            if (codec->getScanlineOrder() == SkCodec::kTopDown_SkScanlineOrder) {
                row.reset(new uint8_t[info.minRowBytes()]);
            }

            const int centerX = info.width() / 2;
            const int centerY = info.height() / 2;
            int w = fSubsetWidth;
            int h = fSubsetHeight;
            do {
                SkDEBUGCODE(SkCodec::Result result = )
                codec->startScanlineDecode(info, nullptr, colors, &colorCount);
                SkASSERT(SkCodec::kSuccess == result);

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

                SkDEBUGCODE(result = ) codec->skipScanlines(subsetStartY);
                SkASSERT(SkCodec::kSuccess == result);

                switch (codec->getScanlineOrder()) {
                    case SkCodec::kTopDown_SkScanlineOrder:
                        for (int y = 0; y < subsetHeight; y++) {
                            SkDEBUGCODE(result = ) codec->getScanlines(row.get(), 1, 0);
                            SkASSERT(SkCodec::kSuccess == result);

                            memcpy(bitmap.getAddr(0, y), row.get() + subsetStartX * bpp,
                                    subsetWidth * bpp);
                        }
                        break;
                    case SkCodec::kNone_SkScanlineOrder: {
                        // decode all scanlines that intersect the subset, and copy the subset
                        // into the output.
                        SkImageInfo stripeInfo = info.makeWH(info.width(), subsetHeight);
                        SkBitmap stripeBm;
                        alloc_pixels(&stripeBm, stripeInfo, colors, colorCount);

                        SkDEBUGCODE(result = ) codec->getScanlines(stripeBm.getPixels(),
                                subsetHeight, stripeBm.rowBytes());
                        SkASSERT(SkCodec::kSuccess == result);

                        for (int y = 0; y < subsetHeight; y++) {
                            memcpy(bitmap.getAddr(0, y),
                                   stripeBm.getAddr(subsetStartX, y),
                                   subsetWidth * bpp);
                        }
                        break;
                    }
                    default:
                        // We currently are only testing kTopDown and kNone, which are the only
                        // two used by the subsets we care about. skbug.com/4428
                        SkASSERT(false);
                }

                w <<= 1;
                h <<= 1;
            } while (w < 2 * info.width() || h < 2 * info.height());
        }
    } else {
        for (int count = 0; count < n; count++) {
            int width, height;
            SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(fStream));
            SkAssertResult(decoder->buildTileIndex(fStream->duplicate(), &width, &height));

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
                SkAssertResult(decoder->decodeSubset(&bitmap, rect, fColorType));
                w <<= 1;
                h <<= 1;
            } while (w < 2 * width || h < 2 * height);
        }
    }
}
