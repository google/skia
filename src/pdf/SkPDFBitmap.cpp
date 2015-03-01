/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkFlate.h"
#include "SkPDFBitmap.h"
#include "SkPDFCanon.h"
#include "SkPDFCatalog.h"
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

static size_t pixel_count(const SkBitmap& bm) {
    return SkToSizeT(bm.width()) * SkToSizeT(bm.height());
}

// write a single byte to a stream n times.
static void fill_stream(SkWStream* out, char value, size_t n) {
    char buffer[4096];
    memset(buffer, value, sizeof(buffer));
    while (n) {
        size_t k = SkTMin(n, sizeof(buffer));
        out->write(buffer, k);
        n -= k;
    }
}

static SkPMColor get_pmcolor_neighbor_avg_color(const SkBitmap& bitmap,
                                                int xOrig,
                                                int yOrig) {
    SkASSERT(kN32_SkColorType == bitmap.colorType());
    SkASSERT(bitmap.getPixels());
    uint8_t count = 0;
    unsigned r = 0;
    unsigned g = 0;
    unsigned b = 0;
    for (int y = yOrig - 1; y <= yOrig + 1; ++y) {
        if (y < 0 || y >= bitmap.height()) {
            continue;
        }
        uint32_t* src = bitmap.getAddr32(0, y);
        for (int x = xOrig - 1; x <= xOrig + 1; ++x) {
            if (x < 0 || x >= bitmap.width()) {
                continue;
            }
            SkPMColor pmColor = src[x];
            U8CPU alpha = SkGetPackedA32(pmColor);
            if (alpha != SK_AlphaTRANSPARENT) {
                uint32_t s = SkUnPreMultiply::GetScale(alpha);
                r += SkUnPreMultiply::ApplyScale(s, SkGetPackedR32(pmColor));
                g += SkUnPreMultiply::ApplyScale(s, SkGetPackedG32(pmColor));
                b += SkUnPreMultiply::ApplyScale(s, SkGetPackedB32(pmColor));
                ++count;
            }
        }
    }
    if (count == 0) {
        return SkPackARGB32NoCheck(SK_AlphaOPAQUE, 0, 0, 0);
    } else {
        return SkPackARGB32NoCheck(
                SK_AlphaOPAQUE, r / count, g / count, b / count);
    }
}

static void pmcolor_to_rgb24(const SkBitmap& bm, SkWStream* out) {
    SkASSERT(kN32_SkColorType == bm.colorType());
    if (!bm.getPixels()) {
        fill_stream(out, '\xFF', 3 * pixel_count(bm));
        return;
    }
    size_t scanlineLength = 3 * bm.width();
    SkAutoTMalloc<uint8_t> scanline(scanlineLength);
    for (int y = 0; y < bm.height(); ++y) {
        uint8_t* dst = scanline.get();
        const SkPMColor* src = bm.getAddr32(0, y);
        for (int x = 0; x < bm.width(); ++x) {
            SkPMColor color = *src++;
            U8CPU alpha = SkGetPackedA32(color);
            if (alpha != SK_AlphaTRANSPARENT) {
                uint32_t s = SkUnPreMultiply::GetScale(alpha);
                *dst++ = SkUnPreMultiply::ApplyScale(s, SkGetPackedR32(color));
                *dst++ = SkUnPreMultiply::ApplyScale(s, SkGetPackedG32(color));
                *dst++ = SkUnPreMultiply::ApplyScale(s, SkGetPackedB32(color));
            } else {
                /* It is necessary to average the color component of
                   transparent pixels with their surrounding neighbors
                   since the PDF renderer may separately re-sample the
                   alpha and color channels when the image is not
                   displayed at its native resolution. Since an alpha
                   of zero gives no information about the color
                   component, the pathological case is a white image
                   with sharp transparency bounds - the color channel
                   goes to black, and the should-be-transparent pixels
                   are rendered as grey because of the separate soft
                   mask and color resizing. e.g.: gm/bitmappremul.cpp */
                color = get_pmcolor_neighbor_avg_color(bm, x, y);
                *dst++ = SkGetPackedR32(color);
                *dst++ = SkGetPackedG32(color);
                *dst++ = SkGetPackedB32(color);
            }
        }
        out->write(scanline.get(), scanlineLength);
    }
}

