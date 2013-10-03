/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFImage.h"

#include "SkBitmap.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkData.h"
#include "SkFlate.h"
#include "SkPDFCatalog.h"
#include "SkRect.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkUnPreMultiply.h"

static const int kNoColorTransform = 0;

static bool skip_compression(SkPDFCatalog* catalog) {
    return SkToBool(catalog->getDocumentFlags() &
                    SkPDFDocument::kFavorSpeedOverSize_Flags);
}

static size_t get_uncompressed_size(const SkBitmap& bitmap,
                                    const SkIRect& srcRect) {
    switch (bitmap.getConfig()) {
        case SkBitmap::kIndex8_Config:
            return srcRect.width() * srcRect.height();
        case SkBitmap::kARGB_4444_Config:
            return ((srcRect.width() * 3 + 1) / 2) * srcRect.height();
        case SkBitmap::kRGB_565_Config:
            return srcRect.width() * 3 * srcRect.height();
        case SkBitmap::kARGB_8888_Config:
            return srcRect.width() * 3 * srcRect.height();
        case SkBitmap::kA1_Config:
        case SkBitmap::kA8_Config:
            return 1;
        default:
            SkASSERT(false);
            return 0;
    }
}

static SkStream* extract_index8_image(const SkBitmap& bitmap,
                                      const SkIRect& srcRect) {
    const int rowBytes = srcRect.width();
    SkStream* stream = SkNEW_ARGS(SkMemoryStream,
                                  (get_uncompressed_size(bitmap, srcRect)));
    uint8_t* dst = (uint8_t*)stream->getMemoryBase();

    for (int y = srcRect.fTop; y < srcRect.fBottom; y++) {
        memcpy(dst, bitmap.getAddr8(srcRect.fLeft, y), rowBytes);
        dst += rowBytes;
    }
    return stream;
}

static SkStream* extract_argb4444_data(const SkBitmap& bitmap,
                                       const SkIRect& srcRect,
                                       bool extractAlpha,
                                       bool* isOpaque,
                                       bool* isTransparent) {
    SkStream* stream;
    uint8_t* dst = NULL;
    if (extractAlpha) {
        const int alphaRowBytes = (srcRect.width() + 1) / 2;
        stream = SkNEW_ARGS(SkMemoryStream,
                            (alphaRowBytes * srcRect.height()));
    } else {
        stream = SkNEW_ARGS(SkMemoryStream,
                            (get_uncompressed_size(bitmap, srcRect)));
    }
    dst = (uint8_t*)stream->getMemoryBase();

    for (int y = srcRect.fTop; y < srcRect.fBottom; y++) {
        uint16_t* src = bitmap.getAddr16(0, y);
        int x;
        for (x = srcRect.fLeft; x + 1 < srcRect.fRight; x += 2) {
            if (extractAlpha) {
                dst[0] = (SkGetPackedA4444(src[x]) << 4) |
                    SkGetPackedA4444(src[x + 1]);
                *isOpaque &= dst[0] == SK_AlphaOPAQUE;
                *isTransparent &= dst[0] == SK_AlphaTRANSPARENT;
                dst++;
            } else {
                dst[0] = (SkGetPackedR4444(src[x]) << 4) |
                    SkGetPackedG4444(src[x]);
                dst[1] = (SkGetPackedB4444(src[x]) << 4) |
                    SkGetPackedR4444(src[x + 1]);
                dst[2] = (SkGetPackedG4444(src[x + 1]) << 4) |
                    SkGetPackedB4444(src[x + 1]);
                dst += 3;
            }
        }
        if (srcRect.width() & 1) {
            if (extractAlpha) {
                dst[0] = (SkGetPackedA4444(src[x]) << 4);
                *isOpaque &= dst[0] == (SK_AlphaOPAQUE & 0xF0);
                *isTransparent &= dst[0] == (SK_AlphaTRANSPARENT & 0xF0);
                dst++;

            } else {
                dst[0] = (SkGetPackedR4444(src[x]) << 4) |
                    SkGetPackedG4444(src[x]);
                dst[1] = (SkGetPackedB4444(src[x]) << 4);
                dst += 2;
            }
        }
    }
    return stream;
}

