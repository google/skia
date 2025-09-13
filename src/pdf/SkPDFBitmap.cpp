/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkPDFBitmap.h"

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkExecutor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/docs/SkPDFDocument.h"
#include "include/encode/SkICC.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkTo.h"
#include "modules/skcms/skcms.h"
#include "src/codec/SkCodecPriv.h"
#include "src/core/SkColorPriv.h"
#include "src/core/SkTHash.h"
#include "src/pdf/SkDeflate.h"
#include "src/pdf/SkPDFDocumentPriv.h"
#include "src/pdf/SkPDFTypes.h"
#include "src/pdf/SkPDFUnion.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <functional>
#include <memory>
#include <optional>
#include <utility>

namespace {

// write a single byte to a stream n times.
void fill_stream(SkWStream* out, char value, size_t n) {
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
SkColor get_neighbor_avg_color(const SkPixmap& bm, int xOrig, int yOrig) {
    SkASSERT(kBGRA_8888_SkColorType == bm.colorType());
    unsigned r = 0, g = 0, b = 0, n = 0;
    // Clamp the range to the edge of the bitmap.
    int ymin = std::max(0, yOrig - 1);
    int ymax = std::min(yOrig + 1, bm.height() - 1);
    int xmin = std::max(0, xOrig - 1);
    int xmax = std::min(xOrig + 1, bm.width() - 1);
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

enum class SkPDFStreamFormat { DCT, Flate, Uncompressed };

template <typename T>
void emit_image_stream(SkPDFDocument* doc,
                       SkPDFIndirectReference ref,
                       T writeStream,
                       SkISize size,
                       SkPDFUnion&& colorSpace,
                       SkPDFIndirectReference sMask,
                       size_t length,
                       SkPDFStreamFormat format) {
    if (!ref) {
        return;
    }
    SkPDFDict pdfDict("XObject");
    pdfDict.insertName("Subtype", "Image");
    pdfDict.insertInt("Width", size.width());
    pdfDict.insertInt("Height", size.height());
    pdfDict.insertUnion("ColorSpace", std::move(colorSpace));
    if (sMask) {
        pdfDict.insertRef("SMask", sMask);
    }
    pdfDict.insertInt("BitsPerComponent", 8);
    switch (format) {
        case SkPDFStreamFormat::DCT: pdfDict.insertName("Filter", "DCTDecode"); break;
        case SkPDFStreamFormat::Flate: pdfDict.insertName("Filter", "FlateDecode"); break;
        case SkPDFStreamFormat::Uncompressed: break;
    }
    if (format == SkPDFStreamFormat::DCT) {
        pdfDict.insertInt("ColorTransform", 0);
    }
    pdfDict.insertInt("Length", length);
    doc->emitStream(pdfDict, std::move(writeStream), ref);
}

size_t do_deflated_alpha(const SkPixmap& pm, SkPDFDocument* doc, SkPDFIndirectReference ref) {
    SkPDF::Metadata::CompressionLevel compressionLevel = doc->metadata().fCompressionLevel;
    SkPDFStreamFormat format = compressionLevel == SkPDF::Metadata::CompressionLevel::None
                             ? SkPDFStreamFormat::Uncompressed
                             : SkPDFStreamFormat::Flate;
    SkDynamicMemoryWStream buffer;
    SkWStream* stream = &buffer;
    std::optional<SkDeflateWStream> deflateWStream;
    if (format == SkPDFStreamFormat::Flate) {
        deflateWStream.emplace(&buffer, SkToInt(compressionLevel));
        stream = &*deflateWStream;
    }
    if (kAlpha_8_SkColorType == pm.colorType()) {
        SkASSERT(pm.rowBytes() == (size_t)pm.width());
        stream->write(pm.addr8(), pm.width() * pm.height());
    } else {
        SkASSERT(pm.alphaType() == kUnpremul_SkAlphaType);
        SkASSERT(pm.colorType() == kBGRA_8888_SkColorType);
        SkASSERT(pm.rowBytes() == (size_t)pm.width() * 4);
        const uint32_t* ptr = pm.addr32();
        const uint32_t* stop = ptr + pm.height() * pm.width();

        uint8_t byteBuffer[4092];
        uint8_t* bufferStop = byteBuffer + std::size(byteBuffer);
        uint8_t* dst = byteBuffer;
        while (ptr != stop) {
            *dst++ = 0xFF & ((*ptr++) >> SK_BGRA_A32_SHIFT);
            if (dst == bufferStop) {
                stream->write(byteBuffer, sizeof(byteBuffer));
                dst = byteBuffer;
            }
        }
        stream->write(byteBuffer, dst - byteBuffer);
    }
    if (deflateWStream) {
        deflateWStream->finalize();
    }

    size_t length = SkToInt(buffer.bytesWritten());
    emit_image_stream(doc, ref, [&buffer](SkWStream* stream) { buffer.writeToAndReset(stream); },
                      pm.info().dimensions(), SkPDFUnion::Name("DeviceGray"),
                      SkPDFIndirectReference(), length, format);
    return length;
}

SkPDFUnion write_icc_profile(SkPDFDocument* doc, sk_sp<SkData>&& icc, int channels) {
    SkPDFIndirectReference iccStreamRef;
    {
        static SkMutex iccProfileMapMutex;
        SkAutoMutexExclusive lock(iccProfileMapMutex);

        SkPDFIndirectReference* ref = doc->fICCProfileMap.find(SkPDFIccProfileKey{icc, channels});
        if (ref) {
            iccStreamRef = *ref;
        } else {
            std::unique_ptr<SkPDFDict> iccStreamDict = SkPDFMakeDict();
            iccStreamDict->insertInt("N", channels);
            iccStreamRef = SkPDFStreamOut(std::move(iccStreamDict), SkMemoryStream::Make(icc), doc);
            doc->fICCProfileMap.set(SkPDFIccProfileKey{icc, channels}, iccStreamRef);
        }
    }

    std::unique_ptr<SkPDFArray> iccPDF = SkPDFMakeArray();
    iccPDF->appendName("ICCBased");
    iccPDF->appendRef(iccStreamRef);
    return SkPDFUnion::Object(std::move(iccPDF));
}

bool icc_channel_mismatch(const skcms_ICCProfile* iccProfile, int expectedChannels) {
    int iccChannels = -1;
    if (iccProfile) {
        iccChannels = skcms_GetInputChannelCount(iccProfile);
    }
    return 0 < iccChannels && expectedChannels != iccChannels;
}

size_t do_deflated_image(const SkPixmap& pm,
                         SkPDFDocument* doc,
                         bool isOpaque,
                         SkPDFIndirectReference ref) {
    SkPDF::Metadata::CompressionLevel compressionLevel = doc->metadata().fCompressionLevel;
    SkPDFStreamFormat format = compressionLevel == SkPDF::Metadata::CompressionLevel::None
                             ? SkPDFStreamFormat::Uncompressed
                             : SkPDFStreamFormat::Flate;
    SkDynamicMemoryWStream dynamic;
    SkNullWStream writeOnly;
    SkWStream* buffer = ref ? static_cast<SkWStream*>(&dynamic) : &writeOnly;
    SkWStream* stream = buffer;
    std::optional<SkDeflateWStream> deflateWStream;
    if (format == SkPDFStreamFormat::Flate) {
        deflateWStream.emplace(buffer, SkToInt(compressionLevel));
        stream = &*deflateWStream;
    }
    SkPDFUnion colorSpace = SkPDFUnion::Name("DeviceGray");
    int channels;
    switch (pm.colorType()) {
        case kAlpha_8_SkColorType:
            channels = 1;
            fill_stream(stream, '\x00', pm.width() * pm.height());
            break;
        case kGray_8_SkColorType:
            channels = 1;
            SkASSERT(isOpaque);
            SkASSERT(pm.rowBytes() == (size_t)pm.width());
            stream->write(pm.addr8(), pm.width() * pm.height());
            break;
        default:
            colorSpace = SkPDFUnion::Name("DeviceRGB");
            channels = 3;
            SkASSERT(pm.alphaType() == kUnpremul_SkAlphaType);
            SkASSERT(pm.colorType() == kBGRA_8888_SkColorType);
            SkASSERT(pm.rowBytes() == (size_t)pm.width() * 4);
            uint8_t byteBuffer[3072];
            static_assert(std::size(byteBuffer) % 3 == 0, "");
            uint8_t* bufferStop = byteBuffer + std::size(byteBuffer);
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
                        stream->write(byteBuffer, sizeof(byteBuffer));
                        dst = byteBuffer;
                    }
                }
            }
            stream->write(byteBuffer, dst - byteBuffer);
    }
    if (deflateWStream) {
        deflateWStream->finalize();
    }

