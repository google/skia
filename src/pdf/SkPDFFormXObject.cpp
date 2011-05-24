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
#include "SkPDFUtils.h"
#include "SkStream.h"
#include "SkTypes.h"

SkPDFFormXObject::SkPDFFormXObject(SkPDFDevice* device) {
    // We don't want to keep around device because we'd have two copies
    // of content, so reference or copy everything we need (content and
    // resources).
    device->getResources(&fResources);

    SkRefPtr<SkStream> content = device->content();
    content->unref();  // SkRefPtr and content() both took a reference.
    fStream = new SkPDFStream(content.get());
    fStream->unref();  // SkRefPtr and new both took a reference.

    insert("Type", new SkPDFName("XObject"))->unref();
    insert("Subtype", new SkPDFName("Form"))->unref();
    insert("BBox", device->getMediaBox().get());
    insert("Resources", device->getResourceDict().get());

    // We invert the initial transform and apply that to the xobject so that
    // it doesn't get applied twice. We can't just undo it because it's
    // embedded in things like shaders and images.
    if (!device->initialTransform().isIdentity()) {
        SkMatrix inverse;
        inverse.reset();
        device->initialTransform().invert(&inverse);
        insert("Matrix", SkPDFUtils::MatrixToArray(inverse))->unref();
    }

    // Right now SkPDFFormXObject is only used for saveLayer, which implies
    // isolated blending.  Do this conditionally if that changes.
    SkRefPtr<SkPDFDict> group = new SkPDFDict("Group");
    group->unref();  // SkRefPtr and new both took a reference.
    group->insert("S", new SkPDFName("Transparency"))->unref();
    group->insert("I", new SkPDFBool(true))->unref();  // Isolated.
    insert("Group", group.get());
}

SkPDFFormXObject::~SkPDFFormXObject() {
    fResources.unrefAll();
}

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
    resourceList->setReserve(resourceList->count() + fResources.count());
    for (int i = 0; i < fResources.count(); i++) {
        resourceList->push(fResources[i]);
        fResources[i]->ref();
    }
}

SkPDFObject* SkPDFFormXObject::insert(SkPDFName* key, SkPDFObject* value) {
    return fStream->insert(key, value);
}

SkPDFObject* SkPDFFormXObject::insert(const char key[], SkPDFObject* value) {
    return fStream->insert(key, value);
}