static SkStream* extract_rgb565_image(const SkBitmap& bitmap,
                                      const SkIRect& srcRect) {
    SkStream* stream = SkNEW_ARGS(SkMemoryStream,
                                  (get_uncompressed_size(bitmap,
                                                     srcRect)));
    uint8_t* dst = (uint8_t*)stream->getMemoryBase();
    for (int y = srcRect.fTop; y < srcRect.fBottom; y++) {
        uint16_t* src = bitmap.getAddr16(0, y);
        for (int x = srcRect.fLeft; x < srcRect.fRight; x++) {
            dst[0] = SkGetPackedR16(src[x]);
            dst[1] = SkGetPackedG16(src[x]);
            dst[2] = SkGetPackedB16(src[x]);
            dst += 3;
        }
    }
    return stream;
}

static SkStream* extract_argb8888_data(const SkBitmap& bitmap,
                                       const SkIRect& srcRect,
                                       bool extractAlpha,
                                       bool* isOpaque,
                                       bool* isTransparent) {
    SkStream* stream;
    if (extractAlpha) {
        stream = SkNEW_ARGS(SkMemoryStream,
                            (srcRect.width() * srcRect.height()));
    } else {
        stream = SkNEW_ARGS(SkMemoryStream,
                            (get_uncompressed_size(bitmap, srcRect)));
    }
    uint8_t* dst = (uint8_t*)stream->getMemoryBase();

    for (int y = srcRect.fTop; y < srcRect.fBottom; y++) {
        uint32_t* src = bitmap.getAddr32(0, y);
        for (int x = srcRect.fLeft; x < srcRect.fRight; x++) {
            if (extractAlpha) {
                dst[0] = SkGetPackedA32(src[x]);
                *isOpaque &= dst[0] == SK_AlphaOPAQUE;
                *isTransparent &= dst[0] == SK_AlphaTRANSPARENT;
                dst++;
            } else {
                dst[0] = SkGetPackedR32(src[x]);
                dst[1] = SkGetPackedG32(src[x]);
                dst[2] = SkGetPackedB32(src[x]);
                dst += 3;
            }
        }
    }
    return stream;
}

static SkStream* extract_a1_alpha(const SkBitmap& bitmap,
                                  const SkIRect& srcRect,
                                  bool* isOpaque,
                                  bool* isTransparent) {
    const int alphaRowBytes = (srcRect.width() + 7) / 8;
    SkStream* stream = SkNEW_ARGS(SkMemoryStream,
                                  (alphaRowBytes * srcRect.height()));
    uint8_t* alphaDst = (uint8_t*)stream->getMemoryBase();

    int offset1 = srcRect.fLeft % 8;
    int offset2 = 8 - offset1;

    for (int y = srcRect.fTop; y < srcRect.fBottom; y++) {
        uint8_t* src = bitmap.getAddr1(0, y);
        // This may read up to one byte after src, but the
        // potentially invalid bits are never used for computation.
        for (int x = srcRect.fLeft; x < srcRect.fRight; x += 8)  {
            if (offset1) {
                alphaDst[0] = src[x / 8] << offset1 |
                    src[x / 8 + 1] >> offset2;
            } else {
                alphaDst[0] = src[x / 8];
            }
            if (x + 7 < srcRect.fRight) {
                *isOpaque &= alphaDst[0] == SK_AlphaOPAQUE;
                *isTransparent &= alphaDst[0] == SK_AlphaTRANSPARENT;
            }
            alphaDst++;
        }
        // Calculate the mask of bits we're interested in within the
        // last byte of alphaDst.
        // width mod 8  == 1 -> 0x80 ... width mod 8 == 7 -> 0xFE
        uint8_t mask = ~((1 << (8 - (srcRect.width() % 8))) - 1);
        if (srcRect.width() % 8) {
            *isOpaque &= (alphaDst[-1] & mask) == (SK_AlphaOPAQUE & mask);
            *isTransparent &=
                    (alphaDst[-1] & mask) == (SK_AlphaTRANSPARENT & mask);
        }
    }
    return stream;
}

