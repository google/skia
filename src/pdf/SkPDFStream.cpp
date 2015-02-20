
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkData.h"
#include "SkFlate.h"
#include "SkPDFCatalog.h"
#include "SkPDFStream.h"
#include "SkStream.h"
#include "SkStreamPriv.h"

SkPDFStream::SkPDFStream(SkStream* stream) : fState(kUnused_State) {
    this->setData(stream);
}

SkPDFStream::SkPDFStream(SkData* data) : fState(kUnused_State) {
    this->setData(data);
}

SkPDFStream::SkPDFStream(const SkPDFStream& pdfStream)
        : SkPDFDict(),
          fState(kUnused_State) {
    this->setData(pdfStream.fDataStream.get());
    bool removeLength = true;
    // Don't uncompress an already compressed stream, but we could.
    if (pdfStream.fState == kCompressed_State) {
        fState = kCompressed_State;
        removeLength = false;
    }
    this->mergeFrom(pdfStream);
    if (removeLength) {
        this->remove("Length");
    }
}

SkPDFStream::~SkPDFStream() {}

void SkPDFStream::emitObject(SkWStream* stream, SkPDFCatalog* catalog) {
    if (!this->populate(catalog)) {
        return fSubstitute->emitObject(stream, catalog);
    }

    this->INHERITED::emitObject(stream, catalog);
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
    // Code assumes that the stream starts at the beginning and is rewindable.
    if (stream) {
        // SkStreamRewindableFromSkStream will try stream->duplicate().
        fDataStream.reset(SkStreamRewindableFromSkStream(stream));
        SkASSERT(fDataStream.get());
    } else {
        // Use an empty memory stream.
        fDataStream.reset(SkNEW(SkMemoryStream));
    }
}

size_t SkPDFStream::dataSize() const {
    SkASSERT(fDataStream->hasLength());
    return fDataStream->getLength();
}

bool SkPDFStream::populate(SkPDFCatalog* catalog) {
#ifdef SK_NO_FLATE
    if (fState == kUnused_State) {
        fState = kNoCompression_State;
        insertInt("Length", this->dataSize());
    }
    return true;

#else  // !SK_NO_FLATE

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
            insertName("Filter", "FlateDecode");
        }
        fState = kCompressed_State;
        insertInt("Length", this->dataSize());
    }
    else if (fState == kNoCompression_State) {
        if (!fSubstitute.get()) {
            fSubstitute.reset(new SkPDFStream(*this));
            catalog->setSubstitute(this, fSubstitute.get());
        }
        return false;
    }
    return true;
#endif  // SK_NO_FLATE
}
