//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/Cache.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/CallDAG.h"
#include "compiler/translator/ForLoopUnroll.h"
#include "compiler/translator/Initialize.h"
#include "compiler/translator/InitializeParseContext.h"
#include "compiler/translator/InitializeVariables.h"
#include "compiler/translator/ParseContext.h"
#include "compiler/translator/PruneEmptyDeclarations.h"
#include "compiler/translator/RegenerateStructNames.h"
#include "compiler/translator/RemovePow.h"
#include "compiler/translator/RenameFunction.h"
#include "compiler/translator/ScalarizeVecAndMatConstructorArgs.h"
#include "compiler/translator/UnfoldShortCircuitAST.h"
#include "compiler/translator/ValidateLimitations.h"
#include "compiler/translator/ValidateOutputs.h"
#include "compiler/translator/VariablePacker.h"
#include "compiler/translator/depgraph/DependencyGraph.h"
#include "compiler/translator/depgraph/DependencyGraphOutput.h"
#include "compiler/translator/timing/RestrictFragmentShaderTiming.h"
#include "compiler/translator/timing/RestrictVertexShaderTiming.h"
#include "third_party/compiler/ArrayBoundsClamper.h"
#include "angle_gl.h"
#include "common/utilities.h"

bool IsWebGLBasedSpec(ShShaderSpec spec)
{
    return (spec == SH_WEBGL_SPEC ||
            spec == SH_CSS_SHADERS_SPEC ||
            spec == SH_WEBGL2_SPEC);
}

bool IsGLSL130OrNewer(ShShaderOutput output)
{
    return (output == SH_GLSL_130_OUTPUT ||
            output == SH_GLSL_140_OUTPUT ||
            output == SH_GLSL_150_CORE_OUTPUT ||
            output == SH_GLSL_330_CORE_OUTPUT ||
            output == SH_GLSL_400_CORE_OUTPUT ||
            output == SH_GLSL_410_CORE_OUTPUT ||
            output == SH_GLSL_420_CORE_OUTPUT ||
            output == SH_GLSL_430_CORE_OUTPUT ||
            output == SH_GLSL_440_CORE_OUTPUT ||
            output == SH_GLSL_450_CORE_OUTPUT);
}

size_t GetGlobalMaxTokenSize(ShShaderSpec spec)
{
    // WebGL defines a max token legnth of 256, while ES2 leaves max token
    // size undefined. ES3 defines a max size of 1024 characters.
    switch (spec)
    {
      case SH_WEBGL_SPEC:
      case SH_CSS_SHADERS_SPEC:
        return 256;
      default:
        return 1024;
    }
}

namespace {

class TScopedPoolAllocator
{
  public:
    TScopedPoolAllocator(TPoolAllocator* allocator) : mAllocator(allocator)
    {
        mAllocator->push();
        SetGlobalPoolAllocator(mAllocator);
    }
    ~TScopedPoolAllocator()
    {
        SetGlobalPoolAllocator(NULL);
        mAllocator->pop();
    }

  private:
    TPoolAllocator* mAllocator;
};

class TScopedSymbolTableLevel
{
  public:
    TScopedSymbolTableLevel(TSymbolTable* table) : mTable(table)
    {
        ASSERT(mTable->atBuiltInLevel());
        mTable->push();
    }
    ~TScopedSymbolTableLevel()
    {
        while (!mTable->atBuiltInLevel())
            mTable->pop();
    }

  private:
    TSymbolTable* mTable;
};

int MapSpecToShaderVersion(ShShaderSpec spec)
{
    switch (spec)
    {
      case SH_GLES2_SPEC:
      case SH_WEBGL_SPEC:
      case SH_CSS_SHADERS_SPEC:
        return 100;
      case SH_GLES3_SPEC:
      case SH_WEBGL2_SPEC:
        return 300;
      default:
        UNREACHABLE();
        return 0;
    }
}

}  // namespace

TShHandleBase::TShHandleBase()
{
    allocator.push();
    SetGlobalPoolAllocator(&allocator);
}

