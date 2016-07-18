/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkData.h"
#include "SkDeflate.h"
#include "SkPDFStream.h"
#include "SkStreamPriv.h"

SkPDFStream::~SkPDFStream() {}

void SkPDFStream::drop() {
    fCompressedData.reset(nullptr);
    this->SkPDFDict::drop();
}

void SkPDFStream::emitObject(SkWStream* stream,
                             const SkPDFObjNumMap& objNumMap,
                             const SkPDFSubstituteMap& substitutes) const {
    SkASSERT(fCompressedData);
    this->INHERITED::emitObject(stream, objNumMap, substitutes);
    // duplicate (a cheap operation) preserves const on fCompressedData.
    std::unique_ptr<SkStreamAsset> dup(fCompressedData->duplicate());
    SkASSERT(dup);
    SkASSERT(dup->hasLength());
    stream->writeText(" stream\n");
    stream->writeStream(dup.get(), dup->getLength());
    stream->writeText("\nendstream");
}

void SkPDFStream::setData(SkStreamAsset* stream) {
    SkASSERT(!fCompressedData);  // Only call this function once.
    SkASSERT(stream);
    // Code assumes that the stream starts at the beginning.

    #ifdef SK_PDF_LESS_COMPRESSION
    fCompressedData.reset(stream->duplicate());
    SkASSERT(fCompressedData && fCompressedData->hasLength());
    this->insertInt("Length", fCompressedData->getLength());
    #else

    SkASSERT(stream->hasLength());
    SkDynamicMemoryWStream compressedData;
    SkDeflateWStream deflateWStream(&compressedData);
    SkStreamCopy(&deflateWStream, stream);
    deflateWStream.finalize();
    size_t compressedLength = compressedData.bytesWritten();
    size_t originalLength = stream->getLength();

    if (originalLength <= compressedLength + strlen("/Filter_/FlateDecode_")) {
        fCompressedData.reset(stream->duplicate());
        this->insertInt("Length", originalLength);
        return;
    }
    fCompressedData.reset(compressedData.detachAsStream());
    this->insertName("Filter", "FlateDecode");
    this->insertInt("Length", compressedLength);
    #endif
}
