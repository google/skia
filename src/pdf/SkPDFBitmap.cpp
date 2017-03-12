/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkData.h"
#include "SkDeflate.h"
#include "SkImage_Base.h"
#include "SkJpegInfo.h"
#include "SkPDFBitmap.h"
#include "SkPDFCanon.h"
#include "SkPDFTypes.h"
#include "SkStream.h"
#include "SkUnPreMultiply.h"

void image_get_ro_pixels(const SkImage* image, SkBitmap* dst) {
    SkColorSpace* legacyColorSpace = nullptr;
    if(as_IB(image)->getROPixels(dst, legacyColorSpace)
       && dst->dimensions() == image->dimensions()) {
        if (dst->colorType() != kIndex_8_SkColorType) {
            return;
        }
        // We must check to see if the bitmap has a color table.
        SkAutoLockPixels autoLockPixels(*dst);
        if (!dst->getColorTable()) {
            // We can't use an indexed bitmap with no colortable.
            dst->reset();
        } else {
            return;
        }
    }
    // no pixels or wrong size: fill with zeros.
    dst->setInfo(SkImageInfo::MakeN32(image->width(), image->height(), image->alphaType()));
}

bool image_compute_is_opaque(const SkImage* image) {
    if (image->isOpaque()) {
        return true;
    }
    // keep output PDF small at cost of possible resource use.
    SkBitmap bm;
    image_get_ro_pixels(image, &bm);
    return SkBitmap::ComputeIsOpaque(bm);
}

////////////////////////////////////////////////////////////////////////////////

static void pdf_stream_begin(SkWStream* stream) {
    static const char streamBegin[] = " stream\n";
    stream->write(streamBegin, strlen(streamBegin));
}

static void pdf_stream_end(SkWStream* stream) {
    static const char streamEnd[] = "\nendstream";
    stream->write(streamEnd, strlen(streamEnd));
}

////////////////////////////////////////////////////////////////////////////////

// write a single byte to a stream n times.
static void fill_stream(SkWStream* out, char value, size_t n) {
    char buffer[4096];
    memset(buffer, value, sizeof(buffer));
    for (size_t i = 0; i < n / sizeof(buffer); ++i) {
        out->write(buffer, sizeof(buffer));
    }
    out->write(buffer, n % sizeof(buffer));
}

// TODO(reed@): Decide if these five functions belong in SkColorPriv.h
static bool SkIsBGRA(SkColorType ct) {
    SkASSERT(kBGRA_8888_SkColorType == ct || kRGBA_8888_SkColorType == ct);
    return kBGRA_8888_SkColorType == ct;
}

// Interpret value as the given 4-byte SkColorType (BGRA_8888 or
// RGBA_8888) and return the appropriate component.  Each component
// should be interpreted according to the associated SkAlphaType and
// SkColorProfileType.
static U8CPU SkGetA32Component(uint32_t value, SkColorType ct) {
    return (value >> (SkIsBGRA(ct) ? SK_BGRA_A32_SHIFT : SK_RGBA_A32_SHIFT)) & 0xFF;
}
static U8CPU SkGetR32Component(uint32_t value, SkColorType ct) {
    return (value >> (SkIsBGRA(ct) ? SK_BGRA_R32_SHIFT : SK_RGBA_R32_SHIFT)) & 0xFF;
}
static U8CPU SkGetG32Component(uint32_t value, SkColorType ct) {
    return (value >> (SkIsBGRA(ct) ? SK_BGRA_G32_SHIFT : SK_RGBA_G32_SHIFT)) & 0xFF;
}
static U8CPU SkGetB32Component(uint32_t value, SkColorType ct) {
    return (value >> (SkIsBGRA(ct) ? SK_BGRA_B32_SHIFT : SK_RGBA_B32_SHIFT)) & 0xFF;
}

// unpremultiply and extract R, G, B components.
static void pmcolor_to_rgb24(uint32_t color, uint8_t* rgb, SkColorType ct) {
    uint32_t s = SkUnPreMultiply::GetScale(SkGetA32Component(color, ct));
    rgb[0] = SkUnPreMultiply::ApplyScale(s, SkGetR32Component(color, ct));
    rgb[1] = SkUnPreMultiply::ApplyScale(s, SkGetG32Component(color, ct));
    rgb[2] = SkUnPreMultiply::ApplyScale(s, SkGetB32Component(color, ct));
}

