
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
#include "SkPDFResourceDict.h"
#include "SkPDFUtils.h"
#include "SkStream.h"
#include "SkTypes.h"

SkPDFFormXObject::SkPDFFormXObject(SkPDFDevice* device) {
    // We don't want to keep around device because we'd have two copies
    // of content, so reference or copy everything we need (content and
    // resources).
    SkTSet<SkPDFObject*> emptySet;
    SkPDFResourceDict* resourceDict = device->getResourceDict();
    resourceDict->getReferencedResources(emptySet, &fResources, false);

    SkAutoTUnref<SkStream> content(device->content());
    setData(content.get());

    SkAutoTUnref<SkPDFArray> bboxArray(device->copyMediaBox());
    init(NULL, resourceDict, bboxArray);

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
}

/**
 * Creates a FormXObject from a content stream and associated resources.
 */
SkPDFFormXObject::SkPDFFormXObject(SkStream* content, SkRect bbox,
                                   SkPDFResourceDict* resourceDict) {
    SkTSet<SkPDFObject*> emptySet;
    resourceDict->getReferencedResources(emptySet, &fResources, false);

    setData(content);

    SkAutoTUnref<SkPDFArray> bboxArray(SkPDFUtils::RectToArray(bbox));
    init("DeviceRGB", resourceDict, bboxArray);
}

/**
 * Common initialization code.
 * Note that bbox is unreferenced here, so calling code does not need worry.
 */
void SkPDFFormXObject::init(const char* colorSpace,
                            SkPDFDict* resourceDict, SkPDFArray* bbox) {
    insertName("Type", "XObject");
    insertName("Subtype", "Form");
    insert("Resources", resourceDict);
    insert("BBox", bbox);

    // Right now SkPDFFormXObject is only used for saveLayer, which implies
    // isolated blending.  Do this conditionally if that changes.
    SkAutoTUnref<SkPDFDict> group(new SkPDFDict("Group"));
    group->insertName("S", "Transparency");

    if (colorSpace != NULL) {
        group->insertName("CS", colorSpace);
    }
    group->insert("I", new SkPDFBool(true))->unref();  // Isolated.
    insert("Group", group.get());
}

SkPDFFormXObject::~SkPDFFormXObject() {
    fResources.unrefAll();
}

void SkPDFFormXObject::getResources(
        const SkTSet<SkPDFObject*>& knownResourceObjects,
        SkTSet<SkPDFObject*>* newResourceObjects) {
    GetResourcesHelper(&fResources.toArray(),
                       knownResourceObjects,
                       newResourceObjects);
}