TShHandleBase::~TShHandleBase()
{
    SetGlobalPoolAllocator(NULL);
    allocator.popAll();
}

TCompiler::TCompiler(sh::GLenum type, ShShaderSpec spec, ShShaderOutput output)
    : shaderType(type),
      shaderSpec(spec),
      outputType(output),
      maxUniformVectors(0),
      maxExpressionComplexity(0),
      maxCallStackDepth(0),
      fragmentPrecisionHigh(false),
      clampingStrategy(SH_CLAMP_WITH_CLAMP_INTRINSIC),
      builtInFunctionEmulator(),
      mSourcePath(NULL)
{
}

TCompiler::~TCompiler()
{
}

bool TCompiler::shouldRunLoopAndIndexingValidation(int compileOptions) const
{
    // If compiling an ESSL 1.00 shader for WebGL, or if its been requested through the API,
    // validate loop and indexing as well (to verify that the shader only uses minimal functionality
    // of ESSL 1.00 as in Appendix A of the spec).
    return (IsWebGLBasedSpec(shaderSpec) && shaderVersion == 100) ||
           (compileOptions & SH_VALIDATE_LOOP_INDEXING);
}

bool TCompiler::Init(const ShBuiltInResources& resources)
{
    shaderVersion = 100;
    maxUniformVectors = (shaderType == GL_VERTEX_SHADER) ?
        resources.MaxVertexUniformVectors :
        resources.MaxFragmentUniformVectors;
    maxExpressionComplexity = resources.MaxExpressionComplexity;
    maxCallStackDepth = resources.MaxCallStackDepth;

    SetGlobalPoolAllocator(&allocator);

    // Generate built-in symbol table.
    if (!InitBuiltInSymbolTable(resources))
        return false;
    InitExtensionBehavior(resources, extensionBehavior);
    fragmentPrecisionHigh = resources.FragmentPrecisionHigh == 1;

    arrayBoundsClamper.SetClampingStrategy(resources.ArrayIndexClampingStrategy);
    clampingStrategy = resources.ArrayIndexClampingStrategy;

    hashFunction = resources.HashFunction;

    return true;
}

TIntermNode *TCompiler::compileTreeForTesting(const char* const shaderStrings[],
    size_t numStrings, int compileOptions)
{
    return compileTreeImpl(shaderStrings, numStrings, compileOptions);
}

