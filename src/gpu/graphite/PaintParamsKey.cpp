/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PaintParamsKey.h"

#include "src/base/SkArenaAlloc.h"
#include "src/base/SkAutoMalloc.h"
#include "src/base/SkBase64.h"
#include "src/base/SkStringView.h"
#include "src/gpu/graphite/Caps.h"
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

void PaintParamsKeyBuilder::validateData(size_t dataSize) {
    SkASSERT(!fStack.empty()); // addData() called within code snippet block

    const ShaderSnippet* snippet = fDict->getEntry(fStack.back().fCodeSnippetID);
    SkASSERT(snippet->storesData()); // addData() only called for ShaderSnippets that support it
    SkASSERT(fStack.back().fDataSize < 0); // And only called once
    fStack.back().fDataSize = SkTo<int>(dataSize);
}

void PaintParamsKeyBuilder::popStack() {
    SkASSERT(!fStack.empty());
    SkASSERT(fStack.back().fNumActualChildren == fStack.back().fNumExpectedChildren);
    const bool expectsData = fDict->getEntry(fStack.back().fCodeSnippetID)->storesData();
    const bool hasData = fStack.back().fDataSize >= 0;
    SkASSERT(expectsData == hasData);
    fStack.pop_back();
}

#endif // SK_DEBUG

//--------------------------------------------------------------------------------------------------
// PaintParamsKey

PaintParamsKey PaintParamsKey::clone(SkArenaAlloc* arena) const {
    uint32_t* newData = arena->makeArrayDefault<uint32_t>(fData.size());
    memcpy(newData, fData.data(), fData.size_bytes());
    return PaintParamsKey({newData, fData.size()});
}


const ShaderNode* PaintParamsKey::createNode(const ShaderCodeDictionary* dict,
                                             int* currentIndex,
                                             SkArenaAlloc* arena) const {
    SkASSERT(*currentIndex < SkTo<int>(fData.size()));
    const int32_t index = (*currentIndex)++;
    const int32_t id = fData[index];

    const ShaderSnippet* entry = dict->getEntry(id);
    if (!entry) {
        SKGPU_LOG_E("Unknown snippet ID in key: %d", id);
        return nullptr;
    }

    SkSpan<const uint32_t> dataSpan = {};
    if (entry->storesData()) {
        // If a snippet stores data, then the subsequent paint key index signifies the length of
        // its data. Determine this data length and iterate currentIndex past it.
        const int storedDataLengthIdx = (*currentIndex)++;
        SkASSERT(storedDataLengthIdx < SkTo<int>(fData.size()));
        const int dataLength = fData[storedDataLengthIdx];
        SkASSERT(storedDataLengthIdx + dataLength < SkTo<int>(fData.size()));

        // Gather the data contents (length can now be inferred by the consumers of the data) to
        // pass into ShaderNode creation. Iterate the paint key index past the data indices.
        dataSpan = fData.subspan(storedDataLengthIdx + 1, dataLength);
        *currentIndex += dataLength;
    }

    const ShaderNode** childArray = arena->makeArray<const ShaderNode*>(entry->fNumChildren);
    for (int i = 0; i < entry->fNumChildren; ++i) {
        const ShaderNode* child = this->createNode(dict, currentIndex, arena);
        if (!child) {
            return nullptr;
        }
        childArray[i] = child;
    }

    return arena->make<ShaderNode>(entry,
                                   SkSpan(childArray, entry->fNumChildren),
                                   id,
                                   index,
                                   dataSpan);
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

static int key_to_string(SkString* str,
                         const ShaderCodeDictionary* dict,
                         SkSpan<const uint32_t> keyData,
                         int currentIndex,
                         bool includeData,
                         int indent) {
    SkASSERT(currentIndex < SkTo<int>(keyData.size()));

    const bool multiline = indent >= 0;
    if (multiline) {
        // Format for multi-line printing
        str->appendf("%*c", 2 * indent, ' ');
    }

    uint32_t id = keyData[currentIndex++];
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

    if (entry->storesData()) {
        SkASSERT(currentIndex + 1 < SkTo<int>(keyData.size()));
        const int dataLength = keyData[currentIndex++];
        SkASSERT(currentIndex + dataLength < SkTo<int>(keyData.size()));

        // Define a compact representation for the common case of shader snippets using just one
        // dynamic sampler. Immutable samplers require a data length > 1 to be represented while a
        // dynamic sampler is represented with just one, so we can simply consult the data length.
        if (dataLength == 1) {
            str->append("(0)");
        } else {
            str->append("(");
            str->appendU32(dataLength);
            if (includeData) {
                // Encode data in base64 to shorten it
                str->append(": ");
                SkAutoMalloc encodedData{SkBase64::EncodedSize(dataLength)};
                char* dst = static_cast<char*>(encodedData.get());
                size_t encodedLen = SkBase64::Encode(&keyData[currentIndex], dataLength, dst);
                str->append(dst, encodedLen);
            }
            str->append(")");
        }

        currentIndex += dataLength;
    }

    if (entry->fNumChildren > 0) {
        if (multiline) {
            str->append(":\n");
            indent++;
        } else {
            str->append(" [ ");
        }

        for (int i = 0; i < entry->fNumChildren; ++i) {
            currentIndex = key_to_string(str, dict, keyData, currentIndex, includeData, indent);
        }

        if (!multiline) {
            str->append("]");
        }
    }

    if (!multiline) {
        str->append(" ");
    } else if (entry->fNumChildren == 0) {
        str->append("\n");
    }
    return currentIndex;
}

SkString PaintParamsKey::toString(const ShaderCodeDictionary* dict, bool includeData) const {
    SkString str;
    const int keySize = SkTo<int>(fData.size());
    for (int currentIndex = 0; currentIndex < keySize;) {
        currentIndex = key_to_string(&str, dict, fData, currentIndex, includeData, /*indent=*/-1);
    }
    return str.isEmpty() ? SkString("(empty)") : str;
}

#ifdef SK_DEBUG

void PaintParamsKey::dump(const ShaderCodeDictionary* dict, UniquePaintParamsID id) const {
    const int keySize = SkTo<int>(fData.size());

    SkDebugf("--------------------------------------\n");
    SkDebugf("PaintParamsKey %u (keySize: %d):\n", id.asUInt(), keySize);

    int currentIndex = 0;
    while (currentIndex < keySize) {
        SkString nodeStr;
        currentIndex = key_to_string(&nodeStr, dict, fData, currentIndex,
                                     /*includeData=*/true, /*indent=*/1);
        SkDebugf("%s", nodeStr.c_str());
    }
}

#endif // SK_DEBUG

} // namespace skgpu::graphite