static SkStream* extract_a8_alpha(const SkBitmap& bitmap,
                                  const SkIRect& srcRect,
                                  bool* isOpaque,
                                  bool* isTransparent) {
    const int alphaRowBytes = srcRect.width();
    SkStream* stream = SkNEW_ARGS(SkMemoryStream,
                                  (alphaRowBytes * srcRect.height()));
    uint8_t* alphaDst = (uint8_t*)stream->getMemoryBase();

    for (int y = srcRect.fTop; y < srcRect.fBottom; y++) {
        uint8_t* src = bitmap.getAddr8(0, y);
        for (int x = srcRect.fLeft; x < srcRect.fRight; x++) {
            alphaDst[0] = src[x];
            *isOpaque &= alphaDst[0] == SK_AlphaOPAQUE;
            *isTransparent &= alphaDst[0] == SK_AlphaTRANSPARENT;
            alphaDst++;
        }
    }
    return stream;
}

static SkStream* create_black_image() {
    SkStream* stream = SkNEW_ARGS(SkMemoryStream, (1));
    ((uint8_t*)stream->getMemoryBase())[0] = 0;
    return stream;
}

/**
 * Extract either the color or image data from a SkBitmap into a SkStream.
 * @param bitmap        Bitmap to extract data from.
 * @param srcRect       Region in the bitmap to extract.
 * @param extractAlpha  Set to true to extract the alpha data or false to
 *                      extract the color data.
 * @param isTransparent Pointer to a bool to output whether the alpha is
 *                      completely transparent. May be NULL. Only valid when
 *                      extractAlpha == true.
 * @return              Unencoded image data, or NULL if either data was not
 *                      available or alpha data was requested but the image was
 *                      entirely transparent or opaque.
 */
static SkStream* extract_image_data(const SkBitmap& bitmap,
                                    const SkIRect& srcRect,
                                    bool extractAlpha, bool* isTransparent) {
    SkBitmap::Config config = bitmap.config();
    if (extractAlpha && (config == SkBitmap::kIndex8_Config ||
            config == SkBitmap::kRGB_565_Config)) {
        if (isTransparent != NULL) {
            *isTransparent = false;
        }
        return NULL;
    }
    bool isOpaque = true;
    bool transparent = extractAlpha;
    SkStream* stream = NULL;

    bitmap.lockPixels();
    switch (config) {
        case SkBitmap::kIndex8_Config:
            if (!extractAlpha) {
                stream = extract_index8_image(bitmap, srcRect);
            }
            break;
        case SkBitmap::kARGB_4444_Config:
            stream = extract_argb4444_data(bitmap, srcRect, extractAlpha,
                                           &isOpaque, &transparent);
            break;
        case SkBitmap::kRGB_565_Config:
            if (!extractAlpha) {
                stream = extract_rgb565_image(bitmap, srcRect);
            }
            break;
        case SkBitmap::kARGB_8888_Config:
            stream = extract_argb8888_data(bitmap, srcRect, extractAlpha,
                                           &isOpaque, &transparent);
            break;
        case SkBitmap::kA1_Config:
            if (!extractAlpha) {
                stream = create_black_image();
            } else {
                stream = extract_a1_alpha(bitmap, srcRect,
                                          &isOpaque, &transparent);
            }
            break;
        case SkBitmap::kA8_Config:
            if (!extractAlpha) {
                stream = create_black_image();
            } else {
                stream = extract_a8_alpha(bitmap, srcRect,
                                          &isOpaque, &transparent);
            }
            break;
        default:
            SkASSERT(false);
    }
    bitmap.unlockPixels();

    if (isTransparent != NULL) {
        *isTransparent = transparent;
    }
    if (extractAlpha && (transparent || isOpaque)) {
        SkSafeUnref(stream);
        return NULL;
    }
    return stream;
}

