/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFResourceDict.h"
#include "SkPDFTypes.h"
#include "SkStream.h"

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

template <typename T>
static void add_subdict(std::vector<sk_sp<T>> resourceList,
                        SkPDFResourceType type,
                        SkPDFDict* dst) {
    if (0 == resourceList.size()) {
        return;
    }
    auto resources = sk_make_sp<SkPDFDict>();
    for (size_t i = 0; i < resourceList.size(); i++) {
        char buffer[kMaxResourceNameLength];
        char* end = get_resource_name(buffer, type, SkToInt(i));
        resources->insertObjRef(SkString(buffer, (size_t)(end - buffer)),
                                std::move(resourceList[i]));
    }
    static const char* kResourceTypeNames[] = {
        "ExtGState",
        "Pattern",
        "XObject",
        "Font"
    };
    SkASSERT((unsigned)type < SK_ARRAY_COUNT(kResourceTypeNames));
    dst->insertObject(kResourceTypeNames[(unsigned)type], std::move(resources));
}

sk_sp<SkPDFDict> SkPDFMakeResourceDict(std::vector<sk_sp<SkPDFObject>> graphicStateResources,
                                       std::vector<sk_sp<SkPDFObject>> shaderResources,
                                       std::vector<sk_sp<SkPDFObject>> xObjectResources,
                                       std::vector<sk_sp<SkPDFFont>> fontResources) {
    auto dict = sk_make_sp<SkPDFDict>();
    add_subdict(std::move(graphicStateResources), SkPDFResourceType::kExtGState, dict.get());
    add_subdict(std::move(shaderResources),       SkPDFResourceType::kPattern,   dict.get());
    add_subdict(std::move(xObjectResources),      SkPDFResourceType::kXObject,   dict.get());
    add_subdict(std::move(fontResources),         SkPDFResourceType::kFont,      dict.get());
    return dict;
}
