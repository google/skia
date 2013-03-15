
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkImageDecoder.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkPackBits.h"

#include "gif_lib.h"

class SkGIFImageDecoder : public SkImageDecoder {
public:
    virtual Format getFormat() const SK_OVERRIDE {
        return kGIF_Format;
    }

protected:
    virtual bool onDecode(SkStream* stream, SkBitmap* bm, Mode mode) SK_OVERRIDE;

private:
    typedef SkImageDecoder INHERITED;
};

static const uint8_t gStartingIterlaceYValue[] = {
    0, 4, 2, 1
};
static const uint8_t gDeltaIterlaceYValue[] = {
    8, 8, 4, 2
};

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

//#define GIF_STAMP       "GIF"    /* First chars in file - GIF stamp. */
//#define GIF_STAMP_LEN   (sizeof(GIF_STAMP) - 1)

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

static bool error_return(GifFileType* gif, const SkBitmap& bm,
                         const char msg[]) {
#if 0
    SkDebugf("libgif error <%s> bitmap [%d %d] pixels %p colortable %p\n",
             msg, bm.width(), bm.height(), bm.getPixels(), bm.getColorTable());
#endif
    return false;
}

bool SkGIFImageDecoder::onDecode(SkStream* sk_stream, SkBitmap* bm, Mode mode) {
#if GIFLIB_MAJOR < 5
    GifFileType* gif = DGifOpen(sk_stream, DecodeCallBackProc);
#else
    GifFileType* gif = DGifOpen(sk_stream, DecodeCallBackProc, NULL);
#endif
    if (NULL == gif) {
        return error_return(gif, *bm, "DGifOpen");
    }

    SkAutoTCallIProc<GifFileType, DGifCloseFile> acp(gif);

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

    do {
        if (DGifGetRecordType(gif, &recType) == GIF_ERROR) {
            return error_return(gif, *bm, "DGifGetRecordType");
        }

        switch (recType) {
        case IMAGE_DESC_RECORD_TYPE: {
            if (DGifGetImageDesc(gif) == GIF_ERROR) {
                return error_return(gif, *bm, "IMAGE_DESC_RECORD_TYPE");
            }

            if (gif->ImageCount < 1) {    // sanity check
                return error_return(gif, *bm, "ImageCount < 1");
            }

            width = gif->SWidth;
            height = gif->SHeight;
            if (width <= 0 || height <= 0 ||
                !this->chooseFromOneChoice(SkBitmap::kIndex8_Config,
                                           width, height)) {
                return error_return(gif, *bm, "chooseFromOneChoice");
            }

            if (SkImageDecoder::kDecodeBounds_Mode == mode) {
                bm->setConfig(SkBitmap::kIndex8_Config, width, height);
                return true;
            }

            // No Bitmap reuse supported for this format
            if (!bm->isNull()) {
                return false;
            }

            bm->setConfig(SkBitmap::kIndex8_Config, width, height);
            SavedImage* image = &gif->SavedImages[gif->ImageCount-1];
            const GifImageDesc& desc = image->ImageDesc;

            // check for valid descriptor
            if (   (desc.Top | desc.Left) < 0 ||
                    desc.Left + desc.Width > width ||
                    desc.Top + desc.Height > height) {
                return error_return(gif, *bm, "TopLeft");
            }

            // now we decode the colortable
            int colorCount = 0;
            {
                const ColorMapObject* cmap = find_colormap(gif);
                if (NULL == cmap) {
                    return error_return(gif, *bm, "null cmap");
                }

                colorCount = cmap->ColorCount;
                SkColorTable* ctable = SkNEW_ARGS(SkColorTable, (colorCount));
                SkPMColor* colorPtr = ctable->lockColors();
                for (int index = 0; index < colorCount; index++)
                    colorPtr[index] = SkPackARGB32(0xFF,
                                                   cmap->Colors[index].Red,
                                                   cmap->Colors[index].Green,
                                                   cmap->Colors[index].Blue);

                transpIndex = find_transpIndex(temp_save, colorCount);
                if (transpIndex < 0)
                    ctable->setFlags(ctable->getFlags() | SkColorTable::kColorsAreOpaque_Flag);
                else
                    colorPtr[transpIndex] = 0; // ram in a transparent SkPMColor
                ctable->unlockColors(true);

                SkAutoUnref aurts(ctable);
                if (!this->allocPixelRef(bm, ctable)) {
                    return error_return(gif, *bm, "allocPixelRef");
                }
            }

            SkAutoLockPixels alp(*bm);

            // time to decode the scanlines
            //
            uint8_t*  scanline = bm->getAddr8(0, 0);
            const int rowBytes = bm->rowBytes();
            const int innerWidth = desc.Width;
            const int innerHeight = desc.Height;

            // abort if either inner dimension is <= 0
            if (innerWidth <= 0 || innerHeight <= 0) {
                return error_return(gif, *bm, "non-pos inner width/height");
            }

            // are we only a subset of the total bounds?
            if ((desc.Top | desc.Left) > 0 ||
                 innerWidth < width || innerHeight < height)
            {
                int fill;
                if (transpIndex >= 0) {
                    fill = transpIndex;
                } else {
                    fill = gif->SBackGroundColor;
                }
                // check for valid fill index/color
                if (static_cast<unsigned>(fill) >=
                        static_cast<unsigned>(colorCount)) {
                    fill = 0;
                }
                memset(scanline, fill, bm->getSize());
                // bump our starting address
                scanline += desc.Top * rowBytes + desc.Left;
            }

            // now decode each scanline
            if (gif->Image.Interlace)
            {
                GifInterlaceIter iter(innerHeight);
                for (int y = 0; y < innerHeight; y++)
                {
                    uint8_t* row = scanline + iter.currY() * rowBytes;
                    if (DGifGetLine(gif, row, innerWidth) == GIF_ERROR) {
                        return error_return(gif, *bm, "interlace DGifGetLine");
                    }
                    iter.next();
                }
            }
            else
            {
                // easy, non-interlace case
                for (int y = 0; y < innerHeight; y++) {
                    if (DGifGetLine(gif, scanline, innerWidth) == GIF_ERROR) {
                        return error_return(gif, *bm, "DGifGetLine");
                    }
                    scanline += rowBytes;
                }
            }
            goto DONE;
            } break;

        case EXTENSION_RECORD_TYPE:
#if GIFLIB_MAJOR < 5
            if (DGifGetExtension(gif, &temp_save.Function,
                                 &extData) == GIF_ERROR) {
#else
            if (DGifGetExtension(gif, &extFunction, &extData) == GIF_ERROR) {
#endif
                return error_return(gif, *bm, "DGifGetExtension");
            }

            while (extData != NULL) {
                /* Create an extension block with our data */
#if GIFLIB_MAJOR < 5
                if (AddExtensionBlock(&temp_save, extData[0],
                                      &extData[1]) == GIF_ERROR) {
#else
                if (GifAddExtensionBlock(&gif->ExtensionBlockCount,
                                         &gif->ExtensionBlocks,
                                         extFunction,
                                         extData[0],
                                         &extData[1]) == GIF_ERROR) {
#endif
                    return error_return(gif, *bm, "AddExtensionBlock");
                }
                if (DGifGetExtensionNext(gif, &extData) == GIF_ERROR) {
                    return error_return(gif, *bm, "DGifGetExtensionNext");
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

DONE:
    return true;
}

///////////////////////////////////////////////////////////////////////////////
DEFINE_DECODER_CREATOR(GIFImageDecoder);
///////////////////////////////////////////////////////////////////////////////

#include "SkTRegistry.h"

static SkImageDecoder* sk_libgif_dfactory(SkStream* stream) {
    char buf[GIF_STAMP_LEN];
    if (stream->read(buf, GIF_STAMP_LEN) == GIF_STAMP_LEN) {
        if (memcmp(GIF_STAMP,   buf, GIF_STAMP_LEN) == 0 ||
                memcmp(GIF87_STAMP, buf, GIF_STAMP_LEN) == 0 ||
                memcmp(GIF89_STAMP, buf, GIF_STAMP_LEN) == 0) {
            return SkNEW(SkGIFImageDecoder);
        }
    }
    return NULL;
}

static SkTRegistry<SkImageDecoder*, SkStream*> gReg(sk_libgif_dfactory);
