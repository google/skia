/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "Resources.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkImageGenerator.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkPixelRef.h"

#ifndef SK_IGNORE_ETC1_SUPPORT

#include "etc1.h"

// This takes the etc1 data pointed to by orig, and copies it `factor` times in each
// dimension. The return value is the new data or NULL on error.
static etc1_byte* create_expanded_etc1_bitmap(const uint8_t* orig, int factor) {
    SkASSERT(orig);
    SkASSERT(factor > 1);

    const etc1_byte* origData = reinterpret_cast<const etc1_byte*>(orig);
    if (!etc1_pkm_is_valid(orig)) {
        return NULL;
    }

    etc1_uint32 origWidth = etc1_pkm_get_width(origData);
    etc1_uint32 origHeight = etc1_pkm_get_height(origData);

    // The width and height must be aligned along block boundaries
    static const etc1_uint32 kETC1BlockWidth = 4;
    static const etc1_uint32 kETC1BlockHeight = 4;
    if ((origWidth % kETC1BlockWidth) != 0 ||
        (origHeight % kETC1BlockHeight) != 0) {
        return NULL;
    }

    // The picture must be at least as large as a block.
    if (origWidth <= kETC1BlockWidth || origHeight <= kETC1BlockHeight) {
        return NULL;
    }

    etc1_uint32 newWidth = origWidth * factor;
    etc1_uint32 newHeight = origHeight * factor;

    etc1_uint32 newDataSz = etc1_get_encoded_data_size(newWidth, newHeight);
    etc1_byte* newData = reinterpret_cast<etc1_byte *>(
        sk_malloc_throw(newDataSz + ETC_PKM_HEADER_SIZE));
    etc1_pkm_format_header(newData, newWidth, newHeight);

    etc1_byte* copyInto = newData;

    copyInto += ETC_PKM_HEADER_SIZE;
    origData += ETC_PKM_HEADER_SIZE;

    etc1_uint32 origBlocksX = (origWidth >> 2);
    etc1_uint32 origBlocksY = (origHeight >> 2);
    etc1_uint32 newBlocksY = (newHeight >> 2);
    etc1_uint32 origRowSzInBytes = origBlocksX * ETC1_ENCODED_BLOCK_SIZE;

    for (etc1_uint32 j = 0; j < newBlocksY; ++j) {
        const etc1_byte* rowStart = origData + ((j % origBlocksY) * origRowSzInBytes);
        for(etc1_uint32 i = 0; i < newWidth; i += origWidth) {
            memcpy(copyInto, rowStart, origRowSzInBytes);
            copyInto += origRowSzInBytes;
        }
    }
    return newData;
}

// This is the base class for all of the benches in this file. In general
// the ETC1 benches should all be working on the same data. Due to the
// simplicity of the PKM file, that data is the 128x128 mandrill etc1
// compressed texture repeated by some factor (currently 8 -> 1024x1024)
class ETCBitmapBenchBase : public Benchmark {
public:
    ETCBitmapBenchBase() : fPKMData(loadPKM()) {
        if (NULL == fPKMData) {
            SkDebugf("Could not load PKM data!");
        }
    }

protected:
    SkAutoDataUnref fPKMData;

private:
    SkData* loadPKM() {
        SkString pkmFilename = GetResourcePath("mandrill_128.pkm");
        // Expand the data
        SkAutoDataUnref fileData(SkData::NewFromFileName(pkmFilename.c_str()));
        if (NULL == fileData) {
            SkDebugf("Could not open the file. Did you forget to set the resourcePath?\n");
            return NULL;
        }

        const etc1_uint32 kExpansionFactor = 8;
        etc1_byte* expandedETC1 =
            create_expanded_etc1_bitmap(fileData->bytes(), kExpansionFactor);
        if (NULL == expandedETC1) {
            SkDebugf("Error expanding ETC1 data by factor of %d\n", kExpansionFactor);
            return NULL;
        }

        etc1_uint32 width = etc1_pkm_get_width(expandedETC1);
        etc1_uint32 height = etc1_pkm_get_width(expandedETC1);
        etc1_uint32 dataSz = ETC_PKM_HEADER_SIZE + etc1_get_encoded_data_size(width, height);
        return SkData::NewFromMalloc(expandedETC1, dataSz);
    }

