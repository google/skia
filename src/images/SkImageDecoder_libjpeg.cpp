
/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkJpegUtility.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkScaledBitmapSampler.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkTime.h"
#include "SkUtils.h"
#include "SkRect.h"
#include "SkCanvas.h"

#include <stdio.h>
extern "C" {
    #include "jpeglib.h"
    #include "jerror.h"
}

// Uncomment to enable the code path used by the Android framework with their
// custom image decoders.
//#if defined(SK_BUILD_FOR_ANDROID) && defined(SK_DEBUG)
//  #define SK_BUILD_FOR_ANDROID_FRAMEWORK
//#endif

// These enable timing code that report milliseconds for an encoding/decoding
//#define TIME_ENCODE
//#define TIME_DECODE

// this enables our rgb->yuv code, which is faster than libjpeg on ARM
// disable for the moment, as we have some glitches when width != multiple of 4
#define WE_CONVERT_TO_YUV

// If ANDROID_RGB is defined by in the jpeg headers it indicates that jpeg offers
// support for two additional formats (1) JCS_RGBA_8888 and (2) JCS_RGB_565.

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static void overwrite_mem_buffer_size(j_decompress_ptr cinfo) {
#ifdef SK_BUILD_FOR_ANDROID
    /* Check if the device indicates that it has a large amount of system memory
     * if so, increase the memory allocation to 30MB instead of the default 5MB.
     */
#ifdef ANDROID_LARGE_MEMORY_DEVICE
    cinfo->mem->max_memory_to_use = 30 * 1024 * 1024;
#else
    cinfo->mem->max_memory_to_use = 5 * 1024 * 1024;
#endif
#endif // SK_BUILD_FOR_ANDROID
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class SkJPEGImageIndex {
public:
    SkJPEGImageIndex(SkStream* stream, SkImageDecoder* decoder)
        : fSrcMgr(stream, decoder, true) {}

    ~SkJPEGImageIndex() {
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
        jpeg_destroy_huffman_index(&fHuffmanIndex);
#endif
        jpeg_finish_decompress(&fCInfo);
        jpeg_destroy_decompress(&fCInfo);
    }

    /**
     * Init the cinfo struct using libjpeg and apply any necessary
     * customizations.
     */
    void initializeInfo() {
        jpeg_create_decompress(&fCInfo);
        overwrite_mem_buffer_size(&fCInfo);
        fCInfo.src = &fSrcMgr;
    }

    jpeg_decompress_struct* cinfo() { return &fCInfo; }

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    huffman_index* huffmanIndex() { return &fHuffmanIndex; }
#endif

private:
    skjpeg_source_mgr  fSrcMgr;
    jpeg_decompress_struct fCInfo;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    huffman_index fHuffmanIndex;
#endif
};

class SkJPEGImageDecoder : public SkImageDecoder {
public:
    SkJPEGImageDecoder() {
        fImageIndex = NULL;
        fImageWidth = 0;
        fImageHeight = 0;
    }

    virtual ~SkJPEGImageDecoder() {
        SkDELETE(fImageIndex);
    }

    virtual Format getFormat() const {
        return kJPEG_Format;
    }

protected:
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    virtual bool onBuildTileIndex(SkStream *stream, int *width, int *height) SK_OVERRIDE;
    virtual bool onDecodeRegion(SkBitmap* bitmap, const SkIRect& rect) SK_OVERRIDE;
#endif
    virtual bool onDecode(SkStream* stream, SkBitmap* bm, Mode) SK_OVERRIDE;

private:
    SkJPEGImageIndex* fImageIndex;
    int fImageWidth;
    int fImageHeight;

    typedef SkImageDecoder INHERITED;
};

//////////////////////////////////////////////////////////////////////////

/* Automatically clean up after throwing an exception */
class JPEGAutoClean {
public:
    JPEGAutoClean(): cinfo_ptr(NULL) {}
    ~JPEGAutoClean() {
        if (cinfo_ptr) {
            jpeg_destroy_decompress(cinfo_ptr);
        }
    }
    void set(jpeg_decompress_struct* info) {
        cinfo_ptr = info;
    }
private:
    jpeg_decompress_struct* cinfo_ptr;
};

///////////////////////////////////////////////////////////////////////////////

/*  If we need to better match the request, we might examine the image and
     output dimensions, and determine if the downsampling jpeg provided is
     not sufficient. If so, we can recompute a modified sampleSize value to
     make up the difference.

     To skip this additional scaling, just set sampleSize = 1; below.
 */
static int recompute_sampleSize(int sampleSize,
                                const jpeg_decompress_struct& cinfo) {
    return sampleSize * cinfo.output_width / cinfo.image_width;
}

static bool valid_output_dimensions(const jpeg_decompress_struct& cinfo) {
    /* These are initialized to 0, so if they have non-zero values, we assume
       they are "valid" (i.e. have been computed by libjpeg)
     */
    return 0 != cinfo.output_width && 0 != cinfo.output_height;
}

static bool skip_src_rows(jpeg_decompress_struct* cinfo, void* buffer, int count) {
    for (int i = 0; i < count; i++) {
        JSAMPLE* rowptr = (JSAMPLE*)buffer;
        int row_count = jpeg_read_scanlines(cinfo, &rowptr, 1);
        if (1 != row_count) {
            return false;
        }
    }
    return true;
}

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
static bool skip_src_rows_tile(jpeg_decompress_struct* cinfo,
                               huffman_index *index, void* buffer, int count) {
    for (int i = 0; i < count; i++) {
        JSAMPLE* rowptr = (JSAMPLE*)buffer;
        int row_count = jpeg_read_tile_scanline(cinfo, index, &rowptr);
        if (1 != row_count) {
            return false;
        }
    }
    return true;
}
#endif

// This guy exists just to aid in debugging, as it allows debuggers to just
// set a break-point in one place to see all error exists.
static bool return_false(const jpeg_decompress_struct& cinfo,
                         const SkBitmap& bm, const char msg[]) {
#ifdef SK_DEBUG
    SkDebugf("libjpeg error %d <%s> from %s [%d %d]", cinfo.err->msg_code,
             cinfo.err->jpeg_message_table[cinfo.err->msg_code], msg,
             bm.width(), bm.height());
#endif
    return false;   // must always return false
}

// Convert a scanline of CMYK samples to RGBX in place. Note that this
// method moves the "scanline" pointer in its processing
static void convert_CMYK_to_RGB(uint8_t* scanline, unsigned int width) {
    // At this point we've received CMYK pixels from libjpeg. We
    // perform a crude conversion to RGB (based on the formulae
    // from easyrgb.com):
    //  CMYK -> CMY
    //    C = ( C * (1 - K) + K )      // for each CMY component
    //  CMY -> RGB
    //    R = ( 1 - C ) * 255          // for each RGB component
    // Unfortunately we are seeing inverted CMYK so all the original terms
    // are 1-. This yields:
    //  CMYK -> CMY
    //    C = ( (1-C) * (1 - (1-K) + (1-K) ) -> C = 1 - C*K
    // The conversion from CMY->RGB remains the same
    for (unsigned int x = 0; x < width; ++x, scanline += 4) {
        scanline[0] = SkMulDiv255Round(scanline[0], scanline[3]);
        scanline[1] = SkMulDiv255Round(scanline[1], scanline[3]);
        scanline[2] = SkMulDiv255Round(scanline[2], scanline[3]);
        scanline[3] = 255;
    }
}

bool SkJPEGImageDecoder::onDecode(SkStream* stream, SkBitmap* bm, Mode mode) {
#ifdef TIME_DECODE
    SkAutoTime atm("JPEG Decode");
#endif

    JPEGAutoClean autoClean;

    jpeg_decompress_struct  cinfo;
    skjpeg_error_mgr        errorManager;
    skjpeg_source_mgr       srcManager(stream, this, false);

    cinfo.err = jpeg_std_error(&errorManager);
    errorManager.error_exit = skjpeg_error_exit;

    // All objects need to be instantiated before this setjmp call so that
    // they will be cleaned up properly if an error occurs.
    if (setjmp(errorManager.fJmpBuf)) {
        return return_false(cinfo, *bm, "setjmp");
    }

    jpeg_create_decompress(&cinfo);
    autoClean.set(&cinfo);

    overwrite_mem_buffer_size(&cinfo);

    //jpeg_stdio_src(&cinfo, file);
    cinfo.src = &srcManager;

    int status = jpeg_read_header(&cinfo, true);
    if (status != JPEG_HEADER_OK) {
        return return_false(cinfo, *bm, "read_header");
    }

    /*  Try to fulfill the requested sampleSize. Since jpeg can do it (when it
        can) much faster that we, just use their num/denom api to approximate
        the size.
    */
    int sampleSize = this->getSampleSize();

    if (this->getPreferQualityOverSpeed()) {
        cinfo.dct_method = JDCT_ISLOW;
    } else {
        cinfo.dct_method = JDCT_IFAST;
    }

    cinfo.scale_num = 1;
    cinfo.scale_denom = sampleSize;

    /* this gives about 30% performance improvement. In theory it may
       reduce the visual quality, in practice I'm not seeing a difference
     */
    cinfo.do_fancy_upsampling = 0;

    /* this gives another few percents */
    cinfo.do_block_smoothing = 0;

    /* default format is RGB */
    if (cinfo.jpeg_color_space == JCS_CMYK) {
        // libjpeg cannot convert from CMYK to RGB - here we set up
        // so libjpeg will give us CMYK samples back and we will
        // later manually convert them to RGB
        cinfo.out_color_space = JCS_CMYK;
    } else {
        cinfo.out_color_space = JCS_RGB;
    }

    SkBitmap::Config config = this->getPrefConfig(k32Bit_SrcDepth, false);
    // only these make sense for jpegs
    if (config != SkBitmap::kARGB_8888_Config &&
        config != SkBitmap::kARGB_4444_Config &&
        config != SkBitmap::kRGB_565_Config) {
        config = SkBitmap::kARGB_8888_Config;
    }

#ifdef ANDROID_RGB
    cinfo.dither_mode = JDITHER_NONE;
    if (SkBitmap::kARGB_8888_Config == config && JCS_CMYK != cinfo.out_color_space) {
        cinfo.out_color_space = JCS_RGBA_8888;
    } else if (SkBitmap::kRGB_565_Config == config && JCS_CMYK != cinfo.out_color_space) {
        cinfo.out_color_space = JCS_RGB_565;
        if (this->getDitherImage()) {
            cinfo.dither_mode = JDITHER_ORDERED;
        }
    }
#endif

    if (1 == sampleSize && SkImageDecoder::kDecodeBounds_Mode == mode) {
        bm->setConfig(config, cinfo.image_width, cinfo.image_height);
        bm->setIsOpaque(true);
        return true;
    }

    /*  image_width and image_height are the original dimensions, available
        after jpeg_read_header(). To see the scaled dimensions, we have to call
        jpeg_start_decompress(), and then read output_width and output_height.
    */
    if (!jpeg_start_decompress(&cinfo)) {
        /*  If we failed here, we may still have enough information to return
            to the caller if they just wanted (subsampled bounds). If sampleSize
            was 1, then we would have already returned. Thus we just check if
            we're in kDecodeBounds_Mode, and that we have valid output sizes.

            One reason to fail here is that we have insufficient stream data
            to complete the setup. However, output dimensions seem to get
            computed very early, which is why this special check can pay off.
         */
        if (SkImageDecoder::kDecodeBounds_Mode == mode && valid_output_dimensions(cinfo)) {
            SkScaledBitmapSampler smpl(cinfo.output_width, cinfo.output_height,
                                       recompute_sampleSize(sampleSize, cinfo));
            bm->setConfig(config, smpl.scaledWidth(), smpl.scaledHeight());
            bm->setIsOpaque(true);
            return true;
        } else {
            return return_false(cinfo, *bm, "start_decompress");
        }
    }
    sampleSize = recompute_sampleSize(sampleSize, cinfo);

    // should we allow the Chooser (if present) to pick a config for us???
    if (!this->chooseFromOneChoice(config, cinfo.output_width, cinfo.output_height)) {
        return return_false(cinfo, *bm, "chooseFromOneChoice");
    }

    SkScaledBitmapSampler sampler(cinfo.output_width, cinfo.output_height, sampleSize);

    bm->lockPixels();
    JSAMPLE* rowptr = (JSAMPLE*)bm->getPixels();
    bm->unlockPixels();
    bool reuseBitmap = (rowptr != NULL);

    if (reuseBitmap) {
        if (sampler.scaledWidth() != bm->width() ||
            sampler.scaledHeight() != bm->height()) {
            // Dimensions must match
            return false;
        } else if (SkImageDecoder::kDecodeBounds_Mode == mode) {
            return true;
        }
    } else {
        bm->setConfig(config, sampler.scaledWidth(), sampler.scaledHeight());
        bm->setIsOpaque(true);
        if (SkImageDecoder::kDecodeBounds_Mode == mode) {
            return true;
        }
        if (!this->allocPixelRef(bm, NULL)) {
            return return_false(cinfo, *bm, "allocPixelRef");
        }
    }

    SkAutoLockPixels alp(*bm);

#ifdef ANDROID_RGB
    /* short-circuit the SkScaledBitmapSampler when possible, as this gives
       a significant performance boost.
    */
    if (sampleSize == 1 &&
        ((config == SkBitmap::kARGB_8888_Config &&
                cinfo.out_color_space == JCS_RGBA_8888) ||
        (config == SkBitmap::kRGB_565_Config &&
                cinfo.out_color_space == JCS_RGB_565)))
    {
        rowptr = (JSAMPLE*)bm->getPixels();
        INT32 const bpr =  bm->rowBytes();

        while (cinfo.output_scanline < cinfo.output_height) {
            int row_count = jpeg_read_scanlines(&cinfo, &rowptr, 1);
            // if row_count == 0, then we didn't get a scanline, so abort.
            // if we supported partial images, we might return true in this case
            if (0 == row_count) {
                return return_false(cinfo, *bm, "read_scanlines");
            }
            if (this->shouldCancelDecode()) {
                return return_false(cinfo, *bm, "shouldCancelDecode");
            }
            rowptr += bpr;
        }
        if (reuseBitmap) {
            bm->notifyPixelsChanged();
        }
        jpeg_finish_decompress(&cinfo);
        return true;
    }
#endif

    // check for supported formats
    SkScaledBitmapSampler::SrcConfig sc;
    if (JCS_CMYK == cinfo.out_color_space) {
        // In this case we will manually convert the CMYK values to RGB
        sc = SkScaledBitmapSampler::kRGBX;
    } else if (3 == cinfo.out_color_components && JCS_RGB == cinfo.out_color_space) {
        sc = SkScaledBitmapSampler::kRGB;
#ifdef ANDROID_RGB
    } else if (JCS_RGBA_8888 == cinfo.out_color_space) {
        sc = SkScaledBitmapSampler::kRGBX;
    } else if (JCS_RGB_565 == cinfo.out_color_space) {
        sc = SkScaledBitmapSampler::kRGB_565;
#endif
    } else if (1 == cinfo.out_color_components &&
               JCS_GRAYSCALE == cinfo.out_color_space) {
        sc = SkScaledBitmapSampler::kGray;
    } else {
        return return_false(cinfo, *bm, "jpeg colorspace");
    }

    if (!sampler.begin(bm, sc, this->getDitherImage())) {
        return return_false(cinfo, *bm, "sampler.begin");
    }

    // The CMYK work-around relies on 4 components per pixel here
    SkAutoMalloc srcStorage(cinfo.output_width * 4);
    uint8_t* srcRow = (uint8_t*)srcStorage.get();

    //  Possibly skip initial rows [sampler.srcY0]
    if (!skip_src_rows(&cinfo, srcRow, sampler.srcY0())) {
        return return_false(cinfo, *bm, "skip rows");
    }

    // now loop through scanlines until y == bm->height() - 1
    for (int y = 0;; y++) {
        JSAMPLE* rowptr = (JSAMPLE*)srcRow;
        int row_count = jpeg_read_scanlines(&cinfo, &rowptr, 1);
        if (0 == row_count) {
            return return_false(cinfo, *bm, "read_scanlines");
        }
        if (this->shouldCancelDecode()) {
            return return_false(cinfo, *bm, "shouldCancelDecode");
        }

        if (JCS_CMYK == cinfo.out_color_space) {
            convert_CMYK_to_RGB(srcRow, cinfo.output_width);
        }

        sampler.next(srcRow);
        if (bm->height() - 1 == y) {
            // we're done
            break;
        }

        if (!skip_src_rows(&cinfo, srcRow, sampler.srcDY() - 1)) {
            return return_false(cinfo, *bm, "skip rows");
        }
    }

    // we formally skip the rest, so we don't get a complaint from libjpeg
    if (!skip_src_rows(&cinfo, srcRow,
                       cinfo.output_height - cinfo.output_scanline)) {
        return return_false(cinfo, *bm, "skip rows");
    }
    if (reuseBitmap) {
        bm->notifyPixelsChanged();
    }
    jpeg_finish_decompress(&cinfo);

    return true;
}

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
bool SkJPEGImageDecoder::onBuildTileIndex(SkStream* stream, int *width, int *height) {

    SkJPEGImageIndex* imageIndex = SkNEW_ARGS(SkJPEGImageIndex, (stream, this));
    jpeg_decompress_struct* cinfo = imageIndex->cinfo();
    huffman_index* huffmanIndex = imageIndex->huffmanIndex();

    skjpeg_error_mgr sk_err;
    cinfo->err = jpeg_std_error(&sk_err);
    sk_err.error_exit = skjpeg_error_exit;

    // All objects need to be instantiated before this setjmp call so that
    // they will be cleaned up properly if an error occurs.
    if (setjmp(sk_err.fJmpBuf)) {
        return false;
    }

    // create the cinfo used to create/build the huffmanIndex
    imageIndex->initializeInfo();
    cinfo->do_fancy_upsampling = 0;
    cinfo->do_block_smoothing = 0;

    int status = jpeg_read_header(cinfo, true);
    if (JPEG_HEADER_OK != status) {
        SkDELETE(imageIndex);
        return false;
    }

    jpeg_create_huffman_index(cinfo, huffmanIndex);
    cinfo->scale_num = 1;
    cinfo->scale_denom = 1;
    if (!jpeg_build_huffman_index(cinfo, huffmanIndex)) {
        SkDELETE(imageIndex);
        return false;
    }

    // destroy the cinfo used to create/build the huffman index
    jpeg_destroy_decompress(cinfo);

    // Init decoder to image decode mode
    imageIndex->initializeInfo();

    status = jpeg_read_header(cinfo, true);
    if (JPEG_HEADER_OK != status) {
        SkDELETE(imageIndex);
        return false;
    }

    cinfo->out_color_space = JCS_RGBA_8888;
    cinfo->do_fancy_upsampling = 0;
    cinfo->do_block_smoothing = 0;

    // instead of jpeg_start_decompress() we start a tiled decompress
    jpeg_start_tile_decompress(cinfo);

    cinfo->scale_num = 1;
    *height = cinfo->output_height;
    *width = cinfo->output_width;
    fImageWidth = *width;
    fImageHeight = *height;

    SkDELETE(fImageIndex);
    fImageIndex = imageIndex;

    return true;
}

bool SkJPEGImageDecoder::onDecodeRegion(SkBitmap* bm, const SkIRect& region) {
    if (NULL == fImageIndex) {
        return false;
    }
    jpeg_decompress_struct* cinfo = fImageIndex->cinfo();

    SkIRect rect = SkIRect::MakeWH(fImageWidth, fImageHeight);
    if (!rect.intersect(region)) {
        // If the requested region is entirely outside the image return false
        return false;
    }


    skjpeg_error_mgr errorManager;
    cinfo->err = jpeg_std_error(&errorManager);
    errorManager.error_exit = skjpeg_error_exit;
    if (setjmp(errorManager.fJmpBuf)) {
        return false;
    }

    int requestedSampleSize = this->getSampleSize();
    cinfo->scale_denom = requestedSampleSize;

    if (this->getPreferQualityOverSpeed()) {
        cinfo->dct_method = JDCT_ISLOW;
    } else {
        cinfo->dct_method = JDCT_IFAST;
    }

    SkBitmap::Config config = this->getPrefConfig(k32Bit_SrcDepth, false);
    if (config != SkBitmap::kARGB_8888_Config &&
        config != SkBitmap::kARGB_4444_Config &&
        config != SkBitmap::kRGB_565_Config) {
        config = SkBitmap::kARGB_8888_Config;
    }

    /* default format is RGB */
    cinfo->out_color_space = JCS_RGB;

#ifdef ANDROID_RGB
    cinfo->dither_mode = JDITHER_NONE;
    if (SkBitmap::kARGB_8888_Config == config) {
        cinfo->out_color_space = JCS_RGBA_8888;
    } else if (SkBitmap::kRGB_565_Config == config) {
        cinfo->out_color_space = JCS_RGB_565;
        if (this->getDitherImage()) {
            cinfo->dither_mode = JDITHER_ORDERED;
        }
    }
#endif

    int startX = rect.fLeft;
    int startY = rect.fTop;
    int width = rect.width();
    int height = rect.height();

    jpeg_init_read_tile_scanline(cinfo, fImageIndex->huffmanIndex(),
                                 &startX, &startY, &width, &height);
    int skiaSampleSize = recompute_sampleSize(requestedSampleSize, *cinfo);
    int actualSampleSize = skiaSampleSize * (DCTSIZE / cinfo->min_DCT_scaled_size);

    SkScaledBitmapSampler sampler(width, height, skiaSampleSize);

    SkBitmap bitmap;
    bitmap.setConfig(config, sampler.scaledWidth(), sampler.scaledHeight());
    bitmap.setIsOpaque(true);

    // Check ahead of time if the swap(dest, src) is possible or not.
    // If yes, then we will stick to AllocPixelRef since it's cheaper with the
    // swap happening. If no, then we will use alloc to allocate pixels to
    // prevent garbage collection.
    int w = rect.width() / actualSampleSize;
    int h = rect.height() / actualSampleSize;
    bool swapOnly = (rect == region) && bm->isNull() &&
                    (w == bitmap.width()) && (h == bitmap.height()) &&
                    ((startX - rect.x()) / actualSampleSize == 0) &&
                    ((startY - rect.y()) / actualSampleSize == 0);
    if (swapOnly) {
        if (!this->allocPixelRef(&bitmap, NULL)) {
            return return_false(*cinfo, bitmap, "allocPixelRef");
        }
    } else {
        if (!bitmap.allocPixels()) {
            return return_false(*cinfo, bitmap, "allocPixels");
        }
    }

    SkAutoLockPixels alp(bitmap);

#ifdef ANDROID_RGB
    /* short-circuit the SkScaledBitmapSampler when possible, as this gives
       a significant performance boost.
    */
    if (skiaSampleSize == 1 &&
        ((config == SkBitmap::kARGB_8888_Config &&
                cinfo->out_color_space == JCS_RGBA_8888) ||
        (config == SkBitmap::kRGB_565_Config &&
                cinfo->out_color_space == JCS_RGB_565)))
    {
        JSAMPLE* rowptr = (JSAMPLE*)bitmap.getPixels();
        INT32 const bpr = bitmap.rowBytes();
        int rowTotalCount = 0;

        while (rowTotalCount < height) {
            int rowCount = jpeg_read_tile_scanline(cinfo,
                                                   fImageIndex->huffmanIndex(),
                                                   &rowptr);
            // if row_count == 0, then we didn't get a scanline, so abort.
            // if we supported partial images, we might return true in this case
            if (0 == rowCount) {
                return return_false(*cinfo, bitmap, "read_scanlines");
            }
            if (this->shouldCancelDecode()) {
                return return_false(*cinfo, bitmap, "shouldCancelDecode");
            }
            rowTotalCount += rowCount;
            rowptr += bpr;
        }

        if (swapOnly) {
            bm->swap(bitmap);
        } else {
            cropBitmap(bm, &bitmap, actualSampleSize, region.x(), region.y(),
                       region.width(), region.height(), startX, startY);
        }
        return true;
    }
#endif

    // check for supported formats
    SkScaledBitmapSampler::SrcConfig sc;
    if (JCS_CMYK == cinfo->out_color_space) {
        // In this case we will manually convert the CMYK values to RGB
        sc = SkScaledBitmapSampler::kRGBX;
    } else if (3 == cinfo->out_color_components && JCS_RGB == cinfo->out_color_space) {
        sc = SkScaledBitmapSampler::kRGB;
#ifdef ANDROID_RGB
    } else if (JCS_RGBA_8888 == cinfo->out_color_space) {
        sc = SkScaledBitmapSampler::kRGBX;
    } else if (JCS_RGB_565 == cinfo->out_color_space) {
        sc = SkScaledBitmapSampler::kRGB_565;
#endif
    } else if (1 == cinfo->out_color_components &&
               JCS_GRAYSCALE == cinfo->out_color_space) {
        sc = SkScaledBitmapSampler::kGray;
    } else {
        return return_false(*cinfo, *bm, "jpeg colorspace");
    }

    if (!sampler.begin(&bitmap, sc, this->getDitherImage())) {
        return return_false(*cinfo, bitmap, "sampler.begin");
    }

    // The CMYK work-around relies on 4 components per pixel here
    SkAutoMalloc  srcStorage(width * 4);
    uint8_t* srcRow = (uint8_t*)srcStorage.get();

    //  Possibly skip initial rows [sampler.srcY0]
    if (!skip_src_rows_tile(cinfo, fImageIndex->huffmanIndex(), srcRow, sampler.srcY0())) {
        return return_false(*cinfo, bitmap, "skip rows");
    }

    // now loop through scanlines until y == bitmap->height() - 1
    for (int y = 0;; y++) {
        JSAMPLE* rowptr = (JSAMPLE*)srcRow;
        int row_count = jpeg_read_tile_scanline(cinfo, fImageIndex->huffmanIndex(), &rowptr);
        if (0 == row_count) {
            return return_false(*cinfo, bitmap, "read_scanlines");
        }
        if (this->shouldCancelDecode()) {
            return return_false(*cinfo, bitmap, "shouldCancelDecode");
        }

        if (JCS_CMYK == cinfo->out_color_space) {
            convert_CMYK_to_RGB(srcRow, width);
        }

        sampler.next(srcRow);
        if (bitmap.height() - 1 == y) {
            // we're done
            break;
        }

        if (!skip_src_rows_tile(cinfo, fImageIndex->huffmanIndex(), srcRow,
                                sampler.srcDY() - 1)) {
            return return_false(*cinfo, bitmap, "skip rows");
        }
    }
    if (swapOnly) {
        bm->swap(bitmap);
    } else {
        cropBitmap(bm, &bitmap, actualSampleSize, region.x(), region.y(),
                   region.width(), region.height(), startX, startY);
    }
    return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////

#include "SkColorPriv.h"

// taken from jcolor.c in libjpeg
#if 0   // 16bit - precise but slow
    #define CYR     19595   // 0.299
    #define CYG     38470   // 0.587
    #define CYB      7471   // 0.114

    #define CUR    -11059   // -0.16874
    #define CUG    -21709   // -0.33126
    #define CUB     32768   // 0.5

    #define CVR     32768   // 0.5
    #define CVG    -27439   // -0.41869
    #define CVB     -5329   // -0.08131

    #define CSHIFT  16
#else      // 8bit - fast, slightly less precise
    #define CYR     77    // 0.299
    #define CYG     150    // 0.587
    #define CYB      29    // 0.114

    #define CUR     -43    // -0.16874
    #define CUG    -85    // -0.33126
    #define CUB     128    // 0.5

    #define CVR      128   // 0.5
    #define CVG     -107   // -0.41869
    #define CVB      -21   // -0.08131

    #define CSHIFT  8
#endif

static void rgb2yuv_32(uint8_t dst[], SkPMColor c) {
    int r = SkGetPackedR32(c);
    int g = SkGetPackedG32(c);
    int b = SkGetPackedB32(c);

    int  y = ( CYR*r + CYG*g + CYB*b ) >> CSHIFT;
    int  u = ( CUR*r + CUG*g + CUB*b ) >> CSHIFT;
    int  v = ( CVR*r + CVG*g + CVB*b ) >> CSHIFT;

    dst[0] = SkToU8(y);
    dst[1] = SkToU8(u + 128);
    dst[2] = SkToU8(v + 128);
}

static void rgb2yuv_4444(uint8_t dst[], U16CPU c) {
    int r = SkGetPackedR4444(c);
    int g = SkGetPackedG4444(c);
    int b = SkGetPackedB4444(c);

    int  y = ( CYR*r + CYG*g + CYB*b ) >> (CSHIFT - 4);
    int  u = ( CUR*r + CUG*g + CUB*b ) >> (CSHIFT - 4);
    int  v = ( CVR*r + CVG*g + CVB*b ) >> (CSHIFT - 4);

    dst[0] = SkToU8(y);
    dst[1] = SkToU8(u + 128);
    dst[2] = SkToU8(v + 128);
}

static void rgb2yuv_16(uint8_t dst[], U16CPU c) {
    int r = SkGetPackedR16(c);
    int g = SkGetPackedG16(c);
    int b = SkGetPackedB16(c);

    int  y = ( 2*CYR*r + CYG*g + 2*CYB*b ) >> (CSHIFT - 2);
    int  u = ( 2*CUR*r + CUG*g + 2*CUB*b ) >> (CSHIFT - 2);
    int  v = ( 2*CVR*r + CVG*g + 2*CVB*b ) >> (CSHIFT - 2);

    dst[0] = SkToU8(y);
    dst[1] = SkToU8(u + 128);
    dst[2] = SkToU8(v + 128);
}

///////////////////////////////////////////////////////////////////////////////

typedef void (*WriteScanline)(uint8_t* SK_RESTRICT dst,
                              const void* SK_RESTRICT src, int width,
                              const SkPMColor* SK_RESTRICT ctable);

static void Write_32_YUV(uint8_t* SK_RESTRICT dst,
                         const void* SK_RESTRICT srcRow, int width,
                         const SkPMColor*) {
    const uint32_t* SK_RESTRICT src = (const uint32_t*)srcRow;
    while (--width >= 0) {
#ifdef WE_CONVERT_TO_YUV
        rgb2yuv_32(dst, *src++);
#else
        uint32_t c = *src++;
        dst[0] = SkGetPackedR32(c);
        dst[1] = SkGetPackedG32(c);
        dst[2] = SkGetPackedB32(c);
#endif
        dst += 3;
    }
}

static void Write_4444_YUV(uint8_t* SK_RESTRICT dst,
                           const void* SK_RESTRICT srcRow, int width,
                           const SkPMColor*) {
    const SkPMColor16* SK_RESTRICT src = (const SkPMColor16*)srcRow;
    while (--width >= 0) {
#ifdef WE_CONVERT_TO_YUV
        rgb2yuv_4444(dst, *src++);
#else
        SkPMColor16 c = *src++;
        dst[0] = SkPacked4444ToR32(c);
        dst[1] = SkPacked4444ToG32(c);
        dst[2] = SkPacked4444ToB32(c);
#endif
        dst += 3;
    }
}

static void Write_16_YUV(uint8_t* SK_RESTRICT dst,
                         const void* SK_RESTRICT srcRow, int width,
                         const SkPMColor*) {
    const uint16_t* SK_RESTRICT src = (const uint16_t*)srcRow;
    while (--width >= 0) {
#ifdef WE_CONVERT_TO_YUV
        rgb2yuv_16(dst, *src++);
#else
        uint16_t c = *src++;
        dst[0] = SkPacked16ToR32(c);
        dst[1] = SkPacked16ToG32(c);
        dst[2] = SkPacked16ToB32(c);
#endif
        dst += 3;
    }
}

static void Write_Index_YUV(uint8_t* SK_RESTRICT dst,
                            const void* SK_RESTRICT srcRow, int width,
                            const SkPMColor* SK_RESTRICT ctable) {
    const uint8_t* SK_RESTRICT src = (const uint8_t*)srcRow;
    while (--width >= 0) {
#ifdef WE_CONVERT_TO_YUV
        rgb2yuv_32(dst, ctable[*src++]);
#else
        uint32_t c = ctable[*src++];
        dst[0] = SkGetPackedR32(c);
        dst[1] = SkGetPackedG32(c);
        dst[2] = SkGetPackedB32(c);
#endif
        dst += 3;
    }
}

static WriteScanline ChooseWriter(const SkBitmap& bm) {
    switch (bm.config()) {
        case SkBitmap::kARGB_8888_Config:
            return Write_32_YUV;
        case SkBitmap::kRGB_565_Config:
            return Write_16_YUV;
        case SkBitmap::kARGB_4444_Config:
            return Write_4444_YUV;
        case SkBitmap::kIndex8_Config:
            return Write_Index_YUV;
        default:
            return NULL;
    }
}

class SkJPEGImageEncoder : public SkImageEncoder {
protected:
    virtual bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality) {
#ifdef TIME_ENCODE
        SkAutoTime atm("JPEG Encode");
#endif

        const WriteScanline writer = ChooseWriter(bm);
        if (NULL == writer) {
            return false;
        }

        SkAutoLockPixels alp(bm);
        if (NULL == bm.getPixels()) {
            return false;
        }

        jpeg_compress_struct    cinfo;
        skjpeg_error_mgr        sk_err;
        skjpeg_destination_mgr  sk_wstream(stream);

        // allocate these before set call setjmp
        SkAutoMalloc    oneRow;
        SkAutoLockColors ctLocker;

        cinfo.err = jpeg_std_error(&sk_err);
        sk_err.error_exit = skjpeg_error_exit;
        if (setjmp(sk_err.fJmpBuf)) {
            return false;
        }
        jpeg_create_compress(&cinfo);

        cinfo.dest = &sk_wstream;
        cinfo.image_width = bm.width();
        cinfo.image_height = bm.height();
        cinfo.input_components = 3;
#ifdef WE_CONVERT_TO_YUV
        cinfo.in_color_space = JCS_YCbCr;
#else
        cinfo.in_color_space = JCS_RGB;
#endif
        cinfo.input_gamma = 1;

        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
        cinfo.dct_method = JDCT_IFAST;

        jpeg_start_compress(&cinfo, TRUE);

        const int       width = bm.width();
        uint8_t*        oneRowP = (uint8_t*)oneRow.reset(width * 3);

        const SkPMColor* colors = ctLocker.lockColors(bm);
        const void*      srcRow = bm.getPixels();

        while (cinfo.next_scanline < cinfo.image_height) {
            JSAMPROW row_pointer[1];    /* pointer to JSAMPLE row[s] */

            writer(oneRowP, srcRow, width, colors);
            row_pointer[0] = oneRowP;
            (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
            srcRow = (const void*)((const char*)srcRow + bm.rowBytes());
        }

        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);

        return true;
    }
};

///////////////////////////////////////////////////////////////////////////////
DEFINE_DECODER_CREATOR(JPEGImageDecoder);
DEFINE_ENCODER_CREATOR(JPEGImageEncoder);
///////////////////////////////////////////////////////////////////////////////

#include "SkTRegistry.h"

static SkImageDecoder* sk_libjpeg_dfactory(SkStream* stream) {
    static const unsigned char gHeader[] = { 0xFF, 0xD8, 0xFF };
    static const size_t HEADER_SIZE = sizeof(gHeader);

    char buffer[HEADER_SIZE];
    size_t len = stream->read(buffer, HEADER_SIZE);

    if (len != HEADER_SIZE) {
        return NULL;   // can't read enough
    }
    if (memcmp(buffer, gHeader, HEADER_SIZE)) {
        return NULL;
    }
    return SkNEW(SkJPEGImageDecoder);
}

static SkImageEncoder* sk_libjpeg_efactory(SkImageEncoder::Type t) {
    return (SkImageEncoder::kJPEG_Type == t) ? SkNEW(SkJPEGImageEncoder) : NULL;
}


static SkTRegistry<SkImageDecoder*, SkStream*> gDReg(sk_libjpeg_dfactory);
static SkTRegistry<SkImageEncoder*, SkImageEncoder::Type> gEReg(sk_libjpeg_efactory);
