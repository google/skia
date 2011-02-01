/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkFlate.h"
#include "SkPDFCatalog.h"
#include "SkPDFStream.h"
#include "SkStream.h"

SkPDFStream::SkPDFStream(SkStream* stream) {
    if (SkFlate::HaveFlate())
        SkAssertResult(SkFlate::Deflate(stream, &fCompressedData));

    if (SkFlate::HaveFlate() &&
            fCompressedData.getOffset() < stream->getLength()) {
        fLength = fCompressedData.getOffset();
        fDict.insert("Filter", new SkPDFName("FlateDecode"))->unref();
    } else {
        fCompressedData.reset();
        fPlainData = stream;
        fLength = fPlainData->getLength();
    }
    fDict.insert("Length", new SkPDFInt(fLength))->unref();
}

SkPDFStream::~SkPDFStream() {
}

void SkPDFStream::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                             bool indirect) {
    if (indirect)
        return emitIndirectObject(stream, catalog);

    fDict.emitObject(stream, catalog, false);
    stream->writeText(" stream\n");
    if (fPlainData.get())
        stream->write(fPlainData->getMemoryBase(), fLength);
    else
        stream->write(fCompressedData.getStream(), fLength);
    stream->writeText("\nendstream");
}

size_t SkPDFStream::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    if (indirect)
        return getIndirectOutputSize(catalog);

    return fDict.getOutputSize(catalog, false) +
        strlen(" stream\n\nendstream") + fLength;
}

SkPDFObject* SkPDFStream::insert(SkPDFName* key, SkPDFObject* value) {
    return fDict.insert(key, value);
}

SkPDFObject* SkPDFStream::insert(const char key[], SkPDFObject* value) {
    return fDict.insert(key, value);
}
