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

#include "SkPDFFormXObject.h"

#include "SkMatrix.h"
#include "SkPDFCatalog.h"
#include "SkPDFDevice.h"
#include "SkStream.h"
#include "SkTypes.h"

SkPDFFormXObject::SkPDFFormXObject(SkPDFDevice* device, const SkMatrix& matrix)
        : fContent(device->content(false)),
          fDevice(device) {
    SkMemoryStream* stream_data = new SkMemoryStream(fContent.c_str(),
                                                     fContent.size());
    SkAutoUnref stream_data_unref(stream_data);
    fStream = new SkPDFStream(stream_data);
    fStream->unref();  // SkRefPtr and new both took a reference.

    SkRefPtr<SkPDFName> typeValue = new SkPDFName("XObject");
    typeValue->unref();  // SkRefPtr and new both took a reference.
    insert("Type", typeValue.get());

    SkRefPtr<SkPDFName> subTypeValue = new SkPDFName("Form");
    subTypeValue->unref();  // SkRefPtr and new both took a reference.
    insert("Subtype", subTypeValue.get());

    insert("BBox", device->getMediaBox().get());
    insert("Resources", device->getResourceDict().get());

    if (!matrix.isIdentity()) {
        SkRefPtr<SkPDFArray> transformArray = new SkPDFArray();
        transformArray->unref();  // SkRefPtr and new both took a reference.
        transformArray->reserve(6);
        SkScalar transform[6];
        SkAssertResult(matrix.pdfTransform(transform));
        for (size_t i = 0; i < SK_ARRAY_COUNT(transform); i++) {
            SkRefPtr<SkPDFScalar> val = new SkPDFScalar(transform[i]);
            val->unref();  // SkRefPtr and new both took a reference.
            transformArray->append(val.get());
        }
        insert("Matrix", transformArray.get());
    }
}

SkPDFFormXObject::~SkPDFFormXObject() {}

void SkPDFFormXObject::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                             bool indirect) {
    if (indirect)
        return emitIndirectObject(stream, catalog);

    fStream->emitObject(stream, catalog, indirect);
}

size_t SkPDFFormXObject::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    if (indirect)
        return getIndirectOutputSize(catalog);

    return fStream->getOutputSize(catalog, indirect);
}

void SkPDFFormXObject::getResources(SkTDArray<SkPDFObject*>* resourceList) {
    fDevice->getResources(resourceList);
}

void SkPDFFormXObject::insert(SkPDFName* key, SkPDFObject* value) {
    fStream->insert(key, value);
}

void SkPDFFormXObject::insert(const char key[], SkPDFObject* value) {
    fStream->insert(key, value);
}
