/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "src/pdf/SkPDFResourceDict.h"
#include "src/pdf/SkPDFTypes.h"

// Sanity check that the values of enum ResourceType correspond to the
// expected values as defined in the arrays below.
// If these are failing, you may need to update the kResourceTypePrefixes
// and kResourceTypeNames arrays below.
static_assert(0 == (int)SkPDFResourceType::kExtGState, "resource_type_mismatch");
static_assert(1 == (int)SkPDFResourceType::kPattern,   "resource_type_mismatch");
static_assert(2 == (int)SkPDFResourceType::kXObject,   "resource_type_mismatch");
static_assert(3 == (int)SkPDFResourceType::kFont,      "resource_type_mismatch");

// One extra character for the Prefix.
constexpr size_t kMaxResourceNameLength = 1 + SkStrAppendS32_MaxSize;

// returns pointer just past end of what's written into `dst`.
static char* get_resource_name(char dst[kMaxResourceNameLength], SkPDFResourceType type, int key) {
    static const char kResourceTypePrefixes[] = {
        'G',  // kExtGState
        'P',  // kPattern
        'X',  // kXObject
        'F'   // kFont
    };
    SkASSERT((unsigned)type < SK_ARRAY_COUNT(kResourceTypePrefixes));
    dst[0] = kResourceTypePrefixes[(unsigned)type];
    return SkStrAppendS32(dst + 1, key);
}

void SkPDFWriteResourceName(SkWStream* dst, SkPDFResourceType type, int key) {
    // One extra character for the leading '/'.
    char buffer[1 + kMaxResourceNameLength];
    buffer[0] = '/';
    char* end = get_resource_name(buffer + 1, type, key);
    dst->write(buffer, (size_t)(end - buffer));
}

static const char* resource_name(SkPDFResourceType type) {
    static const char* kResourceTypeNames[] = {
        "ExtGState",
        "Pattern",
        "XObject",
        "Font"
    };
    SkASSERT((unsigned)type < SK_ARRAY_COUNT(kResourceTypeNames));
    return kResourceTypeNames[(unsigned)type];
}

static SkString resource(SkPDFResourceType type, int index) {
    char buffer[kMaxResourceNameLength];
    char* end = get_resource_name(buffer, type, index);
    return SkString(buffer, (size_t)(end - buffer));
}

static void add_subdict(const std::vector<SkPDFIndirectReference>& resourceList,
                        SkPDFResourceType type,
                        SkPDFDict* dst) {
    if (!resourceList.empty()) {
        auto resources = SkPDFMakeDict();
        for (SkPDFIndirectReference ref : resourceList) {
            resources->insertRef(resource(type, ref.fValue), ref);
        }
        dst->insertObject(resource_name(type), std::move(resources));
    }
}

static std::unique_ptr<SkPDFArray> make_proc_set() {
    auto procSets = SkPDFMakeArray();
    static const char kProcs[][7] = { "PDF", "Text", "ImageB", "ImageC", "ImageI"};
    procSets->reserve(SK_ARRAY_COUNT(kProcs));
    for (const char* proc : kProcs) {
        procSets->appendName(proc);
    }
    return procSets;
}

std::unique_ptr<SkPDFDict> SkPDFMakeResourceDict(
        const std::vector<SkPDFIndirectReference>& graphicStateResources,
        const std::vector<SkPDFIndirectReference>& shaderResources,
        const std::vector<SkPDFIndirectReference>& xObjectResources,
        const std::vector<SkPDFIndirectReference>& fontResources) {
    auto dict = SkPDFMakeDict();
    dict->insertObject("ProcSet", make_proc_set());
    add_subdict(graphicStateResources, SkPDFResourceType::kExtGState, dict.get());
    add_subdict(shaderResources,       SkPDFResourceType::kPattern,   dict.get());
    add_subdict(xObjectResources,      SkPDFResourceType::kXObject,   dict.get());
    add_subdict(fontResources,         SkPDFResourceType::kFont,      dict.get());
    return dict;
}
