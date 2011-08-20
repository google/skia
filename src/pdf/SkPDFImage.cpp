
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
#include "SkPaint.h"
#include "SkPackBits.h"
#include "SkPDFCatalog.h"
#include "SkRect.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkUnPreMultiply.h"

namespace {

void extractImageData(const SkBitmap& bitmap, const SkIRect& srcRect,
                      SkStream** imageData, SkStream** alphaData) {
    SkMemoryStream* image = NULL;
    SkMemoryStream* alpha = NULL;
    bool hasAlpha = false;
    bool isTransparent = false;

    bitmap.lockPixels();
    switch (bitmap.getConfig()) {
        case SkBitmap::kIndex8_Config: {
            const int rowBytes = srcRect.width();
            image = new SkMemoryStream(rowBytes * srcRect.height());
            uint8_t* dst = (uint8_t*)image->getMemoryBase();
            for (int y = srcRect.fTop; y < srcRect.fBottom; y++) {
                memcpy(dst, bitmap.getAddr8(srcRect.fLeft, y), rowBytes);
                dst += rowBytes;
            }
            break;
        }
        case SkBitmap::kRLE_Index8_Config: {
            const int rowBytes = srcRect.width();
            image = new SkMemoryStream(rowBytes * srcRect.height());
            uint8_t* dst = (uint8_t*)image->getMemoryBase();
            const SkBitmap::RLEPixels* rle =
                (const SkBitmap::RLEPixels*)bitmap.getPixels();
            for (int y = srcRect.fTop; y < srcRect.fBottom; y++) {
                SkPackBits::Unpack8(dst, srcRect.fLeft, rowBytes,
                                    rle->packedAtY(y));
                dst += rowBytes;
            }
            break;
        }
        case SkBitmap::kARGB_4444_Config: {
            isTransparent = true;
            const int rowBytes = (srcRect.width() * 3 + 1) / 2;
            const int alphaRowBytes = (srcRect.width() + 1) / 2;
            image = new SkMemoryStream(rowBytes * srcRect.height());
            alpha = new SkMemoryStream(alphaRowBytes * srcRect.height());
            uint8_t* dst = (uint8_t*)image->getMemoryBase();
            uint8_t* alphaDst = (uint8_t*)alpha->getMemoryBase();
            for (int y = srcRect.fTop; y < srcRect.fBottom; y++) {
                uint16_t* src = bitmap.getAddr16(0, y);
                int x;
                for (x = srcRect.fLeft; x + 1 < srcRect.fRight; x += 2) {
                    dst[0] = (SkGetPackedR4444(src[x]) << 4) |
                        SkGetPackedG4444(src[x]);
                    dst[1] = (SkGetPackedB4444(src[x]) << 4) |
                        SkGetPackedR4444(src[x + 1]);
                    dst[2] = (SkGetPackedG4444(src[x + 1]) << 4) |
                        SkGetPackedB4444(src[x + 1]);
                    dst += 3;
                    alphaDst[0] = (SkGetPackedA4444(src[x]) << 4) |
                        SkGetPackedA4444(src[x + 1]);
                    if (alphaDst[0] != 0xFF) {
                        hasAlpha = true;
                    }
                    if (alphaDst[0]) {
                        isTransparent = false;
                    }
                    alphaDst++;
                }
                if (srcRect.width() & 1) {
                    dst[0] = (SkGetPackedR4444(src[x]) << 4) |
                        SkGetPackedG4444(src[x]);
                    dst[1] = (SkGetPackedB4444(src[x]) << 4);
                    dst += 2;
                    alphaDst[0] = (SkGetPackedA4444(src[x]) << 4);
                    if (alphaDst[0] != 0xF0) {
                        hasAlpha = true;
                    }
                    if (alphaDst[0] & 0xF0) {
                        isTransparent = false;
                    }
                    alphaDst++;
                }
            }
            break;
        }
        case SkBitmap::kRGB_565_Config: {
            const int rowBytes = srcRect.width() * 3;
            image = new SkMemoryStream(rowBytes * srcRect.height());
            uint8_t* dst = (uint8_t*)image->getMemoryBase();
            for (int y = srcRect.fTop; y < srcRect.fBottom; y++) {
                uint16_t* src = bitmap.getAddr16(0, y);
                for (int x = srcRect.fLeft; x < srcRect.fRight; x++) {
                    dst[0] = SkGetPackedR16(src[x]);
                    dst[1] = SkGetPackedG16(src[x]);
                    dst[2] = SkGetPackedB16(src[x]);
                    dst += 3;
                }
            }
            break;
        }
        case SkBitmap::kARGB_8888_Config: {
            isTransparent = true;
            const int rowBytes = srcRect.width() * 3;
            image = new SkMemoryStream(rowBytes * srcRect.height());
            alpha = new SkMemoryStream(srcRect.width() * srcRect.height());
            uint8_t* dst = (uint8_t*)image->getMemoryBase();
            uint8_t* alphaDst = (uint8_t*)alpha->getMemoryBase();
            for (int y = srcRect.fTop; y < srcRect.fBottom; y++) {
                uint32_t* src = bitmap.getAddr32(0, y);
                for (int x = srcRect.fLeft; x < srcRect.fRight; x++) {
                    dst[0] = SkGetPackedR32(src[x]);
                    dst[1] = SkGetPackedG32(src[x]);
                    dst[2] = SkGetPackedB32(src[x]);
                    dst += 3;
                    alphaDst[0] = SkGetPackedA32(src[x]);
                    if (alphaDst[0] != 0xFF) {
                        hasAlpha = true;
                    }
                    if (alphaDst[0]) {
                        isTransparent = false;
                    }
                    alphaDst++;
                }
            }
            break;
        }
        case SkBitmap::kA1_Config: {
            isTransparent = true;
            image = new SkMemoryStream(1);
            ((uint8_t*)image->getMemoryBase())[0] = 0;

            const int alphaRowBytes = (srcRect.width() + 7) / 8;
            alpha = new SkMemoryStream(alphaRowBytes * srcRect.height());
            uint8_t* alphaDst = (uint8_t*)alpha->getMemoryBase();
            int offset1 = srcRect.fLeft % 8;
            int offset2 = 8 - offset1;
            for (int y = srcRect.fTop; y < srcRect.fBottom; y++) {
                uint8_t* src = bitmap.getAddr1(0, y);
                // This may read up to one byte after src, but the potentially
                // invalid bits are never used for computation.
                for (int x = srcRect.fLeft; x < srcRect.fRight; x += 8)  {
                    if (offset1) {
                        alphaDst[0] = src[x / 8] << offset1 |
                            src[x / 8 + 1] >> offset2;
                    } else {
                        alphaDst[0] = src[x / 8];
                    }
                    if (x + 7 < srcRect.fRight && alphaDst[0] != 0xFF) {
                        hasAlpha = true;
                    }
                    if (x + 7 < srcRect.fRight && alphaDst[0]) {
                        isTransparent = false;
                    }
                    alphaDst++;
                }
                // Calculate the mask of bits we're interested in within the
                // last byte of alphaDst.
                // width mod 8  == 1 -> 0x80 ... width mod 8 == 7 -> 0xFE
                uint8_t mask = ~((1 << (8 - (srcRect.width() % 8))) - 1);
                if (srcRect.width() % 8 && (alphaDst[-1] & mask) != mask) {
                    hasAlpha = true;
                }
                if (srcRect.width() % 8 && (alphaDst[-1] & mask)) {
                    isTransparent = false;
                }
            }
            break;
        }
        case SkBitmap::kA8_Config: {
            isTransparent = true;
            image = new SkMemoryStream(1);
            ((uint8_t*)image->getMemoryBase())[0] = 0;

            const int alphaRowBytes = srcRect.width();
            alpha = new SkMemoryStream(alphaRowBytes * srcRect.height());
            uint8_t* alphaDst = (uint8_t*)alpha->getMemoryBase();
            for (int y = srcRect.fTop; y < srcRect.fBottom; y++) {
                uint8_t* src = bitmap.getAddr8(0, y);
                for (int x = srcRect.fLeft; x < srcRect.fRight; x++) {
                    alphaDst[0] = src[x];
                    if (alphaDst[0] != 0xFF) {
                        hasAlpha = true;
                    }
                    if (alphaDst[0]) {
                        isTransparent = false;
                    }
                    alphaDst++;
                }
            }
            break;
        }
        default:
            SkASSERT(false);
    }
    bitmap.unlockPixels();

    if (isTransparent) {
        SkSafeUnref(image);
    } else {
        *imageData = image;
    }

    if (isTransparent || !hasAlpha) {
        SkSafeUnref(alpha);
    } else {
        *alphaData = alpha;
    }
}

SkPDFArray* makeIndexedColorSpace(SkColorTable* table) {
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

};  // namespace

// static
SkPDFImage* SkPDFImage::CreateImage(const SkBitmap& bitmap,
                                    const SkIRect& srcRect,
                                    const SkPaint& paint) {
    if (bitmap.getConfig() == SkBitmap::kNo_Config) {
        return NULL;
    }

    SkStream* imageData = NULL;
    SkStream* alphaData = NULL;
    extractImageData(bitmap, srcRect, &imageData, &alphaData);
    SkAutoUnref unrefImageData(imageData);
    SkAutoUnref unrefAlphaData(alphaData);
    if (!imageData) {
        SkASSERT(!alphaData);
        return NULL;
    }

    SkPDFImage* image =
        new SkPDFImage(imageData, bitmap, srcRect, false, paint);

    if (alphaData != NULL) {
        image->addSMask(new SkPDFImage(alphaData, bitmap, srcRect, true,
                                       paint))->unref();
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

void SkPDFImage::getResources(SkTDArray<SkPDFObject*>* resourceList) {
    GetResourcesHelper(&fResources, resourceList);
}

SkPDFImage::SkPDFImage(SkStream* imageData, const SkBitmap& bitmap,
                       const SkIRect& srcRect, bool doingAlpha,
                       const SkPaint& paint) {
    this->setData(imageData);
    SkBitmap::Config config = bitmap.getConfig();
    bool alphaOnly = (config == SkBitmap::kA1_Config ||
                      config == SkBitmap::kA8_Config);

    insertName("Type", "XObject");
    insertName("Subtype", "Image");

    if (!doingAlpha && alphaOnly) {
        // For alpha only images, we stretch a single pixel of black for
        // the color/shape part.
        SkRefPtr<SkPDFInt> one = new SkPDFInt(1);
        one->unref();  // SkRefPtr and new both took a reference.
        insert("Width", one.get());
        insert("Height", one.get());
    } else {
        insertInt("Width", srcRect.width());
        insertInt("Height", srcRect.height());
    }

    // if (!image mask) {
    if (doingAlpha || alphaOnly) {
        insertName("ColorSpace", "DeviceGray");
    } else if (config == SkBitmap::kIndex8_Config ||
        config == SkBitmap::kRLE_Index8_Config) {
        insert("ColorSpace",
               makeIndexedColorSpace(bitmap.getColorTable()))->unref();
    } else {
        insertName("ColorSpace", "DeviceRGB");
    }
    // }

    int bitsPerComp = 8;
    if (config == SkBitmap::kARGB_4444_Config) {
        bitsPerComp = 4;
    } else if (doingAlpha && config == SkBitmap::kA1_Config) {
        bitsPerComp = 1;
    }
    insertInt("BitsPerComponent", bitsPerComp);

    if (config == SkBitmap::kRGB_565_Config) {
        SkRefPtr<SkPDFInt> zeroVal = new SkPDFInt(0);
        zeroVal->unref();  // SkRefPtr and new both took a reference.
        SkRefPtr<SkPDFScalar> scale5Val =
                new SkPDFScalar(8.2258f);  // 255/2^5-1
        scale5Val->unref();  // SkRefPtr and new both took a reference.
        SkRefPtr<SkPDFScalar> scale6Val =
                new SkPDFScalar(4.0476f);  // 255/2^6-1
        scale6Val->unref();  // SkRefPtr and new both took a reference.
        SkRefPtr<SkPDFArray> decodeValue = new SkPDFArray();
        decodeValue->unref();  // SkRefPtr and new both took a reference.
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