TIntermNode *TCompiler::compileTreeImpl(const char *const shaderStrings[],
                                        size_t numStrings,
                                        const int compileOptions)
{
    clearResults();

    ASSERT(numStrings > 0);
    ASSERT(GetGlobalPoolAllocator());

    // Reset the extension behavior for each compilation unit.
    ResetExtensionBehavior(extensionBehavior);

    // First string is path of source file if flag is set. The actual source follows.
    size_t firstSource = 0;
    if (compileOptions & SH_SOURCE_PATH)
    {
        mSourcePath = shaderStrings[0];
        ++firstSource;
    }

    bool debugShaderPrecision = getResources().WEBGL_debug_shader_precision == 1;
    TIntermediate intermediate(infoSink);
    TParseContext parseContext(symbolTable, extensionBehavior, intermediate,
                               shaderType, shaderSpec, compileOptions, true,
                               infoSink, debugShaderPrecision);

    parseContext.setFragmentPrecisionHigh(fragmentPrecisionHigh);
    SetGlobalParseContext(&parseContext);

    // We preserve symbols at the built-in level from compile-to-compile.
    // Start pushing the user-defined symbols at global level.
    TScopedSymbolTableLevel scopedSymbolLevel(&symbolTable);

    // Parse shader.
    bool success =
        (PaParseStrings(numStrings - firstSource, &shaderStrings[firstSource], nullptr, &parseContext) == 0) &&
        (parseContext.getTreeRoot() != nullptr);

    shaderVersion = parseContext.getShaderVersion();
    if (success && MapSpecToShaderVersion(shaderSpec) < shaderVersion)
    {
        infoSink.info.prefix(EPrefixError);
        infoSink.info << "unsupported shader version";
        success = false;
    }

    TIntermNode *root = nullptr;

    if (success)
    {
        mPragma = parseContext.pragma();
        if (mPragma.stdgl.invariantAll)
        {
            symbolTable.setGlobalInvariant();
        }

        root = parseContext.getTreeRoot();
        root = intermediate.postProcess(root);

        // Disallow expressions deemed too complex.
        if (success && (compileOptions & SH_LIMIT_EXPRESSION_COMPLEXITY))
            success = limitExpressionComplexity(root);

        // Create the function DAG and check there is no recursion
        if (success)
            success = initCallDag(root);

        if (success && (compileOptions & SH_LIMIT_CALL_STACK_DEPTH))
            success = checkCallDepth();

        // Checks which functions are used and if "main" exists
        if (success)
        {
            functionMetadata.clear();
            functionMetadata.resize(mCallDag.size());
            success = tagUsedFunctions();
        }

        if (success && !(compileOptions & SH_DONT_PRUNE_UNUSED_FUNCTIONS))
            success = pruneUnusedFunctions(root);

        // Prune empty declarations to work around driver bugs and to keep declaration output simple.
        if (success)
            PruneEmptyDeclarations(root);

        if (success && shaderVersion == 300 && shaderType == GL_FRAGMENT_SHADER)
            success = validateOutputs(root);

        if (success && shouldRunLoopAndIndexingValidation(compileOptions))
            success = validateLimitations(root);

        if (success && (compileOptions & SH_TIMING_RESTRICTIONS))
            success = enforceTimingRestrictions(root, (compileOptions & SH_DEPENDENCY_GRAPH) != 0);

        if (success && shaderSpec == SH_CSS_SHADERS_SPEC)
            rewriteCSSShader(root);

        // Unroll for-loop markup needs to happen after validateLimitations pass.
        if (success && (compileOptions & SH_UNROLL_FOR_LOOP_WITH_INTEGER_INDEX))
        {
            ForLoopUnrollMarker marker(ForLoopUnrollMarker::kIntegerIndex);
            root->traverse(&marker);
        }
        if (success && (compileOptions & SH_UNROLL_FOR_LOOP_WITH_SAMPLER_ARRAY_INDEX))
        {
            ForLoopUnrollMarker marker(ForLoopUnrollMarker::kSamplerArrayIndex);
            root->traverse(&marker);
            if (marker.samplerArrayIndexIsFloatLoopIndex())
            {
                infoSink.info.prefix(EPrefixError);
                infoSink.info << "sampler array index is float loop index";
                success = false;
            }
        }

        // Built-in function emulation needs to happen after validateLimitations pass.
        if (success)
        {
            initBuiltInFunctionEmulator(&builtInFunctionEmulator, compileOptions);
            builtInFunctionEmulator.MarkBuiltInFunctionsForEmulation(root);
        }

        // Clamping uniform array bounds needs to happen after validateLimitations pass.
        if (success && (compileOptions & SH_CLAMP_INDIRECT_ARRAY_BOUNDS))
            arrayBoundsClamper.MarkIndirectArrayBoundsForClamping(root);

        if (success && shaderType == GL_VERTEX_SHADER && (compileOptions & SH_INIT_GL_POSITION))
            initializeGLPosition(root);

        if (success && (compileOptions & SH_UNFOLD_SHORT_CIRCUIT))
        {
            UnfoldShortCircuitAST unfoldShortCircuit;
            root->traverse(&unfoldShortCircuit);
            unfoldShortCircuit.updateTree();
        }

        if (success && (compileOptions & SH_REMOVE_POW_WITH_CONSTANT_EXPONENT))
        {
            RemovePow(root);
        }

        if (success && shouldCollectVariables(compileOptions))
        {
            collectVariables(root);
            if (compileOptions & SH_ENFORCE_PACKING_RESTRICTIONS)
            {
                success = enforcePackingRestrictions();
                if (!success)
                {
                    infoSink.info.prefix(EPrefixError);
                    infoSink.info << "too many uniforms";
                }
            }
            if (success && shaderType == GL_VERTEX_SHADER &&
                (compileOptions & SH_INIT_VARYINGS_WITHOUT_STATIC_USE))
                initializeVaryingsWithoutStaticUse(root);
        }

        if (success && (compileOptions & SH_SCALARIZE_VEC_AND_MAT_CONSTRUCTOR_ARGS))
        {
            ScalarizeVecAndMatConstructorArgs scalarizer(
                shaderType, fragmentPrecisionHigh);
            root->traverse(&scalarizer);
        }

        if (success && (compileOptions & SH_REGENERATE_STRUCT_NAMES))
        {
            RegenerateStructNames gen(symbolTable, shaderVersion);
            root->traverse(&gen);
        }
    }

    SetGlobalParseContext(NULL);
    if (success)
        return root;

    return NULL;
}

