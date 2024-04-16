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

namespace skgpu::graphite {

class ShaderCodeDictionary;
class ShaderNode;

// This class is a compact representation of the shader needed to implement a given
// PaintParams. Its structure is a series of nodes where each node consists of:
//   4 bytes: code-snippet ID
//   N child nodes, where N is the constant number of children defined by the ShaderCodeDictionary
//     for the node's snippet ID.
//
// All children of a child node are stored in the key before the next child is encoded in the key,
// e.g. iterating the data in a key is a depth-first traversal of the node tree.
class PaintParamsKey {
public:
    // PaintParamsKey can only be created by using a PaintParamsKeyBuilder or by cloning the key
    // data from a Builder-owned key, but they can be passed around by value after that.
    constexpr PaintParamsKey(const PaintParamsKey&) = default;

    ~PaintParamsKey() = default;
    PaintParamsKey& operator=(const PaintParamsKey&) = default;

    static constexpr PaintParamsKey Invalid() { return PaintParamsKey(SkSpan<const int32_t>()); }
    bool isValid() const { return !fData.empty(); }

    // Return a PaintParamsKey whose data is owned by the provided arena and is not attached to
    // a PaintParamsKeyBuilder. The caller must ensure that the SkArenaAlloc remains alive longer
    // than the returned key.
    PaintParamsKey clone(SkArenaAlloc*) const;

    // Converts the key into a forest of ShaderNode trees. If the key is valid this will return at
    // least one root node. If the key contains unknown shader snippet IDs, returns an empty span.
    // All shader nodes, and the returned span's backing data, are owned by the provided arena.
    // TODO: Strengthen PaintParams key generation so we can assume there's only ever one root node
    // representing the final blend (either a shader blend (with 2 children: main effect & dst) or
    // a fixed function blend (with 1 child being the main effect)).
    SkSpan<const ShaderNode*> getRootNodes(const ShaderCodeDictionary*, SkArenaAlloc*) const;

    // Converts the key to a structured list of snippet names for debugging or labeling purposes.
    SkString toString(const ShaderCodeDictionary* dict) const;

#ifdef SK_DEBUG
    void dump(const ShaderCodeDictionary*) const;
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

private:
    friend class PaintParamsKeyBuilder;   // for the parented-data ctor

    constexpr PaintParamsKey(SkSpan<const int32_t> span) : fData(span) {}

    // Returns null if the node or any of its children have an invalid snippet ID. Recursively
    // creates a node and all of its children, incrementing 'currentIndex' by the total number of
    // nodes created.
    const ShaderNode* createNode(const ShaderCodeDictionary*,
                                 int* currentIndex,
                                 SkArenaAlloc* arena) const;

    // The memory referenced in 'fData' is always owned by someone else. It either shares the span
    // of from the Builder, or clone() puts the span in an arena.
    SkSpan<const int32_t> fData;
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
    skia_private::TArray<int32_t> fData;

#ifdef SK_DEBUG
    void pushStack(int32_t codeSnippetID);
    void popStack();

    // Information about the current block being written
    struct StackFrame {
        int fCodeSnippetID;
        int fNumExpectedChildren;
        int fNumActualChildren = 0;
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