static SkPDFArray* make_indexed_color_space(SkColorTable* table) {
    SkPDFArray* result = new SkPDFArray();
    result->reserve(4);
    result->appendName("Indexed");
    result->appendName("DeviceRGB");
    result->appendInt(table->count() - 1);

    // Potentially, this could be represented in fewer bytes with a stream.
    // Max size as a string is 1.5k.
    SkString index;
    for (int i = 0; i < table->count(); i++) {
        char buf[3];
        SkColor color = SkUnPreMultiply::PMColorToColor((*table)[i]);
        buf[0] = SkGetPackedR32(color);
        buf[1] = SkGetPackedG32(color);
        buf[2] = SkGetPackedB32(color);
        index.append(buf, 3);
    }
    result->append(new SkPDFString(index))->unref();
    return result;
}

// static
SkPDFImage* SkPDFImage::CreateImage(const SkBitmap& bitmap,
                                    const SkIRect& srcRect,
                                    SkPicture::EncodeBitmap encoder) {
    if (bitmap.getConfig() == SkBitmap::kNo_Config) {
        return NULL;
    }

    bool isTransparent = false;
    SkAutoTUnref<SkStream> alphaData;
    if (!bitmap.isOpaque()) {
        // Note that isOpaque is not guaranteed to return false for bitmaps
        // with alpha support but a completely opaque alpha channel,
        // so alphaData may still be NULL if we have a completely opaque
        // (or transparent) bitmap.
        alphaData.reset(
                extract_image_data(bitmap, srcRect, true, &isTransparent));
    }
    if (isTransparent) {
        return NULL;
    }

    SkPDFImage* image = SkNEW_ARGS(SkPDFImage, (NULL, bitmap,
                                                false, srcRect, encoder));
    if (alphaData.get() != NULL) {
        SkAutoTUnref<SkPDFImage> mask(
                SkNEW_ARGS(SkPDFImage, (alphaData.get(), bitmap,
                                        true, srcRect, NULL)));
        image->addSMask(mask);
    }

    return image;
}

SkPDFImage::~SkPDFImage() {
    fResources.unrefAll();
}

SkPDFImage* SkPDFImage::addSMask(SkPDFImage* mask) {
    fResources.push(mask);
    mask->ref();
    insert("SMask", new SkPDFObjRef(mask))->unref();
    return mask;
}

void SkPDFImage::getResources(const SkTSet<SkPDFObject*>& knownResourceObjects,
                              SkTSet<SkPDFObject*>* newResourceObjects) {
    GetResourcesHelper(&fResources, knownResourceObjects, newResourceObjects);
}

SkPDFImage::SkPDFImage(SkStream* stream,
                       const SkBitmap& bitmap,
                       bool isAlpha,
                       const SkIRect& srcRect,
                       SkPicture::EncodeBitmap encoder)
    : fIsAlpha(isAlpha),
      fSrcRect(srcRect),
      fEncoder(encoder) {

    if (bitmap.isImmutable()) {
        fBitmap = bitmap;
    } else {
        bitmap.deepCopyTo(&fBitmap, bitmap.config());
        fBitmap.setImmutable();
    }

    if (stream != NULL) {
        setData(stream);
        fStreamValid = true;
    } else {
        fStreamValid = false;
    }

    SkBitmap::Config config = fBitmap.getConfig();

    insertName("Type", "XObject");
    insertName("Subtype", "Image");

    bool alphaOnly = (config == SkBitmap::kA1_Config ||
                      config == SkBitmap::kA8_Config);

    if (!isAlpha && alphaOnly) {
        // For alpha only images, we stretch a single pixel of black for
        // the color/shape part.
        SkAutoTUnref<SkPDFInt> one(new SkPDFInt(1));
        insert("Width", one.get());
        insert("Height", one.get());
    } else {
        insertInt("Width", fSrcRect.width());
        insertInt("Height", fSrcRect.height());
    }

    if (isAlpha || alphaOnly) {
        insertName("ColorSpace", "DeviceGray");
    } else if (config == SkBitmap::kIndex8_Config) {
        SkAutoLockPixels alp(fBitmap);
        insert("ColorSpace",
               make_indexed_color_space(fBitmap.getColorTable()))->unref();
    } else {
        insertName("ColorSpace", "DeviceRGB");
    }

    int bitsPerComp = 8;
    if (config == SkBitmap::kARGB_4444_Config) {
        bitsPerComp = 4;
    } else if (isAlpha && config == SkBitmap::kA1_Config) {
        bitsPerComp = 1;
    }
    insertInt("BitsPerComponent", bitsPerComp);

    if (config == SkBitmap::kRGB_565_Config) {
        SkASSERT(!isAlpha);
        SkAutoTUnref<SkPDFInt> zeroVal(new SkPDFInt(0));
        SkAutoTUnref<SkPDFScalar> scale5Val(
                new SkPDFScalar(SkFloatToScalar(8.2258f)));  // 255/2^5-1
        SkAutoTUnref<SkPDFScalar> scale6Val(
                new SkPDFScalar(SkFloatToScalar(4.0476f)));  // 255/2^6-1
        SkAutoTUnref<SkPDFArray> decodeValue(new SkPDFArray());
        decodeValue->reserve(6);
        decodeValue->append(zeroVal.get());
        decodeValue->append(scale5Val.get());
        decodeValue->append(zeroVal.get());
        decodeValue->append(scale6Val.get());
        decodeValue->append(zeroVal.get());
        decodeValue->append(scale5Val.get());
        insert("Decode", decodeValue.get());
    }
}