/* It is necessary to average the color component of transparent
   pixels with their surrounding neighbors since the PDF renderer may
   separately re-sample the alpha and color channels when the image is
   not displayed at its native resolution. Since an alpha of zero
   gives no information about the color component, the pathological
   case is a white image with sharp transparency bounds - the color
   channel goes to black, and the should-be-transparent pixels are
   rendered as grey because of the separate soft mask and color
   resizing. e.g.: gm/bitmappremul.cpp */
static void get_neighbor_avg_color(const SkBitmap& bm,
                                   int xOrig,
                                   int yOrig,
                                   uint8_t rgb[3],
                                   SkColorType ct) {
    unsigned a = 0, r = 0, g = 0, b = 0;
    // Clamp the range to the edge of the bitmap.
    int ymin = SkTMax(0, yOrig - 1);
    int ymax = SkTMin(yOrig + 1, bm.height() - 1);
    int xmin = SkTMax(0, xOrig - 1);
    int xmax = SkTMin(xOrig + 1, bm.width() - 1);
    for (int y = ymin; y <= ymax; ++y) {
        uint32_t* scanline = bm.getAddr32(0, y);
        for (int x = xmin; x <= xmax; ++x) {
            uint32_t color = scanline[x];
            a += SkGetA32Component(color, ct);
            r += SkGetR32Component(color, ct);
            g += SkGetG32Component(color, ct);
            b += SkGetB32Component(color, ct);
        }
    }
    if (a > 0) {
        rgb[0] = SkToU8(255 * r / a);
        rgb[1] = SkToU8(255 * g / a);
        rgb[2] = SkToU8(255 * b / a);
    } else {
        rgb[0] = rgb[1] = rgb[2] = 0;
    }
}

static size_t pixel_count(const SkBitmap& bm) {
    return SkToSizeT(bm.width()) * SkToSizeT(bm.height());
}

static const SkBitmap& not4444(const SkBitmap& input, SkBitmap* copy) {
    if (input.colorType() != kARGB_4444_SkColorType) {
        return input;
    }
    // ARGB_4444 is rarely used, so we can do a wasteful tmp copy.
    SkAssertResult(input.copyTo(copy, kN32_SkColorType));
    copy->setImmutable();
    return *copy;
}

static size_t pdf_color_component_count(SkColorType ct) {
    switch (ct) {
        case kRGB_565_SkColorType:
        case kARGB_4444_SkColorType:
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            return 3;
        case kAlpha_8_SkColorType:
        case kIndex_8_SkColorType:
        case kGray_8_SkColorType:
            return 1;
        case kUnknown_SkColorType:
        default:
            SkDEBUGFAIL("unexpected color type");
            return 0;
    }
}

