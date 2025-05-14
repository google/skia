/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PaintParamsKey_DEFINED
#define skgpu_graphite_PaintParamsKey_DEFINED

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkMacros.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkChecksum.h"
#include "src/gpu/graphite/BuiltInCodeSnippetID.h"

#include <limits>
#include <cstring> // for memcmp

class SkArenaAlloc;
struct SkSamplingOptions;
enum class SkTileMode;

namespace skgpu::graphite {

class Caps;
class ShaderCodeDictionary;
class ShaderNode;
class TextureProxy;
class UniquePaintParamsID;

/**
 * This class is a compact representation of the shader needed to implement a given
 * PaintParams. Its structure is a series of nodes where each node consists of:
 *   4 bytes: code-snippet ID
 *   N child nodes, where N is the constant number of children defined by the ShaderCodeDictionary
 *     for the node's snippet ID.
 *
 * Some snippet definitions support embedding data into the PaintParamsKey, used when something
 * external to the generated SkSL needs produce unique pipelines (e.g. immutable samplers). For
 * snippets that store data, the data is stored immediately after the ID as:
 *   4 bytes: code-snippet ID
 *   4 bytes: data length
 *   0-M: variable length data
 *   N child nodes
 *
 * All children of a child node are stored in the key before the next child is encoded in the key,
 * e.g. iterating the data in a key is a depth-first traversal of the node tree.
 *
 * The PaintParamsKey stores multiple root nodes, with each root representing an effect tree that
 * affects different parts of the shading pipeline. The key is can only hold 2 or 3 roots:
 *  1. Color root node: produces the "src" color used in final blending with the "dst" color.
 *  2. Final blend node: defines the blend function combining src and dst colors. If this is a
 *     FixedBlend snippet the final pipeline may be able to lift it to HW blending.
 *  3. Clipping: optional, produces analytic coverage from a clip shader or shape.
 *
 * Logically the root effects produce a src color and the src coverage (augmenting any other
 * coverage coming from the RenderStep). A single src shading node could be used instead of the
 * two for color and blending, but its structure would always be:
 *
 *    [ BlendCompose [ [ color-root-node ] surface-color [ final-blend ] ] ]
 *
 * where "surface-color" would be a special snippet that produces the current dst color value.
 * To keep PaintParamsKeys memory cost lower, the BlendCompose and "surface-color" nodes are implied
 * when generating the SkSL and pipeline.
 */
class PaintParamsKey {
public:
    // PaintParamsKey can only be created by using a PaintParamsKeyBuilder or by cloning the key
    // data from a Builder-owned key, but they can be passed around by value after that.
    constexpr PaintParamsKey(const PaintParamsKey&) = default;

    constexpr PaintParamsKey(SkSpan<const uint32_t> span) : fData(span) {}

    ~PaintParamsKey() = default;
    PaintParamsKey& operator=(const PaintParamsKey&) = default;

    static constexpr PaintParamsKey Invalid() { return PaintParamsKey(SkSpan<const uint32_t>()); }
    bool isValid() const { return !fData.empty(); }

    // Return a PaintParamsKey whose data is owned by the provided arena and is not attached to
    // a PaintParamsKeyBuilder. The caller must ensure that the SkArenaAlloc remains alive longer
    // than the returned key.
    PaintParamsKey clone(SkArenaAlloc*) const;

    // Converts the key into a forest of ShaderNode trees. If the key is valid this will return at
    // least one root node. If the key contains unknown shader snippet IDs, returns an empty span.
    // All shader nodes, and the returned span's backing data, are owned by the provided arena.
    //
    // A valid key will produce either 2 or 3 root nodes. The first root node represents how the
    // source color is computed. The second node defines the final blender between the calculated
    // source color and the current pixel's dst color. If provided, the third node calculates an
    // additional analytic coverage value to combine with the geometry's coverage.
    //
    // Before returning the ShaderNode trees, this method decides which ShaderNode expressions to
    // lift to the vertex shader, depending on how many varyings are available.
    SkSpan<const ShaderNode*> getRootNodes(const Caps*,
                                           const ShaderCodeDictionary*,
                                           SkArenaAlloc*,
                                           int availableVaryings) const;

    // Converts the key to a structured list of snippet information for debugging or labeling
    // purposes.
    SkString toString(const ShaderCodeDictionary* dict) const;

#ifdef SK_DEBUG
    void dump(const ShaderCodeDictionary*, UniquePaintParamsID) const;
#endif

    bool operator==(const PaintParamsKey& that) const {
        return fData.size() == that.fData.size() &&
               !memcmp(fData.data(), that.fData.data(), fData.size());
    }
    bool operator!=(const PaintParamsKey& that) const { return !(*this == that); }

    struct Hash {
        uint32_t operator()(const PaintParamsKey& k) const {
            return SkChecksum::Hash32(k.fData.data(), k.fData.size_bytes());
        }
    };

    SkSpan<const uint32_t> data() const { return fData; }