SkPDFImage::SkPDFImage(SkPDFImage& pdfImage)
    : SkPDFStream(pdfImage),
      fBitmap(pdfImage.fBitmap),
      fIsAlpha(pdfImage.fIsAlpha),
      fSrcRect(pdfImage.fSrcRect),
      fEncoder(pdfImage.fEncoder),
      fStreamValid(pdfImage.fStreamValid) {
    // Nothing to do here - the image params are already copied in SkPDFStream's
    // constructor, and the bitmap will be regenerated and encoded in
    // populate.
}

bool SkPDFImage::populate(SkPDFCatalog* catalog) {
    if (getState() == kUnused_State) {
        // Initializing image data for the first time.
        SkDynamicMemoryWStream dctCompressedWStream;
        if (!skip_compression(catalog) && fEncoder &&
                get_uncompressed_size(fBitmap, fSrcRect) > 1) {
            SkBitmap subset;
            // Extract subset
            if (!fBitmap.extractSubset(&subset, fSrcRect)) {
                // TODO(edisonn) It fails only for kA1_Config, if that is a
                // major concern we will fix it later, so far it is NYI.
                return false;
            }
            size_t pixelRefOffset = 0;
            SkAutoTUnref<SkData> data(fEncoder(&pixelRefOffset, subset));
            if (data.get() && data->size() < get_uncompressed_size(fBitmap,
                                                                   fSrcRect)) {
                SkAutoTUnref<SkStream> stream(SkNEW_ARGS(SkMemoryStream,
                                                         (data)));
                setData(stream.get());

                insertName("Filter", "DCTDecode");
                insertInt("ColorTransform", kNoColorTransform);
                insertInt("Length", getData()->getLength());
                setState(kCompressed_State);
                return true;
            }
        }
        // Fallback method
        if (!fStreamValid) {
            SkAutoTUnref<SkStream> stream(
                    extract_image_data(fBitmap, fSrcRect, fIsAlpha, NULL));
            setData(stream);
            fStreamValid = true;
        }
        return INHERITED::populate(catalog);
    } else if (getState() == kNoCompression_State &&
            !skip_compression(catalog) &&
            (SkFlate::HaveFlate() || fEncoder)) {
        // Compression has not been requested when the stream was first created,
        // but the new catalog wants it compressed.
        if (!getSubstitute()) {
            SkPDFStream* substitute = SkNEW_ARGS(SkPDFImage, (*this));
            setSubstitute(substitute);
            catalog->setSubstitute(this, substitute);
        }
        return false;
    }
    return true;
}
