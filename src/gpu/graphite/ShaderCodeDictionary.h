/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ShaderCodeDictionary_DEFINED
#define skgpu_graphite_ShaderCodeDictionary_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkMacros.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkThreadAnnotations.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkSpinlock.h"
#include "src/core/SkTHash.h"
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/BuiltInCodeSnippetID.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/Uniform.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

class SkRuntimeEffect;

namespace skgpu {
class Swizzle;
}

namespace skgpu::graphite {

class Caps;
class RenderStep;
class RuntimeEffectDictionary;

// TODO: How to represent the type (e.g., 2D) of texture being sampled?
class TextureAndSampler {
public:
    constexpr TextureAndSampler(const char* name) : fName(name) {}

    const char* name() const { return fName; }

private:
    const char* fName;
};

enum class SnippetRequirementFlags : uint32_t {
    kNone = 0x0,
    kLocalCoords = 0x1,
    kPriorStageOutput = 0x2,  // AKA the "input" color, or the "src" argument for a blender
    kBlenderDstColor = 0x4,  // The "dst" argument for a blender
    kSurfaceColor = 0x8,
};
SK_MAKE_BITMASK_OPS(SnippetRequirementFlags)

class ShaderInfo;
class ShaderNode;

// ShaderSnippets define the "ABI" of a SkSL module function and its required uniform data, as
// well as functions for generating the invoking SkSL. Snippets are composed into an effect tree
// using ShaderNodes.
struct ShaderSnippet {
    using GeneratePreambleForSnippetFn = std::string (*)(const ShaderInfo& shaderInfo,
                                                         const ShaderNode*);
    struct Args {
        std::string fPriorStageOutput;
        std::string fBlenderDstColor;
        std::string fFragCoord;
    };
    using GenerateExpressionForSnippetFn = std::string (*)(const ShaderInfo& shaderInfo,
                                                           const ShaderNode*,
                                                           const Args& args);

    ShaderSnippet() = default;

    ShaderSnippet(const char* name,
                  SkSpan<const Uniform> uniforms,
                  SkEnumBitMask<SnippetRequirementFlags> snippetRequirementFlags,
                  SkSpan<const TextureAndSampler> texturesAndSamplers,
                  const char* functionName,
                  GenerateExpressionForSnippetFn expressionGenerator,
                  GeneratePreambleForSnippetFn preambleGenerator,
                  int numChildren)
        : fName(name)
        , fUniforms(uniforms)
        , fSnippetRequirementFlags(snippetRequirementFlags)
        , fTexturesAndSamplers(texturesAndSamplers)
        , fStaticFunctionName(functionName)
        , fExpressionGenerator(expressionGenerator)
        , fPreambleGenerator(preambleGenerator)
        , fNumChildren(numChildren) {}

    bool needsLocalCoords() const {
        return SkToBool(fSnippetRequirementFlags & SnippetRequirementFlags::kLocalCoords);
    }
    bool needsPriorStageOutput() const {
        return SkToBool(fSnippetRequirementFlags & SnippetRequirementFlags::kPriorStageOutput);
    }
    bool needsBlenderDstColor() const {
        return SkToBool(fSnippetRequirementFlags & SnippetRequirementFlags::kBlenderDstColor);
    }

    const char* fName = nullptr;
    SkSpan<const Uniform> fUniforms;
    SkEnumBitMask<SnippetRequirementFlags> fSnippetRequirementFlags{SnippetRequirementFlags::kNone};
    SkSpan<const TextureAndSampler> fTexturesAndSamplers;
    const char* fStaticFunctionName = nullptr;
    GenerateExpressionForSnippetFn fExpressionGenerator = nullptr;
    GeneratePreambleForSnippetFn fPreambleGenerator = nullptr;
    int fNumChildren = 0;
};

// ShaderNodes organize snippets into an effect tree, and provide random access to the dynamically
// bound child snippets. Each node has a fixed number of children defined by its code ID
// (either a BuiltInCodeSnippetID or a runtime effect's assigned ID). All children are non-null.
// A ShaderNode tree represents a decompressed PaintParamsKey.
class ShaderNode {
public:
    // ShaderNodes should be created in conjunction with an SkArenaAlloc that owns all nodes.
    ShaderNode(const ShaderSnippet* snippet,
               SkSpan<const ShaderNode*> children,
               int codeID,
               int keyIndex)
            : fEntry(snippet)
            , fChildren(children)
            , fCodeID(codeID)
            , fKeyIndex(keyIndex)
            , fRequiredFlags(snippet->fSnippetRequirementFlags) {
        SkASSERT(children.size() == (size_t) fEntry->fNumChildren);
        // TODO: RuntimeEffects can actually mask off requirements if they invoke a child with
        // explicit arguments.
        for (const ShaderNode* child : children) {
            fRequiredFlags |= child->requiredFlags();
        }
    }

