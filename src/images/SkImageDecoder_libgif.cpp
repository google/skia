/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkColorTable.h"
#include "SkImageDecoder.h"
#include "SkRTConf.h"
#include "SkScaledBitmapSampler.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkUtils.h"

#include "gif_lib.h"

class SkGIFImageDecoder : public SkImageDecoder {
public:
    Format getFormat() const override {
        return kGIF_Format;
    }

protected:
    Result onDecode(SkStream* stream, SkBitmap* bm, Mode mode) override;

private:
    typedef SkImageDecoder INHERITED;
};

static const uint8_t gStartingIterlaceYValue[] = {
    0, 4, 2, 1
};
static const uint8_t gDeltaIterlaceYValue[] = {
    8, 8, 4, 2
};

SK_CONF_DECLARE(bool, c_suppressGIFImageDecoderWarnings,
                "images.gif.suppressDecoderWarnings", true,
                "Suppress GIF warnings and errors when calling image decode "
                "functions.");


/*  Implement the GIF interlace algorithm in an iterator.
    1) grab every 8th line beginning at 0
    2) grab every 8th line beginning at 4
    3) grab every 4th line beginning at 2
    4) grab every 2nd line beginning at 1
*/
class GifInterlaceIter {
public:
    GifInterlaceIter(int height) : fHeight(height) {
        fStartYPtr = gStartingIterlaceYValue;
        fDeltaYPtr = gDeltaIterlaceYValue;

        fCurrY = *fStartYPtr++;
        fDeltaY = *fDeltaYPtr++;
    }

    int currY() const {
        SkASSERT(fStartYPtr);
        SkASSERT(fDeltaYPtr);
        return fCurrY;
    }

    void next() {
        SkASSERT(fStartYPtr);
        SkASSERT(fDeltaYPtr);

        int y = fCurrY + fDeltaY;
        // We went from an if statement to a while loop so that we iterate
        // through fStartYPtr until a valid row is found. This is so that images
        // that are smaller than 5x5 will not trash memory.
        while (y >= fHeight) {
            if (gStartingIterlaceYValue +
                    SK_ARRAY_COUNT(gStartingIterlaceYValue) == fStartYPtr) {
                // we done
                SkDEBUGCODE(fStartYPtr = NULL;)
                SkDEBUGCODE(fDeltaYPtr = NULL;)
                y = 0;
            } else {
                y = *fStartYPtr++;
                fDeltaY = *fDeltaYPtr++;
            }
        }
        fCurrY = y;
    }

private:
    const int fHeight;
    int fCurrY;
    int fDeltaY;
    const uint8_t* fStartYPtr;
    const uint8_t* fDeltaYPtr;
};

///////////////////////////////////////////////////////////////////////////////

static int DecodeCallBackProc(GifFileType* fileType, GifByteType* out,
                              int size) {
    SkStream* stream = (SkStream*) fileType->UserData;
    return (int) stream->read(out, size);
}

void CheckFreeExtension(SavedImage* Image) {
    if (Image->ExtensionBlocks) {
#if GIFLIB_MAJOR < 5
        FreeExtension(Image);
#else
        GifFreeExtensions(&Image->ExtensionBlockCount, &Image->ExtensionBlocks);
#endif
    }
}

// return NULL on failure
static const ColorMapObject* find_colormap(const GifFileType* gif) {
    const ColorMapObject* cmap = gif->Image.ColorMap;
    if (NULL == cmap) {
        cmap = gif->SColorMap;
    }

    if (NULL == cmap) {
        // no colormap found
        return NULL;
    }
    // some sanity checks
    if (cmap && ((unsigned)cmap->ColorCount > 256 ||
                 cmap->ColorCount != (1 << cmap->BitsPerPixel))) {
        cmap = NULL;
    }
    return cmap;
}

// return -1 if not found (i.e. we're completely opaque)
static int find_transpIndex(const SavedImage& image, int colorCount) {
    int transpIndex = -1;
    for (int i = 0; i < image.ExtensionBlockCount; ++i) {
        const ExtensionBlock* eb = image.ExtensionBlocks + i;
        if (eb->Function == 0xF9 && eb->ByteCount == 4) {
            if (eb->Bytes[0] & 1) {
                transpIndex = (unsigned char)eb->Bytes[3];
                // check for valid transpIndex
                if (transpIndex >= colorCount) {
                    transpIndex = -1;
                }
                break;
            }
        }
    }
    return transpIndex;
}

static SkImageDecoder::Result error_return(const SkBitmap& bm, const char msg[]) {
    if (!c_suppressGIFImageDecoderWarnings) {
        SkDebugf("libgif error [%s] bitmap [%d %d] pixels %p colortable %p\n",
                 msg, bm.width(), bm.height(), bm.getPixels(),
                 bm.getColorTable());
    }
    return SkImageDecoder::kFailure;
}