static void bitmap_to_pdf_pixels(const SkBitmap& bitmap, SkWStream* out) {
    if (!bitmap.getPixels()) {
        size_t size = pixel_count(bitmap) *
                      pdf_color_component_count(bitmap.colorType());
        fill_stream(out, '\x00', size);
        return;
    }
    SkBitmap copy;
    const SkBitmap& bm = not4444(bitmap, &copy);
    SkAutoLockPixels autoLockPixels(bm);
    SkColorType colorType = bm.colorType();
    SkAlphaType alphaType = bm.alphaType();
    switch (colorType) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType: {
            SkASSERT(3 == pdf_color_component_count(colorType));
            SkAutoTMalloc<uint8_t> scanline(3 * bm.width());
            for (int y = 0; y < bm.height(); ++y) {
                const uint32_t* src = bm.getAddr32(0, y);
                uint8_t* dst = scanline.get();
                for (int x = 0; x < bm.width(); ++x) {
                    if (alphaType == kPremul_SkAlphaType) {
                        uint32_t color = *src++;
                        U8CPU alpha = SkGetA32Component(color, colorType);
                        if (alpha != SK_AlphaTRANSPARENT) {
                            pmcolor_to_rgb24(color, dst, colorType);
                        } else {
                            get_neighbor_avg_color(bm, x, y, dst, colorType);
                        }
                        dst += 3;
                    } else {
                        uint32_t color = *src++;
                        *dst++ = SkGetR32Component(color, colorType);
                        *dst++ = SkGetG32Component(color, colorType);
                        *dst++ = SkGetB32Component(color, colorType);
                    }
                }
                out->write(scanline.get(), 3 * bm.width());
            }
            return;
        }
        case kRGB_565_SkColorType: {
            SkASSERT(3 == pdf_color_component_count(colorType));
            SkAutoTMalloc<uint8_t> scanline(3 * bm.width());
            for (int y = 0; y < bm.height(); ++y) {
                const uint16_t* src = bm.getAddr16(0, y);
                uint8_t* dst = scanline.get();
                for (int x = 0; x < bm.width(); ++x) {
                    U16CPU color565 = *src++;
                    *dst++ = SkPacked16ToR32(color565);
                    *dst++ = SkPacked16ToG32(color565);
                    *dst++ = SkPacked16ToB32(color565);
                }
                out->write(scanline.get(), 3 * bm.width());
            }
            return;
        }
        case kAlpha_8_SkColorType:
            SkASSERT(1 == pdf_color_component_count(colorType));
            fill_stream(out, '\x00', pixel_count(bm));
            return;
        case kGray_8_SkColorType:
        case kIndex_8_SkColorType:
            SkASSERT(1 == pdf_color_component_count(colorType));
            // these two formats need no transformation to serialize.
            for (int y = 0; y < bm.height(); ++y) {
                out->write(bm.getAddr8(0, y), bm.width());
            }
            return;
        case kUnknown_SkColorType:
        case kARGB_4444_SkColorType:
        default:
            SkDEBUGFAIL("unexpected color type");
    }
}

////////////////////////////////////////////////////////////////////////////////

static void bitmap_alpha_to_a8(const SkBitmap& bitmap, SkWStream* out) {
    if (!bitmap.getPixels()) {
        fill_stream(out, '\xFF', pixel_count(bitmap));
        return;
    }
    SkBitmap copy;
    const SkBitmap& bm = not4444(bitmap, &copy);
    SkAutoLockPixels autoLockPixels(bm);
    SkColorType colorType = bm.colorType();
    switch (colorType) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType: {
            SkAutoTMalloc<uint8_t> scanline(bm.width());
            for (int y = 0; y < bm.height(); ++y) {
                uint8_t* dst = scanline.get();
                const SkPMColor* src = bm.getAddr32(0, y);
                for (int x = 0; x < bm.width(); ++x) {
                    *dst++ = SkGetA32Component(*src++, colorType);
                }
                out->write(scanline.get(), bm.width());
            }
            return;
        }
        case kAlpha_8_SkColorType:
            for (int y = 0; y < bm.height(); ++y) {
                out->write(bm.getAddr8(0, y), bm.width());
            }
            return;
        case kIndex_8_SkColorType: {
            SkColorTable* ct = bm.getColorTable();
            SkASSERT(ct);
            SkAutoTMalloc<uint8_t> scanline(bm.width());
            for (int y = 0; y < bm.height(); ++y) {
                uint8_t* dst = scanline.get();
                const uint8_t* src = bm.getAddr8(0, y);
                for (int x = 0; x < bm.width(); ++x) {
                    *dst++ = SkGetPackedA32((*ct)[*src++]);
                }
                out->write(scanline.get(), bm.width());
            }
            return;
        }
        case kRGB_565_SkColorType:
        case kGray_8_SkColorType:
            SkDEBUGFAIL("color type has no alpha");
            return;
        case kARGB_4444_SkColorType:
            SkDEBUGFAIL("4444 color type should have been converted to N32");
            return;
        case kUnknown_SkColorType:
        default:
            SkDEBUGFAIL("unexpected color type");
    }
}

