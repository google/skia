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
    setData(content.get());

    insertName("Type", "XObject");
    insertName("Subtype", "Form");
    insert("BBox", device->getMediaBox().get());
    insert("Resources", device->getResourceDict());

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
    group->insertName("S", "Transparency");
    group->insert("I", new SkPDFBool(true))->unref();  // Isolated.
    insert("Group", group.get());
}

SkPDFFormXObject::~SkPDFFormXObject() {
    fResources.unrefAll();
}

void SkPDFFormXObject::getResources(SkTDArray<SkPDFObject*>* resourceList) {
    GetResourcesHelper(&fResources, resourceList);
}