    if (pm.colorSpace()) {
        skcms_ICCProfile iccProfile;
        pm.colorSpace()->toProfile(&iccProfile);
        if (!icc_channel_mismatch(&iccProfile, channels)) {
            sk_sp<SkData> iccData = SkWriteICCProfile(&iccProfile, "");
            colorSpace = write_icc_profile(doc, std::move(iccData), channels);
        }
    }

    size_t length = buffer->bytesWritten();
    SkPDFIndirectReference sMask;
    if (!isOpaque && ref) {
        sMask = doc->reserveRef();
    }
    emit_image_stream(doc, ref, [&dynamic](SkWStream* stream) { dynamic.writeToAndReset(stream); },
                      pm.info().dimensions(), std::move(colorSpace), sMask, length, format);
    if (!isOpaque) {
        length += do_deflated_alpha(pm, doc, sMask);
    }
    return length;
}

size_t do_jpeg(sk_sp<SkData> data, SkColorSpace* imageColorSpace, SkPDFDocument* doc, SkISize size,
             SkPDFIndirectReference ref) {
    if (!ref) {
        return data->size();
    }

    SkPDF::DecodeJpegCallback decodeJPEG = doc->metadata().jpegDecoder;
    if (!decodeJPEG) {
        return 0;
    }
    std::unique_ptr<SkCodec> codec = decodeJPEG(data);
    if (!codec) {
        return 0;
    }

    SkISize jpegSize = codec->dimensions();
    const SkEncodedInfo& encodedInfo = SkCodecPriv::GetEncodedInfo(codec.get());
    SkEncodedInfo::Color jpegColorType = encodedInfo.color();
    SkEncodedOrigin exifOrientation = codec->getOrigin();

    bool yuv = jpegColorType == SkEncodedInfo::kYUV_Color;
    bool goodColorType = yuv || jpegColorType == SkEncodedInfo::kGray_Color;
    if (jpegSize != size  // Safety check.
            || !goodColorType
            || kTopLeft_SkEncodedOrigin != exifOrientation) {
        return 0;
    }

    int channels = yuv ? 3 : 1;
    SkPDFUnion colorSpace = yuv ? SkPDFUnion::Name("DeviceRGB") : SkPDFUnion::Name("DeviceGray");

    if (sk_sp<SkData> encodedIccProfileData = encodedInfo.profileData();
        encodedIccProfileData && !icc_channel_mismatch(encodedInfo.profile(), channels))
    {
        colorSpace = write_icc_profile(doc, std::move(encodedIccProfileData), channels);
    } else if (const skcms_ICCProfile* codecIccProfile = codec->getICCProfile();
               codecIccProfile && !icc_channel_mismatch(codecIccProfile, channels))
    {
        sk_sp<SkData> codecIccData = SkWriteICCProfile(codecIccProfile, "");
        colorSpace = write_icc_profile(doc, std::move(codecIccData), channels);
    } else if (imageColorSpace) {
        skcms_ICCProfile imageIccProfile;
        imageColorSpace->toProfile(&imageIccProfile);
        if (!icc_channel_mismatch(&imageIccProfile, channels)) {
            sk_sp<SkData> imageIccData = SkWriteICCProfile(&imageIccProfile, "");
            colorSpace = write_icc_profile(doc, std::move(imageIccData), channels);
        }
    }

    emit_image_stream(doc, ref,
                      [&data](SkWStream* dst) { dst->write(data->data(), data->size()); },
                      jpegSize, std::move(colorSpace),
                      SkPDFIndirectReference(), SkToInt(data->size()), SkPDFStreamFormat::DCT);
    return data->size();
}

