
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkData.h"
#include "SkFlate.h"
#include "SkPDFStream.h"
#include "SkStream.h"
#include "SkStreamPriv.h"

SkPDFStream::SkPDFStream(SkStream* stream) : fState(kUnused_State) {
    this->setData(stream);
}

SkPDFStream::SkPDFStream(SkData* data) : fState(kUnused_State) {
    this->setData(data);
}

SkPDFStream::~SkPDFStream() {}

void SkPDFStream::emitObject(SkWStream* stream,
                             const SkPDFObjNumMap& objNumMap,
                             const SkPDFSubstituteMap& substitutes) {
    if (fState == kUnused_State) {
        fState = kNoCompression_State;
        SkDynamicMemoryWStream compressedData;

        SkAssertResult(
                SkFlate::Deflate(fDataStream.get(), &compressedData));
        SkAssertResult(fDataStream->rewind());
        if (compressedData.getOffset() < this->dataSize()) {
            SkAutoTDelete<SkStream> compressed(
                    compressedData.detachAsStream());
            this->setData(compressed.get());
            this->insertName("Filter", "FlateDecode");
        }
        fState = kCompressed_State;
        this->insertInt("Length", this->dataSize());
    }
    this->INHERITED::emitObject(stream, objNumMap, substitutes);
    stream->writeText(" stream\n");
    stream->writeStream(fDataStream.get(), fDataStream->getLength());
    SkAssertResult(fDataStream->rewind());
    stream->writeText("\nendstream");
}

SkPDFStream::SkPDFStream() : fState(kUnused_State) {}

void SkPDFStream::setData(SkData* data) {
    // FIXME: Don't swap if the data is the same.
    fDataStream.reset(SkNEW_ARGS(SkMemoryStream, (data)));
}

void SkPDFStream::setData(SkStream* stream) {
    SkASSERT(stream);
    // Code assumes that the stream starts at the beginning and is rewindable.
    // SkStreamRewindableFromSkStream will try stream->duplicate().
    fDataStream.reset(SkStreamRewindableFromSkStream(stream));
    SkASSERT(fDataStream.get());
}

size_t SkPDFStream::dataSize() const {
    SkASSERT(fDataStream->hasLength());
    return fDataStream->getLength();
}