static void pmcolor_alpha_to_a8(const SkBitmap& bm, SkWStream* out) {
    SkASSERT(kN32_SkColorType == bm.colorType());
    if (!bm.getPixels()) {
        fill_stream(out, '\xFF', pixel_count(bm));
        return;
    }
    size_t scanlineLength = bm.width();
    SkAutoTMalloc<uint8_t> scanline(scanlineLength);
    for (int y = 0; y < bm.height(); ++y) {
        uint8_t* dst = scanline.get();
        const SkPMColor* src = bm.getAddr32(0, y);
        for (int x = 0; x < bm.width(); ++x) {
            *dst++ = SkGetPackedA32(*src++);
        }
        out->write(scanline.get(), scanlineLength);
    }
}

////////////////////////////////////////////////////////////////////////////////

namespace {
// This SkPDFObject only outputs the alpha layer of the given bitmap.
class PDFAlphaBitmap : public SkPDFObject {
public:
    PDFAlphaBitmap(const SkBitmap& bm) : fBitmap(bm) {}
    ~PDFAlphaBitmap() {}
    void emitObject(SkWStream*, SkPDFCatalog*) SK_OVERRIDE;
    void addResources(SkTSet<SkPDFObject*>*, SkPDFCatalog*) const SK_OVERRIDE {}

private:
    const SkBitmap fBitmap;
    void emitDict(SkWStream*, SkPDFCatalog*, size_t, bool) const;
};

void PDFAlphaBitmap::emitObject(SkWStream* stream, SkPDFCatalog* catalog) {
    SkAutoLockPixels autoLockPixels(fBitmap);

#ifndef SK_NO_FLATE
    // Write to a temporary buffer to get the compressed length.
    SkDynamicMemoryWStream buffer;
    SkDeflateWStream deflateWStream(&buffer);
    pmcolor_alpha_to_a8(fBitmap, &deflateWStream);
    deflateWStream.finalize();  // call before detachAsStream().
    SkAutoTDelete<SkStreamAsset> asset(buffer.detachAsStream());

    this->emitDict(stream, catalog, asset->getLength(), /*deflate=*/true);
    pdf_stream_begin(stream);
    stream->writeStream(asset.get(), asset->getLength());
    pdf_stream_end(stream);
#else
    this->emitDict(stream, catalog, pixel_count(fBitmap), /*deflate=*/false);
    pdf_stream_begin(stream);
    pmcolor_alpha_to_a8(fBitmap, stream);
    pdf_stream_end(stream);
#endif  // SK_NO_FLATE
}

void PDFAlphaBitmap::emitDict(SkWStream* stream,
                              SkPDFCatalog* catalog,
                              size_t length,
                              bool deflate) const {
    SkPDFDict pdfDict("XObject");
    pdfDict.insertName("Subtype", "Image");
    pdfDict.insertInt("Width", fBitmap.width());
    pdfDict.insertInt("Height", fBitmap.height());
    pdfDict.insertName("ColorSpace", "DeviceGray");
    pdfDict.insertInt("BitsPerComponent", 8);
    if (deflate) {
        pdfDict.insertName("Filter", "FlateDecode");
    }
    pdfDict.insertInt("Length", length);
    pdfDict.emitObject(stream, catalog);
}
}  // namespace

////////////////////////////////////////////////////////////////////////////////

void SkPDFBitmap::addResources(SkTSet<SkPDFObject*>* resourceSet,
                               SkPDFCatalog* catalog) const {
    if (fSMask.get()) {
        resourceSet->add(fSMask.get());
    }
}

