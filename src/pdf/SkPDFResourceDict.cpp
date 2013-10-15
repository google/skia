/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFResourceDict.h"
#include "SkPostConfig.h"

SK_DEFINE_INST_COUNT(SkPDFResourceDict)

// Sanity check that the values of enum SkPDFResourceType correspond to the
// expected values as defined in the arrays below.
// If these are failing, you may need to update the resource_type_prefixes
// and resource_type_names arrays below.
SK_COMPILE_ASSERT(SkPDFResourceDict::kExtGState_ResourceType == 0,
                  resource_type_mismatch);
SK_COMPILE_ASSERT(SkPDFResourceDict::kPattern_ResourceType == 1,
                  resource_type_mismatch);
SK_COMPILE_ASSERT(SkPDFResourceDict::kXObject_ResourceType == 2,
                  resource_type_mismatch);
SK_COMPILE_ASSERT(SkPDFResourceDict::kFont_ResourceType == 3,
                  resource_type_mismatch);

static const char resource_type_prefixes[] = {
        'G',
        'P',
        'X',
        'F'
};

static const char* resource_type_names[] = {
        "ExtGState",
        "Pattern",
        "XObject",
        "Font"
};

static char get_resource_type_prefix(
        SkPDFResourceDict::SkPDFResourceType type) {
    SkASSERT(type >= 0);
    SkASSERT(type < SkPDFResourceDict::kResourceTypeCount);

    return resource_type_prefixes[type];
}

static const char* get_resource_type_name(
        SkPDFResourceDict::SkPDFResourceType type) {
    SkASSERT(type >= 0);
    SkASSERT(type < SkPDFResourceDict::kResourceTypeCount);

    return resource_type_names[type];
}

SkPDFResourceDict::SkPDFResourceDict() : SkPDFDict() {
    const char procs[][7] = {"PDF", "Text", "ImageB", "ImageC", "ImageI"};
    SkPDFArray* procSets = SkNEW(SkPDFArray());

    procSets->reserve(SK_ARRAY_COUNT(procs));
    for (size_t i = 0; i < SK_ARRAY_COUNT(procs); i++) {
        procSets->appendName(procs[i]);
    }
    insert("ProcSets", procSets)->unref();

    // Actual sub-dicts will be lazily added later
    fTypes.setCount(kResourceTypeCount);
    for (int i=0; i < kResourceTypeCount; i++) {
        fTypes[i] = NULL;
    }
}

SkPDFObject* SkPDFResourceDict::insertResourceAsReference(
        SkPDFResourceType type, int key, SkPDFObject* value) {
    SkAutoTUnref<SkPDFObjRef> ref(SkNEW_ARGS(SkPDFObjRef, (value)));
    insertResource(type, key, ref);
    fResources.add(value);

    return value;
}

void SkPDFResourceDict::getReferencedResources(
        const SkTSet<SkPDFObject*>& knownResourceObjects,
        SkTSet<SkPDFObject*>* newResourceObjects,
        bool recursive) const {
    // TODO: reserve not correct if we need to recursively explore.
    newResourceObjects->setReserve(newResourceObjects->count() +
                                   fResources.count());

    for (int i = 0; i < fResources.count(); i++) {
        if (!knownResourceObjects.contains(fResources[i]) &&
                !newResourceObjects->contains(fResources[i])) {
            newResourceObjects->add(fResources[i]);
            fResources[i]->ref();
            if (recursive) {
                fResources[i]->getResources(knownResourceObjects,
                                            newResourceObjects);
            }
        }
    }
}

SkString SkPDFResourceDict::getResourceName(
        SkPDFResourceType type, int key) {
    SkString keyString;
    keyString.printf("%c%d", get_resource_type_prefix(type), key);
    return keyString;
}

SkPDFObject* SkPDFResourceDict::insertResource(
        SkPDFResourceType type, int key, SkPDFObject* value) {
    SkPDFDict* typeDict = fTypes[type];
    if (NULL == typeDict) {
        SkAutoTUnref<SkPDFDict> newDict(SkNEW(SkPDFDict()));
        SkAutoTUnref<SkPDFName> typeName(
                SkNEW_ARGS(SkPDFName, (get_resource_type_name(type))));
        insert(typeName, newDict);  // ref counting handled here
        fTypes[type] = newDict;
        typeDict = newDict.get();
    }

    SkAutoTUnref<SkPDFName> keyName(
            SkNEW_ARGS(SkPDFName, (getResourceName(type, key))));
    typeDict->insert(keyName, value);
    return value;
}
