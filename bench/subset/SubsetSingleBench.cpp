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
    const char* colorName = color_type_to_str(fColorType);

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

void SubsetSingleBench::onDraw(int n, SkCanvas* canvas) {
    // When the color type is kIndex8, we will need to store the color table.  If it is
    // used, it will be initialized by the codec.
    int colorCount;
    SkPMColor colors[256];
    if (fUseCodec) {
        for (int count = 0; count < n; count++) {
            SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(fStream->duplicate()));
            const SkImageInfo info = codec->getInfo().makeColorType(fColorType);

            SkDEBUGCODE(SkCodec::Result result =)
            codec->startScanlineDecode(info, nullptr, colors, &colorCount);
            SkASSERT(result == SkCodec::kSuccess);

            SkBitmap bitmap;
            SkImageInfo subsetInfo = info.makeWH(fSubsetWidth, fSubsetHeight);
            alloc_pixels(&bitmap, subsetInfo, colors, colorCount);

            SkDEBUGCODE(int lines = ) codec->skipScanlines(fOffsetTop);
            SkASSERT((uint32_t) lines == fOffsetTop);

            const uint32_t bpp = info.bytesPerPixel();

            switch (codec->getScanlineOrder()) {
                case SkCodec::kTopDown_SkScanlineOrder: {
                    SkAutoTDeleteArray<uint8_t> row(new uint8_t[info.minRowBytes()]);
                    for (uint32_t y = 0; y < fSubsetHeight; y++) {
                        SkDEBUGCODE(lines = ) codec->getScanlines(row.get(), 1, 0);
                        SkASSERT(lines == 1);

                        memcpy(bitmap.getAddr(0, y), row.get() + fOffsetLeft * bpp,
                                fSubsetWidth * bpp);
                    }
                    break;
                }
                case SkCodec::kNone_SkScanlineOrder: {
                    // decode all scanlines that intersect the subset, and copy the subset
                    // into the output.
                    SkImageInfo stripeInfo = info.makeWH(info.width(), fSubsetHeight);
                    SkBitmap stripeBm;
                    alloc_pixels(&stripeBm, stripeInfo, colors, colorCount);

                    SkDEBUGCODE(lines = ) codec->getScanlines(stripeBm.getPixels(), fSubsetHeight,
                                                               stripeBm.rowBytes());
                    SkASSERT((uint32_t) lines == fSubsetHeight);

                    for (uint32_t y = 0; y < fSubsetHeight; y++) {
                        memcpy(bitmap.getAddr(0, y), stripeBm.getAddr(fOffsetLeft, y),
                                fSubsetWidth * bpp);
                    }
                    break;
                }
                default:
                    // We currently are only testing kTopDown and kNone, which are the only
                    // two used by the subsets we care about. skbug.com/4428
                    SkASSERT(false);

            }
        }
    } else {
        for (int count = 0; count < n; count++) {
            SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(fStream));
            int width, height;
            SkAssertResult(decoder->buildTileIndex(fStream->duplicate(), &width, &height));
            SkBitmap bitmap;
            SkIRect rect = SkIRect::MakeXYWH(fOffsetLeft, fOffsetTop, fSubsetWidth,
                    fSubsetHeight);
            SkAssertResult(decoder->decodeSubset(&bitmap, rect, fColorType));
        }
    }
}
