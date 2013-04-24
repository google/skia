/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkData.h"
#include "SkFlate.h"
#include "SkPDFCatalog.h"
#include "SkPDFImageStream.h"
#include "SkStream.h"

#define kNoColorTransform 0

static bool skip_compression(SkPDFCatalog* catalog) {
    return SkToBool(catalog->getDocumentFlags() &
                    SkPDFDocument::kFavorSpeedOverSize_Flags);
}

// TODO(edisonn): Use SkData (after removing deprecated constructor in SkPDFStream).
SkPDFImageStream::SkPDFImageStream(SkStream* stream,
                                   const SkBitmap& bitmap,
                                   const SkIRect& srcRect,
                                   EncodeToDCTStream encoder)
    : SkPDFStream(stream),
      fBitmap(bitmap),
      fSrcRect(srcRect),
      fEncoder(encoder) {
}

SkPDFImageStream::SkPDFImageStream(const SkPDFImageStream& pdfStream)
        : SkPDFStream(pdfStream),
          fBitmap(pdfStream.fBitmap),
          fSrcRect(pdfStream.fSrcRect),
          fEncoder(pdfStream.fEncoder) {
}

SkPDFImageStream::~SkPDFImageStream() {}

bool SkPDFImageStream::populate(SkPDFCatalog* catalog) {
    if (getState() == kUnused_State) {
        if (!skip_compression(catalog)) {
            SkDynamicMemoryWStream dctCompressedWStream;
            if (!fEncoder || !fEncoder(&dctCompressedWStream, fBitmap, fSrcRect)) {
                return INHERITED::populate(catalog);
            }

            if (dctCompressedWStream.getOffset() < getData()->getLength()) {
                SkData* data = dctCompressedWStream.copyToData();
                SkMemoryStream* stream = SkNEW_ARGS(SkMemoryStream, (data));
                setData(stream);
                stream->unref();
                if (data) {
                    // copyToData and new SkMemoryStream both call ref(), supress one.
                    data->unref();
                }

                insertName("Filter", "DCTDecode");
                insertInt("ColorTransform", kNoColorTransform);
                setState(kCompressed_State);
            }
        }
        setState(kNoCompression_State);
        insertInt("Length", getData()->getLength());
    } else if (getState() == kNoCompression_State && !skip_compression(catalog) &&
               (SkFlate::HaveFlate() || fEncoder)) {
        // Compression has not been requested when the stream was first created.
        // But a new Catalog would want it compressed.
        if (!getSubstitute()) {
            SkPDFImageStream* substitute = SkNEW_ARGS(SkPDFImageStream, (*this));
            setSubstitute(substitute);
            catalog->setSubstitute(this, substitute);
        }
        return false;
    }
    return true;
}
