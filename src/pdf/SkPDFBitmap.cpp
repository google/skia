/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFBitmap.h"

#include "SkColorData.h"
#include "SkData.h"
#include "SkDeflate.h"
#include "SkImage.h"
#include "SkImageInfoPriv.h"
#include "SkJpegInfo.h"
#include "SkPDFDocumentPriv.h"
#include "SkPDFTypes.h"
#include "SkPDFUtils.h"
#include "SkStream.h"
#include "SkTo.h"

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

/* It is necessary to average the color component of transparent
   pixels with their surrounding neighbors since the PDF renderer may
   separately re-sample the alpha and color channels when the image is
   not displayed at its native resolution. Since an alpha of zero
   gives no information about the color component, the pathological
   case is a white image with sharp transparency bounds - the color
   channel goes to black, and the should-be-transparent pixels are
   rendered as grey because of the separate soft mask and color
   resizing. e.g.: gm/bitmappremul.cpp */
static SkColor get_neighbor_avg_color(const SkPixmap& bm, int xOrig, int yOrig) {
    SkASSERT(kBGRA_8888_SkColorType == bm.colorType());
    unsigned r = 0, g = 0, b = 0, n = 0;
    // Clamp the range to the edge of the bitmap.
    int ymin = SkTMax(0, yOrig - 1);
    int ymax = SkTMin(yOrig + 1, bm.height() - 1);
    int xmin = SkTMax(0, xOrig - 1);
    int xmax = SkTMin(xOrig + 1, bm.width() - 1);
    for (int y = ymin; y <= ymax; ++y) {
        const SkColor* scanline = bm.addr32(0, y);
        for (int x = xmin; x <= xmax; ++x) {
            SkColor color = scanline[x];
            if (color != SK_ColorTRANSPARENT) {
                r += SkColorGetR(color);
                g += SkColorGetG(color);
                b += SkColorGetB(color);
                n++;
            }
        }
    }
    return n > 0 ? SkColorSetRGB(SkToU8(r / n), SkToU8(g / n), SkToU8(b / n))
                 : SK_ColorTRANSPARENT;
}

static void emit_stream(SkDynamicMemoryWStream* src, SkWStream* dst) {
    dst->writeText(" stream\n");
    src->writeToAndReset(dst);
    dst->writeText("\nendstream");
}

static void emit_dict(SkWStream* stream, SkISize size, const char* colorSpace,
                      const SkPDFIndirectReference* smask, int length) {
    SkPDFDict pdfDict("XObject");
    pdfDict.insertName("Subtype", "Image");
    pdfDict.insertInt("Width", size.width());
    pdfDict.insertInt("Height", size.height());
    pdfDict.insertName("ColorSpace", colorSpace);
    if (smask) {
        pdfDict.insertRef("SMask", *smask);
    }
    pdfDict.insertInt("BitsPerComponent", 8);
    pdfDict.insertName("Filter", "FlateDecode");
    pdfDict.insertInt("Length", length);
    pdfDict.emitObject(stream);
}

static SkPDFIndirectReference do_deflated_alpha(const SkPixmap& pm, SkPDFDocument* doc,
                                                SkPDFIndirectReference ref) {
    SkDynamicMemoryWStream buffer;
    SkDeflateWStream deflateWStream(&buffer);
    if (kAlpha_8_SkColorType == pm.colorType()) {
        SkASSERT(pm.rowBytes() == (size_t)pm.width());
        buffer.write(pm.addr8(), pm.width() * pm.height());
    } else {
        SkASSERT(pm.alphaType() == kUnpremul_SkAlphaType);
        SkASSERT(pm.colorType() == kBGRA_8888_SkColorType);
        SkASSERT(pm.rowBytes() == (size_t)pm.width() * 4);
        const uint32_t* ptr = pm.addr32();
        const uint32_t* stop = ptr + pm.height() * pm.width();

        uint8_t byteBuffer[4092];
        uint8_t* bufferStop = byteBuffer + SK_ARRAY_COUNT(byteBuffer);
        uint8_t* dst = byteBuffer;
        while (ptr != stop) {
            *dst++ = 0xFF & ((*ptr++) >> SK_BGRA_A32_SHIFT);
            if (dst == bufferStop) {
                deflateWStream.write(byteBuffer, sizeof(byteBuffer));
                dst = byteBuffer;
            }
        }
        deflateWStream.write(byteBuffer, dst - byteBuffer);
    }
    deflateWStream.finalize();
    SkWStream* stream = doc->beginObject(ref);
    emit_dict(stream, pm.info().dimensions(), "DeviceGray", nullptr, buffer.bytesWritten());
    emit_stream(&buffer, stream);
    doc->endObject();
    return ref;
}