bool TCompiler::compile(const char* const shaderStrings[],
    size_t numStrings, int compileOptions)
{
    if (numStrings == 0)
        return true;

    TScopedPoolAllocator scopedAlloc(&allocator);
    TIntermNode *root = compileTreeImpl(shaderStrings, numStrings, compileOptions);

    if (root)
    {
        if (compileOptions & SH_INTERMEDIATE_TREE)
            TIntermediate::outputTree(root, infoSink.info);

        if (compileOptions & SH_OBJECT_CODE)
            translate(root, compileOptions);

        // The IntermNode tree doesn't need to be deleted here, since the
        // memory will be freed in a big chunk by the PoolAllocator.
        return true;
    }
    return false;
}

bool TCompiler::InitBuiltInSymbolTable(const ShBuiltInResources &resources)
{
    compileResources = resources;
    setResourceString();

    assert(symbolTable.isEmpty());
    symbolTable.push();   // COMMON_BUILTINS
    symbolTable.push();   // ESSL1_BUILTINS
    symbolTable.push();   // ESSL3_BUILTINS

    TPublicType integer;
    integer.type = EbtInt;
    integer.primarySize = 1;
    integer.secondarySize = 1;
    integer.array = false;

    TPublicType floatingPoint;
    floatingPoint.type = EbtFloat;
    floatingPoint.primarySize = 1;
    floatingPoint.secondarySize = 1;
    floatingPoint.array = false;

    TPublicType sampler;
    sampler.primarySize = 1;
    sampler.secondarySize = 1;
    sampler.array = false;

    switch(shaderType)
    {
      case GL_FRAGMENT_SHADER:
        symbolTable.setDefaultPrecision(integer, EbpMedium);
        break;
      case GL_VERTEX_SHADER:
        symbolTable.setDefaultPrecision(integer, EbpHigh);
        symbolTable.setDefaultPrecision(floatingPoint, EbpHigh);
        break;
      default:
        assert(false && "Language not supported");
    }
    // We set defaults for all the sampler types, even those that are
    // only available if an extension exists.
    for (int samplerType = EbtGuardSamplerBegin + 1;
         samplerType < EbtGuardSamplerEnd; ++samplerType)
    {
        sampler.type = static_cast<TBasicType>(samplerType);
        symbolTable.setDefaultPrecision(sampler, EbpLow);
    }

    InsertBuiltInFunctions(shaderType, shaderSpec, resources, symbolTable);

    IdentifyBuiltIns(shaderType, shaderSpec, resources, symbolTable);

    return true;
}

