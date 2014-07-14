
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

static bool skip_compression(SkPDFCatalog* catalog) {
    return SkToBool(catalog->getDocumentFlags() &
                    SkPDFDocument::kFavorSpeedOverSize_Flags);
}

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

void SkPDFStream::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                             bool indirect) {
    if (indirect) {
        return emitIndirectObject(stream, catalog);
    }
    SkAutoMutexAcquire lock(fMutex);  // multiple threads could be calling emit
    if (!this->populate(catalog)) {
        return fSubstitute->emitObject(stream, catalog, indirect);
    }

    this->INHERITED::emitObject(stream, catalog, false);
    stream->writeText(" stream\n");
    stream->writeStream(fDataStream.get(), fDataStream->getLength());
    SkAssertResult(fDataStream->rewind());
    stream->writeText("\nendstream");
}

size_t SkPDFStream::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    if (indirect) {
        return getIndirectOutputSize(catalog);
    }
    SkAutoMutexAcquire lock(fMutex);  // multiple threads could be calling emit
    if (!this->populate(catalog)) {
        return fSubstitute->getOutputSize(catalog, indirect);
    }

    return this->INHERITED::getOutputSize(catalog, false) +
        strlen(" stream\n\nendstream") + this->dataSize();
}

SkPDFStream::SkPDFStream() : fState(kUnused_State) {}

void SkPDFStream::setData(SkData* data) {
    fMemoryStream.setData(data);
    if (&fMemoryStream != fDataStream.get()) {
        fDataStream.reset(SkRef(&fMemoryStream));
    }
}

void SkPDFStream::setData(SkStream* stream) {
    // Code assumes that the stream starts at the beginning and is rewindable.
    if (&fMemoryStream == fDataStream.get()) {
        SkASSERT(&fMemoryStream != stream);
        fMemoryStream.setData(NULL);
    }
    SkASSERT(0 == fMemoryStream.getLength());
    if (stream) {
        // SkStreamRewindableFromSkStream will try stream->duplicate().
        fDataStream.reset(SkStreamRewindableFromSkStream(stream));
        SkASSERT(fDataStream.get());
    } else {
        fDataStream.reset(SkRef(&fMemoryStream));
    }
}

size_t SkPDFStream::dataSize() const {
    SkASSERT(fDataStream->hasLength());
    return fDataStream->getLength();
}

bool SkPDFStream::populate(SkPDFCatalog* catalog) {
    if (fState == kUnused_State) {
        if (!skip_compression(catalog) && SkFlate::HaveFlate()) {
            SkDynamicMemoryWStream compressedData;

            SkAssertResult(
                    SkFlate::Deflate(fDataStream.get(), &compressedData));
            SkAssertResult(fDataStream->rewind());
            if (compressedData.getOffset() < this->dataSize()) {
                SkAutoTUnref<SkStream> compressed(
                        compressedData.detachAsStream());
                this->setData(compressed.get());
                insertName("Filter", "FlateDecode");
            }
            fState = kCompressed_State;
        } else {
            fState = kNoCompression_State;
        }
        insertInt("Length", this->dataSize());
    } else if (fState == kNoCompression_State && !skip_compression(catalog) &&
               SkFlate::HaveFlate()) {
        if (!fSubstitute.get()) {
            fSubstitute.reset(new SkPDFStream(*this));
            catalog->setSubstitute(this, fSubstitute.get());
        }
        return false;
    }
    return true;
}
