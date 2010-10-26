/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkPDFImage.h"

#include "SkBitmap.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkPaint.h"
#include "SkPackBits.h"
#include "SkPDFCatalog.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkUnPreMultiply.h"

namespace {

SkMemoryStream* extractImageData(const SkBitmap& bitmap) {
    SkMemoryStream* result;

    bitmap.lockPixels();
    switch (bitmap.getConfig()) {
        case SkBitmap::kIndex8_Config:
            result = new SkMemoryStream(bitmap.getPixels(), bitmap.getSize(),
                                        true);
            break;
        case SkBitmap::kRLE_Index8_Config: {
            result = new SkMemoryStream(bitmap.getSize());
            const SkBitmap::RLEPixels* rle =
                (const SkBitmap::RLEPixels*)bitmap.getPixels();
            uint8_t* dst = (uint8_t*)result->getMemoryBase();
            const int width = bitmap.width();
            for (int y = 0; y < bitmap.height(); y++) {
                SkPackBits::Unpack8(rle->packedAtY(y), width, dst);
                dst += width;
            }
            break;
        }
        case SkBitmap::kARGB_4444_Config: {
            const int width = bitmap.width();
            const int rowBytes = (width * 3 + 1) / 2;
            result = new SkMemoryStream(rowBytes * bitmap.height());
            uint8_t* dst = (uint8_t*)result->getMemoryBase();
            for (int y = 0; y < bitmap.height(); y++) {
                uint16_t* src = bitmap.getAddr16(0, y);
                for (int x = 0; x < width; x += 2) {
                    dst[0] = (SkGetPackedR4444(src[0]) << 4) |
                        SkGetPackedG4444(src[0]);
                    dst[1] = (SkGetPackedB4444(src[0]) << 4) |
                        SkGetPackedR4444(src[1]);
                    dst[2] = (SkGetPackedG4444(src[1]) << 4) |
                        SkGetPackedB4444(src[1]);
                    src += 2;
                    dst += 3;
                }
                if (width & 1) {
                    dst[0] = (SkGetPackedR4444(src[0]) << 4) |
                        SkGetPackedG4444(src[0]);
                    dst[1] = (SkGetPackedB4444(src[0]) << 4);
                }
            }
            break;
        }
        case SkBitmap::kRGB_565_Config: {
            const int width = bitmap.width();
            const int rowBytes = width * 3;
            result = new SkMemoryStream(rowBytes * bitmap.height());
            uint8_t* dst = (uint8_t*)result->getMemoryBase();
            for (int y = 0; y < bitmap.height(); y++) {
                uint16_t* src = bitmap.getAddr16(0, y);
                for (int x = 0; x < width; x++) {
                    dst[0] = SkGetPackedR16(src[0]);
                    dst[1] = SkGetPackedG16(src[0]);
                    dst[2] = SkGetPackedB16(src[0]);
                    src++;
                    dst += 3;
                }
            }
            break;
        }
        case SkBitmap::kARGB_8888_Config: {
            const int width = bitmap.width();
            const int rowBytes = width * 3;
            result = new SkMemoryStream(rowBytes * bitmap.height());
            uint8_t* dst = (uint8_t*)result->getMemoryBase();
            for (int y = 0; y < bitmap.height(); y++) {
                uint32_t* src = bitmap.getAddr32(0, y);
                for (int x = 0; x < width; x++) {
                    dst[0] = SkGetPackedR32(src[0]);
                    dst[1] = SkGetPackedG32(src[0]);
                    dst[2] = SkGetPackedB32(src[0]);
                    src++;
                    dst += 3;
                }
            }
            break;
        }
        default:
            SkASSERT(false);
    }
    bitmap.unlockPixels();
    return result;
}

SkPDFArray* makeIndexedColorSpace(SkColorTable* table) {
    SkPDFArray* result = new SkPDFArray();
    result->reserve(4);
    SkRefPtr<SkPDFName> indexedName = new SkPDFName("Indexed");
    indexedName->unref();  // SkRefPtr and new both took a reference.
    result->append(indexedName.get());

    SkRefPtr<SkPDFName> rgbName = new SkPDFName("DeviceRGB");
    rgbName->unref();  // SkRefPtr and new both took a reference.
    result->append(rgbName.get());

    rgbName->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFInt> countValue = new SkPDFInt(table->count() - 1);
    result->append(countValue.get());

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
    SkRefPtr<SkPDFString> indexValue = new SkPDFString(index);
    indexValue->unref();  // SkRefPtr and new both took a reference.
    result->append(indexValue.get());
    return result;
}

};  // namespace