void SkPDFBitmap::emitObject(SkWStream* stream, SkPDFCatalog* catalog) {
    SkAutoLockPixels autoLockPixels(fBitmap);

#ifndef SK_NO_FLATE
    // Write to a temporary buffer to get the compressed length.
    SkDynamicMemoryWStream buffer;
    SkDeflateWStream deflateWStream(&buffer);
    pmcolor_to_rgb24(fBitmap, &deflateWStream);
    deflateWStream.finalize();  // call before detachAsStream().
    SkAutoTDelete<SkStreamAsset> asset(buffer.detachAsStream());

    this->emitDict(stream, catalog, asset->getLength(), /*deflate=*/true);
    pdf_stream_begin(stream);
    stream->writeStream(asset.get(), asset->getLength());
    pdf_stream_end(stream);
#else
    this->emitDict(stream, catalog, 3 * pixel_count(fBitmap), /*deflate=*/false);
    pdf_stream_begin(stream);
    pmcolor_to_rgb24(fBitmap, stream);
    pdf_stream_end(stream);
    return;
#endif  // SK_NO_FLATE
}

void SkPDFBitmap::emitDict(SkWStream* stream,
                           SkPDFCatalog* catalog,
                           size_t length,
                           bool deflate) const {
    SkPDFDict pdfDict("XObject");
    pdfDict.insertName("Subtype", "Image");
    pdfDict.insertInt("Width", fBitmap.width());
    pdfDict.insertInt("Height", fBitmap.height());
    pdfDict.insertName("ColorSpace", "DeviceRGB");
    pdfDict.insertInt("BitsPerComponent", 8);
    if (fSMask) {
        pdfDict.insert("SMask", new SkPDFObjRef(fSMask))->unref();
    }
    if (deflate) {
        pdfDict.insertName("Filter", "FlateDecode");
    }
    pdfDict.insertInt("Length", length);
    pdfDict.emitObject(stream, catalog);
}

SkPDFBitmap::SkPDFBitmap(const SkBitmap& bm,
                         SkPDFObject* smask)
    : fBitmap(bm), fSMask(smask) {}

SkPDFBitmap::~SkPDFBitmap() {}

////////////////////////////////////////////////////////////////////////////////
static bool is_transparent(const SkBitmap& bm) {
    SkAutoLockPixels autoLockPixels(bm);
    if (NULL == bm.getPixels()) {
        return true;
    }
    SkASSERT(kN32_SkColorType == bm.colorType());
    for (int y = 0; y < bm.height(); ++y) {
        U8CPU alpha = 0;
        const SkPMColor* src = bm.getAddr32(0, y);
        for (int x = 0; x < bm.width(); ++x) {
            alpha |= SkGetPackedA32(*src++);
        }
        if (alpha) {
            return false;
        }
    }
    return true;
}

SkPDFBitmap* SkPDFBitmap::Create(SkPDFCanon* canon,
                                 const SkBitmap& bitmap,
                                 const SkIRect& subset) {
    SkASSERT(canon);
    if (kN32_SkColorType != bitmap.colorType()) {
        // TODO(halcanary): support other colortypes.
        return NULL;
    }
    SkBitmap bm;
    // Should extractSubset be done by the SkPDFDevice?
    if (!bitmap.extractSubset(&bm, subset)) {
        return NULL;
    }
    if (bm.drawsNothing()) {
        return NULL;
    }
    if (!bm.isImmutable()) {
        SkBitmap copy;
        if (!bm.copyTo(&copy)) {
            return NULL;
        }
        copy.setImmutable();
        bm = copy;
    }

    SkPDFBitmap* pdfBitmap = canon->findBitmap(bm);
    if (pdfBitmap) {
        return SkRef(pdfBitmap);
    }
    SkPDFObject* smask = NULL;
    if (!bm.isOpaque() && !SkBitmap::ComputeIsOpaque(bm)) {
        if (is_transparent(bm)) {
            return NULL;
        }
        // PDFAlphaBitmaps do not get directly canonicalized (they
        // are refed by the SkPDFBitmap).
        smask = SkNEW_ARGS(PDFAlphaBitmap, (bm));
    }
    pdfBitmap = SkNEW_ARGS(SkPDFBitmap, (bm, smask));
    canon->addBitmap(pdfBitmap);
    return pdfBitmap;
}