void TCompiler::setResourceString()
{
    std::ostringstream strstream;
    strstream << ":MaxVertexAttribs:" << compileResources.MaxVertexAttribs
              << ":MaxVertexUniformVectors:" << compileResources.MaxVertexUniformVectors
              << ":MaxVaryingVectors:" << compileResources.MaxVaryingVectors
              << ":MaxVertexTextureImageUnits:" << compileResources.MaxVertexTextureImageUnits
              << ":MaxCombinedTextureImageUnits:" << compileResources.MaxCombinedTextureImageUnits
              << ":MaxTextureImageUnits:" << compileResources.MaxTextureImageUnits
              << ":MaxFragmentUniformVectors:" << compileResources.MaxFragmentUniformVectors
              << ":MaxDrawBuffers:" << compileResources.MaxDrawBuffers
              << ":OES_standard_derivatives:" << compileResources.OES_standard_derivatives
              << ":OES_EGL_image_external:" << compileResources.OES_EGL_image_external
              << ":ARB_texture_rectangle:" << compileResources.ARB_texture_rectangle
              << ":EXT_draw_buffers:" << compileResources.EXT_draw_buffers
              << ":FragmentPrecisionHigh:" << compileResources.FragmentPrecisionHigh
              << ":MaxExpressionComplexity:" << compileResources.MaxExpressionComplexity
              << ":MaxCallStackDepth:" << compileResources.MaxCallStackDepth
              << ":EXT_blend_func_extended:" << compileResources.EXT_blend_func_extended
              << ":EXT_frag_depth:" << compileResources.EXT_frag_depth
              << ":EXT_shader_texture_lod:" << compileResources.EXT_shader_texture_lod
              << ":EXT_shader_framebuffer_fetch:" << compileResources.EXT_shader_framebuffer_fetch
              << ":NV_shader_framebuffer_fetch:" << compileResources.NV_shader_framebuffer_fetch
              << ":ARM_shader_framebuffer_fetch:" << compileResources.ARM_shader_framebuffer_fetch
              << ":MaxVertexOutputVectors:" << compileResources.MaxVertexOutputVectors
              << ":MaxFragmentInputVectors:" << compileResources.MaxFragmentInputVectors
              << ":MinProgramTexelOffset:" << compileResources.MinProgramTexelOffset
              << ":MaxProgramTexelOffset:" << compileResources.MaxProgramTexelOffset
              << ":MaxDualSourceDrawBuffers:" << compileResources.MaxDualSourceDrawBuffers
              << ":NV_draw_buffers:" << compileResources.NV_draw_buffers
              << ":WEBGL_debug_shader_precision:" << compileResources.WEBGL_debug_shader_precision;

    builtInResourcesString = strstream.str();
}

void TCompiler::clearResults()
{
    arrayBoundsClamper.Cleanup();
    infoSink.info.erase();
    infoSink.obj.erase();
    infoSink.debug.erase();

    attributes.clear();
    outputVariables.clear();
    uniforms.clear();
    expandedUniforms.clear();
    varyings.clear();
    interfaceBlocks.clear();

    builtInFunctionEmulator.Cleanup();

    nameMap.clear();

    mSourcePath = NULL;
}

bool TCompiler::initCallDag(TIntermNode *root)
{
    mCallDag.clear();

    switch (mCallDag.init(root, &infoSink.info))
    {
      case CallDAG::INITDAG_SUCCESS:
        return true;
      case CallDAG::INITDAG_RECURSION:
        infoSink.info.prefix(EPrefixError);
        infoSink.info << "Function recursion detected";
        return false;
      case CallDAG::INITDAG_UNDEFINED:
        infoSink.info.prefix(EPrefixError);
        infoSink.info << "Unimplemented function detected";
        return false;
    }

    UNREACHABLE();
    return true;
}

bool TCompiler::checkCallDepth()
{
    std::vector<int> depths(mCallDag.size());

    for (size_t i = 0; i < mCallDag.size(); i++)
    {
        int depth = 0;
        auto &record = mCallDag.getRecordFromIndex(i);

        for (auto &calleeIndex : record.callees)
        {
            depth = std::max(depth, depths[calleeIndex] + 1);
        }

        depths[i] = depth;

        if (depth >= maxCallStackDepth)
        {
            // Trace back the function chain to have a meaningful info log.
            infoSink.info.prefix(EPrefixError);
            infoSink.info << "Call stack too deep (larger than " << maxCallStackDepth
                          << ") with the following call chain: " << record.name;

            int currentFunction = static_cast<int>(i);
            int currentDepth = depth;

            while (currentFunction != -1)
            {
                infoSink.info << " -> " << mCallDag.getRecordFromIndex(currentFunction).name;

                int nextFunction = -1;
                for (auto& calleeIndex : mCallDag.getRecordFromIndex(currentFunction).callees)
                {
                    if (depths[calleeIndex] == currentDepth - 1)
                    {
                        currentDepth--;
                        nextFunction = calleeIndex;
                    }
                }

                currentFunction = nextFunction;
            }

            return false;
        }
    }

    return true;
}

