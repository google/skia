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
    // Check that addData() is only called for snippets that support it and is only called once
    const ShaderSnippet* snippet = fDict->getEntry(fStack.back().fCodeSnippetID);
    SkASSERT(snippet->storesSamplerDescData());
    SkASSERT(fStack.back().fDataSize < 0);

    fStack.back().fDataSize = SkTo<int>(dataSize);
}

void PaintParamsKeyBuilder::popStack() {
    SkASSERT(!fStack.empty());
    SkASSERT(fStack.back().fNumActualChildren == fStack.back().fNumExpectedChildren);
    const bool expectsData = fDict->getEntry(fStack.back().fCodeSnippetID)->storesSamplerDescData();
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

ShaderNode* PaintParamsKey::createNode(const ShaderCodeDictionary* dict,
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
    if (entry->storesSamplerDescData()) {
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

    ShaderNode** childArray = arena->makeArray<ShaderNode*>(entry->fNumChildren);
    for (int i = 0; i < entry->fNumChildren; ++i) {
        ShaderNode* child = this->createNode(dict, currentIndex, arena);
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

// Traverse a ShaderNode tree, attempting to lift any coordinate modification expressions.
// Returns whether any of the given nodes need local coordinate inputs after lifting.
bool lift_coord_expressions(SkSpan<ShaderNode*> nodes, int* availableVaryings) {
    bool anyNeedLocalCoords = false;

    for (ShaderNode* node : nodes) {
        bool curNeedsLocalCoords =
                SkToBool(node->requiredFlags() & SnippetRequirementFlags::kLocalCoords);

        // Lift expressions from nodes whose liftable expressions are on coordinate inputs.
        if (*availableVaryings > 0 && curNeedsLocalCoords &&
            node->entry()->fLiftableExpressionType ==
                    ShaderSnippet::LiftableExpressionType::kLocalCoords) {
            --*availableVaryings;

#if !defined(SK_USE_LEGACY_UNIFORM_LIFTING_GRAPHITE)
            // We can potentially lift the nested expressions under here as well.
            const bool childNeedsOurCoords =
                    lift_coord_expressions(node->children(), availableVaryings);
            // If no child needs our lifted coords, we can omit them from the fragment shader
            // entirely, and only use them in the vertex shader for calculating other coords.
            if (!childNeedsOurCoords) {
                node->setOmitExpressionFlag();
            } else {
                node->setLiftExpressionFlag();
            }
#else
            node->setLiftExpressionFlag();
#endif
            // Since we lifted the coordinate expression here, this node no longer needs a local
            // coords argument.
            curNeedsLocalCoords = false;
            node->unsetLocalCoordsFlag();

#if !defined(SK_USE_LEGACY_UNIFORM_LIFTING_GRAPHITE)
        // If the node passes through its local coords to its children, we check if those perform
        // modifications that can be lifted.
        } else if (*availableVaryings > 0 &&
                   node->requiredFlags() & SnippetRequirementFlags::kPassthroughLocalCoords) {
            // Assume that this node doesn't need local coordinates unless its actual shader snippet
            // entry does, or one of its children does even after accounting for lifting.
            const bool entryNeedsLocalCoords = node->entry()->needsLocalCoords();
            const bool childNeedsLocalCoords = lift_coord_expressions(node->children(),
                                                                      availableVaryings);
            curNeedsLocalCoords = entryNeedsLocalCoords || childNeedsLocalCoords;
            if (!curNeedsLocalCoords) {
                node->unsetLocalCoordsFlag();
            }
#endif
        }

        anyNeedLocalCoords |= curNeedsLocalCoords;
    }

    return anyNeedLocalCoords;
}

// Traverse a list of ShaderNodes, attempting to lift any expressions that resolve to a color.
// For now, this does not recurse into ShaderNodes' lists of children. In practice we only lift
// solid color expressions, and we only care to lift such expressions if there is no other fragment
// shader work (i.e., if the solid color expression is a root node in a shader's ShaderNode tree).
// If there is other fragment shader work, we'll likely be accessing other fragment shader uniforms,
// the color value will likely be cached, and lifting may not be worth the extra varying.
void lift_color_expressions(SkSpan<ShaderNode*> nodes, int* availableVaryings) {
#if !defined(SK_USE_LEGACY_UNIFORM_LIFTING_GRAPHITE)
    for (ShaderNode* node : nodes) {
        if (*availableVaryings > 0 &&
            node->entry()->fLiftableExpressionType ==
                    ShaderSnippet::LiftableExpressionType::kPriorStageOutput) {
            --*availableVaryings;
            node->setLiftExpressionFlag();
        }
    }
#endif
}

SkSpan<const ShaderNode*> PaintParamsKey::getRootNodes(const Caps* caps,
                                                       const ShaderCodeDictionary* dict,
                                                       SkArenaAlloc* arena,
                                                       int availableVaryings) const {
    // TODO: Once the PaintParamsKey creation is organized to represent a single tree starting at
    // the final blend, there will only be a single root node and this can be simplified.
    // For now, we don't know how many roots there are, so collect them into a local array before
    // copying into the arena.
    const int keySize = SkTo<int>(fData.size());

    // Normal PaintParams creation will have up to 7 roots for the different stages.
    STArray<7, ShaderNode*> roots;
    int currentIndex = 0;
    while (currentIndex < keySize) {
        ShaderNode* root = this->createNode(dict, &currentIndex, arena);
        if (!root) {
            return {}; // a bad key
        }
        roots.push_back(root);
    }

    // See what expressions we can lift to the vertex shader.
    const bool hasClipNode = roots.size() > 2;
    SkSpan<ShaderNode*> liftableNodes(roots.data(), hasClipNode ? 2 : roots.size());
    lift_coord_expressions(liftableNodes, &availableVaryings);
    // Don't lift constant expressions if we're using regular UBOs, since lifting is likely only
    // beneficial if we're avoiding a storage buffer access.
    if (caps->storageBufferSupport()) {
        lift_color_expressions(liftableNodes, &availableVaryings);
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

    str->append(entry->fName);

    if (entry->storesSamplerDescData()) {
        SkASSERT(currentIndex + 1 < SkTo<int>(keyData.size()));

        // If an entry stores data, then the next key value reports the quantity of key indices that
        // are used to house the data for this snippet. This way, we know how many indices to
        // iterate over in order to capture the snippet's data before we may encounter another
        // snippet ID.
        // For example:
        // [snippetId using 2 indices worth of data] [2] [dataValue0] [dataValue1] [next snippet ID]
        const int dataIndexCount = keyData[currentIndex++];
        SkASSERT(currentIndex + dataIndexCount < SkTo<int>(keyData.size()));

        // We shorten the string for the common case of no extra data.
        if (dataIndexCount == 0) {
            str->append("(0)");
        } else {
            str->append("(");
            str->appendU32(dataIndexCount);
            str->append(": ");
            // Encode data in base64 to shorten it
            const size_t srcDataSize = dataIndexCount * sizeof(uint32_t); // size in bytes
            SkAutoMalloc encodedData{SkBase64::EncodedSize(srcDataSize)};
            char* dst = static_cast<char*>(encodedData.get());
            size_t encodedLen = SkBase64::Encode(&keyData[currentIndex], srcDataSize, dst);
            str->append(dst, encodedLen);
            str->append(")");
        }
        // Increment current index past the indices which contain data
        currentIndex += dataIndexCount;
    }

    if (entry->fNumChildren > 0) {
        if (multiline) {
            str->append(":\n");
            indent++;
        } else {
            str->append(" [ ");
        }

        for (int i = 0; i < entry->fNumChildren; ++i) {
            currentIndex = key_to_string(str, dict, keyData, currentIndex, indent);
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

SkString PaintParamsKey::toString(const ShaderCodeDictionary* dict) const {
    SkString str;
    const int keySize = SkTo<int>(fData.size());
    for (int currentIndex = 0; currentIndex < keySize;) {
        currentIndex = key_to_string(&str, dict, fData, currentIndex, /*indent=*/-1);
    }
    return str.isEmpty() ? SkString("(empty)") : str;
}

#ifdef SK_DEBUG

void PaintParamsKey::dump(const ShaderCodeDictionary* dict, UniquePaintParamsID id) const {
    const int keySize = SkTo<int>(fData.size());

    SkDebugf("--------------------------------------\n");
    SkDebugf("PaintParamsKey %u (keySize: %d): ", id.asUInt(), keySize);
    const uint32_t* data = fData.data();
    for (int i = 0; i < keySize; ++i) {
        SkDebugf("%x ", data[i]);
    }
    SkDebugf("\n");

    int currentIndex = 0;
    while (currentIndex < keySize) {
        SkString nodeStr;
        currentIndex = key_to_string(&nodeStr, dict, fData, currentIndex, /*indent=*/1);
        SkDebugf("%s", nodeStr.c_str());
    }
}

#endif // SK_DEBUG

namespace {

// check a single block and, recursively, all its children
[[nodiscard]] bool is_block_valid(const ShaderCodeDictionary* dict,
                                  SkSpan<const uint32_t> keyData,
                                  int* currentIndex) {
    if (*currentIndex >= SkTo<int>(keyData.size())) {
        return false;
    }

    uint32_t id = keyData[(*currentIndex)++];
    if (id >= kBuiltInCodeSnippetIDCount &&
        !SkKnownRuntimeEffects::IsSkiaKnownRuntimeEffect(id) &&
        !dict->isUserDefinedKnownRuntimeEffect(id)) {
        return false;
    }

    auto entry = dict->getEntry(id);
    if (!entry) {
        return false;
    }

    if (entry->storesSamplerDescData()) {
        if (*currentIndex + 1 >= SkTo<int>(keyData.size())) {
            return false;
        }

        const int dataLength = keyData[(*currentIndex)++];

        if (*currentIndex + dataLength >= SkTo<int>(keyData.size())) {
            return false;
        }

        *currentIndex += dataLength;
    }

    if (entry->fNumChildren > 0) {
        for (int i = 0; i < entry->fNumChildren; ++i) {
            if (!is_block_valid(dict, keyData, currentIndex)) {
                return false;
            }
        }
    }

    return true;
}

} // anonymous namespace


bool PaintParamsKey::isSerializable(const ShaderCodeDictionary* dict) const {
    const int keySize = SkTo<int>(fData.size());

    int currentIndex = 0;
    while (currentIndex < keySize) {
        if (!is_block_valid(dict, fData, &currentIndex)) {
            return false;
        }
    }

    return true;
}

} // namespace skgpu::graphite
