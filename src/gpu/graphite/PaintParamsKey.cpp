/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PaintParamsKey.h"

#include "src/base/SkArenaAlloc.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

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

PaintParamsKey::BlockReader PaintParamsKey::reader(const ShaderCodeDictionary* dict,
                                                   int headerOffset) const {
    return BlockReader(dict, fData, headerOffset);
}

PaintParamsKey PaintParamsKey::clone(SkArenaAlloc* arena) const {
    int32_t* newData = arena->makeArrayDefault<int32_t>(fData.size());
    memcpy(newData, fData.data(), fData.size_bytes());
    return PaintParamsKey({newData, fData.size()});
}

#ifdef SK_DEBUG

// This just iterates over the top-level blocks calling block-specific dump methods.
void PaintParamsKey::dump(const ShaderCodeDictionary* dict) const {
    const int size = SkTo<int>(fData.size());
    SkDebugf("--------------------------------------\n");
    SkDebugf("PaintParamsKey (%d):\n", size);

    int curHeaderOffset = 0;
    while (curHeaderOffset < size) {
        BlockReader reader = this->reader(dict, curHeaderOffset);
        reader.dump(dict, /*indent=*/0);
        curHeaderOffset += reader.blockSize();
    }
}
#endif // SK_DEBUG

namespace {

// TODO: This is inefficient but lets us reconstruct the BlockReaders without storing the size in
// the key. In a follow-up CL, BlockReader will be removed and a ShaderNode tree will be constructed
// once for a key during SkSL generation that has more conventional access patterns. Then this
// helper will go away entirely because the entire tree can be created in a single key traversal.
int skip_children_offset(const ShaderCodeDictionary* dict,
                         SkSpan<const int32_t> keySpan,
                         int firstChildOffset,
                         int numChildrenToSkip) {
    // Should only be called for snippets/blocks that have children to advance over
    SkASSERT(!keySpan.empty());

    int offset = firstChildOffset;
    for (int i = 0; i < numChildrenToSkip; ++i) {
        // Always advance 1 over the code snippet ID for the child
        int32_t childID = keySpan[offset++];
        const ShaderSnippet* entry = dict->getEntry(childID);
        SkASSERT(entry);

        if (entry->fNumChildren > 0) {
            offset += skip_children_offset(dict, keySpan, offset, entry->fNumChildren);
        }
    }
    return offset - firstChildOffset;
}

constexpr skgpu::BlendInfo make_simple_blendInfo(skgpu::BlendCoeff srcCoeff,
                                                 skgpu::BlendCoeff dstCoeff) {
    return { skgpu::BlendEquation::kAdd,
             srcCoeff,
             dstCoeff,
             SK_PMColor4fTRANSPARENT,
             skgpu::BlendModifiesDst(skgpu::BlendEquation::kAdd, srcCoeff, dstCoeff) };
}

static constexpr int kNumCoeffModes = (int)SkBlendMode::kLastCoeffMode + 1;
static constexpr skgpu::BlendInfo gBlendTable[kNumCoeffModes] = {
        /* clear */      make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kZero),
        /* src */        make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kZero),
        /* dst */        make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kOne),
        /* src-over */   make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISA),
        /* dst-over */   make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kOne),
        /* src-in */     make_simple_blendInfo(skgpu::BlendCoeff::kDA,   skgpu::BlendCoeff::kZero),
        /* dst-in */     make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kSA),
        /* src-out */    make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kZero),
        /* dst-out */    make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kISA),
        /* src-atop */   make_simple_blendInfo(skgpu::BlendCoeff::kDA,   skgpu::BlendCoeff::kISA),
        /* dst-atop */   make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kSA),
        /* xor */        make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kISA),
        /* plus */       make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kOne),
        /* modulate */   make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kSC),
        /* screen */     make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISC)
};