bool TCompiler::tagUsedFunctions()
{
    // Search from main, starting from the end of the DAG as it usually is the root.
    for (size_t i = mCallDag.size(); i-- > 0;)
    {
        if (mCallDag.getRecordFromIndex(i).name == "main(")
        {
            internalTagUsedFunction(i);
            return true;
        }
    }

    infoSink.info.prefix(EPrefixError);
    infoSink.info << "Missing main()";
    return false;
}

void TCompiler::internalTagUsedFunction(size_t index)
{
    if (functionMetadata[index].used)
    {
        return;
    }

    functionMetadata[index].used = true;

    for (int calleeIndex : mCallDag.getRecordFromIndex(index).callees)
    {
        internalTagUsedFunction(calleeIndex);
    }
}

// A predicate for the stl that returns if a top-level node is unused
class TCompiler::UnusedPredicate
{
  public:
    UnusedPredicate(const CallDAG *callDag, const std::vector<FunctionMetadata> *metadatas)
        : mCallDag(callDag),
          mMetadatas(metadatas)
    {
    }

    bool operator ()(TIntermNode *node)
    {
        const TIntermAggregate *asAggregate = node->getAsAggregate();

        if (asAggregate == nullptr)
        {
            return false;
        }

        if (!(asAggregate->getOp() == EOpFunction || asAggregate->getOp() == EOpPrototype))
        {
            return false;
        }

        size_t callDagIndex = mCallDag->findIndex(asAggregate);
        if (callDagIndex == CallDAG::InvalidIndex)
        {
            // This happens only for unimplemented prototypes which are thus unused
            ASSERT(asAggregate->getOp() == EOpPrototype);
            return true;
        }

        ASSERT(callDagIndex < mMetadatas->size());
        return !(*mMetadatas)[callDagIndex].used;
    }

  private:
    const CallDAG *mCallDag;
    const std::vector<FunctionMetadata> *mMetadatas;
};

bool TCompiler::pruneUnusedFunctions(TIntermNode *root)
{
    TIntermAggregate *rootNode = root->getAsAggregate();
    ASSERT(rootNode != nullptr);

    UnusedPredicate isUnused(&mCallDag, &functionMetadata);
    TIntermSequence *sequence = rootNode->getSequence();

    if (!sequence->empty())
    {
        sequence->erase(std::remove_if(sequence->begin(), sequence->end(), isUnused), sequence->end());
    }

    return true;
}

bool TCompiler::validateOutputs(TIntermNode* root)
{
    ValidateOutputs validateOutputs(getExtensionBehavior(), compileResources.MaxDrawBuffers);
    root->traverse(&validateOutputs);
    return (validateOutputs.validateAndCountErrors(infoSink.info) == 0);
}

void TCompiler::rewriteCSSShader(TIntermNode* root)
{
    RenameFunction renamer("main(", "css_main(");
    root->traverse(&renamer);
}

bool TCompiler::validateLimitations(TIntermNode* root)
{
    ValidateLimitations validate(shaderType, infoSink.info);
    root->traverse(&validate);
    return validate.numErrors() == 0;
}

bool TCompiler::enforceTimingRestrictions(TIntermNode* root, bool outputGraph)
{
    if (shaderSpec != SH_WEBGL_SPEC)
    {
        infoSink.info << "Timing restrictions must be enforced under the WebGL spec.";
        return false;
    }

    if (shaderType == GL_FRAGMENT_SHADER)
    {
        TDependencyGraph graph(root);

        // Output any errors first.
        bool success = enforceFragmentShaderTimingRestrictions(graph);

        // Then, output the dependency graph.
        if (outputGraph)
        {
            TDependencyGraphOutput output(infoSink.info);
            output.outputAllSpanningTrees(graph);
        }

        return success;
    }
    else
    {
        return enforceVertexShaderTimingRestrictions(root);
    }
}