static SkPDFIndirectReference do_deflated_image(const SkPixmap& pm,
                                                SkPDFDocument* doc,
                                                bool isOpaque) {
    SkPDFIndirectReference sMask;
    if (!isOpaque) {
        sMask = doc->reserve();
    }
    SkDynamicMemoryWStream buffer;
    SkDeflateWStream deflateWStream(&buffer);
    const char* colorSpace = "DeviceGray";
    switch (pm.colorType()) {
        case kAlpha_8_SkColorType:
            fill_stream(&deflateWStream, '\x00', pm.width() * pm.height());
            break;
        case kGray_8_SkColorType:
            SkASSERT(sMask.fValue = -1);
            SkASSERT(pm.rowBytes() == (size_t)pm.width());
            deflateWStream.write(pm.addr8(), pm.width() * pm.height());
            break;
        default:
            colorSpace = "DeviceRGB";
            SkASSERT(pm.alphaType() == kUnpremul_SkAlphaType);
            SkASSERT(pm.colorType() == kBGRA_8888_SkColorType);
            SkASSERT(pm.rowBytes() == (size_t)pm.width() * 4);
            uint8_t byteBuffer[3072];
            static_assert(SK_ARRAY_COUNT(byteBuffer) % 3 == 0, "");
            uint8_t* bufferStop = byteBuffer + SK_ARRAY_COUNT(byteBuffer);
            uint8_t* dst = byteBuffer;
            for (int y = 0; y < pm.height(); ++y) {
                const SkColor* src = pm.addr32(0, y);
                for (int x = 0; x < pm.width(); ++x) {
                    SkColor color = *src++;
                    if (SkColorGetA(color) == SK_AlphaTRANSPARENT) {
                        color = get_neighbor_avg_color(pm, x, y);
                    }
                    *dst++ = SkColorGetR(color);
                    *dst++ = SkColorGetG(color);
                    *dst++ = SkColorGetB(color);
                    if (dst == bufferStop) {
                        deflateWStream.write(byteBuffer, sizeof(byteBuffer));
                        dst = byteBuffer;
                    }
                }
            }
            deflateWStream.write(byteBuffer, dst - byteBuffer);
    }
    deflateWStream.finalize();
    SkPDFIndirectReference ref = doc->reserve();
    SkWStream* stream = doc->beginObject(ref);
    emit_dict(stream, pm.info().dimensions(), colorSpace,
              sMask.fValue != -1 ? &sMask : nullptr,
              buffer.bytesWritten());
    emit_stream(&buffer, stream);
    doc->endObject();
    if (!isOpaque) {
        do_deflated_alpha(pm, doc, sMask);
    }
    return ref;
}

static bool do_jpeg(const SkData& data, SkPDFDocument* doc, SkISize size,
                    SkPDFIndirectReference* result) {
    SkISize jpegSize;
    SkEncodedInfo::Color jpegColorType;
    SkEncodedOrigin exifOrientation;
    if (!SkGetJpegInfo(data.data(), data.size(), &jpegSize,
                       &jpegColorType, &exifOrientation)) {
        return false;
    }
    bool yuv = jpegColorType == SkEncodedInfo::kYUV_Color;
    bool goodColorType = yuv || jpegColorType == SkEncodedInfo::kGray_Color;
    if (jpegSize != size  // Sanity check.
            || !goodColorType
            || kTopLeft_SkEncodedOrigin != exifOrientation) {
        return false;
    }
    #ifdef SK_PDF_IMAGE_STATS
    gJpegImageObjects.fetch_add(1);
    #endif
    SkPDFIndirectReference ref = doc->reserve();
    *result = ref;
    SkWStream* stream = doc->beginObject(ref);

    SkPDFDict pdfDict("XObject");
    pdfDict.insertName("Subtype", "Image");
    pdfDict.insertInt("Width", jpegSize.width());
    pdfDict.insertInt("Height", jpegSize.height());
    if (yuv) {
        pdfDict.insertName("ColorSpace", "DeviceRGB");
    } else {
        pdfDict.insertName("ColorSpace", "DeviceGray");
    }
    pdfDict.insertInt("BitsPerComponent", 8);
    pdfDict.insertName("Filter", "DCTDecode");
    pdfDict.insertInt("ColorTransform", 0);
    pdfDict.insertInt("Length", SkToInt(data.size()));
    pdfDict.emitObject(stream);
    stream->writeText(" stream\n");
    stream->write(data.data(), data.size());
    stream->writeText("\nendstream");
    doc->endObject();
    return true;
}

static SkBitmap to_pixels(const SkImage* image) {
    SkBitmap bm;
    int w = image->width(),
        h = image->height();
    switch (image->colorType()) {
        case kAlpha_8_SkColorType:
            bm.allocPixels(SkImageInfo::MakeA8(w, h));
            break;
        case kGray_8_SkColorType:
            bm.allocPixels(SkImageInfo::Make(w, h, kGray_8_SkColorType, kOpaque_SkAlphaType));
            break;
        default: {
            // TODO: makeColorSpace(sRGB) or actually tag the images
            SkAlphaType at = bm.isOpaque() ? kOpaque_SkAlphaType : kUnpremul_SkAlphaType;
            bm.allocPixels(SkImageInfo::Make(w, h, kBGRA_8888_SkColorType, at));
        }
    }
    if (!image->readPixels(bm.pixmap(), 0, 0)) {
        bm.eraseColor(SkColorSetARGB(0xFF, 0, 0, 0));
    }
    return bm;
}

SkPDFIndirectReference SkPDFSerializeImage(const SkImage* img,
                                           SkPDFDocument* doc,
                                           int encodingQuality) {
    SkPDFIndirectReference result;
    SkASSERT(img);
    SkASSERT(doc);
    SkASSERT(encodingQuality >= 0);
    SkISize dimensions = img->dimensions();
    sk_sp<SkData> data = img->refEncodedData();
    if (data && do_jpeg(*data, doc, dimensions, &result)) {
        return result;
    }
    SkBitmap bm = to_pixels(img);
    SkPixmap pm = bm.pixmap();
    bool isOpaque = pm.isOpaque() || pm.computeIsOpaque();
    if (encodingQuality <= 100 && isOpaque) {
        sk_sp<SkData> data = img->encodeToData(SkEncodedImageFormat::kJPEG, encodingQuality);
        if (data && do_jpeg(*data, doc, dimensions, &result)) {
            return result;
        }
    }
    return do_deflated_image(pm, doc, isOpaque);
}