    int32_t codeSnippetId() const { return fCodeID; }
    int32_t keyIndex() const { return fKeyIndex; }
    const ShaderSnippet* entry() const { return fEntry; }

    SkEnumBitMask<SnippetRequirementFlags> requiredFlags() const { return fRequiredFlags; }

    int numChildren() const { return fEntry->fNumChildren; }
    SkSpan<const ShaderNode*> children() const { return fChildren; }
    const ShaderNode* child(int childIndex) const { return fChildren[childIndex]; }

private:
    const ShaderSnippet* fEntry; // Owned by the ShaderCodeDictionary
    SkSpan<const ShaderNode*> fChildren; // Owned by the ShaderInfo's arena

    int32_t fCodeID;
    int32_t fKeyIndex; // index back to PaintParamsKey, unique across nodes within a ShaderInfo

    SkEnumBitMask<SnippetRequirementFlags> fRequiredFlags;
};

// ShaderInfo holds all root ShaderNodes defined for a PaintParams as well as the extracted fixed
// function blending parameters and other aggregate requirements for the effect trees that have
// been linked into a single fragment program (sans any RenderStep fragment work and fixed SkSL
// logic required for all rendering in Graphite).
class ShaderInfo {
public:
    ShaderInfo(UniquePaintParamsID id,
               const ShaderCodeDictionary* dict,
               const RuntimeEffectDictionary* rteDict,
               const char* ssboIndex);

    bool needsLocalCoords() const {
        return SkToBool(fSnippetRequirementFlags & SnippetRequirementFlags::kLocalCoords);
    }
    bool needsSurfaceColor() const {
        return SkToBool(fSnippetRequirementFlags & SnippetRequirementFlags::kSurfaceColor);
    }
    const RuntimeEffectDictionary* runtimeEffectDictionary() const {
        return fRuntimeEffectDictionary;
    }
    const char* ssboIndex() const { return fSsboIndex; }

    const skgpu::BlendInfo& blendInfo() const { return fBlendInfo; }

    std::string toSkSL(const Caps* caps,
                       const RenderStep* step,
                       bool useStorageBuffers,
                       int* numTexturesAndSamplersUsed,
                       int* numPaintUniforms,
                       int* renderStepUniformTotalBytes,
                       int* paintUniformsTotalBytes,
                       Swizzle writeSwizzle);

private:
    // All shader nodes and arrays of children pointers are held in this arena
    SkArenaAlloc fShaderNodeAlloc{256};

    const RuntimeEffectDictionary* fRuntimeEffectDictionary;
    const char* fSsboIndex;

    // De-compressed shader tree from a PaintParamsKey with accumulated blend info and requirements.
    // The blendInfo doesn't contribute to the program's SkSL but contains the fixed-function state
    // required to function correctly, which the program's caller is responsible for configuring.
    // TODO: There should really only be one root node representing the final blend, which has a
    // child defining how the src color is calculated.
    SkSpan<const ShaderNode*> fRootNodes;
    SkBlendMode fBlendMode = SkBlendMode::kClear;
    skgpu::BlendInfo fBlendInfo;
    SkEnumBitMask<SnippetRequirementFlags> fSnippetRequirementFlags;
};

// ShaderCodeDictionary is a thread-safe dictionary of ShaderSnippets to code IDs for use with
// creating PaintParamKeys, as well as assigning unique IDs to each encountered PaintParamKey.
// It defines ShaderSnippets for every BuiltInCodeSnippetID and maintains records for IDs per
// SkRuntimeEffect, including de-duplicating equivalent SkRuntimeEffect objects.
class ShaderCodeDictionary {
public:
    ShaderCodeDictionary();