static void gif_warning(const SkBitmap& bm, const char msg[]) {
    if (!c_suppressGIFImageDecoderWarnings) {
        SkDebugf("libgif warning [%s] bitmap [%d %d] pixels %p colortable %p\n",
                 msg, bm.width(), bm.height(), bm.getPixels(),
                 bm.getColorTable());
    }
}

/**
 *  Skip rows in the source gif image.
 *  @param gif Source image.
 *  @param dst Scratch output needed by gif library call. Must be >= width bytes.
 *  @param width Bytes per row in the source image.
 *  @param rowsToSkip Number of rows to skip.
 *  @return True on success, false on GIF_ERROR.
 */
static bool skip_src_rows(GifFileType* gif, uint8_t* dst, int width, int rowsToSkip) {
    for (int i = 0; i < rowsToSkip; i++) {
        if (DGifGetLine(gif, dst, width) == GIF_ERROR) {
            return false;
        }
    }
    return true;
}

/**
 *  GIFs with fewer then 256 color entries will sometimes index out of
 *  bounds of the color table (this is malformed, but libgif does not
 *  check sicne it is rare).  This function checks for this error and
 *  fixes it.  This makes the output image consistantly deterministic.
 */
static void sanitize_indexed_bitmap(SkBitmap* bm) {
    if ((kIndex_8_SkColorType == bm->colorType()) && !(bm->empty())) {
        SkAutoLockPixels alp(*bm);
        if (bm->getPixels()) {
            SkColorTable* ct = bm->getColorTable();  // Index8 must have it.
            SkASSERT(ct != NULL);
            uint32_t count = ct->count();
            SkASSERT(count > 0);
            SkASSERT(count <= 0x100);
            if (count != 0x100) {  // Full colortables can't go wrong.
                // Count is a power of 2; asserted elsewhere.
                uint8_t byteMask = (~(count - 1));
                bool warning = false;
                uint8_t* addr = static_cast<uint8_t*>(bm->getPixels());
                int height = bm->height();
                int width = bm->width();
                size_t rowBytes = bm->rowBytes();
                while (--height >= 0) {
                    uint8_t* ptr = addr;
                    int x = width;
                    while (--x >= 0) {
                        if (0 != ((*ptr) & byteMask)) {
                            warning = true;
                            *ptr = 0;
                        }
                        ++ptr;
                    }
                    addr += rowBytes;
                }
                if (warning) {
                    gif_warning(*bm, "Index out of bounds.");
                }
            }
        }
    }
}

namespace {
// This function is a template argument, so can't be static.
int close_gif(GifFileType* gif) {
#if GIFLIB_MAJOR < 5 || (GIFLIB_MAJOR == 5 && GIFLIB_MINOR == 0)
    return DGifCloseFile(gif);
#else
    return DGifCloseFile(gif, NULL);
#endif
}
}//namespace

