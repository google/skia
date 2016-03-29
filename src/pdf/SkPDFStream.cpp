/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkData.h"
#include "SkDeflate.h"
#include "SkPDFStream.h"
#include "SkStream.h"
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
    std::unique_ptr<SkStreamRewindable> dup(fCompressedData->duplicate());
    SkASSERT(dup);
    SkASSERT(dup->hasLength());
    stream->writeText(" stream\n");
    stream->writeStream(dup.get(), dup->getLength());
    stream->writeText("\nendstream");
}


void SkPDFStream::setData(SkStream* stream) {
    SkASSERT(!fCompressedData);  // Only call this function once.
    SkASSERT(stream);
    // Code assumes that the stream starts at the beginning.

    #ifdef SK_PDF_LESS_COMPRESSION
    std::unique_ptr<SkStreamRewindable> duplicate(stream->duplicate());
    if (duplicate && duplicate->hasLength()) {
        this->insertInt("Length", duplicate->getLength());
        fCompressedData.reset(duplicate.release());
        return;
    }
    #endif

    SkDynamicMemoryWStream compressedData;
    SkDeflateWStream deflateWStream(&compressedData);
    SkStreamCopy(&deflateWStream, stream);
    deflateWStream.finalize();
    size_t length = compressedData.bytesWritten();

    if (stream->hasLength()) {
        std::unique_ptr<SkStreamRewindable> dup(stream->duplicate());
        if (dup && dup->hasLength() &&
            dup->getLength() <= length + strlen("/Filter_/FlateDecode_")) {
            this->insertInt("Length", dup->getLength());
            fCompressedData.reset(dup.release());
            return;
        }
    }
    fCompressedData.reset(compressedData.detachAsStream());
    this->insertName("Filter", "FlateDecode");
    this->insertInt("Length", length);
}