SkPDFImage::SkPDFImage(const SkBitmap& bitmap, const SkPaint& paint) {
    SkBitmap::Config config = bitmap.getConfig();

    // TODO(vandebo) Handle alpha and alpha only images correctly.
    SkASSERT(config == SkBitmap::kRGB_565_Config ||
             config == SkBitmap::kARGB_4444_Config ||
             config == SkBitmap::kARGB_8888_Config ||
             config == SkBitmap::kIndex8_Config ||
             config == SkBitmap::kRLE_Index8_Config);

    SkMemoryStream* image_data = extractImageData(bitmap);
    SkAutoUnref image_data_unref(image_data);
    fStream = new SkPDFStream(image_data);
    fStream->unref();  // SkRefPtr and new both took a reference.

    SkRefPtr<SkPDFName> typeValue = new SkPDFName("XObject");
    typeValue->unref();  // SkRefPtr and new both took a reference.
    insert("Type", typeValue.get());

    SkRefPtr<SkPDFName> subTypeValue = new SkPDFName("Image");
    subTypeValue->unref();  // SkRefPtr and new both took a reference.
    insert("Subtype", subTypeValue.get());

    SkRefPtr<SkPDFInt> widthValue = new SkPDFInt(bitmap.width());
    widthValue->unref();  // SkRefPtr and new both took a reference.
    insert("Width", widthValue.get());

    SkRefPtr<SkPDFInt> heightValue = new SkPDFInt(bitmap.height());
    heightValue->unref();  // SkRefPtr and new both took a reference.
    insert("Height", heightValue.get());

    // if (!image mask) {
    SkRefPtr<SkPDFObject> colorSpaceValue;
    if (config == SkBitmap::kIndex8_Config ||
        config == SkBitmap::kRLE_Index8_Config) {
        colorSpaceValue = makeIndexedColorSpace(bitmap.getColorTable());
    } else {
        colorSpaceValue = new SkPDFName("DeviceRGB");
    }
    colorSpaceValue->unref();  // SkRefPtr and new both took a reference.
    insert("ColorSpace", colorSpaceValue.get());
    // }

    int bitsPerComp = bitmap.bytesPerPixel() * 2;
    if (bitsPerComp == 0) {
        SkASSERT(config == SkBitmap::kA1_Config);
        bitsPerComp = 1;
    } else if (bitsPerComp == 2 ||
               (bitsPerComp == 4 && config == SkBitmap::kRGB_565_Config)) {
        bitsPerComp = 8;
    }
    SkRefPtr<SkPDFInt> bitsPerCompValue = new SkPDFInt(bitsPerComp);
    bitsPerCompValue->unref();  // SkRefPtr and new both took a reference.
    insert("BitsPerComponent", bitsPerCompValue.get());

    if (config == SkBitmap::kRGB_565_Config) {
        SkRefPtr<SkPDFInt> zeroVal = new SkPDFInt(0);
        zeroVal->unref();  // SkRefPtr and new both took a reference.
        SkRefPtr<SkPDFScalar> scale5Val = new SkPDFScalar(8.2258);  // 255/2^5-1
        scale5Val->unref();  // SkRefPtr and new both took a reference.
        SkRefPtr<SkPDFScalar> scale6Val = new SkPDFScalar(4.0476);  // 255/2^6-1
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

SkPDFImage::~SkPDFImage() {}

void SkPDFImage::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                             bool indirect) {
    if (indirect)
        return emitIndirectObject(stream, catalog);

    fStream->emitObject(stream, catalog, indirect);
}

size_t SkPDFImage::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    if (indirect)
        return getIndirectOutputSize(catalog);

    return fStream->getOutputSize(catalog, indirect);
}

void SkPDFImage::insert(SkPDFName* key, SkPDFObject* value) {
    fStream->insert(key, value);
}

void SkPDFImage::insert(const char key[], SkPDFObject* value) {
    fStream->insert(key, value);
}
