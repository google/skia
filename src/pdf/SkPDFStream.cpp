
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

static bool skip_compression(SkPDFCatalog* catalog) {
    return SkToBool(catalog->getDocumentFlags() &
                    SkPDFDocument::kNoCompression_Flags);
}

SkPDFStream::SkPDFStream(SkStream* stream)
    : fState(kUnused_State),
      fData(stream) {
}

SkPDFStream::SkPDFStream(SkData* data) : fState(kUnused_State) {
    SkMemoryStream* stream = new SkMemoryStream;
    stream->setData(data);
    fData = stream;
    fData->unref();  // SkRefPtr and new both took a reference.
}

SkPDFStream::SkPDFStream(const SkPDFStream& pdfStream)
        : SkPDFDict(),
          fState(kUnused_State),
          fData(pdfStream.fData) {
    bool removeLength = true;
    // Don't uncompress an already compressed stream, but we could.
    if (pdfStream.fState == kCompressed_State) {
        fState = kCompressed_State;
        removeLength = false;
    }
    SkPDFDict::Iter dict(pdfStream);
    SkPDFName* key;
    SkPDFObject* value;
    SkPDFName lengthName("Length");
    for (key = dict.next(&value); key != NULL; key = dict.next(&value)) {
        if (removeLength && *key == lengthName) {
            continue;
        }
        this->insert(key, value);
    }
}

SkPDFStream::~SkPDFStream() {}

void SkPDFStream::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                             bool indirect) {
    if (indirect) {
        return emitIndirectObject(stream, catalog);
    }
    if (!this->populate(catalog)) {
        return fSubstitute->emitObject(stream, catalog, indirect);
    }

    this->INHERITED::emitObject(stream, catalog, false);
    stream->writeText(" stream\n");
    stream->write(fData->getMemoryBase(), fData->getLength());
    stream->writeText("\nendstream");
}

size_t SkPDFStream::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    if (indirect) {
        return getIndirectOutputSize(catalog);
    }
    if (!this->populate(catalog)) {
        return fSubstitute->getOutputSize(catalog, indirect);
    }

    return this->INHERITED::getOutputSize(catalog, false) +
        strlen(" stream\n\nendstream") + fData->getLength();
}

SkPDFStream::SkPDFStream() : fState(kUnused_State) {}

void SkPDFStream::setData(SkStream* stream) {
    fData = stream;
}

bool SkPDFStream::populate(SkPDFCatalog* catalog) {
    if (fState == kUnused_State) {
        if (!skip_compression(catalog) && SkFlate::HaveFlate()) {
            SkDynamicMemoryWStream compressedData;

            SkAssertResult(SkFlate::Deflate(fData.get(), &compressedData));
            if (compressedData.getOffset() < fData->getLength()) {
                SkMemoryStream* stream = new SkMemoryStream;
                stream->setData(compressedData.copyToData())->unref();
                fData = stream;
                fData->unref();  // SkRefPtr and new both took a reference.
                insertName("Filter", "FlateDecode");
            }
            fState = kCompressed_State;
        } else {
            fState = kNoCompression_State;
        }
        insertInt("Length", fData->getLength());
    } else if (fState == kNoCompression_State && !skip_compression(catalog) &&
               SkFlate::HaveFlate()) {
        if (!fSubstitute.get()) {
            fSubstitute = new SkPDFStream(*this);
            fSubstitute->unref();  // SkRefPtr and new both took a reference.
            catalog->setSubstitute(this, fSubstitute.get());
        }
        return false;
    }
    return true;
}