void add_block_to_shader_info(const ShaderCodeDictionary* dict,
                              const PaintParamsKey::BlockReader& reader,
                              ShaderInfo* result) {
    result->add(reader);
    result->addFlags(dict->getEntry(reader.codeSnippetId())->fSnippetRequirementFlags);

    if (reader.codeSnippetId() < kBuiltInCodeSnippetIDCount &&
        reader.codeSnippetId() >= kFixedFunctionBlendModeIDOffset) {
        SkASSERT(reader.numChildren() == 0);

        // Set SkBlendInfo on the ShaderInfo if the block represents a fixed function blend. The
        // key and paint are defined in such a way that this can occur at most once.
        int coeffBlendMode = reader.codeSnippetId() - kFixedFunctionBlendModeIDOffset;
        SkASSERT(coeffBlendMode >= 0 &&
                 static_cast<SkBlendMode>(coeffBlendMode) <= SkBlendMode::kLastCoeffMode);
        result->setBlendInfo(gBlendTable[coeffBlendMode]);
    } else {
        // The child blocks appear right after the parent block's header in the key and go
        // right after the parent's SnippetEntry in the shader info
        for (int i = 0; i < reader.numChildren(); ++i) {
            add_block_to_shader_info(dict, reader.child(dict, i), result);
        }
    }
}

} // anonymous

void PaintParamsKey::toShaderInfo(const ShaderCodeDictionary* dict,
                                  ShaderInfo* result) const {
    const int size = SkTo<int>(fData.size());
    int curHeaderOffset = 0;
    while (curHeaderOffset < size) {
        BlockReader reader = this->reader(dict, curHeaderOffset);
        add_block_to_shader_info(dict, reader, result);
        curHeaderOffset += reader.blockSize();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PaintParamsKey::BlockReader::BlockReader(const ShaderCodeDictionary* dict,
                                         SkSpan<const int32_t> parentSpan,
                                         int offsetInParent) {
    int32_t codeSnippetID = parentSpan[offsetInParent];
    fEntry = dict->getEntry(codeSnippetID);
    SkASSERT(fEntry);

    int blockSize = 1; // always at least 1 for the block's ID
    if (fEntry->fNumChildren > 0) {
        blockSize += skip_children_offset(dict,
                                          parentSpan,
                                          offsetInParent + 1,
                                          fEntry->fNumChildren);
    }

    fBlock = parentSpan.subspan(offsetInParent, blockSize);
}

int PaintParamsKey::BlockReader::numChildren() const { return fEntry->fNumChildren; }

PaintParamsKey::BlockReader PaintParamsKey::BlockReader::child(
        const ShaderCodeDictionary* dict,
        int childIndex) const {
    SkASSERT(childIndex < fEntry->fNumChildren);

    int childOffset = 1; // skip parent ID
    childOffset += skip_children_offset(dict, fBlock, childOffset, childIndex);
    return BlockReader(dict, fBlock, childOffset);
}

#ifdef SK_DEBUG

static void output_indent(int indent) {
    SkDebugf("%*c", 4 * indent, ' ');
}

void PaintParamsKey::BlockReader::dump(const ShaderCodeDictionary* dict, int indent) const {
    uint8_t id = static_cast<uint8_t>(this->codeSnippetId());
    uint8_t blockSize = this->blockSize();

    auto entry = dict->getEntry(id);
    if (!entry) {
        output_indent(indent);
        SkDebugf("unknown block! (%dB)\n", blockSize);
    }

    output_indent(indent);
    SkDebugf("%s block (%dB)\n", entry->fStaticFunctionName, blockSize);

    for (int i = 0; i < this->numChildren(); ++i) {
        output_indent(indent);
        // TODO: it would be nice if the names of the children were also stored (i.e., "src"/"dst")
        SkDebugf("child %d:\n", i);

        PaintParamsKey::BlockReader childReader = this->child(dict, i);
        childReader.dump(dict, indent+1);
    }
}

#endif // SK_DEBUG

} // namespace skgpu::graphite
