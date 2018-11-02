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
#include "SkPDFCanon.h"
#include "SkPDFTypes.h"
#include "SkPDFUtils.h"
#include "SkStream.h"
#include "SkTo.h"

bool image_compute_is_opaque(const SkImage* image) {
    if (image->isOpaque()) {
        return true;
    }
    // keep output PDF small at cost of possible resource use.
    SkBitmap bm;
    // if image can not be read, treat as transparent.
    return SkPDFUtils::ToBitmap(image, &bm) && SkBitmap::ComputeIsOpaque(bm);
}

////////////////////////////////////////////////////////////////////////////////

static const char kStreamBegin[] = " stream\n";

static const char kStreamEnd[] = "\nendstream";

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
static SkColor get_neighbor_avg_color(const SkBitmap& bm, int xOrig, int yOrig) {
    SkASSERT(kBGRA_8888_SkColorType == bm.colorType());
    unsigned r = 0, g = 0, b = 0, n = 0;
    // Clamp the range to the edge of the bitmap.
    int ymin = SkTMax(0, yOrig - 1);
    int ymax = SkTMin(yOrig + 1, bm.height() - 1);
    int xmin = SkTMax(0, xOrig - 1);
    int xmax = SkTMin(xOrig + 1, bm.width() - 1);
    for (int y = ymin; y <= ymax; ++y) {
        const SkColor* scanline = bm.getAddr32(0, y);
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

static size_t pixel_count(const SkImage* image) {
    return SkToSizeT(image->width()) * SkToSizeT(image->height());
}

static size_t pdf_color_component_count(SkColorType ct) {
    // Single-channel formats remain that way, all others are converted to RGB
    return SkColorTypeIsAlphaOnly(ct) || SkColorTypeIsGray(ct) ? 1 : 3;
}

static void image_to_pdf_pixels(const SkImage* image, SkWStream* out) {
    if (kAlpha_8_SkColorType == image->colorType()) {
        fill_stream(out, '\x00', pixel_count(image));
    } else if (kGray_8_SkColorType == image->colorType()) {
        SkBitmap gray;
        gray.allocPixels(SkImageInfo::Make(image->width(), image->height(),
                                           kGray_8_SkColorType,
                                           kPremul_SkAlphaType));
        if (image->readPixels(gray.pixmap(), 0, 0)) {
            out->write(gray.getPixels(), gray.computeByteSize());
        } else {
            fill_stream(out, '\x00', pixel_count(image));
        }
    } else {
        SkBitmap bgra;
        // TODO: makeColorSpace(sRGB) or actually tag the images
        bgra.allocPixels(SkImageInfo::Make(image->width(), image->height(),
                                           kBGRA_8888_SkColorType,
                                           kUnpremul_SkAlphaType));
        if (image->readPixels(bgra.pixmap(), 0, 0)) {
            SkAutoTMalloc<uint8_t> scanline(3 * bgra.width());
            for (int y = 0; y < bgra.height(); ++y) {
                const SkColor* src = bgra.getAddr32(0, y);
                uint8_t* dst = scanline.get();
                for (int x = 0; x < bgra.width(); ++x) {
                    SkColor color = *src++;
                    if (SkColorGetA(color) == SK_AlphaTRANSPARENT) {
                        color = get_neighbor_avg_color(bgra, x, y);
                    }
                    *dst++ = SkColorGetR(color);
                    *dst++ = SkColorGetG(color);
                    *dst++ = SkColorGetB(color);
                }
                out->write(scanline.get(), 3 * bgra.width());
            }
        } else {
            fill_stream(out, '\x00', 3 * pixel_count(image));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

static void image_alpha_to_a8(const SkImage* image, SkWStream* out) {
    SkBitmap alpha;
    alpha.allocPixels(SkImageInfo::MakeA8(image->width(), image->height()));
    if (image->readPixels(alpha.pixmap(), 0, 0)) {
        out->write(alpha.getPixels(), alpha.computeByteSize());
    } else {
        fill_stream(out, '\xFF', pixel_count(image));
    }
}

static void emit_image_xobject(SkWStream* stream,
                               const SkImage* image,
                               bool alpha,
                               const sk_sp<SkPDFObject>& smask,
                               const SkPDFObjNumMap& objNumMap) {
    // Write to a temporary buffer to get the compressed length.
    SkDynamicMemoryWStream buffer;
    SkDeflateWStream deflateWStream(&buffer);
    if (alpha) {
        image_alpha_to_a8(image, &deflateWStream);
    } else {
        image_to_pdf_pixels(image, &deflateWStream);
    }
    deflateWStream.finalize();  // call before buffer.bytesWritten().

    SkPDFDict pdfDict("XObject");
    pdfDict.insertName("Subtype", "Image");
    pdfDict.insertInt("Width", image->width());
    pdfDict.insertInt("Height", image->height());
    if (alpha) {
        pdfDict.insertName("ColorSpace", "DeviceGray");
    } else if (1 == pdf_color_component_count(image->colorType())) {
        pdfDict.insertName("ColorSpace", "DeviceGray");
    } else {
        pdfDict.insertName("ColorSpace", "DeviceRGB");
    }
    if (smask) {
        pdfDict.insertObjRef("SMask", smask);
    }
    pdfDict.insertInt("BitsPerComponent", 8);
    pdfDict.insertName("Filter", "FlateDecode");
    pdfDict.insertInt("Length", buffer.bytesWritten());
    pdfDict.emitObject(stream, objNumMap);

    stream->writeText(kStreamBegin);
    buffer.writeToAndReset(stream);
    stream->writeText(kStreamEnd);
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
    PDFJpegBitmap(SkISize size, sk_sp<SkData> data, bool isYUV)
        : fSize(size), fData(std::move(data)), fIsYUV(isYUV) { SkASSERT(fData); }
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
    stream->writeText(kStreamBegin);
    stream->write(fData->data(), fData->size());
    stream->writeText(kStreamEnd);
}
}  // namespace

////////////////////////////////////////////////////////////////////////////////
sk_sp<PDFJpegBitmap> make_jpeg_bitmap(sk_sp<SkData> data, SkISize size) {
    SkISize jpegSize;
    SkEncodedInfo::Color jpegColorType;
    SkEncodedOrigin exifOrientation;
    if (data && SkGetJpegInfo(data->data(), data->size(), &jpegSize,
                              &jpegColorType, &exifOrientation)) {
        bool yuv = jpegColorType == SkEncodedInfo::kYUV_Color;
        bool goodColorType = yuv || jpegColorType == SkEncodedInfo::kGray_Color;
        if (jpegSize == size  // Sanity check.
                && goodColorType
                && kTopLeft_SkEncodedOrigin == exifOrientation) {
            // hold on to data, not image.
            #ifdef SK_PDF_IMAGE_STATS
            gJpegImageObjects.fetch_add(1);
            #endif
            return sk_make_sp<PDFJpegBitmap>(jpegSize, std::move(data), yuv);
        }
    }
    return nullptr;
}

sk_sp<SkPDFObject> SkPDFCreateBitmapObject(sk_sp<SkImage> image, int encodingQuality) {
    SkASSERT(image);
    SkASSERT(encodingQuality >= 0);
    SkISize dimensions = image->dimensions();
    sk_sp<SkData> data = image->refEncodedData();
    if (auto jpeg = make_jpeg_bitmap(std::move(data), dimensions)) {
        return std::move(jpeg);
    }

    const bool isOpaque = image_compute_is_opaque(image.get());

    if (encodingQuality <= 100 && isOpaque) {
        data = image->encodeToData(SkEncodedImageFormat::kJPEG, encodingQuality);
        if (auto jpeg = make_jpeg_bitmap(std::move(data), dimensions)) {
            return std::move(jpeg);
        }
    }

    sk_sp<SkPDFObject> smask;
    if (!isOpaque) {
        smask = sk_make_sp<PDFAlphaBitmap>(image);
    }
    #ifdef SK_PDF_IMAGE_STATS
    gRegularImageObjects.fetch_add(1);
    #endif
    return sk_make_sp<PDFDefaultBitmap>(std::move(image), std::move(smask));
}
