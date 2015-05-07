/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkData.h"
#include "SkFlate.h"
#include "SkImageGenerator.h"
#include "SkJpegInfo.h"
#include "SkPDFBitmap.h"
#include "SkPDFCanon.h"
#include "SkPixelRef.h"
#include "SkStream.h"
#include "SkUnPreMultiply.h"

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

// unpremultiply and extract R, G, B components.
static void pmcolor_to_rgb24(SkPMColor pmColor, uint8_t* rgb) {
    uint32_t s = SkUnPreMultiply::GetScale(SkGetPackedA32(pmColor));
    rgb[0] = SkUnPreMultiply::ApplyScale(s, SkGetPackedR32(pmColor));
    rgb[1] = SkUnPreMultiply::ApplyScale(s, SkGetPackedG32(pmColor));
    rgb[2] = SkUnPreMultiply::ApplyScale(s, SkGetPackedB32(pmColor));
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
                                   uint8_t rgb[3]) {
    SkASSERT(kN32_SkColorType == bm.colorType());
    unsigned a = 0, r = 0, g = 0, b = 0;
    // Clamp the range to the edge of the bitmap.
    int ymin = SkTMax(0, yOrig - 1);
    int ymax = SkTMin(yOrig + 1, bm.height() - 1);
    int xmin = SkTMax(0, xOrig - 1);
    int xmax = SkTMin(xOrig + 1, bm.width() - 1);
    for (int y = ymin; y <= ymax; ++y) {
        SkPMColor* scanline = bm.getAddr32(0, y);
        for (int x = xmin; x <= xmax; ++x) {
            SkPMColor pmColor = scanline[x];
            a += SkGetPackedA32(pmColor);
            r += SkGetPackedR32(pmColor);
            g += SkGetPackedG32(pmColor);
            b += SkGetPackedB32(pmColor);
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
        case kN32_SkColorType:
        case kRGB_565_SkColorType:
        case kARGB_4444_SkColorType:
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
    switch (bm.colorType()) {
        case kN32_SkColorType: {
            SkASSERT(3 == pdf_color_component_count(bitmap.colorType()));
            SkAutoTMalloc<uint8_t> scanline(3 * bm.width());
            for (int y = 0; y < bm.height(); ++y) {
                const SkPMColor* src = bm.getAddr32(0, y);
                uint8_t* dst = scanline.get();
                for (int x = 0; x < bm.width(); ++x) {
                    SkPMColor color = *src++;
                    U8CPU alpha = SkGetPackedA32(color);
                    if (alpha != SK_AlphaTRANSPARENT) {
                        pmcolor_to_rgb24(color, dst);
                    } else {
                        get_neighbor_avg_color(bm, x, y, dst);
                    }
                    dst += 3;
                }
                out->write(scanline.get(), 3 * bm.width());
            }
            return;
        }
        case kRGB_565_SkColorType: {
            SkASSERT(3 == pdf_color_component_count(bitmap.colorType()));
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
            SkASSERT(1 == pdf_color_component_count(bitmap.colorType()));
            fill_stream(out, '\x00', pixel_count(bm));
            return;
        case kGray_8_SkColorType:
        case kIndex_8_SkColorType:
            SkASSERT(1 == pdf_color_component_count(bitmap.colorType()));
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
    switch (bm.colorType()) {
        case kN32_SkColorType: {
            SkAutoTMalloc<uint8_t> scanline(bm.width());
            for (int y = 0; y < bm.height(); ++y) {
                uint8_t* dst = scanline.get();
                const SkPMColor* src = bm.getAddr32(0, y);
                for (int x = 0; x < bm.width(); ++x) {
                    *dst++ = SkGetPackedA32(*src++);
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

////////////////////////////////////////////////////////////////////////////////

namespace {
// This SkPDFObject only outputs the alpha layer of the given bitmap.
class PDFAlphaBitmap : public SkPDFObject {
public:
    PDFAlphaBitmap(const SkBitmap& bm) : fBitmap(bm) {}
    ~PDFAlphaBitmap() {}
    void emitObject(SkWStream*,
                    const SkPDFObjNumMap&,
                    const SkPDFSubstituteMap&) override;

private:
    const SkBitmap fBitmap;
};

void PDFAlphaBitmap::emitObject(SkWStream* stream,
                                const SkPDFObjNumMap& objNumMap,
                                const SkPDFSubstituteMap& substitutes) {
    SkAutoLockPixels autoLockPixels(fBitmap);
    SkASSERT(fBitmap.colorType() != kIndex_8_SkColorType ||
             fBitmap.getColorTable());

    // Write to a temporary buffer to get the compressed length.
    SkDynamicMemoryWStream buffer;
    SkDeflateWStream deflateWStream(&buffer);
    bitmap_alpha_to_a8(fBitmap, &deflateWStream);
    deflateWStream.finalize();  // call before detachAsStream().
    SkAutoTDelete<SkStreamAsset> asset(buffer.detachAsStream());

    SkPDFDict pdfDict("XObject");
    pdfDict.insertName("Subtype", "Image");
    pdfDict.insertInt("Width", fBitmap.width());
    pdfDict.insertInt("Height", fBitmap.height());
    pdfDict.insertName("ColorSpace", "DeviceGray");
    pdfDict.insertInt("BitsPerComponent", 8);
    pdfDict.insertName("Filter", "FlateDecode");
    pdfDict.insertInt("Length", asset->getLength());
    pdfDict.emitObject(stream, objNumMap, substitutes);

    pdf_stream_begin(stream);
    stream->writeStream(asset.get(), asset->getLength());
    pdf_stream_end(stream);
}
}  // namespace

////////////////////////////////////////////////////////////////////////////////

namespace {
class PDFDefaultBitmap : public SkPDFBitmap {
public:
    const SkAutoTUnref<SkPDFObject> fSMask;
    void emitObject(SkWStream*,
                    const SkPDFObjNumMap&,
                    const SkPDFSubstituteMap&) override;
    void addResources(SkPDFObjNumMap*,
                      const SkPDFSubstituteMap&) const override;
    PDFDefaultBitmap(const SkBitmap& bm, SkPDFObject* smask)
        : SkPDFBitmap(bm), fSMask(smask) {}
};
}  // namespace

void PDFDefaultBitmap::addResources(
        SkPDFObjNumMap* catalog,
        const SkPDFSubstituteMap& substitutes) const {
    if (fSMask.get()) {
        SkPDFObject* obj = substitutes.getSubstitute(fSMask.get());
        SkASSERT(obj);
        if (catalog->addObject(obj)) {
            obj->addResources(catalog, substitutes);
        }
    }
}

static SkPDFArray* make_indexed_color_space(const SkColorTable* table) {
    SkPDFArray* result = SkNEW(SkPDFArray);
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
        pmcolor_to_rgb24(colors[i], tablePtr);
        tablePtr += 3;
    }
    SkString tableString(tableArray, 3 * table->count());
    result->appendString(tableString);
    return result;
}

void PDFDefaultBitmap::emitObject(SkWStream* stream,
                                  const SkPDFObjNumMap& objNumMap,
                                  const SkPDFSubstituteMap& substitutes) {
    SkAutoLockPixels autoLockPixels(fBitmap);
    SkASSERT(fBitmap.colorType() != kIndex_8_SkColorType ||
             fBitmap.getColorTable());

    // Write to a temporary buffer to get the compressed length.
    SkDynamicMemoryWStream buffer;
    SkDeflateWStream deflateWStream(&buffer);
    bitmap_to_pdf_pixels(fBitmap, &deflateWStream);
    deflateWStream.finalize();  // call before detachAsStream().
    SkAutoTDelete<SkStreamAsset> asset(buffer.detachAsStream());

    SkPDFDict pdfDict("XObject");
    pdfDict.insertName("Subtype", "Image");
    pdfDict.insertInt("Width", fBitmap.width());
    pdfDict.insertInt("Height", fBitmap.height());
    if (fBitmap.colorType() == kIndex_8_SkColorType) {
        SkASSERT(1 == pdf_color_component_count(fBitmap.colorType()));
        pdfDict.insertObject("ColorSpace",
                             make_indexed_color_space(fBitmap.getColorTable()));
    } else if (1 == pdf_color_component_count(fBitmap.colorType())) {
        pdfDict.insertName("ColorSpace", "DeviceGray");
    } else {
        pdfDict.insertName("ColorSpace", "DeviceRGB");
    }
    pdfDict.insertInt("BitsPerComponent", 8);
    if (fSMask) {
        pdfDict.insertObjRef("SMask", SkRef(fSMask.get()));
    }
    pdfDict.insertName("Filter", "FlateDecode");
    pdfDict.insertInt("Length", asset->getLength());
    pdfDict.emitObject(stream, objNumMap, substitutes);

    pdf_stream_begin(stream);
    stream->writeStream(asset.get(), asset->getLength());
    pdf_stream_end(stream);
}

////////////////////////////////////////////////////////////////////////////////

static const SkBitmap& immutable_bitmap(const SkBitmap& bm, SkBitmap* copy) {
    if (bm.isImmutable()) {
        return bm;
    }
    bm.copyTo(copy);
    copy->setImmutable();
    return *copy;
}

namespace {
/**
 *  This PDFObject assumes that its constructor was handed YUV JFIF
 *  Jpeg-encoded data that can be directly embedded into a PDF.
 */
class PDFJpegBitmap : public SkPDFBitmap {
public:
    SkAutoTUnref<SkData> fData;
    bool fIsYUV;
    PDFJpegBitmap(const SkBitmap& bm, SkData* data, bool isYUV)
        : SkPDFBitmap(bm), fData(SkRef(data)), fIsYUV(isYUV) {}
    void emitObject(SkWStream*,
                    const SkPDFObjNumMap&,
                    const SkPDFSubstituteMap&) override;
};

void PDFJpegBitmap::emitObject(SkWStream* stream,
                               const SkPDFObjNumMap& objNumMap,
                               const SkPDFSubstituteMap& substituteMap) {
    SkPDFDict pdfDict("XObject");
    pdfDict.insertName("Subtype", "Image");
    pdfDict.insertInt("Width", fBitmap.width());
    pdfDict.insertInt("Height", fBitmap.height());
    if (fIsYUV) {
        pdfDict.insertName("ColorSpace", "DeviceRGB");
    } else {
        pdfDict.insertName("ColorSpace", "DeviceGray");
    }
    pdfDict.insertInt("BitsPerComponent", 8);
    pdfDict.insertName("Filter", "DCTDecode");
    pdfDict.insertInt("ColorTransform", 0);
    pdfDict.insertInt("Length", SkToInt(fData->size()));
    pdfDict.emitObject(stream, objNumMap, substituteMap);
    pdf_stream_begin(stream);
    stream->write(fData->data(), fData->size());
    pdf_stream_end(stream);
}
}  // namespace

////////////////////////////////////////////////////////////////////////////////

SkPDFBitmap* SkPDFBitmap::Create(SkPDFCanon* canon, const SkBitmap& bitmap) {
    SkASSERT(canon);
    if (!SkColorTypeIsValid(bitmap.colorType()) ||
        kUnknown_SkColorType == bitmap.colorType()) {
        return NULL;
    }
    SkBitmap copy;
    const SkBitmap& bm = immutable_bitmap(bitmap, &copy);
    if (bm.drawsNothing()) {
        return NULL;
    }
    if (SkPDFBitmap* canonBitmap = canon->findBitmap(bm)) {
        return SkRef(canonBitmap);
    }

    if (bm.pixelRef() && bm.pixelRefOrigin().isZero() &&
        bm.dimensions() == bm.pixelRef()->info().dimensions()) {
        // Requires the bitmap to be backed by lazy pixels.
        SkAutoTUnref<SkData> data(bm.pixelRef()->refEncodedData());
        SkJFIFInfo info;
        if (data && SkIsJFIF(data, &info)) {
            bool yuv = info.fType == SkJFIFInfo::kYCbCr;
            SkPDFBitmap* pdfBitmap = SkNEW_ARGS(PDFJpegBitmap, (bm, data, yuv));
            canon->addBitmap(pdfBitmap);
            return pdfBitmap;
        }
    }

    SkPDFObject* smask = NULL;
    if (!bm.isOpaque() && !SkBitmap::ComputeIsOpaque(bm)) {
        smask = SkNEW_ARGS(PDFAlphaBitmap, (bm));
    }
    SkPDFBitmap* pdfBitmap = SkNEW_ARGS(PDFDefaultBitmap, (bm, smask));
    canon->addBitmap(pdfBitmap);
    return pdfBitmap;
}
