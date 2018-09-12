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
static_assert(0 == SkPDFResourceDict::kExtGState_ResourceType, "resource_type_mismatch");
static_assert(1 == SkPDFResourceDict::kPattern_ResourceType,   "resource_type_mismatch");
static_assert(2 == SkPDFResourceDict::kXObject_ResourceType,   "resource_type_mismatch");
static_assert(3 == SkPDFResourceDict::kFont_ResourceType,      "resource_type_mismatch");

constexpr size_t kMaxResourceNameLength = 1 + SkStrAppendS32_MaxSize;

static char* get_resource_name(char dst[kMaxResourceNameLength],
                               SkPDFResourceDict::ResourceType type, int key) {
    static const char kResourceTypePrefixes[] = {
        'G',
        'P',
        'X',
        'F'
    };
    SkASSERT((unsigned)type < SK_ARRAY_COUNT(kResourceTypePrefixes));
    dst[0] = kResourceTypePrefixes[type];
    return SkStrAppendS32(dst + 1, key);
}

void SkPDFResourceDict::WriteResourceName(
        SkWStream* dst, SkPDFResourceDict::ResourceType type, int key) {
    char buffer[1 + kMaxResourceNameLength];
    buffer[0] = '/';
    char* end = get_resource_name(buffer + 1, type, key);
    dst->write(buffer, (size_t)(end - buffer));
}

template <typename T>
static void add_subdict(
        std::vector<sk_sp<T>> resourceList,
        SkPDFResourceDict::ResourceType type,
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
    dst->insertObject(kResourceTypeNames[type], std::move(resources));
}

sk_sp<SkPDFDict> SkPDFResourceDict::Make(std::vector<sk_sp<SkPDFObject>> graphicStateResources,
                                         std::vector<sk_sp<SkPDFObject>> shaderResources,
                                         std::vector<sk_sp<SkPDFObject>> xObjectResources,
                                         std::vector<sk_sp<SkPDFFont>> fontResources)
{
    auto dict = sk_make_sp<SkPDFDict>();
    static const char kProcs[][7] = {
        "PDF", "Text", "ImageB", "ImageC", "ImageI"};
    auto procSets = sk_make_sp<SkPDFArray>();

    procSets->reserve(SK_ARRAY_COUNT(kProcs));
    for (size_t i = 0; i < SK_ARRAY_COUNT(kProcs); i++) {
        procSets->appendName(kProcs[i]);
    }
    dict->insertObject("ProcSets", std::move(procSets));

    if (graphicStateResources.size() > 0) {
        add_subdict(std::move(graphicStateResources), kExtGState_ResourceType, dict.get());
    }
    if (shaderResources.size() > 0) {
        add_subdict(std::move(shaderResources), kPattern_ResourceType, dict.get());
    }
    if (xObjectResources.size() > 0) {
        add_subdict(std::move(xObjectResources), kXObject_ResourceType, dict.get());
    }
    if (fontResources.size() > 0) {
        add_subdict(std::move(fontResources), kFont_ResourceType, dict.get());
    }
    return dict;
}
