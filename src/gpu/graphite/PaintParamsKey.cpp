/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PaintParamsKey.h"

#include "src/base/SkArenaAlloc.h"
#include "src/base/SkStringView.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

using namespace skia_private;

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
// PaintParamsKeyBuilder

#ifdef SK_DEBUG

void PaintParamsKeyBuilder::checkReset() {
    SkASSERT(!fLocked);
    SkASSERT(fData.empty());
    SkASSERT(fStack.empty());
}

void PaintParamsKeyBuilder::pushStack(int32_t codeSnippetID) {
    SkASSERT(fDict->isValidID(codeSnippetID));

    if (!fStack.empty()) {
        fStack.back().fNumActualChildren++;
        SkASSERT(fStack.back().fNumActualChildren <= fStack.back().fNumExpectedChildren);
    }

    const ShaderSnippet* snippet = fDict->getEntry(codeSnippetID);
    fStack.push_back({codeSnippetID, snippet->fNumChildren});
}

void PaintParamsKeyBuilder::popStack() {
    SkASSERT(!fStack.empty());
    SkASSERT(fStack.back().fNumActualChildren == fStack.back().fNumExpectedChildren);
    fStack.pop_back();
}

#endif // SK_DEBUG

//--------------------------------------------------------------------------------------------------
// PaintParamsKey

PaintParamsKey PaintParamsKey::clone(SkArenaAlloc* arena) const {
    int32_t* newData = arena->makeArrayDefault<int32_t>(fData.size());
    memcpy(newData, fData.data(), fData.size_bytes());
    return PaintParamsKey({newData, fData.size()});
}


const ShaderNode* PaintParamsKey::createNode(const ShaderCodeDictionary* dict,
                                             int* currentIndex,
                                             SkArenaAlloc* arena) const {
    SkASSERT(*currentIndex < SkTo<int>(fData.size()));
    const int32_t index = (*currentIndex)++;
    const int32_t id = fData[index];

    auto entry = dict->getEntry(id);
    if (!entry) {
        SKGPU_LOG_E("Unknown snippet ID in key: %d", id);
        return nullptr;
    }

    const ShaderNode** childArray = arena->makeArray<const ShaderNode*>(entry->fNumChildren);
    for (int i = 0; i < entry->fNumChildren; ++i) {
        const ShaderNode* child = this->createNode(dict, currentIndex, arena);
        if (!child) {
            return nullptr;
        }
        childArray[i] = child;
    }

    return arena->make<ShaderNode>(entry, SkSpan(childArray, entry->fNumChildren), id, index);
}

SkSpan<const ShaderNode*> PaintParamsKey::getRootNodes(const ShaderCodeDictionary* dict,
                                                       SkArenaAlloc* arena) const {
    // TODO: Once the PaintParamsKey creation is organized to represent a single tree starting at
    // the final blend, there will only be a single root node and this can be simplified.
    // For now, we don't know how many roots there are, so collect them into a local array before
    // copying into the arena.
    const int keySize = SkTo<int>(fData.size());

    // Normal PaintParams creation will have up to 7 roots for the different stages.
    STArray<7, const ShaderNode*> roots;
    int currentIndex = 0;
    while (currentIndex < keySize) {
        const ShaderNode* root = this->createNode(dict, &currentIndex, arena);
        if (!root) {
            return {}; // a bad key
        }
        roots.push_back(root);
    }

    // Copy the accumulated roots into a span stored in the arena
    const ShaderNode** rootSpan = arena->makeArray<const ShaderNode*>(roots.size());
    memcpy(rootSpan, roots.data(), roots.size_bytes());
    return SkSpan(rootSpan, roots.size());
}

#if defined(GRAPHITE_TEST_UTILS)

int key_to_string(SkString* str,
                  const ShaderCodeDictionary* dict,
                  SkSpan<const int32_t> keyData,
                  int currentIndex) {
    SkASSERT(currentIndex < SkTo<int>(keyData.size()));

    int32_t id = keyData[currentIndex++];
    auto entry = dict->getEntry(id);
    if (!entry) {
        str->append("UnknownCodeSnippetID:");
        str->appendS32(id);
        str->append(" ");
        return currentIndex;
    }

    std::string_view name = entry->fName;
    if (skstd::ends_with(name, "Shader")) {
        name.remove_suffix(6);
    }
    str->append(name);

    if (entry->fNumChildren > 0) {
        str->append(" [ ");
        for (int i = 0; i < entry->fNumChildren; ++i) {
            currentIndex = key_to_string(str, dict, keyData, currentIndex);
        }
        str->append("]");
    }

    str->append(" ");
    return currentIndex;
}

SkString PaintParamsKey::toString(const ShaderCodeDictionary* dict) const {
    SkString str;
    const int keySize = SkTo<int>(fData.size());
    for (int currentIndex = 0; currentIndex < keySize; ) {
        currentIndex = key_to_string(&str, dict, fData, currentIndex);
    }
    return str.isEmpty() ? SkString("(empty)") : str;
}

#endif  // defined(GRAPHITE_TEST_UTILS)

#ifdef SK_DEBUG

int dump_node(const ShaderCodeDictionary* dict,
              SkSpan<const int32_t> keyData,
              int currentIndex,
              int indent) {
    SkASSERT(currentIndex < SkTo<int>(keyData.size()));

    SkDebugf("%*c", 2 * indent, ' ');

    int32_t id = keyData[currentIndex++];
    auto entry = dict->getEntry(id);
    if (!entry) {
        SkDebugf("[%d] unknown block!\n", id);
        return currentIndex;
    }

    SkDebugf("[%d] %s\n", id, entry->fStaticFunctionName);
    for (int i = 0; i < entry->fNumChildren; ++i) {
        currentIndex = dump_node(dict, keyData, currentIndex, indent + 1);
    }
    return currentIndex;
}

void PaintParamsKey::dump(const ShaderCodeDictionary* dict) const {
    const int keySize = SkTo<int>(fData.size());

    SkDebugf("--------------------------------------\n");
    SkDebugf("PaintParamsKey (%d):\n", keySize);

    int currentIndex = 0;
    while (currentIndex < keySize) {
        currentIndex = dump_node(dict, fData, currentIndex, 1);
    }
}

#endif // SK_DEBUG

} // namespace skgpu::graphite