bool TCompiler::limitExpressionComplexity(TIntermNode* root)
{
    TMaxDepthTraverser traverser(maxExpressionComplexity+1);
    root->traverse(&traverser);

    if (traverser.getMaxDepth() > maxExpressionComplexity)
    {
        infoSink.info << "Expression too complex.";
        return false;
    }

    TDependencyGraph graph(root);

    for (TFunctionCallVector::const_iterator iter = graph.beginUserDefinedFunctionCalls();
         iter != graph.endUserDefinedFunctionCalls();
         ++iter)
    {
        TGraphFunctionCall* samplerSymbol = *iter;
        TDependencyGraphTraverser graphTraverser;
        samplerSymbol->traverse(&graphTraverser);
    }

    return true;
}

bool TCompiler::enforceFragmentShaderTimingRestrictions(const TDependencyGraph& graph)
{
    RestrictFragmentShaderTiming restrictor(infoSink.info);
    restrictor.enforceRestrictions(graph);
    return restrictor.numErrors() == 0;
}

bool TCompiler::enforceVertexShaderTimingRestrictions(TIntermNode* root)
{
    RestrictVertexShaderTiming restrictor(infoSink.info);
    restrictor.enforceRestrictions(root);
    return restrictor.numErrors() == 0;
}

void TCompiler::collectVariables(TIntermNode* root)
{
    sh::CollectVariables collect(&attributes,
                                 &outputVariables,
                                 &uniforms,
                                 &varyings,
                                 &interfaceBlocks,
                                 hashFunction,
                                 symbolTable);
    root->traverse(&collect);

    // This is for enforcePackingRestriction().
    sh::ExpandUniforms(uniforms, &expandedUniforms);
}

bool TCompiler::enforcePackingRestrictions()
{
    VariablePacker packer;
    return packer.CheckVariablesWithinPackingLimits(maxUniformVectors, expandedUniforms);
}

void TCompiler::initializeGLPosition(TIntermNode* root)
{
    InitializeVariables::InitVariableInfoList variables;
    InitializeVariables::InitVariableInfo var(
        "gl_Position", TType(EbtFloat, EbpUndefined, EvqPosition, 4));
    variables.push_back(var);
    InitializeVariables initializer(variables);
    root->traverse(&initializer);
}

void TCompiler::initializeVaryingsWithoutStaticUse(TIntermNode* root)
{
    InitializeVariables::InitVariableInfoList variables;
    for (size_t ii = 0; ii < varyings.size(); ++ii)
    {
        const sh::Varying& varying = varyings[ii];
        if (varying.staticUse)
            continue;
        unsigned char primarySize = static_cast<unsigned char>(gl::VariableColumnCount(varying.type));
        unsigned char secondarySize = static_cast<unsigned char>(gl::VariableRowCount(varying.type));
        TType type(EbtFloat, EbpUndefined, EvqVaryingOut, primarySize, secondarySize, varying.isArray());
        TString name = varying.name.c_str();
        if (varying.isArray())
        {
            type.setArraySize(varying.arraySize);
            name = name.substr(0, name.find_first_of('['));
        }

        InitializeVariables::InitVariableInfo var(name, type);
        variables.push_back(var);
    }
    InitializeVariables initializer(variables);
    root->traverse(&initializer);
}

const TExtensionBehavior& TCompiler::getExtensionBehavior() const
{
    return extensionBehavior;
}

const char *TCompiler::getSourcePath() const
{
    return mSourcePath;
}

const ShBuiltInResources& TCompiler::getResources() const
{
    return compileResources;
}

const ArrayBoundsClamper& TCompiler::getArrayBoundsClamper() const
{
    return arrayBoundsClamper;
}

ShArrayIndexClampingStrategy TCompiler::getArrayIndexClampingStrategy() const
{
    return clampingStrategy;
}

const BuiltInFunctionEmulator& TCompiler::getBuiltInFunctionEmulator() const
{
    return builtInFunctionEmulator;
}

void TCompiler::writePragma()
{
    TInfoSinkBase &sink = infoSink.obj;
    if (mPragma.stdgl.invariantAll)
        sink << "#pragma STDGL invariant(all)\n";
}