static sk_sp<SkPDFArray> make_indexed_color_space(
        const SkColorTable* table,
        SkAlphaType alphaType) {
    auto result = sk_make_sp<SkPDFArray>();
    result->reserve(4);
    result->appendName("Indexed");
    result->appendName("DeviceRGB");
    SkASSERT(table);
    if (table->count() < 1) {
        result->appendInt(0);
        char shortTableArray[3] = {0, 0, 0};
        SkString tableString(shortTableArray, SK_ARRAY_COUNT(shortTableArray));
        result->appendString(tableString);
        return result;
    }
    result->appendInt(table->count() - 1);  // maximum color index.

    // Potentially, this could be represented in fewer bytes with a stream.
    // Max size as a string is 1.5k.
    char tableArray[256 * 3];
    SkASSERT(3u * table->count() <= SK_ARRAY_COUNT(tableArray));
    uint8_t* tablePtr = reinterpret_cast<uint8_t*>(tableArray);
    const SkPMColor* colors = table->readColors();
    for (int i = 0; i < table->count(); i++) {
        if (alphaType == kPremul_SkAlphaType) {
            pmcolor_to_rgb24(colors[i], tablePtr, kN32_SkColorType);
            tablePtr += 3;
        } else {
            *tablePtr++ = SkGetR32Component(colors[i], kN32_SkColorType);
            *tablePtr++ = SkGetG32Component(colors[i], kN32_SkColorType);
            *tablePtr++ = SkGetB32Component(colors[i], kN32_SkColorType);
        }
    }
    SkString tableString(tableArray, 3 * table->count());
    result->appendString(tableString);
    return result;
}

static void emit_image_xobject(SkWStream* stream,
                               const SkImage* image,
                               bool alpha,
                               const sk_sp<SkPDFObject>& smask,
                               const SkPDFObjNumMap& objNumMap) {
    SkBitmap bitmap;
    image_get_ro_pixels(image, &bitmap);      // TODO(halcanary): test
    SkAutoLockPixels autoLockPixels(bitmap);  // with malformed images.

    // Write to a temporary buffer to get the compressed length.
    SkDynamicMemoryWStream buffer;
    SkDeflateWStream deflateWStream(&buffer);
    if (alpha) {
        bitmap_alpha_to_a8(bitmap, &deflateWStream);
    } else {
        bitmap_to_pdf_pixels(bitmap, &deflateWStream);
    }
    deflateWStream.finalize();  // call before detachAsStream().
    std::unique_ptr<SkStreamAsset> asset(buffer.detachAsStream());

    SkPDFDict pdfDict("XObject");
    pdfDict.insertName("Subtype", "Image");
    pdfDict.insertInt("Width", bitmap.width());
    pdfDict.insertInt("Height", bitmap.height());
    if (alpha) {
        pdfDict.insertName("ColorSpace", "DeviceGray");
    } else if (bitmap.colorType() == kIndex_8_SkColorType) {
        SkASSERT(1 == pdf_color_component_count(bitmap.colorType()));
        pdfDict.insertObject("ColorSpace",
                             make_indexed_color_space(bitmap.getColorTable(),
                                                      bitmap.alphaType()));
    } else if (1 == pdf_color_component_count(bitmap.colorType())) {
        pdfDict.insertName("ColorSpace", "DeviceGray");
    } else {
        pdfDict.insertName("ColorSpace", "DeviceRGB");
    }
    if (smask) {
        pdfDict.insertObjRef("SMask", smask);
    }
    pdfDict.insertInt("BitsPerComponent", 8);
    pdfDict.insertName("Filter", "FlateDecode");
    pdfDict.insertInt("Length", asset->getLength());
    pdfDict.emitObject(stream, objNumMap);

    pdf_stream_begin(stream);
    stream->writeStream(asset.get(), asset->getLength());
    pdf_stream_end(stream);
}

////////////////////////////////////////////////////////////////////////////////

namespace {
// This SkPDFObject only outputs the alpha layer of the given bitmap.
class PDFAlphaBitmap final : public SkPDFObject {
public:
    PDFAlphaBitmap(sk_sp<SkImage> image) : fImage(std::move(image)) { SkASSERT(fImage); }
    void emitObject(SkWStream*  stream,
                    const SkPDFObjNumMap& objNumMap) const override {
        SkASSERT(fImage);
        emit_image_xobject(stream, fImage.get(), true, nullptr, objNumMap);
    }
    void drop() override { fImage = nullptr; }

private:
    sk_sp<SkImage> fImage;
};

}  // namespace

////////////////////////////////////////////////////////////////////////////////

