/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFResourceDict.h"
#include "SkPostConfig.h"

// Sanity check that the values of enum SkPDFResourceType correspond to the
// expected values as defined in the arrays below.
// If these are failing, you may need to update the resource_type_prefixes
// and resource_type_names arrays below.
static_assert(SkPDFResourceDict::kExtGState_ResourceType == 0, "resource_type_mismatch");
static_assert(SkPDFResourceDict::kPattern_ResourceType == 1, "resource_type_mismatch");
static_assert(SkPDFResourceDict::kXObject_ResourceType == 2, "resource_type_mismatch");
static_assert(SkPDFResourceDict::kFont_ResourceType == 3, "resource_type_mismatch");

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
    SkASSERT(type < SK_ARRAY_COUNT(resource_type_names));

    return resource_type_names[type];
}

SkString SkPDFResourceDict::getResourceName(
        SkPDFResourceDict::SkPDFResourceType type, int key) {
    SkString keyString;
    keyString.printf("%c%d", get_resource_type_prefix(type), key);
    return keyString;
}

static void add_subdict(
        const SkTDArray<SkPDFObject*>& resourceList,
        SkPDFResourceDict::SkPDFResourceType type,
        SkPDFDict* dst) {
    if (0 == resourceList.count()) {
        return;
    }
    SkAutoTUnref<SkPDFDict> resources(new SkPDFDict);
    for (int i = 0; i < resourceList.count(); i++) {
        resources->insertObjRef(SkPDFResourceDict::getResourceName(type, i),
                                SkRef(resourceList[i]));
    }
    dst->insertObject(get_resource_type_name(type), resources.detach());
}

SkPDFDict* SkPDFResourceDict::Create(
        const SkTDArray<SkPDFObject*>* gStateResources,
        const SkTDArray<SkPDFObject*>* patternResources,
        const SkTDArray<SkPDFObject*>* xObjectResources,
        const SkTDArray<SkPDFObject*>* fontResources) {
    SkAutoTUnref<SkPDFDict> dict(new SkPDFDict);
    static const char kProcs[][7] = {
        "PDF", "Text", "ImageB", "ImageC", "ImageI"};
    SkAutoTUnref<SkPDFArray> procSets(new SkPDFArray);

    procSets->reserve(SK_ARRAY_COUNT(kProcs));
    for (size_t i = 0; i < SK_ARRAY_COUNT(kProcs); i++) {
        procSets->appendName(kProcs[i]);
    }
    dict->insertObject("ProcSets", procSets.detach());

    if (gStateResources) {
        add_subdict(*gStateResources, kExtGState_ResourceType, dict);
    }
    if (patternResources) {
        add_subdict(*patternResources, kPattern_ResourceType, dict);
    }
    if (xObjectResources) {
        add_subdict(*xObjectResources, kXObject_ResourceType, dict);
    }
    if (fontResources) {
        add_subdict(*fontResources, kFont_ResourceType, dict);
    }
    return dict.detach();
}