SkBitmap to_pixels(const SkImage* image) {
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
            bm.allocPixels(
                SkImageInfo::Make(w, h, kBGRA_8888_SkColorType, at, image->refColorSpace()));
        }
    }
    // TODO: support GPU images in PDFs
    if (!image->readPixels(nullptr, bm.pixmap(), 0, 0)) {
        bm.eraseColor(SkColorSetARGB(0xFF, 0, 0, 0));
    }
    return bm;
}

size_t serialize_image(const SkImage* img,
                       int encodingQuality,
                       SkPDFDocument* doc,
                       SkPDFIndirectReference ref) {
    SkASSERT(img);
    SkASSERT(doc);
    SkASSERT(encodingQuality >= 0);
    SkISize dimensions = img->dimensions();

    if (sk_sp<SkData> data = img->refEncodedData()) {
        if (size_t size = do_jpeg(std::move(data), img->colorSpace(), doc, dimensions, ref)) {
            return size;
        }
    }
    SkBitmap bm = to_pixels(img);
    const SkPixmap& pm = bm.pixmap();
    SkPDF::EncodeJpegCallback encodeJPEG = doc->metadata().jpegEncoder;

    bool isOpaque = pm.isOpaque() || pm.computeIsOpaque();
    if (encodeJPEG && encodingQuality <= 100 && isOpaque) {
        SkDynamicMemoryWStream stream;
        if (encodeJPEG(&stream, pm, encodingQuality)) {
            if (size_t size = do_jpeg(stream.detachAsData(), pm.colorSpace(), doc, dimensions, ref)) {
                return size;
            }
        }
    }
    return do_deflated_image(pm, doc, isOpaque, ref);
}

} // namespace

size_t SkPDFSerializeImageSize(const SkImage* img, SkPDFDocument* doc, int encodingQuality) {
    return serialize_image(img, encodingQuality, doc, SkPDFIndirectReference());
}

SkPDFIndirectReference SkPDFSerializeImage(const SkImage* img,
                                           SkPDFDocument* doc,
                                           int encodingQuality) {
    SkASSERT(img);
    SkASSERT(doc);
    SkPDFIndirectReference ref = doc->reserveRef();
    if (SkExecutor* executor = doc->executor()) {
        SkRef(img);
        doc->incrementJobCount();
        executor->add([img, encodingQuality, doc, ref]() {
            serialize_image(img, encodingQuality, doc, ref);
            SkSafeUnref(img);
            doc->signalJobComplete();
        });
        return ref;
    }
    serialize_image(img, encodingQuality, doc, ref);
    return ref;
}
