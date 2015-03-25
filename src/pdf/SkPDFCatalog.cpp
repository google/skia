
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPDFCatalog.h"

SkPDFCatalog::SkPDFCatalog() {}

SkPDFCatalog::~SkPDFCatalog() {
    fSubstituteMap.foreach(
            [](SkPDFObject*, SkPDFObject** v) { (*v)->unref(); });
}

bool SkPDFCatalog::addObject(SkPDFObject* obj) {
    if (fObjectNumbers.find(obj)) {
        return false;
    }
    fObjectNumbers.set(obj, fObjectNumbers.count() + 1);
    fObjects.push(obj);
    return true;
}

int32_t SkPDFCatalog::getObjectNumber(SkPDFObject* obj) const {
    int32_t* objectNumberFound = fObjectNumbers.find(obj);
    SkASSERT(objectNumberFound);
    return *objectNumberFound;
}

void SkPDFCatalog::setSubstitute(SkPDFObject* original,
                                 SkPDFObject* substitute) {
    SkASSERT(original != substitute);
    SkASSERT(!fSubstituteMap.find(original));
    fSubstituteMap.set(original, SkRef(substitute));
}

SkPDFObject* SkPDFCatalog::getSubstituteObject(SkPDFObject* object) const {
    SkPDFObject** found = fSubstituteMap.find(object);
    return found ? *found : object;
}
