/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFResourceDict.h"
#include "SkPDFTypes.h"
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

char SkPDFResourceDict::GetResourceTypePrefix(
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
    return SkStringPrintf("%c%d", SkPDFResourceDict::GetResourceTypePrefix(type), key);
}

static void add_subdict(
        const SkTDArray<SkPDFObject*>& resourceList,
        SkPDFResourceDict::SkPDFResourceType type,
        SkPDFDict* dst) {
    if (0 == resourceList.count()) {
        return;
    }
    auto resources = sk_make_sp<SkPDFDict>();
    for (int i = 0; i < resourceList.count(); i++) {
        resources->insertObjRef(SkPDFResourceDict::getResourceName(type, i),
                                sk_ref_sp(resourceList[i]));
    }
    dst->insertObject(get_resource_type_name(type), std::move(resources));
}

sk_sp<SkPDFDict> SkPDFResourceDict::Make(
        const SkTDArray<SkPDFObject*>* gStateResources,
        const SkTDArray<SkPDFObject*>* patternResources,
        const SkTDArray<SkPDFObject*>* xObjectResources,
        const SkTDArray<SkPDFObject*>* fontResources) {
    auto dict = sk_make_sp<SkPDFDict>();
    static const char kProcs[][7] = {
        "PDF", "Text", "ImageB", "ImageC", "ImageI"};
    auto procSets = sk_make_sp<SkPDFArray>();

    procSets->reserve(SK_ARRAY_COUNT(kProcs));
    for (size_t i = 0; i < SK_ARRAY_COUNT(kProcs); i++) {
        procSets->appendName(kProcs[i]);
    }
    dict->insertObject("ProcSets", std::move(procSets));

    if (gStateResources) {
        add_subdict(*gStateResources, kExtGState_ResourceType, dict.get());
    }
    if (patternResources) {
        add_subdict(*patternResources, kPattern_ResourceType, dict.get());
    }
    if (xObjectResources) {
        add_subdict(*xObjectResources, kXObject_ResourceType, dict.get());
    }
    if (fontResources) {
        add_subdict(*fontResources, kFont_ResourceType, dict.get());
    }
    return dict;
}
