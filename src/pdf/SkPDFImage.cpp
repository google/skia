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
    switch (bitmap.config()) {
        case SkBitmap::kIndex8_Config:
            return srcRect.width() * srcRect.height();
        case SkBitmap::kARGB_4444_Config:
            return ((srcRect.width() * 3 + 1) / 2) * srcRect.height();
        case SkBitmap::kRGB_565_Config:
            return srcRect.width() * 3 * srcRect.height();
        case SkBitmap::kARGB_8888_Config:
            return srcRect.width() * 3 * srcRect.height();
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

/**
 * Removes the alpha component of an ARGB color (including unpremultiply) while
 * keeping the output in the same format as the input.
 */
static uint32_t remove_alpha_argb8888(uint32_t pmColor) {
    SkColor color = SkUnPreMultiply::PMColorToColor(pmColor);
    return SkPackARGB32NoCheck(SK_AlphaOPAQUE,
                               SkColorGetR(color),
                               SkColorGetG(color),
                               SkColorGetB(color));
}

static uint16_t remove_alpha_argb4444(uint16_t pmColor) {
    return SkPixel32ToPixel4444(
            remove_alpha_argb8888(SkPixel4444ToPixel32(pmColor)));
}

static uint32_t get_argb8888_neighbor_avg_color(const SkBitmap& bitmap,
                                                int xOrig, int yOrig) {
    uint8_t count = 0;
    uint16_t r = 0;
    uint16_t g = 0;
    uint16_t b = 0;

    for (int y = yOrig - 1; y <= yOrig + 1; y++) {
        if (y < 0 || y >= bitmap.height()) {
            continue;
        }
        uint32_t* src = bitmap.getAddr32(0, y);
        for (int x = xOrig - 1; x <= xOrig + 1; x++) {
            if (x < 0 || x >= bitmap.width()) {
                continue;
            }
            if (SkGetPackedA32(src[x]) != SK_AlphaTRANSPARENT) {
                uint32_t color = remove_alpha_argb8888(src[x]);
                r += SkGetPackedR32(color);
                g += SkGetPackedG32(color);
                b += SkGetPackedB32(color);
                count++;
            }
        }
    }

    if (count == 0) {
        return SkPackARGB32NoCheck(SK_AlphaOPAQUE, 0, 0, 0);
    } else {
        return SkPackARGB32NoCheck(SK_AlphaOPAQUE,
                                   r / count, g / count, b / count);
    }
}

static uint16_t get_argb4444_neighbor_avg_color(const SkBitmap& bitmap,
                                                int xOrig, int yOrig) {
    uint8_t count = 0;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    for (int y = yOrig - 1; y <= yOrig + 1; y++) {
        if (y < 0 || y >= bitmap.height()) {
            continue;
        }
        uint16_t* src = bitmap.getAddr16(0, y);
        for (int x = xOrig - 1; x <= xOrig + 1; x++) {
            if (x < 0 || x >= bitmap.width()) {
                continue;
            }
            if ((SkGetPackedA4444(src[x]) & 0x0F) != SK_AlphaTRANSPARENT) {
                uint16_t color = remove_alpha_argb4444(src[x]);
                r += SkGetPackedR4444(color);
                g += SkGetPackedG4444(color);
                b += SkGetPackedB4444(color);
                count++;
            }
        }
    }

    if (count == 0) {
        return SkPackARGB4444(SK_AlphaOPAQUE & 0x0F, 0, 0, 0);
    } else {
        return SkPackARGB4444(SK_AlphaOPAQUE & 0x0F,
                                   r / count, g / count, b / count);
    }
}

static SkBitmap unpremultiply_bitmap(const SkBitmap& bitmap,
                                     const SkIRect& srcRect) {
    SkBitmap outBitmap;
    outBitmap.setConfig(bitmap.config(), srcRect.width(), srcRect.height());
    outBitmap.allocPixels();
    int dstRow = 0;

    outBitmap.lockPixels();
    bitmap.lockPixels();
    switch (bitmap.config()) {
        case SkBitmap::kARGB_4444_Config: {
            for (int y = srcRect.fTop; y < srcRect.fBottom; y++) {
                uint16_t* dst = outBitmap.getAddr16(0, dstRow);
                uint16_t* src = bitmap.getAddr16(0, y);
                for (int x = srcRect.fLeft; x < srcRect.fRight; x++) {
                    uint8_t a = SkGetPackedA4444(src[x]);
                    // It is necessary to average the color component of
                    // transparent pixels with their surrounding neighbors
                    // since the PDF renderer may separately re-sample the
                    // alpha and color channels when the image is not
                    // displayed at its native resolution. Since an alpha of
                    // zero gives no information about the color component,
                    // the pathological case is a white image with sharp
                    // transparency bounds - the color channel goes to black,
                    // and the should-be-transparent pixels are rendered
                    // as grey because of the separate soft mask and color
                    // resizing.
                    if (a == (SK_AlphaTRANSPARENT & 0x0F)) {
                        *dst = get_argb4444_neighbor_avg_color(bitmap, x, y);
                    } else {
                        *dst = remove_alpha_argb4444(src[x]);
                    }
                    dst++;
                }
                dstRow++;
            }
            break;
        }
        case SkBitmap::kARGB_8888_Config: {
            for (int y = srcRect.fTop; y < srcRect.fBottom; y++) {
                uint32_t* dst = outBitmap.getAddr32(0, dstRow);
                uint32_t* src = bitmap.getAddr32(0, y);
                for (int x = srcRect.fLeft; x < srcRect.fRight; x++) {
                    uint8_t a = SkGetPackedA32(src[x]);
                    if (a == SK_AlphaTRANSPARENT) {
                        *dst = get_argb8888_neighbor_avg_color(bitmap, x, y);
                    } else {
                        *dst = remove_alpha_argb8888(src[x]);
                    }
                    dst++;
                }
                dstRow++;
            }
            break;
        }
        default:
            SkASSERT(false);
    }
    bitmap.unlockPixels();
    outBitmap.unlockPixels();

    outBitmap.setImmutable();

    return outBitmap;
}

// static
SkPDFImage* SkPDFImage::CreateImage(const SkBitmap& bitmap,
                                    const SkIRect& srcRect,
                                    SkPicture::EncodeBitmap encoder) {
    if (bitmap.config() == SkBitmap::kNo_Config) {
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

    SkPDFImage* image;
    SkBitmap::Config config = bitmap.config();
    if (alphaData.get() != NULL && (config == SkBitmap::kARGB_8888_Config ||
            config == SkBitmap::kARGB_4444_Config)) {
        SkBitmap unpremulBitmap = unpremultiply_bitmap(bitmap, srcRect);
        image = SkNEW_ARGS(SkPDFImage, (NULL, unpremulBitmap, false,
                           SkIRect::MakeWH(srcRect.width(), srcRect.height()),
                           encoder));
    } else {
        image = SkNEW_ARGS(SkPDFImage, (NULL, bitmap, false, srcRect, encoder));
    }
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

    SkBitmap::Config config = fBitmap.config();

    insertName("Type", "XObject");
    insertName("Subtype", "Image");

    bool alphaOnly = (config == SkBitmap::kA8_Config);

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
    }
    insertInt("BitsPerComponent", bitsPerComp);

    if (config == SkBitmap::kRGB_565_Config) {
        SkASSERT(!isAlpha);
        SkAutoTUnref<SkPDFInt> zeroVal(new SkPDFInt(0));
        SkAutoTUnref<SkPDFScalar> scale5Val(
                new SkPDFScalar(8.2258f));  // 255/2^5-1
        SkAutoTUnref<SkPDFScalar> scale6Val(
                new SkPDFScalar(4.0476f));  // 255/2^6-1
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
