/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PaintParamsKey_DEFINED
#define skgpu_graphite_PaintParamsKey_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkOpts_spi.h"
#include "include/private/base/SkMacros.h"
#include "include/private/base/SkTDArray.h"
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/BuiltInCodeSnippetID.h"

#include <limits>

class SkArenaAlloc;

namespace skgpu::graphite {

class ShaderCodeDictionary;
class ShaderInfo;
struct ShaderSnippet;

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

    // TODO: Remove in favor of arena-allocated node tree that can be created from the key, vs.
    // the current lightweight, index-based view of the key (which complicates SkSL generation).
    class BlockReader {
    public:
        // Returns the combined size of the node's ID and all children.
        int blockSize() const { return SkTo<int>(fBlock.size()); }

        int numChildren() const;

        // Returns the code-snippet ID of this block.
        int32_t codeSnippetId() const { return fBlock[0]; }

        // Return the childIndex-th child's BlockReader
        BlockReader child(const ShaderCodeDictionary*, int childIndex) const;

        const ShaderSnippet* entry() const { return fEntry; }

#ifdef SK_DEBUG
        void dump(const ShaderCodeDictionary*, int indent) const;
#endif

    private:
        friend class PaintParamsKey; // for ctor

        BlockReader(const ShaderCodeDictionary*,
                    SkSpan<const int32_t> parentSpan,
                    int offsetInParent);

        SkSpan<const int32_t> fBlock;
        const ShaderSnippet* fEntry;
    };

    BlockReader reader(const ShaderCodeDictionary*, int headerOffset) const;

    void toShaderInfo(const ShaderCodeDictionary*, ShaderInfo*) const;

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
            return SkOpts::hash_fn(k.fData.data(), k.fData.size_bytes(), 0);
        }
    };

private:
    friend class PaintParamsKeyBuilder;   // for the parented-data ctor

    constexpr PaintParamsKey(SkSpan<const int32_t> span) : fData(span) {}

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
    SkTDArray<int32_t> fData;

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
    SkTDArray<StackFrame> fStack;
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

} // skgpu::graphite

#endif // skgpu_graphite_PaintParamsKey_DEFINED