    UniquePaintParamsID findOrCreate(PaintParamsKeyBuilder*) SK_EXCLUDES(fSpinLock);

    PaintParamsKey lookup(UniquePaintParamsID) const SK_EXCLUDES(fSpinLock);

    SkSpan<const Uniform> getUniforms(BuiltInCodeSnippetID) const;
    SkEnumBitMask<SnippetRequirementFlags> getSnippetRequirementFlags(
            BuiltInCodeSnippetID id) const {
        return fBuiltInCodeSnippets[(int) id].fSnippetRequirementFlags;
    }

    bool isValidID(int snippetID) const;

    // This method can return nullptr
    const ShaderSnippet* getEntry(int codeSnippetID) const;
    const ShaderSnippet* getEntry(BuiltInCodeSnippetID codeSnippetID) const {
        return this->getEntry(SkTo<int>(codeSnippetID));
    }

    int findOrCreateRuntimeEffectSnippet(const SkRuntimeEffect* effect);

    // TODO: Remove or make testing-only
    int addUserDefinedSnippet(const char* name);

private:
    // TODO: this is still experimental but, most likely, it will need to be made thread-safe
    // It returns the code snippet ID to use to identify the supplied user-defined code.
    // TODO: Rename to addRuntimeEffectSnippet().
    int addUserDefinedSnippet(
        const char* name,
        SkSpan<const Uniform> uniforms,
        SkEnumBitMask<SnippetRequirementFlags> snippetRequirementFlags,
        SkSpan<const TextureAndSampler> texturesAndSamplers,
        const char* functionName,
        ShaderSnippet::GenerateExpressionForSnippetFn expressionGenerator,
        ShaderSnippet::GeneratePreambleForSnippetFn preambleGenerator,
        int numChildren);

    const char* addTextToArena(std::string_view text);

    SkSpan<const Uniform> convertUniforms(const SkRuntimeEffect* effect);

    std::array<ShaderSnippet, kBuiltInCodeSnippetIDCount> fBuiltInCodeSnippets;

    // The value returned from 'getEntry' must be stable so, hold the user-defined code snippet
    // entries as pointers.
    skia_private::TArray<std::unique_ptr<ShaderSnippet>> fUserDefinedCodeSnippets;

    // TODO: can we do something better given this should have write-seldom/read-often behavior?
    mutable SkSpinlock fSpinLock;

    using PaintIDMap = skia_private::THashMap<PaintParamsKey,
                                              UniquePaintParamsID,
                                              PaintParamsKey::Hash>;

    PaintIDMap fPaintKeyToID SK_GUARDED_BY(fSpinLock);
    skia_private::TArray<PaintParamsKey> fIDToPaintKey SK_GUARDED_BY(fSpinLock);

    SK_BEGIN_REQUIRE_DENSE
    struct RuntimeEffectKey {
        uint32_t fHash;
        uint32_t fUniformSize;

        bool operator==(RuntimeEffectKey rhs) const {
            return fHash == rhs.fHash && fUniformSize == rhs.fUniformSize;
        }
    };
    SK_END_REQUIRE_DENSE

    // A map from RuntimeEffectKeys (hash plus uniforms) to code-snippet IDs. RuntimeEffectKeys
    // don't track the lifetime of a runtime effect at all; they live forever, and a newly-
    // instantiated runtime effect with the same program as a previously-discarded effect will reuse
    // an existing ID. Entries in the runtime-effect map are never removed; they only disappear when
    // the context is discarded, which takes the ShaderCodeDictionary along with it. However, they
    // are extremely small (< 20 bytes) so the memory footprint should be unnoticeable.
    using RuntimeEffectMap = skia_private::THashMap<RuntimeEffectKey, int32_t>;
    RuntimeEffectMap fRuntimeEffectMap SK_GUARDED_BY(fSpinLock);

    // This arena holds:
    //   - the backing data for PaintParamsKeys in `fPaintKeyToID` and `fIDToPaintKey`
    //   - Uniform data created by `findOrCreateRuntimeEffectSnippet`
    // and in all cases is guarded by `fSpinLock`
    SkArenaAlloc fArena{256};
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ShaderCodeDictionary_DEFINED
