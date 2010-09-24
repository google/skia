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

#include "SkPDFCatalog.h"
#include "SkPDFStream.h"
#include "SkStream.h"

SkPDFStream::SkPDFStream(SkStream* stream) : fData(stream) {
    SkRefPtr<SkPDFName> lenKey = new SkPDFName("Length");
    lenKey->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFInt> lenValue = new SkPDFInt(fData->read(NULL, 0));
    lenValue->unref();  // SkRefPtr and new both took a reference.
    fDict.insert(lenKey.get(), lenValue.get());
}

SkPDFStream::~SkPDFStream() {
}

void SkPDFStream::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                             bool indirect) {
    if (indirect)
        return emitIndirectObject(stream, catalog);

    fDict.emitObject(stream, catalog, false);
    stream->writeText(" stream\n");
    stream->write(fData->getMemoryBase(), fData->read(NULL, 0));
    stream->writeText("endstream\n");
}

size_t SkPDFStream::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    if (indirect)
        return getIndirectOutputSize(catalog);

    return fDict.getOutputSize(catalog, false) +
        strlen(" stream\nendstream\n") + fData->read(NULL, 0);
}

void SkPDFStream::insert(SkPDFName* key, SkPDFObject* value) {
    fDict.insert(key, value);
}