namespace {
class PDFDefaultBitmap final : public SkPDFObject {
public:
    void emitObject(SkWStream* stream,
                    const SkPDFObjNumMap& objNumMap) const override {
        SkASSERT(fImage);
        emit_image_xobject(stream, fImage.get(), false, fSMask, objNumMap);
    }
    void addResources(SkPDFObjNumMap* catalog) const override {
        catalog->addObjectRecursively(fSMask.get());
    }
    void drop() override { fImage = nullptr; fSMask = nullptr; }
    PDFDefaultBitmap(sk_sp<SkImage> image, sk_sp<SkPDFObject> smask)
        : fImage(std::move(image)), fSMask(std::move(smask)) { SkASSERT(fImage); }

private:
    sk_sp<SkImage> fImage;
    sk_sp<SkPDFObject> fSMask;
};
}  // namespace

////////////////////////////////////////////////////////////////////////////////

namespace {
/**
 *  This PDFObject assumes that its constructor was handed YUV or
 *  Grayscale JFIF Jpeg-encoded data that can be directly embedded
 *  into a PDF.
 */
class PDFJpegBitmap final : public SkPDFObject {
public:
    SkISize fSize;
    sk_sp<SkData> fData;
    bool fIsYUV;
    PDFJpegBitmap(SkISize size, SkData* data, bool isYUV)
        : fSize(size), fData(SkRef(data)), fIsYUV(isYUV) { SkASSERT(data); }
    void emitObject(SkWStream*, const SkPDFObjNumMap&) const override;
    void drop() override { fData = nullptr; }
};

void PDFJpegBitmap::emitObject(SkWStream* stream,
                               const SkPDFObjNumMap& objNumMap) const {
    SkASSERT(fData);
    SkPDFDict pdfDict("XObject");
    pdfDict.insertName("Subtype", "Image");
    pdfDict.insertInt("Width", fSize.width());
    pdfDict.insertInt("Height", fSize.height());
    if (fIsYUV) {
        pdfDict.insertName("ColorSpace", "DeviceRGB");
    } else {
        pdfDict.insertName("ColorSpace", "DeviceGray");
    }
    pdfDict.insertInt("BitsPerComponent", 8);
    pdfDict.insertName("Filter", "DCTDecode");
    pdfDict.insertInt("ColorTransform", 0);
    pdfDict.insertInt("Length", SkToInt(fData->size()));
    pdfDict.emitObject(stream, objNumMap);
    pdf_stream_begin(stream);
    stream->write(fData->data(), fData->size());
    pdf_stream_end(stream);
}
}  // namespace

////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPDFObject> SkPDFCreateBitmapObject(sk_sp<SkImage> image,
                                           SkPixelSerializer* pixelSerializer) {
    SkASSERT(image);
    sk_sp<SkData> data(image->refEncoded());
    SkJFIFInfo info;
    if (data && SkIsJFIF(data.get(), &info) &&
        (!pixelSerializer ||
         pixelSerializer->useEncodedData(data->data(), data->size()))) {
        // If there is a SkPixelSerializer, give it a chance to
        // re-encode the JPEG with more compression by returning false
        // from useEncodedData.
        bool yuv = info.fType == SkJFIFInfo::kYCbCr;
        if (info.fSize == image->dimensions()) {  // Sanity check.
            // hold on to data, not image.
            #ifdef SK_PDF_IMAGE_STATS
            gJpegImageObjects.fetch_add(1);
            #endif
            return sk_make_sp<PDFJpegBitmap>(info.fSize, data.get(), yuv);
        }
    }

    if (pixelSerializer) {
        SkBitmap bm;
        SkAutoPixmapUnlock apu;
        SkColorSpace* legacyColorSpace = nullptr;
        if (as_IB(image.get())->getROPixels(&bm, legacyColorSpace) &&
            bm.requestLock(&apu)) {
            data.reset(pixelSerializer->encode(apu.pixmap()));
            if (data && SkIsJFIF(data.get(), &info)) {
                bool yuv = info.fType == SkJFIFInfo::kYCbCr;
                if (info.fSize == image->dimensions()) {  // Sanity check.
                    return sk_make_sp<PDFJpegBitmap>(info.fSize, data.get(), yuv);
                }
            }
        }
    }

    sk_sp<SkPDFObject> smask;
    if (!image_compute_is_opaque(image.get())) {
        smask = sk_make_sp<PDFAlphaBitmap>(image);
    }
    #ifdef SK_PDF_IMAGE_STATS
    gRegularImageObjects.fetch_add(1);
    #endif
    return sk_make_sp<PDFDefaultBitmap>(std::move(image), std::move(smask));
}