    typedef Benchmark INHERITED;
};

// This is the rendering benchmark. Prior to rendering the data, create a
// bitmap using the etc1 data.
class ETCBitmapBench : public ETCBitmapBenchBase {
public:
    ETCBitmapBench(bool decompress, Backend backend)
        : fDecompress(decompress), fBackend(backend) { }

    bool isSuitableFor(Backend backend) override {
        return backend == this->fBackend;
    }

protected:
    const char* onGetName() override {
        if (kGPU_Backend == this->fBackend) {
            if (this->fDecompress) {
                return "etc1bitmap_render_gpu_decompressed";
            } else {
                return "etc1bitmap_render_gpu_compressed";
            }
        } else {
            SkASSERT(kRaster_Backend == this->fBackend);
            if (this->fDecompress) {
                return "etc1bitmap_render_raster_decompressed";
            } else {
                return "etc1bitmap_render_raster_compressed";
            }
        }
    }

    void onPreDraw() override {
        if (NULL == fPKMData) {
            SkDebugf("Failed to load PKM data!\n");
            return;
        }

        // Install pixel ref
        if (!SkInstallDiscardablePixelRef(fPKMData, &(this->fBitmap))) {
            SkDebugf("Could not install discardable pixel ref.\n");
            return;
        }

        // Decompress it if necessary
        if (this->fDecompress) {
            this->fBitmap.lockPixels();
        }
    }

    void onDraw(const int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; ++i) {
            canvas->drawBitmap(this->fBitmap, 0, 0, NULL);
        }
    }

protected:
    SkBitmap fBitmap;
    bool decompress() const { return fDecompress; }
    Backend backend() const { return fBackend; }
private:
    const bool fDecompress;
    const Backend fBackend;
    typedef ETCBitmapBenchBase INHERITED;
};

// This benchmark is identical to the previous benchmark, but it explicitly forces
// an upload to the GPU before each draw call. We do this by notifying the bitmap
// that the pixels have changed (even though they haven't).
class ETCBitmapUploadBench : public ETCBitmapBench {
public:
    ETCBitmapUploadBench(bool decompress, Backend backend)
        : ETCBitmapBench(decompress, backend) { }

protected:
    const char* onGetName() override {
        if (kGPU_Backend == this->backend()) {
            if (this->decompress()) {
                return "etc1bitmap_upload_gpu_decompressed";
            } else {
                return "etc1bitmap_upload_gpu_compressed";
            }
        } else {
            SkASSERT(kRaster_Backend == this->backend());
            if (this->decompress()) {
                return "etc1bitmap_upload_raster_decompressed";
            } else {
                return "etc1bitmap_upload_raster_compressed";
            }
        }
    }

    void onDraw(const int loops, SkCanvas* canvas) override {
        SkPixelRef* pr = fBitmap.pixelRef();
        for (int i = 0; i < loops; ++i) {
            if (pr) {
                pr->notifyPixelsChanged();
            }
            canvas->drawBitmap(this->fBitmap, 0, 0, NULL);
        }
    }

private:
    typedef ETCBitmapBench INHERITED;
};

DEF_BENCH(return new ETCBitmapBench(false, Benchmark::kRaster_Backend);)
DEF_BENCH(return new ETCBitmapBench(true, Benchmark::kRaster_Backend);)

DEF_BENCH(return new ETCBitmapBench(false, Benchmark::kGPU_Backend);)
DEF_BENCH(return new ETCBitmapBench(true, Benchmark::kGPU_Backend);)

DEF_BENCH(return new ETCBitmapUploadBench(false, Benchmark::kRaster_Backend);)
DEF_BENCH(return new ETCBitmapUploadBench(true, Benchmark::kRaster_Backend);)

DEF_BENCH(return new ETCBitmapUploadBench(false, Benchmark::kGPU_Backend);)
DEF_BENCH(return new ETCBitmapUploadBench(true, Benchmark::kGPU_Backend);)

#endif  // SK_IGNORE_ETC1_SUPPORT
