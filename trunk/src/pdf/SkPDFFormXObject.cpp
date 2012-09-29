
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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
    device->getResources(&fResources, false);

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
        if (!device->initialTransform().invert(&inverse)) {
            // The initial transform should be invertible.
            SkASSERT(false);
            inverse.reset();
        }
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