SkImageDecoder::Result SkGIFImageDecoder::onDecode(SkStream* sk_stream, SkBitmap* bm, Mode mode) {
#if GIFLIB_MAJOR < 5
    GifFileType* gif = DGifOpen(sk_stream, DecodeCallBackProc);
#else
    GifFileType* gif = DGifOpen(sk_stream, DecodeCallBackProc, NULL);
#endif
    if (NULL == gif) {
        return error_return(*bm, "DGifOpen");
    }

    SkAutoTCallIProc<GifFileType, close_gif> acp(gif);

    SavedImage temp_save;
    temp_save.ExtensionBlocks=NULL;
    temp_save.ExtensionBlockCount=0;
    SkAutoTCallVProc<SavedImage, CheckFreeExtension> acp2(&temp_save);

    int width, height;
    GifRecordType recType;
    GifByteType *extData;
#if GIFLIB_MAJOR >= 5
    int extFunction;
#endif
    int transpIndex = -1;   // -1 means we don't have it (yet)
    int fillIndex = gif->SBackGroundColor;

    do {
        if (DGifGetRecordType(gif, &recType) == GIF_ERROR) {
            return error_return(*bm, "DGifGetRecordType");
        }

        switch (recType) {
        case IMAGE_DESC_RECORD_TYPE: {
            if (DGifGetImageDesc(gif) == GIF_ERROR) {
                return error_return(*bm, "IMAGE_DESC_RECORD_TYPE");
            }

            if (gif->ImageCount < 1) {    // sanity check
                return error_return(*bm, "ImageCount < 1");
            }

            width = gif->SWidth;
            height = gif->SHeight;

            SavedImage* image = &gif->SavedImages[gif->ImageCount-1];
            const GifImageDesc& desc = image->ImageDesc;

            int imageLeft = desc.Left;
            int imageTop = desc.Top;
            const int innerWidth = desc.Width;
            const int innerHeight = desc.Height;
            if (innerWidth <= 0 || innerHeight <= 0) {
                return error_return(*bm, "invalid dimensions");
            }

            // check for valid descriptor
            if (innerWidth > width) {
                gif_warning(*bm, "image too wide, expanding output to size");
                width = innerWidth;
                imageLeft = 0;
            } else if (imageLeft + innerWidth > width) {
                gif_warning(*bm, "shifting image left to fit");
                imageLeft = width - innerWidth;
            } else if (imageLeft < 0) {
                gif_warning(*bm, "shifting image right to fit");
                imageLeft = 0;
            }


            if (innerHeight > height) {
                gif_warning(*bm, "image too tall,  expanding output to size");
                height = innerHeight;
                imageTop = 0;
            } else if (imageTop + innerHeight > height) {
                gif_warning(*bm, "shifting image up to fit");
                imageTop = height - innerHeight;
            } else if (imageTop < 0) {
                gif_warning(*bm, "shifting image down to fit");
                imageTop = 0;
            }

            SkScaledBitmapSampler sampler(width, height, this->getSampleSize());

            bm->setInfo(SkImageInfo::Make(sampler.scaledWidth(), sampler.scaledHeight(),
                                          kIndex_8_SkColorType, kPremul_SkAlphaType));

            if (SkImageDecoder::kDecodeBounds_Mode == mode) {
                return kSuccess;
            }


            // now we decode the colortable
            int colorCount = 0;
            {
                // Declare colorPtr here for scope.
                SkPMColor colorPtr[256]; // storage for worst-case
                const ColorMapObject* cmap = find_colormap(gif);
                if (cmap != NULL) {
                    SkASSERT(cmap->ColorCount == (1 << (cmap->BitsPerPixel)));
                    colorCount = cmap->ColorCount;
                    if (colorCount > 256) {
                        colorCount = 256;  // our kIndex8 can't support more
                    }
                    for (int index = 0; index < colorCount; index++) {
                        colorPtr[index] = SkPackARGB32(0xFF,
                                                       cmap->Colors[index].Red,
                                                       cmap->Colors[index].Green,
                                                       cmap->Colors[index].Blue);
                    }
                } else {
                    // find_colormap() returned NULL.  Some (rare, broken)
                    // GIFs don't have a color table, so we force one.
                    gif_warning(*bm, "missing colormap");
                    colorCount = 256;
                    sk_memset32(colorPtr, SK_ColorWHITE, colorCount);
                }
                transpIndex = find_transpIndex(temp_save, colorCount);
                if (transpIndex >= 0) {
                    colorPtr[transpIndex] = SK_ColorTRANSPARENT; // ram in a transparent SkPMColor
                    fillIndex = transpIndex;
                } else if (fillIndex >= colorCount) {
                    // gif->SBackGroundColor should be less than colorCount.
                    fillIndex = 0;  // If not, fix it.
                }

                SkAutoTUnref<SkColorTable> ctable(SkNEW_ARGS(SkColorTable, (colorPtr, colorCount)));
                if (!this->allocPixelRef(bm, ctable)) {
                    return error_return(*bm, "allocPixelRef");
                }
            }

            // abort if either inner dimension is <= 0
            if (innerWidth <= 0 || innerHeight <= 0) {
                return error_return(*bm, "non-pos inner width/height");
            }

            SkAutoLockPixels alp(*bm);

            SkAutoMalloc storage(innerWidth);
            uint8_t* scanline = (uint8_t*) storage.get();

            // GIF has an option to store the scanlines of an image, plus a larger background,
            // filled by a fill color. In this case, we will use a subset of the larger bitmap
            // for sampling.
            SkBitmap subset;
            SkBitmap* workingBitmap;
            // are we only a subset of the total bounds?
            if ((imageTop | imageLeft) > 0 ||
                 innerWidth < width || innerHeight < height) {
                // Fill the background.
                memset(bm->getPixels(), fillIndex, bm->getSize());

                // Create a subset of the bitmap.
                SkIRect subsetRect(SkIRect::MakeXYWH(imageLeft / sampler.srcDX(),
                                                     imageTop / sampler.srcDY(),
                                                     innerWidth / sampler.srcDX(),
                                                     innerHeight / sampler.srcDY()));
                if (!bm->extractSubset(&subset, subsetRect)) {
                    return error_return(*bm, "Extract failed.");
                }
                // Update the sampler. We'll now be only sampling into the subset.
                sampler = SkScaledBitmapSampler(innerWidth, innerHeight, this->getSampleSize());
                workingBitmap = &subset;
            } else {
                workingBitmap = bm;
            }

            // bm is already locked, but if we had to take a subset, it must be locked also,
            // so that getPixels() will point to its pixels.
            SkAutoLockPixels alpWorking(*workingBitmap);

            if (!sampler.begin(workingBitmap, SkScaledBitmapSampler::kIndex, *this)) {
                return error_return(*bm, "Sampler failed to begin.");
            }

            // now decode each scanline
            if (gif->Image.Interlace) {
                // Iterate over the height of the source data. The sampler will
                // take care of skipping unneeded rows.
                GifInterlaceIter iter(innerHeight);
                for (int y = 0; y < innerHeight; y++) {
                    if (DGifGetLine(gif, scanline, innerWidth) == GIF_ERROR) {
                        gif_warning(*bm, "interlace DGifGetLine");
                        memset(scanline, fillIndex, innerWidth);
                        for (; y < innerHeight; y++) {
                            sampler.sampleInterlaced(scanline, iter.currY());
                            iter.next();
                        }
                        return kPartialSuccess;
                    }
                    sampler.sampleInterlaced(scanline, iter.currY());
                    iter.next();
                }
            } else {
                // easy, non-interlace case
                const int outHeight = workingBitmap->height();
                skip_src_rows(gif, scanline, innerWidth, sampler.srcY0());
                for (int y = 0; y < outHeight; y++) {
                    if (DGifGetLine(gif, scanline, innerWidth) == GIF_ERROR) {
                        gif_warning(*bm, "DGifGetLine");
                        memset(scanline, fillIndex, innerWidth);
                        for (; y < outHeight; y++) {
                            sampler.next(scanline);
                        }
                        return kPartialSuccess;
                    }
                    // scanline now contains the raw data. Sample it.
                    sampler.next(scanline);
                    if (y < outHeight - 1) {
                        skip_src_rows(gif, scanline, innerWidth, sampler.srcDY() - 1);
                    }
                }
                // skip the rest of the rows (if any)
                int read = (outHeight - 1) * sampler.srcDY() + sampler.srcY0() + 1;
                SkASSERT(read <= innerHeight);
                skip_src_rows(gif, scanline, innerWidth, innerHeight - read);
            }
            sanitize_indexed_bitmap(bm);
            return kSuccess;
            } break;

        case EXTENSION_RECORD_TYPE:
#if GIFLIB_MAJOR < 5
            if (DGifGetExtension(gif, &temp_save.Function,
                                 &extData) == GIF_ERROR) {
#else
            if (DGifGetExtension(gif, &extFunction, &extData) == GIF_ERROR) {
#endif
                return error_return(*bm, "DGifGetExtension");
            }

            while (extData != NULL) {
                /* Create an extension block with our data */
#if GIFLIB_MAJOR < 5
                if (AddExtensionBlock(&temp_save, extData[0],
                                      &extData[1]) == GIF_ERROR) {
#else
                if (GifAddExtensionBlock(&temp_save.ExtensionBlockCount,
                                         &temp_save.ExtensionBlocks,
                                         extFunction,
                                         extData[0],
                                         &extData[1]) == GIF_ERROR) {
#endif
                    return error_return(*bm, "AddExtensionBlock");
                }
                if (DGifGetExtensionNext(gif, &extData) == GIF_ERROR) {
                    return error_return(*bm, "DGifGetExtensionNext");
                }
#if GIFLIB_MAJOR < 5
                temp_save.Function = 0;
#endif
            }
            break;

        case TERMINATE_RECORD_TYPE:
            break;

        default:    /* Should be trapped by DGifGetRecordType */
            break;
        }
    } while (recType != TERMINATE_RECORD_TYPE);

    sanitize_indexed_bitmap(bm);
    return kSuccess;
}

///////////////////////////////////////////////////////////////////////////////
DEFINE_DECODER_CREATOR(GIFImageDecoder);
///////////////////////////////////////////////////////////////////////////////

static bool is_gif(SkStreamRewindable* stream) {
    char buf[GIF_STAMP_LEN];
    if (stream->read(buf, GIF_STAMP_LEN) == GIF_STAMP_LEN) {
        if (memcmp(GIF_STAMP,   buf, GIF_STAMP_LEN) == 0 ||
                memcmp(GIF87_STAMP, buf, GIF_STAMP_LEN) == 0 ||
                memcmp(GIF89_STAMP, buf, GIF_STAMP_LEN) == 0) {
            return true;
        }
    }
    return false;
}

static SkImageDecoder* sk_libgif_dfactory(SkStreamRewindable* stream) {
    if (is_gif(stream)) {
        return SkNEW(SkGIFImageDecoder);
    }
    return NULL;
}

static SkImageDecoder_DecodeReg gReg(sk_libgif_dfactory);

static SkImageDecoder::Format get_format_gif(SkStreamRewindable* stream) {
    if (is_gif(stream)) {
        return SkImageDecoder::kGIF_Format;
    }
    return SkImageDecoder::kUnknown_Format;
}

static SkImageDecoder_FormatReg gFormatReg(get_format_gif);