    // Checks that a given key is viable for serialization and, also, that a deserialized
    // key is, at least, correctly formed. Other than that all the sizes make sense, this method
    // also checks that only Skia-internal shader code snippets appear in the key.
    [[nodiscard]] bool isSerializable(const ShaderCodeDictionary*) const;

private:
    friend class PaintParamsKeyBuilder;   // for the parented-data ctor

    // Returns null if the node or any of its children have an invalid snippet ID. Recursively
    // creates a node and all of its children, incrementing 'currentIndex' by the total number of
    // nodes created.
    ShaderNode* createNode(const ShaderCodeDictionary*,
                           int* currentIndex,
                           SkArenaAlloc* arena) const;

    // The memory referenced in 'fData' is always owned by someone else. It either shares the span
    // from the Builder, or clone() puts the span in an arena.
    SkSpan<const uint32_t> fData;
};

// The PaintParamsKeyBuilder and the PaintParamsKeys snapped from it share the same
// underlying block of memory. When an PaintParamsKey is snapped from the builder it 'locks'
// the memory and 'unlocks' it in its destructor. Because of this relationship, the builder
// can only have one extant key and that key must be destroyed before the builder can be reused
// to create another one.
//
// This arrangement is intended to improve performance in the expected case, where a builder is
// being used in a tight loop to generate keys which can be recycled once they've been used to
// find the dictionary's matching uniqueID. We don't expect the cost of copying the key's memory
// into the dictionary to be prohibitive since that should be infrequent.
class PaintParamsKeyBuilder {
public:
    PaintParamsKeyBuilder(const ShaderCodeDictionary* dict) {
        SkDEBUGCODE(fDict = dict;)
    }

    ~PaintParamsKeyBuilder() { SkASSERT(!fLocked); }

    void beginBlock(BuiltInCodeSnippetID id) { this->beginBlock(static_cast<int32_t>(id)); }
    void beginBlock(int32_t codeSnippetID) {
        SkASSERT(!fLocked);
        SkDEBUGCODE(this->pushStack(codeSnippetID);)
        fData.push_back(codeSnippetID);
    }

    // TODO: Have endBlock() be handled automatically with RAII, in which case we could have it
    // validate the snippet ID being popped off the stack frame.
    void endBlock() {
        SkDEBUGCODE(this->popStack();)
    }

#ifdef SK_DEBUG
    // Check that the builder has been reset to its initial state prior to creating a new key.
    void checkReset();
#endif

    // Helper to add blocks that don't have children
    void addBlock(BuiltInCodeSnippetID id) {
        this->beginBlock(id);
        this->endBlock();
    }

    void addData(SkSpan<const uint32_t> data) {
        // First push the data size followed by the actual data.
        SkDEBUGCODE(this->validateData(data.size()));
        fData.push_back(data.size());
        fData.push_back_n(data.size(), data.begin());
    }

private:
    friend class AutoLockBuilderAsKey; // for lockAsKey() and unlock()

    // Returns a view of this builder as a PaintParamsKey. The Builder cannot be used until the
    // returned Key goes out of scope.
    PaintParamsKey lockAsKey() {
        SkASSERT(!fLocked);       // lockAsKey() is not re-entrant
        SkASSERT(fStack.empty()); // All beginBlocks() had a matching endBlock()

        SkDEBUGCODE(fLocked = true;)
        return PaintParamsKey({fData.data(), fData.size()});
    }

    // Invalidates any PaintParamsKey returned by lockAsKey() unless it has been cloned.
    void unlock() {
        SkASSERT(fLocked);
        fData.clear();

        SkDEBUGCODE(fLocked = false;)
        SkDEBUGCODE(fStack.clear();)
        SkDEBUGCODE(this->checkReset();)
    }

    // The data array uses clear() on unlock so that it's underlying storage and repeated use of the
    // builder will hit a high-water mark and avoid lots of allocations when recording draws.
    skia_private::TArray<uint32_t> fData;

#ifdef SK_DEBUG
    void pushStack(int32_t codeSnippetID);
    void validateData(size_t dataSize);
    void popStack();

    // Information about the current block being written
    struct StackFrame {
        int fCodeSnippetID;
        int fNumExpectedChildren;
        int fNumActualChildren = 0;
        int fDataSize = -1;
    };

    const ShaderCodeDictionary* fDict;
    skia_private::TArray<StackFrame> fStack;
    bool fLocked = false;
#endif
};

class AutoLockBuilderAsKey {
public:
    AutoLockBuilderAsKey(PaintParamsKeyBuilder* builder)
            : fBuilder(builder)
            , fKey(builder->lockAsKey()) {}

    ~AutoLockBuilderAsKey() {
        fBuilder->unlock();
    }

    // Use as a PaintParamsKey
    const PaintParamsKey& operator*() const { return fKey; }
    const PaintParamsKey* operator->() const { return &fKey; }

private:
    PaintParamsKeyBuilder* fBuilder;
    PaintParamsKey fKey;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_PaintParamsKey_DEFINED
