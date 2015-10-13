//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/OutputHLSL.h"

#include <algorithm>
#include <cfloat>
#include <stdio.h>

#include "common/angleutils.h"
#include "common/debug.h"
#include "common/utilities.h"
#include "compiler/translator/BuiltInFunctionEmulator.h"
#include "compiler/translator/BuiltInFunctionEmulatorHLSL.h"
#include "compiler/translator/FlagStd140Structs.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/NodeSearch.h"
#include "compiler/translator/RemoveSwitchFallThrough.h"
#include "compiler/translator/SearchSymbol.h"
#include "compiler/translator/StructureHLSL.h"
#include "compiler/translator/TranslatorHLSL.h"
#include "compiler/translator/UniformHLSL.h"
#include "compiler/translator/UtilsHLSL.h"
#include "compiler/translator/blocklayout.h"
#include "compiler/translator/util.h"

namespace
{

bool IsSequence(TIntermNode *node)
{
    return node->getAsAggregate() != nullptr && node->getAsAggregate()->getOp() == EOpSequence;
}

} // namespace

namespace sh
{

TString OutputHLSL::TextureFunction::name() const
{
    TString name = "gl_texture";

    if (IsSampler2D(sampler))
    {
        name += "2D";
    }
    else if (IsSampler3D(sampler))
    {
        name += "3D";
    }
    else if (IsSamplerCube(sampler))
    {
        name += "Cube";
    }
    else UNREACHABLE();

    if (proj)
    {
        name += "Proj";
    }

    if (offset)
    {
        name += "Offset";
    }

    switch(method)
    {
      case IMPLICIT:                  break;
      case BIAS:                      break;   // Extra parameter makes the signature unique
      case LOD:      name += "Lod";   break;
      case LOD0:     name += "Lod0";  break;
      case LOD0BIAS: name += "Lod0";  break;   // Extra parameter makes the signature unique
      case SIZE:     name += "Size";  break;
      case FETCH:    name += "Fetch"; break;
      case GRAD:     name += "Grad";  break;
      default: UNREACHABLE();
    }

    return name + "(";
}

bool OutputHLSL::TextureFunction::operator<(const TextureFunction &rhs) const
{
    if (sampler < rhs.sampler) return true;
    if (sampler > rhs.sampler) return false;

    if (coords < rhs.coords)   return true;
    if (coords > rhs.coords)   return false;

    if (!proj && rhs.proj)     return true;
    if (proj && !rhs.proj)     return false;

    if (!offset && rhs.offset) return true;
    if (offset && !rhs.offset) return false;

    if (method < rhs.method)   return true;
    if (method > rhs.method)   return false;

    return false;
}

OutputHLSL::OutputHLSL(sh::GLenum shaderType, int shaderVersion,
    const TExtensionBehavior &extensionBehavior,
    const char *sourcePath, ShShaderOutput outputType,
    int numRenderTargets, const std::vector<Uniform> &uniforms,
    int compileOptions)
    : TIntermTraverser(true, true, true),
      mShaderType(shaderType),
      mShaderVersion(shaderVersion),
      mExtensionBehavior(extensionBehavior),
      mSourcePath(sourcePath),
      mOutputType(outputType),
      mCompileOptions(compileOptions),
      mNumRenderTargets(numRenderTargets),
      mCurrentFunctionMetadata(nullptr)
{
    mInsideFunction = false;

    mUsesFragColor = false;
    mUsesFragData = false;
    mUsesDepthRange = false;
    mUsesFragCoord = false;
    mUsesPointCoord = false;
    mUsesFrontFacing = false;
    mUsesPointSize = false;
    mUsesInstanceID = false;
    mUsesFragDepth = false;
    mUsesXor = false;
    mUsesDiscardRewriting = false;
    mUsesNestedBreak = false;
    mRequiresIEEEStrictCompiling = false;

    mUniqueIndex = 0;

    mOutputLod0Function = false;
    mInsideDiscontinuousLoop = false;
    mNestedLoopDepth = 0;

    mExcessiveLoopIndex = NULL;

    mStructureHLSL = new StructureHLSL;
    mUniformHLSL = new UniformHLSL(mStructureHLSL, outputType, uniforms);

    if (mOutputType == SH_HLSL9_OUTPUT)
    {
        // Fragment shaders need dx_DepthRange, dx_ViewCoords and dx_DepthFront.
        // Vertex shaders need a slightly different set: dx_DepthRange, dx_ViewCoords and dx_ViewAdjust.
        // In both cases total 3 uniform registers need to be reserved.
        mUniformHLSL->reserveUniformRegisters(3);
    }

    // Reserve registers for the default uniform block and driver constants
    mUniformHLSL->reserveInterfaceBlockRegisters(2);
}

OutputHLSL::~OutputHLSL()
{
    SafeDelete(mStructureHLSL);
    SafeDelete(mUniformHLSL);
    for (auto &eqFunction : mStructEqualityFunctions)
    {
        SafeDelete(eqFunction);
    }
    for (auto &eqFunction : mArrayEqualityFunctions)
    {
        SafeDelete(eqFunction);
    }
}

void OutputHLSL::output(TIntermNode *treeRoot, TInfoSinkBase &objSink)
{
    const std::vector<TIntermTyped*> &flaggedStructs = FlagStd140ValueStructs(treeRoot);
    makeFlaggedStructMaps(flaggedStructs);

    BuiltInFunctionEmulator builtInFunctionEmulator;
    InitBuiltInFunctionEmulatorForHLSL(&builtInFunctionEmulator);
    builtInFunctionEmulator.MarkBuiltInFunctionsForEmulation(treeRoot);

    // Now that we are done changing the AST, do the analyses need for HLSL generation
    CallDAG::InitResult success = mCallDag.init(treeRoot, &objSink);
    ASSERT(success == CallDAG::INITDAG_SUCCESS);
    UNUSED_ASSERTION_VARIABLE(success);
    mASTMetadataList = CreateASTMetadataHLSL(treeRoot, mCallDag);

    // Output the body and footer first to determine what has to go in the header
    mInfoSinkStack.push(&mBody);
    treeRoot->traverse(this);
    mInfoSinkStack.pop();

    mInfoSinkStack.push(&mFooter);
    if (!mDeferredGlobalInitializers.empty())
    {
        writeDeferredGlobalInitializers(mFooter);
    }
    mInfoSinkStack.pop();

    mInfoSinkStack.push(&mHeader);
    header(&builtInFunctionEmulator);
    mInfoSinkStack.pop();

    objSink << mHeader.c_str();
    objSink << mBody.c_str();
    objSink << mFooter.c_str();

    builtInFunctionEmulator.Cleanup();
}

void OutputHLSL::makeFlaggedStructMaps(const std::vector<TIntermTyped *> &flaggedStructs)
{
    for (unsigned int structIndex = 0; structIndex < flaggedStructs.size(); structIndex++)
    {
        TIntermTyped *flaggedNode = flaggedStructs[structIndex];

        TInfoSinkBase structInfoSink;
        mInfoSinkStack.push(&structInfoSink);

        // This will mark the necessary block elements as referenced
        flaggedNode->traverse(this);

        TString structName(structInfoSink.c_str());
        mInfoSinkStack.pop();

        mFlaggedStructOriginalNames[flaggedNode] = structName;

        for (size_t pos = structName.find('.'); pos != std::string::npos; pos = structName.find('.'))
        {
            structName.erase(pos, 1);
        }

        mFlaggedStructMappedNames[flaggedNode] = "map" + structName;
    }
}

const std::map<std::string, unsigned int> &OutputHLSL::getInterfaceBlockRegisterMap() const
{
    return mUniformHLSL->getInterfaceBlockRegisterMap();
}

const std::map<std::string, unsigned int> &OutputHLSL::getUniformRegisterMap() const
{
    return mUniformHLSL->getUniformRegisterMap();
}

int OutputHLSL::vectorSize(const TType &type) const
{
    int elementSize = type.isMatrix() ? type.getCols() : 1;
    int arraySize = type.isArray() ? type.getArraySize() : 1;

    return elementSize * arraySize;
}

TString OutputHLSL::structInitializerString(int indent, const TStructure &structure, const TString &rhsStructName)
{
    TString init;

    TString preIndentString;
    TString fullIndentString;

    for (int spaces = 0; spaces < (indent * 4); spaces++)
    {
        preIndentString += ' ';
    }

    for (int spaces = 0; spaces < ((indent+1) * 4); spaces++)
    {
        fullIndentString += ' ';
    }

    init += preIndentString + "{\n";

    const TFieldList &fields = structure.fields();
    for (unsigned int fieldIndex = 0; fieldIndex < fields.size(); fieldIndex++)
    {
        const TField &field = *fields[fieldIndex];
        const TString &fieldName = rhsStructName + "." + Decorate(field.name());
        const TType &fieldType = *field.type();

        if (fieldType.getStruct())
        {
            init += structInitializerString(indent + 1, *fieldType.getStruct(), fieldName);
        }
        else
        {
            init += fullIndentString + fieldName + ",\n";
        }
    }

    init += preIndentString + "}" + (indent == 0 ? ";" : ",") + "\n";

    return init;
}

void OutputHLSL::header(const BuiltInFunctionEmulator *builtInFunctionEmulator)
{
    TInfoSinkBase &out = getInfoSink();

    TString varyings;
    TString attributes;
    TString flaggedStructs;

    for (std::map<TIntermTyped*, TString>::const_iterator flaggedStructIt = mFlaggedStructMappedNames.begin(); flaggedStructIt != mFlaggedStructMappedNames.end(); flaggedStructIt++)
    {
        TIntermTyped *structNode = flaggedStructIt->first;
        const TString &mappedName = flaggedStructIt->second;
        const TStructure &structure = *structNode->getType().getStruct();
        const TString &originalName = mFlaggedStructOriginalNames[structNode];

        flaggedStructs += "static " + Decorate(structure.name()) + " " + mappedName + " =\n";
        flaggedStructs += structInitializerString(0, structure, originalName);
        flaggedStructs += "\n";
    }

    for (ReferencedSymbols::const_iterator varying = mReferencedVaryings.begin(); varying != mReferencedVaryings.end(); varying++)
    {
        const TType &type = varying->second->getType();
        const TString &name = varying->second->getSymbol();

        // Program linking depends on this exact format
        varyings += "static " + InterpolationString(type.getQualifier()) + " " + TypeString(type) + " " +
                    Decorate(name) + ArrayString(type) + " = " + initializer(type) + ";\n";
    }

    for (ReferencedSymbols::const_iterator attribute = mReferencedAttributes.begin(); attribute != mReferencedAttributes.end(); attribute++)
    {
        const TType &type = attribute->second->getType();
        const TString &name = attribute->second->getSymbol();

        attributes += "static " + TypeString(type) + " " + Decorate(name) + ArrayString(type) + " = " + initializer(type) + ";\n";
    }

    out << mStructureHLSL->structsHeader();

    out << mUniformHLSL->uniformsHeader(mOutputType, mReferencedUniforms);
    out << mUniformHLSL->interfaceBlocksHeader(mReferencedInterfaceBlocks);

    if (!mEqualityFunctions.empty())
    {
        out << "\n// Equality functions\n\n";
        for (const auto &eqFunction : mEqualityFunctions)
        {
            out << eqFunction->functionDefinition << "\n";
        }
    }
    if (!mArrayAssignmentFunctions.empty())
    {
        out << "\n// Assignment functions\n\n";
        for (const auto &assignmentFunction : mArrayAssignmentFunctions)
        {
            out << assignmentFunction.functionDefinition << "\n";
        }
    }
    if (!mArrayConstructIntoFunctions.empty())
    {
        out << "\n// Array constructor functions\n\n";
        for (const auto &constructIntoFunction : mArrayConstructIntoFunctions)
        {
            out << constructIntoFunction.functionDefinition << "\n";
        }
    }

    if (mUsesDiscardRewriting)
    {
        out << "#define ANGLE_USES_DISCARD_REWRITING\n";
    }

    if (mUsesNestedBreak)
    {
        out << "#define ANGLE_USES_NESTED_BREAK\n";
    }

    if (mRequiresIEEEStrictCompiling)
    {
        out << "#define ANGLE_REQUIRES_IEEE_STRICT_COMPILING\n";
    }

    out << "#ifdef ANGLE_ENABLE_LOOP_FLATTEN\n"
           "#define LOOP [loop]\n"
           "#define FLATTEN [flatten]\n"
           "#else\n"
           "#define LOOP\n"
           "#define FLATTEN\n"
           "#endif\n";

    if (mShaderType == GL_FRAGMENT_SHADER)
    {
        TExtensionBehavior::const_iterator iter = mExtensionBehavior.find("GL_EXT_draw_buffers");
        const bool usingMRTExtension = (iter != mExtensionBehavior.end() && (iter->second == EBhEnable || iter->second == EBhRequire));

        out << "// Varyings\n";
        out <<  varyings;
        out << "\n";

        if (mShaderVersion >= 300)
        {
            for (ReferencedSymbols::const_iterator outputVariableIt = mReferencedOutputVariables.begin(); outputVariableIt != mReferencedOutputVariables.end(); outputVariableIt++)
            {
                const TString &variableName = outputVariableIt->first;
                const TType &variableType = outputVariableIt->second->getType();

                out << "static " + TypeString(variableType) + " out_" + variableName + ArrayString(variableType) +
                       " = " + initializer(variableType) + ";\n";
            }
        }
        else
        {
            const unsigned int numColorValues = usingMRTExtension ? mNumRenderTargets : 1;

            out << "static float4 gl_Color[" << numColorValues << "] =\n"
                   "{\n";
            for (unsigned int i = 0; i < numColorValues; i++)
            {
                out << "    float4(0, 0, 0, 0)";
                if (i + 1 != numColorValues)
                {
                    out << ",";
                }
                out << "\n";
            }

            out << "};\n";
        }

        if (mUsesFragDepth)
        {
            out << "static float gl_Depth = 0.0;\n";
        }

        if (mUsesFragCoord)
        {
            out << "static float4 gl_FragCoord = float4(0, 0, 0, 0);\n";
        }

        if (mUsesPointCoord)
        {
            out << "static float2 gl_PointCoord = float2(0.5, 0.5);\n";
        }

        if (mUsesFrontFacing)
        {
            out << "static bool gl_FrontFacing = false;\n";
        }

        out << "\n";

        if (mUsesDepthRange)
        {
            out << "struct gl_DepthRangeParameters\n"
                   "{\n"
                   "    float near;\n"
                   "    float far;\n"
                   "    float diff;\n"
                   "};\n"
                   "\n";
        }

        if (mOutputType == SH_HLSL11_OUTPUT)
        {
            out << "cbuffer DriverConstants : register(b1)\n"
                   "{\n";

            if (mUsesDepthRange)
            {
                out << "    float3 dx_DepthRange : packoffset(c0);\n";
            }

            if (mUsesFragCoord)
            {
                out << "    float4 dx_ViewCoords : packoffset(c1);\n";
            }

            if (mUsesFragCoord || mUsesFrontFacing)
            {
                out << "    float3 dx_DepthFront : packoffset(c2);\n";
            }

            out << "};\n";
        }
        else
        {
            if (mUsesDepthRange)
            {
                out << "uniform float3 dx_DepthRange : register(c0);";
            }

            if (mUsesFragCoord)
            {
                out << "uniform float4 dx_ViewCoords : register(c1);\n";
            }

            if (mUsesFragCoord || mUsesFrontFacing)
            {
                out << "uniform float3 dx_DepthFront : register(c2);\n";
            }
        }

        out << "\n";

        if (mUsesDepthRange)
        {
            out << "static gl_DepthRangeParameters gl_DepthRange = {dx_DepthRange.x, dx_DepthRange.y, dx_DepthRange.z};\n"
                   "\n";
        }

        if (!flaggedStructs.empty())
        {
            out << "// Std140 Structures accessed by value\n";
            out << "\n";
            out << flaggedStructs;
            out << "\n";
        }

        if (usingMRTExtension && mNumRenderTargets > 1)
        {
            out << "#define GL_USES_MRT\n";
        }

        if (mUsesFragColor)
        {
            out << "#define GL_USES_FRAG_COLOR\n";
        }

        if (mUsesFragData)
        {
            out << "#define GL_USES_FRAG_DATA\n";
        }
    }
    else   // Vertex shader
    {
        out << "// Attributes\n";
        out <<  attributes;
        out << "\n"
               "static float4 gl_Position = float4(0, 0, 0, 0);\n";

        if (mUsesPointSize)
        {
            out << "static float gl_PointSize = float(1);\n";
        }

        if (mUsesInstanceID)
        {
            out << "static int gl_InstanceID;";
        }

        out << "\n"
               "// Varyings\n";
        out <<  varyings;
        out << "\n";

        if (mUsesDepthRange)
        {
            out << "struct gl_DepthRangeParameters\n"
                   "{\n"
                   "    float near;\n"
                   "    float far;\n"
                   "    float diff;\n"
                   "};\n"
                   "\n";
        }

        if (mOutputType == SH_HLSL11_OUTPUT)
        {
            out << "cbuffer DriverConstants : register(b1)\n"
                    "{\n";

            if (mUsesDepthRange)
            {
                out << "    float3 dx_DepthRange : packoffset(c0);\n";
            }

            // dx_ViewAdjust and dx_ViewCoords will only be used in Feature Level 9 shaders.
            // However, we declare it for all shaders (including Feature Level 10+).
            // The bytecode is the same whether we declare it or not, since D3DCompiler removes it if it's unused.
            out << "    float4 dx_ViewAdjust : packoffset(c1);\n";
            out << "    float2 dx_ViewCoords : packoffset(c2);\n";

            out << "};\n"
                   "\n";
        }
        else
        {
            if (mUsesDepthRange)
            {
                out << "uniform float3 dx_DepthRange : register(c0);\n";
            }

            out << "uniform float4 dx_ViewAdjust : register(c1);\n";
            out << "uniform float2 dx_ViewCoords : register(c2);\n"
                   "\n";
        }

        if (mUsesDepthRange)
        {
            out << "static gl_DepthRangeParameters gl_DepthRange = {dx_DepthRange.x, dx_DepthRange.y, dx_DepthRange.z};\n"
                   "\n";
        }

        if (!flaggedStructs.empty())
        {
            out << "// Std140 Structures accessed by value\n";
            out << "\n";
            out << flaggedStructs;
            out << "\n";
        }
    }

    for (TextureFunctionSet::const_iterator textureFunction = mUsesTexture.begin(); textureFunction != mUsesTexture.end(); textureFunction++)
    {
        // Return type
        if (textureFunction->method == TextureFunction::SIZE)
        {
            switch(textureFunction->sampler)
            {
              case EbtSampler2D:            out << "int2 "; break;
              case EbtSampler3D:            out << "int3 "; break;
              case EbtSamplerCube:          out << "int2 "; break;
              case EbtSampler2DArray:       out << "int3 "; break;
              case EbtISampler2D:           out << "int2 "; break;
              case EbtISampler3D:           out << "int3 "; break;
              case EbtISamplerCube:         out << "int2 "; break;
              case EbtISampler2DArray:      out << "int3 "; break;
              case EbtUSampler2D:           out << "int2 "; break;
              case EbtUSampler3D:           out << "int3 "; break;
              case EbtUSamplerCube:         out << "int2 "; break;
              case EbtUSampler2DArray:      out << "int3 "; break;
              case EbtSampler2DShadow:      out << "int2 "; break;
              case EbtSamplerCubeShadow:    out << "int2 "; break;
              case EbtSampler2DArrayShadow: out << "int3 "; break;
              default: UNREACHABLE();
            }
        }
        else   // Sampling function
        {
            switch(textureFunction->sampler)
            {
              case EbtSampler2D:            out << "float4 "; break;
              case EbtSampler3D:            out << "float4 "; break;
              case EbtSamplerCube:          out << "float4 "; break;
              case EbtSampler2DArray:       out << "float4 "; break;
              case EbtISampler2D:           out << "int4 ";   break;
              case EbtISampler3D:           out << "int4 ";   break;
              case EbtISamplerCube:         out << "int4 ";   break;
              case EbtISampler2DArray:      out << "int4 ";   break;
              case EbtUSampler2D:           out << "uint4 ";  break;
              case EbtUSampler3D:           out << "uint4 ";  break;
              case EbtUSamplerCube:         out << "uint4 ";  break;
              case EbtUSampler2DArray:      out << "uint4 ";  break;
              case EbtSampler2DShadow:      out << "float ";  break;
              case EbtSamplerCubeShadow:    out << "float ";  break;
              case EbtSampler2DArrayShadow: out << "float ";  break;
              default: UNREACHABLE();
            }
        }

        // Function name
        out << textureFunction->name();

        // Argument list
        int hlslCoords = 4;

        if (mOutputType == SH_HLSL9_OUTPUT)
        {
            switch(textureFunction->sampler)
            {
              case EbtSampler2D:   out << "sampler2D s";   hlslCoords = 2; break;
              case EbtSamplerCube: out << "samplerCUBE s"; hlslCoords = 3; break;
              default: UNREACHABLE();
            }

            switch(textureFunction->method)
            {
              case TextureFunction::IMPLICIT:                 break;
              case TextureFunction::BIAS:     hlslCoords = 4; break;
              case TextureFunction::LOD:      hlslCoords = 4; break;
              case TextureFunction::LOD0:     hlslCoords = 4; break;
              case TextureFunction::LOD0BIAS: hlslCoords = 4; break;
              default: UNREACHABLE();
            }
        }
        else if (mOutputType == SH_HLSL11_OUTPUT)
        {
            switch(textureFunction->sampler)
            {
              case EbtSampler2D:            out << "Texture2D x, SamplerState s";                hlslCoords = 2; break;
              case EbtSampler3D:            out << "Texture3D x, SamplerState s";                hlslCoords = 3; break;
              case EbtSamplerCube:          out << "TextureCube x, SamplerState s";              hlslCoords = 3; break;
              case EbtSampler2DArray:       out << "Texture2DArray x, SamplerState s";           hlslCoords = 3; break;
              case EbtISampler2D:           out << "Texture2D<int4> x, SamplerState s";          hlslCoords = 2; break;
              case EbtISampler3D:           out << "Texture3D<int4> x, SamplerState s";          hlslCoords = 3; break;
              case EbtISamplerCube:         out << "Texture2DArray<int4> x, SamplerState s";     hlslCoords = 3; break;
              case EbtISampler2DArray:      out << "Texture2DArray<int4> x, SamplerState s";     hlslCoords = 3; break;
              case EbtUSampler2D:           out << "Texture2D<uint4> x, SamplerState s";         hlslCoords = 2; break;
              case EbtUSampler3D:           out << "Texture3D<uint4> x, SamplerState s";         hlslCoords = 3; break;
              case EbtUSamplerCube:         out << "Texture2DArray<uint4> x, SamplerState s";    hlslCoords = 3; break;
              case EbtUSampler2DArray:      out << "Texture2DArray<uint4> x, SamplerState s";    hlslCoords = 3; break;
              case EbtSampler2DShadow:      out << "Texture2D x, SamplerComparisonState s";      hlslCoords = 2; break;
              case EbtSamplerCubeShadow:    out << "TextureCube x, SamplerComparisonState s";    hlslCoords = 3; break;
              case EbtSampler2DArrayShadow: out << "Texture2DArray x, SamplerComparisonState s"; hlslCoords = 3; break;
              default: UNREACHABLE();
            }
        }
        else UNREACHABLE();

        if (textureFunction->method == TextureFunction::FETCH)   // Integer coordinates
        {
            switch(textureFunction->coords)
            {
              case 2: out << ", int2 t"; break;
              case 3: out << ", int3 t"; break;
              default: UNREACHABLE();
            }
        }
        else   // Floating-point coordinates (except textureSize)
        {
            switch(textureFunction->coords)
            {
              case 1: out << ", int lod";  break;   // textureSize()
              case 2: out << ", float2 t"; break;
              case 3: out << ", float3 t"; break;
              case 4: out << ", float4 t"; break;
              default: UNREACHABLE();
            }
        }

        if (textureFunction->method == TextureFunction::GRAD)
        {
            switch(textureFunction->sampler)
            {
              case EbtSampler2D:
              case EbtISampler2D:
              case EbtUSampler2D:
              case EbtSampler2DArray:
              case EbtISampler2DArray:
              case EbtUSampler2DArray:
              case EbtSampler2DShadow:
              case EbtSampler2DArrayShadow:
                out << ", float2 ddx, float2 ddy";
                break;
              case EbtSampler3D:
              case EbtISampler3D:
              case EbtUSampler3D:
              case EbtSamplerCube:
              case EbtISamplerCube:
              case EbtUSamplerCube:
              case EbtSamplerCubeShadow:
                out << ", float3 ddx, float3 ddy";
                break;
              default: UNREACHABLE();
            }
        }

        switch(textureFunction->method)
        {
          case TextureFunction::IMPLICIT:                        break;
          case TextureFunction::BIAS:                            break;   // Comes after the offset parameter
          case TextureFunction::LOD:      out << ", float lod";  break;
          case TextureFunction::LOD0:                            break;
          case TextureFunction::LOD0BIAS:                        break;   // Comes after the offset parameter
          case TextureFunction::SIZE:                            break;
          case TextureFunction::FETCH:    out << ", int mip";    break;
          case TextureFunction::GRAD:                            break;
          default: UNREACHABLE();
        }

        if (textureFunction->offset)
        {
            switch(textureFunction->sampler)
            {
              case EbtSampler2D:            out << ", int2 offset"; break;
              case EbtSampler3D:            out << ", int3 offset"; break;
              case EbtSampler2DArray:       out << ", int2 offset"; break;
              case EbtISampler2D:           out << ", int2 offset"; break;
              case EbtISampler3D:           out << ", int3 offset"; break;
              case EbtISampler2DArray:      out << ", int2 offset"; break;
              case EbtUSampler2D:           out << ", int2 offset"; break;
              case EbtUSampler3D:           out << ", int3 offset"; break;
              case EbtUSampler2DArray:      out << ", int2 offset"; break;
              case EbtSampler2DShadow:      out << ", int2 offset"; break;
              case EbtSampler2DArrayShadow: out << ", int2 offset"; break;
              default: UNREACHABLE();
            }
        }

        if (textureFunction->method == TextureFunction::BIAS ||
            textureFunction->method == TextureFunction::LOD0BIAS)
        {
            out << ", float bias";
        }

        out << ")\n"
               "{\n";

        if (textureFunction->method == TextureFunction::SIZE)
        {
            if (IsSampler2D(textureFunction->sampler) || IsSamplerCube(textureFunction->sampler))
            {
                if (IsSamplerArray(textureFunction->sampler))
                {
                    out << "    uint width; uint height; uint layers; uint numberOfLevels;\n"
                           "    x.GetDimensions(lod, width, height, layers, numberOfLevels);\n";
                }
                else
                {
                    out << "    uint width; uint height; uint numberOfLevels;\n"
                           "    x.GetDimensions(lod, width, height, numberOfLevels);\n";
                }
            }
            else if (IsSampler3D(textureFunction->sampler))
            {
                out << "    uint width; uint height; uint depth; uint numberOfLevels;\n"
                       "    x.GetDimensions(lod, width, height, depth, numberOfLevels);\n";
            }
            else UNREACHABLE();

            switch(textureFunction->sampler)
            {
              case EbtSampler2D:            out << "    return int2(width, height);";         break;
              case EbtSampler3D:            out << "    return int3(width, height, depth);";  break;
              case EbtSamplerCube:          out << "    return int2(width, height);";         break;
              case EbtSampler2DArray:       out << "    return int3(width, height, layers);"; break;
              case EbtISampler2D:           out << "    return int2(width, height);";         break;
              case EbtISampler3D:           out << "    return int3(width, height, depth);";  break;
              case EbtISamplerCube:         out << "    return int2(width, height);";         break;
              case EbtISampler2DArray:      out << "    return int3(width, height, layers);"; break;
              case EbtUSampler2D:           out << "    return int2(width, height);";         break;
              case EbtUSampler3D:           out << "    return int3(width, height, depth);";  break;
              case EbtUSamplerCube:         out << "    return int2(width, height);";         break;
              case EbtUSampler2DArray:      out << "    return int3(width, height, layers);"; break;
              case EbtSampler2DShadow:      out << "    return int2(width, height);";         break;
              case EbtSamplerCubeShadow:    out << "    return int2(width, height);";         break;
              case EbtSampler2DArrayShadow: out << "    return int3(width, height, layers);"; break;
              default: UNREACHABLE();
            }
        }
        else
        {
            if (IsIntegerSampler(textureFunction->sampler) && IsSamplerCube(textureFunction->sampler))
            {
                out << "    float width; float height; float layers; float levels;\n";

                out << "    uint mip = 0;\n";

                out << "    x.GetDimensions(mip, width, height, layers, levels);\n";

                out << "    bool xMajor = abs(t.x) > abs(t.y) && abs(t.x) > abs(t.z);\n";
                out << "    bool yMajor = abs(t.y) > abs(t.z) && abs(t.y) > abs(t.x);\n";
                out << "    bool zMajor = abs(t.z) > abs(t.x) && abs(t.z) > abs(t.y);\n";
                out << "    bool negative = (xMajor && t.x < 0.0f) || (yMajor && t.y < 0.0f) || (zMajor && t.z < 0.0f);\n";

                // FACE_POSITIVE_X = 000b
                // FACE_NEGATIVE_X = 001b
                // FACE_POSITIVE_Y = 010b
                // FACE_NEGATIVE_Y = 011b
                // FACE_POSITIVE_Z = 100b
                // FACE_NEGATIVE_Z = 101b
                out << "    int face = (int)negative + (int)yMajor * 2 + (int)zMajor * 4;\n";

                out << "    float u = xMajor ? -t.z : (yMajor && t.y < 0.0f ? -t.x : t.x);\n";
                out << "    float v = yMajor ? t.z : (negative ? t.y : -t.y);\n";
                out << "    float m = xMajor ? t.x : (yMajor ? t.y : t.z);\n";

                out << "    t.x = (u * 0.5f / m) + 0.5f;\n";
                out << "    t.y = (v * 0.5f / m) + 0.5f;\n";
            }
            else if (IsIntegerSampler(textureFunction->sampler) &&
                     textureFunction->method != TextureFunction::FETCH)
            {
                if (IsSampler2D(textureFunction->sampler))
                {
                    if (IsSamplerArray(textureFunction->sampler))
                    {
                        out << "    float width; float height; float layers; float levels;\n";

                        if (textureFunction->method == TextureFunction::LOD0)
                        {
                            out << "    uint mip = 0;\n";
                        }
                        else if (textureFunction->method == TextureFunction::LOD0BIAS)
                        {
                            out << "    uint mip = bias;\n";
                        }
                        else
                        {
                            if (textureFunction->method == TextureFunction::IMPLICIT ||
                                textureFunction->method == TextureFunction::BIAS)
                            {
                                out << "    x.GetDimensions(0, width, height, layers, levels);\n"
                                       "    float2 tSized = float2(t.x * width, t.y * height);\n"
                                       "    float dx = length(ddx(tSized));\n"
                                       "    float dy = length(ddy(tSized));\n"
                                       "    float lod = log2(max(dx, dy));\n";

                                if (textureFunction->method == TextureFunction::BIAS)
                                {
                                    out << "    lod += bias;\n";
                                }
                            }
                            else if (textureFunction->method == TextureFunction::GRAD)
                            {
                                out << "    x.GetDimensions(0, width, height, layers, levels);\n"
                                       "    float lod = log2(max(length(ddx), length(ddy)));\n";
                            }

                            out << "    uint mip = uint(min(max(round(lod), 0), levels - 1));\n";
                        }

                        out << "    x.GetDimensions(mip, width, height, layers, levels);\n";
                    }
                    else
                    {
                        out << "    float width; float height; float levels;\n";

                        if (textureFunction->method == TextureFunction::LOD0)
                        {
                            out << "    uint mip = 0;\n";
                        }
                        else if (textureFunction->method == TextureFunction::LOD0BIAS)
                        {
                            out << "    uint mip = bias;\n";
                        }
                        else
                        {
                            if (textureFunction->method == TextureFunction::IMPLICIT ||
                                textureFunction->method == TextureFunction::BIAS)
                            {
                                out << "    x.GetDimensions(0, width, height, levels);\n"
                                       "    float2 tSized = float2(t.x * width, t.y * height);\n"
                                       "    float dx = length(ddx(tSized));\n"
                                       "    float dy = length(ddy(tSized));\n"
                                       "    float lod = log2(max(dx, dy));\n";

                                if (textureFunction->method == TextureFunction::BIAS)
                                {
                                    out << "    lod += bias;\n";
                                }
                            }
                            else if (textureFunction->method == TextureFunction::LOD)
                            {
                                out << "    x.GetDimensions(0, width, height, levels);\n";
                            }
                            else if (textureFunction->method == TextureFunction::GRAD)
                            {
                                out << "    x.GetDimensions(0, width, height, levels);\n"
                                       "    float lod = log2(max(length(ddx), length(ddy)));\n";
                            }

                            out << "    uint mip = uint(min(max(round(lod), 0), levels - 1));\n";
                        }

                        out << "    x.GetDimensions(mip, width, height, levels);\n";
                    }
                }
                else if (IsSampler3D(textureFunction->sampler))
                {
                    out << "    float width; float height; float depth; float levels;\n";

                    if (textureFunction->method == TextureFunction::LOD0)
                    {
                        out << "    uint mip = 0;\n";
                    }
                    else if (textureFunction->method == TextureFunction::LOD0BIAS)
                    {
                        out << "    uint mip = bias;\n";
                    }
                    else
                    {
                        if (textureFunction->method == TextureFunction::IMPLICIT ||
                            textureFunction->method == TextureFunction::BIAS)
                        {
                            out << "    x.GetDimensions(0, width, height, depth, levels);\n"
                                   "    float3 tSized = float3(t.x * width, t.y * height, t.z * depth);\n"
                                   "    float dx = length(ddx(tSized));\n"
                                   "    float dy = length(ddy(tSized));\n"
                                   "    float lod = log2(max(dx, dy));\n";

                            if (textureFunction->method == TextureFunction::BIAS)
                            {
                                out << "    lod += bias;\n";
                            }
                        }
                        else if (textureFunction->method == TextureFunction::GRAD)
                        {
                            out << "    x.GetDimensions(0, width, height, depth, levels);\n"
                                   "    float lod = log2(max(length(ddx), length(ddy)));\n";
                        }

                        out << "    uint mip = uint(min(max(round(lod), 0), levels - 1));\n";
                    }

                    out << "    x.GetDimensions(mip, width, height, depth, levels);\n";
                }
                else UNREACHABLE();
            }

            out << "    return ";

            // HLSL intrinsic
            if (mOutputType == SH_HLSL9_OUTPUT)
            {
                switch(textureFunction->sampler)
                {
                  case EbtSampler2D:   out << "tex2D";   break;
                  case EbtSamplerCube: out << "texCUBE"; break;
                  default: UNREACHABLE();
                }

                switch(textureFunction->method)
                {
                  case TextureFunction::IMPLICIT: out << "(s, ";     break;
                  case TextureFunction::BIAS:     out << "bias(s, "; break;
                  case TextureFunction::LOD:      out << "lod(s, ";  break;
                  case TextureFunction::LOD0:     out << "lod(s, ";  break;
                  case TextureFunction::LOD0BIAS: out << "lod(s, ";  break;
                  default: UNREACHABLE();
                }
            }
            else if (mOutputType == SH_HLSL11_OUTPUT)
            {
                if (textureFunction->method == TextureFunction::GRAD)
                {
                    if (IsIntegerSampler(textureFunction->sampler))
                    {
                        out << "x.Load(";
                    }
                    else if (IsShadowSampler(textureFunction->sampler))
                    {
                        out << "x.SampleCmpLevelZero(s, ";
                    }
                    else
                    {
                        out << "x.SampleGrad(s, ";
                    }
                }
                else if (IsIntegerSampler(textureFunction->sampler) ||
                         textureFunction->method == TextureFunction::FETCH)
                {
                    out << "x.Load(";
                }
                else if (IsShadowSampler(textureFunction->sampler))
                {
                    switch(textureFunction->method)
                    {
                      case TextureFunction::IMPLICIT: out << "x.SampleCmp(s, ";          break;
                      case TextureFunction::BIAS:     out << "x.SampleCmp(s, ";          break;
                      case TextureFunction::LOD:      out << "x.SampleCmp(s, ";          break;
                      case TextureFunction::LOD0:     out << "x.SampleCmpLevelZero(s, "; break;
                      case TextureFunction::LOD0BIAS: out << "x.SampleCmpLevelZero(s, "; break;
                      default: UNREACHABLE();
                    }
                }
                else
                {
                    switch(textureFunction->method)
                    {
                      case TextureFunction::IMPLICIT: out << "x.Sample(s, ";      break;
                      case TextureFunction::BIAS:     out << "x.SampleBias(s, ";  break;
                      case TextureFunction::LOD:      out << "x.SampleLevel(s, "; break;
                      case TextureFunction::LOD0:     out << "x.SampleLevel(s, "; break;
                      case TextureFunction::LOD0BIAS: out << "x.SampleLevel(s, "; break;
                      default: UNREACHABLE();
                    }
                }
            }
            else UNREACHABLE();

            // Integer sampling requires integer addresses
            TString addressx = "";
            TString addressy = "";
            TString addressz = "";
            TString close = "";

            if (IsIntegerSampler(textureFunction->sampler) ||
                textureFunction->method == TextureFunction::FETCH)
            {
                switch(hlslCoords)
                {
                  case 2: out << "int3("; break;
                  case 3: out << "int4("; break;
                  default: UNREACHABLE();
                }

                // Convert from normalized floating-point to integer
                if (textureFunction->method != TextureFunction::FETCH)
                {
                    addressx = "int(floor(width * frac((";
                    addressy = "int(floor(height * frac((";

                    if (IsSamplerArray(textureFunction->sampler))
                    {
                        addressz = "int(max(0, min(layers - 1, floor(0.5 + ";
                    }
                    else if (IsSamplerCube(textureFunction->sampler))
                    {
                        addressz = "((((";
                    }
                    else
                    {
                        addressz = "int(floor(depth * frac((";
                    }

                    close = "))))";
                }
            }
            else
            {
                switch(hlslCoords)
                {
                  case 2: out << "float2("; break;
                  case 3: out << "float3("; break;
                  case 4: out << "float4("; break;
                  default: UNREACHABLE();
                }
            }

            TString proj = "";   // Only used for projected textures

            if (textureFunction->proj)
            {
                switch(textureFunction->coords)
                {
                  case 3: proj = " / t.z"; break;
                  case 4: proj = " / t.w"; break;
                  default: UNREACHABLE();
                }
            }

            out << addressx + ("t.x" + proj) + close + ", " + addressy + ("t.y" + proj) + close;

            if (mOutputType == SH_HLSL9_OUTPUT)
            {
                if (hlslCoords >= 3)
                {
                    if (textureFunction->coords < 3)
                    {
                        out << ", 0";
                    }
                    else
                    {
                        out << ", t.z" + proj;
                    }
                }

                if (hlslCoords == 4)
                {
                    switch(textureFunction->method)
                    {
                      case TextureFunction::BIAS:     out << ", bias"; break;
                      case TextureFunction::LOD:      out << ", lod";  break;
                      case TextureFunction::LOD0:     out << ", 0";    break;
                      case TextureFunction::LOD0BIAS: out << ", bias"; break;
                      default: UNREACHABLE();
                    }
                }

                out << "));\n";
            }
            else if (mOutputType == SH_HLSL11_OUTPUT)
            {
                if (hlslCoords >= 3)
                {
                    if (IsIntegerSampler(textureFunction->sampler) && IsSamplerCube(textureFunction->sampler))
                    {
                        out << ", face";
                    }
                    else
                    {
                        out << ", " + addressz + ("t.z" + proj) + close;
                    }
                }

                if (textureFunction->method == TextureFunction::GRAD)
                {
                    if (IsIntegerSampler(textureFunction->sampler))
                    {
                        out << ", mip)";
                    }
                    else if (IsShadowSampler(textureFunction->sampler))
                    {
                        // Compare value
                        if (textureFunction->proj)
                        {
                            // According to ESSL 3.00.4 sec 8.8 p95 on textureProj:
                            // The resulting third component of P' in the shadow forms is used as Dref
                            out << "), t.z" << proj;
                        }
                        else
                        {
                            switch(textureFunction->coords)
                            {
                              case 3: out << "), t.z"; break;
                              case 4: out << "), t.w"; break;
                              default: UNREACHABLE();
                            }
                        }
                    }
                    else
                    {
                        out << "), ddx, ddy";
                    }
                }
                else if (IsIntegerSampler(textureFunction->sampler) ||
                         textureFunction->method == TextureFunction::FETCH)
                {
                    out << ", mip)";
                }
                else if (IsShadowSampler(textureFunction->sampler))
                {
                    // Compare value
                    if (textureFunction->proj)
                    {
                        // According to ESSL 3.00.4 sec 8.8 p95 on textureProj:
                        // The resulting third component of P' in the shadow forms is used as Dref
                        out << "), t.z" << proj;
                    }
                    else
                    {
                        switch(textureFunction->coords)
                        {
                          case 3: out << "), t.z"; break;
                          case 4: out << "), t.w"; break;
                          default: UNREACHABLE();
                        }
                    }
                }
                else
                {
                    switch(textureFunction->method)
                    {
                      case TextureFunction::IMPLICIT: out << ")";       break;
                      case TextureFunction::BIAS:     out << "), bias"; break;
                      case TextureFunction::LOD:      out << "), lod";  break;
                      case TextureFunction::LOD0:     out << "), 0";    break;
                      case TextureFunction::LOD0BIAS: out << "), bias"; break;
                      default: UNREACHABLE();
                    }
                }

                if (textureFunction->offset)
                {
                    out << ", offset";
                }

                out << ");";
            }
            else UNREACHABLE();
        }

        out << "\n"
               "}\n"
               "\n";
    }

    if (mUsesFragCoord)
    {
        out << "#define GL_USES_FRAG_COORD\n";
    }

    if (mUsesPointCoord)
    {
        out << "#define GL_USES_POINT_COORD\n";
    }

    if (mUsesFrontFacing)
    {
        out << "#define GL_USES_FRONT_FACING\n";
    }

    if (mUsesPointSize)
    {
        out << "#define GL_USES_POINT_SIZE\n";
    }

    if (mUsesFragDepth)
    {
        out << "#define GL_USES_FRAG_DEPTH\n";
    }

    if (mUsesDepthRange)
    {
        out << "#define GL_USES_DEPTH_RANGE\n";
    }

    if (mUsesXor)
    {
        out << "bool xor(bool p, bool q)\n"
               "{\n"
               "    return (p || q) && !(p && q);\n"
               "}\n"
               "\n";
    }

    builtInFunctionEmulator->OutputEmulatedFunctions(out);
}

void OutputHLSL::visitSymbol(TIntermSymbol *node)
{
    TInfoSinkBase &out = getInfoSink();

    // Handle accessing std140 structs by value
    if (mFlaggedStructMappedNames.count(node) > 0)
    {
        out << mFlaggedStructMappedNames[node];
        return;
    }

    TString name = node->getSymbol();

    if (name == "gl_DepthRange")
    {
        mUsesDepthRange = true;
        out << name;
    }
    else
    {
        TQualifier qualifier = node->getQualifier();

        if (qualifier == EvqUniform)
        {
            const TType &nodeType = node->getType();
            const TInterfaceBlock *interfaceBlock = nodeType.getInterfaceBlock();

            if (interfaceBlock)
            {
                mReferencedInterfaceBlocks[interfaceBlock->name()] = node;
            }
            else
            {
                mReferencedUniforms[name] = node;
            }

            ensureStructDefined(nodeType);

            out << DecorateUniform(name, nodeType);
        }
        else if (qualifier == EvqAttribute || qualifier == EvqVertexIn)
        {
            mReferencedAttributes[name] = node;
            out << Decorate(name);
        }
        else if (IsVarying(qualifier))
        {
            mReferencedVaryings[name] = node;
            out << Decorate(name);
        }
        else if (qualifier == EvqFragmentOut)
        {
            mReferencedOutputVariables[name] = node;
            out << "out_" << name;
        }
        else if (qualifier == EvqFragColor)
        {
            out << "gl_Color[0]";
            mUsesFragColor = true;
        }
        else if (qualifier == EvqFragData)
        {
            out << "gl_Color";
            mUsesFragData = true;
        }
        else if (qualifier == EvqFragCoord)
        {
            mUsesFragCoord = true;
            out << name;
        }
        else if (qualifier == EvqPointCoord)
        {
            mUsesPointCoord = true;
            out << name;
        }
        else if (qualifier == EvqFrontFacing)
        {
            mUsesFrontFacing = true;
            out << name;
        }
        else if (qualifier == EvqPointSize)
        {
            mUsesPointSize = true;
            out << name;
        }
        else if (qualifier == EvqInstanceID)
        {
            mUsesInstanceID = true;
            out << name;
        }
        else if (name == "gl_FragDepthEXT")
        {
            mUsesFragDepth = true;
            out << "gl_Depth";
        }
        else
        {
            out << DecorateIfNeeded(node->getName());
        }
    }
}

void OutputHLSL::visitRaw(TIntermRaw *node)
{
    getInfoSink() << node->getRawText();
}

void OutputHLSL::outputEqual(Visit visit, const TType &type, TOperator op, TInfoSinkBase &out)
{
    if (type.isScalar() && !type.isArray())
    {
        if (op == EOpEqual)
        {
            outputTriplet(visit, "(", " == ", ")", out);
        }
        else
        {
            outputTriplet(visit, "(", " != ", ")", out);
        }
    }
    else
    {
        if (visit == PreVisit && op == EOpNotEqual)
        {
            out << "!";
        }

        if (type.isArray())
        {
            const TString &functionName = addArrayEqualityFunction(type);
            outputTriplet(visit, (functionName + "(").c_str(), ", ", ")", out);
        }
        else if (type.getBasicType() == EbtStruct)
        {
            const TStructure &structure = *type.getStruct();
            const TString &functionName = addStructEqualityFunction(structure);
            outputTriplet(visit, (functionName + "(").c_str(), ", ", ")", out);
        }
        else
        {
            ASSERT(type.isMatrix() || type.isVector());
            outputTriplet(visit, "all(", " == ", ")", out);
        }
    }
}

bool OutputHLSL::visitBinary(Visit visit, TIntermBinary *node)
{
    TInfoSinkBase &out = getInfoSink();

    // Handle accessing std140 structs by value
    if (mFlaggedStructMappedNames.count(node) > 0)
    {
        out << mFlaggedStructMappedNames[node];
        return false;
    }

    switch (node->getOp())
    {
      case EOpAssign:
        if (node->getLeft()->isArray())
        {
            TIntermAggregate *rightAgg = node->getRight()->getAsAggregate();
            if (rightAgg != nullptr && rightAgg->isConstructor())
            {
                const TString &functionName = addArrayConstructIntoFunction(node->getType());
                out << functionName << "(";
                node->getLeft()->traverse(this);
                TIntermSequence *seq = rightAgg->getSequence();
                for (auto &arrayElement : *seq)
                {
                    out << ", ";
                    arrayElement->traverse(this);
                }
                out << ")";
                return false;
            }
            // ArrayReturnValueToOutParameter should have eliminated expressions where a function call is assigned.
            ASSERT(rightAgg == nullptr || rightAgg->getOp() != EOpFunctionCall);

            const TString &functionName = addArrayAssignmentFunction(node->getType());
            outputTriplet(visit, (functionName + "(").c_str(), ", ", ")");
        }
        else
        {
            outputTriplet(visit, "(", " = ", ")");
        }
        break;
      case EOpInitialize:
        if (visit == PreVisit)
        {
            // GLSL allows to write things like "float x = x;" where a new variable x is defined
            // and the value of an existing variable x is assigned. HLSL uses C semantics (the
            // new variable is created before the assignment is evaluated), so we need to convert
            // this to "float t = x, x = t;".

            TIntermSymbol *symbolNode = node->getLeft()->getAsSymbolNode();
            ASSERT(symbolNode);
            TIntermTyped *expression = node->getRight();

            // TODO (jmadill): do a 'deep' scan to know if an expression is statically const
            if (symbolNode->getQualifier() == EvqGlobal && expression->getQualifier() != EvqConst)
            {
                // For variables which are not constant, defer their real initialization until
                // after we initialize uniforms.
                TIntermBinary *deferredInit = new TIntermBinary(EOpAssign);
                deferredInit->setLeft(node->getLeft());
                deferredInit->setRight(node->getRight());
                deferredInit->setType(node->getType());
                mDeferredGlobalInitializers.push_back(deferredInit);
                const TString &initString = initializer(node->getType());
                node->setRight(new TIntermRaw(node->getType(), initString));
            }
            else if (writeSameSymbolInitializer(out, symbolNode, expression))
            {
                // Skip initializing the rest of the expression
                return false;
            }
        }
        else if (visit == InVisit)
        {
            out << " = ";
        }
        break;
      case EOpAddAssign:               outputTriplet(visit, "(", " += ", ")");          break;
      case EOpSubAssign:               outputTriplet(visit, "(", " -= ", ")");          break;
      case EOpMulAssign:               outputTriplet(visit, "(", " *= ", ")");          break;
      case EOpVectorTimesScalarAssign: outputTriplet(visit, "(", " *= ", ")");          break;
      case EOpMatrixTimesScalarAssign: outputTriplet(visit, "(", " *= ", ")");          break;
      case EOpVectorTimesMatrixAssign:
        if (visit == PreVisit)
        {
            out << "(";
        }
        else if (visit == InVisit)
        {
            out << " = mul(";
            node->getLeft()->traverse(this);
            out << ", transpose(";
        }
        else
        {
            out << ")))";
        }
        break;
      case EOpMatrixTimesMatrixAssign:
        if (visit == PreVisit)
        {
            out << "(";
        }
        else if (visit == InVisit)
        {
            out << " = transpose(mul(transpose(";
            node->getLeft()->traverse(this);
            out << "), transpose(";
        }
        else
        {
            out << "))))";
        }
        break;
      case EOpDivAssign:               outputTriplet(visit, "(", " /= ", ")");          break;
      case EOpIModAssign:              outputTriplet(visit, "(", " %= ", ")");          break;
      case EOpBitShiftLeftAssign:      outputTriplet(visit, "(", " <<= ", ")");         break;
      case EOpBitShiftRightAssign:     outputTriplet(visit, "(", " >>= ", ")");         break;
      case EOpBitwiseAndAssign:        outputTriplet(visit, "(", " &= ", ")");          break;
      case EOpBitwiseXorAssign:        outputTriplet(visit, "(", " ^= ", ")");          break;
      case EOpBitwiseOrAssign:         outputTriplet(visit, "(", " |= ", ")");          break;
      case EOpIndexDirect:
        {
            const TType& leftType = node->getLeft()->getType();
            if (leftType.isInterfaceBlock())
            {
                if (visit == PreVisit)
                {
                    TInterfaceBlock* interfaceBlock = leftType.getInterfaceBlock();
                    const int arrayIndex = node->getRight()->getAsConstantUnion()->getIConst(0);
                    mReferencedInterfaceBlocks[interfaceBlock->instanceName()] = node->getLeft()->getAsSymbolNode();
                    out << mUniformHLSL->interfaceBlockInstanceString(*interfaceBlock, arrayIndex);
                    return false;
                }
            }
            else
            {
                outputTriplet(visit, "", "[", "]");
            }
        }
        break;
      case EOpIndexIndirect:
        // We do not currently support indirect references to interface blocks
        ASSERT(node->getLeft()->getBasicType() != EbtInterfaceBlock);
        outputTriplet(visit, "", "[", "]");
        break;
      case EOpIndexDirectStruct:
        if (visit == InVisit)
        {
            const TStructure* structure = node->getLeft()->getType().getStruct();
            const TIntermConstantUnion* index = node->getRight()->getAsConstantUnion();
            const TField* field = structure->fields()[index->getIConst(0)];
            out << "." + DecorateField(field->name(), *structure);

            return false;
        }
        break;
      case EOpIndexDirectInterfaceBlock:
        if (visit == InVisit)
        {
            const TInterfaceBlock* interfaceBlock = node->getLeft()->getType().getInterfaceBlock();
            const TIntermConstantUnion* index = node->getRight()->getAsConstantUnion();
            const TField* field = interfaceBlock->fields()[index->getIConst(0)];
            out << "." + Decorate(field->name());

            return false;
        }
        break;
      case EOpVectorSwizzle:
        if (visit == InVisit)
        {
            out << ".";

            TIntermAggregate *swizzle = node->getRight()->getAsAggregate();

            if (swizzle)
            {
                TIntermSequence *sequence = swizzle->getSequence();

                for (TIntermSequence::iterator sit = sequence->begin(); sit != sequence->end(); sit++)
                {
                    TIntermConstantUnion *element = (*sit)->getAsConstantUnion();

                    if (element)
                    {
                        int i = element->getIConst(0);

                        switch (i)
                        {
                        case 0: out << "x"; break;
                        case 1: out << "y"; break;
                        case 2: out << "z"; break;
                        case 3: out << "w"; break;
                        default: UNREACHABLE();
                        }
                    }
                    else UNREACHABLE();
                }
            }
            else UNREACHABLE();

            return false;   // Fully processed
        }
        break;
      case EOpAdd:               outputTriplet(visit, "(", " + ", ")"); break;
      case EOpSub:               outputTriplet(visit, "(", " - ", ")"); break;
      case EOpMul:               outputTriplet(visit, "(", " * ", ")"); break;
      case EOpDiv:               outputTriplet(visit, "(", " / ", ")"); break;
      case EOpIMod:              outputTriplet(visit, "(", " % ", ")"); break;
      case EOpBitShiftLeft:      outputTriplet(visit, "(", " << ", ")"); break;
      case EOpBitShiftRight:     outputTriplet(visit, "(", " >> ", ")"); break;
      case EOpBitwiseAnd:        outputTriplet(visit, "(", " & ", ")"); break;
      case EOpBitwiseXor:        outputTriplet(visit, "(", " ^ ", ")"); break;
      case EOpBitwiseOr:         outputTriplet(visit, "(", " | ", ")"); break;
      case EOpEqual:
      case EOpNotEqual:
        outputEqual(visit, node->getLeft()->getType(), node->getOp(), out);
        break;
      case EOpLessThan:          outputTriplet(visit, "(", " < ", ")");   break;
      case EOpGreaterThan:       outputTriplet(visit, "(", " > ", ")");   break;
      case EOpLessThanEqual:     outputTriplet(visit, "(", " <= ", ")");  break;
      case EOpGreaterThanEqual:  outputTriplet(visit, "(", " >= ", ")");  break;
      case EOpVectorTimesScalar: outputTriplet(visit, "(", " * ", ")");   break;
      case EOpMatrixTimesScalar: outputTriplet(visit, "(", " * ", ")");   break;
      case EOpVectorTimesMatrix: outputTriplet(visit, "mul(", ", transpose(", "))"); break;
      case EOpMatrixTimesVector: outputTriplet(visit, "mul(transpose(", "), ", ")"); break;
      case EOpMatrixTimesMatrix: outputTriplet(visit, "transpose(mul(transpose(", "), transpose(", ")))"); break;
      case EOpLogicalOr:
        // HLSL doesn't short-circuit ||, so we assume that || affected by short-circuiting have been unfolded.
        ASSERT(!node->getRight()->hasSideEffects());
        outputTriplet(visit, "(", " || ", ")");
        return true;
      case EOpLogicalXor:
        mUsesXor = true;
        outputTriplet(visit, "xor(", ", ", ")");
        break;
      case EOpLogicalAnd:
        // HLSL doesn't short-circuit &&, so we assume that && affected by short-circuiting have been unfolded.
        ASSERT(!node->getRight()->hasSideEffects());
        outputTriplet(visit, "(", " && ", ")");
        return true;
      default: UNREACHABLE();
    }

    return true;
}

bool OutputHLSL::visitUnary(Visit visit, TIntermUnary *node)
{
    switch (node->getOp())
    {
      case EOpNegative:         outputTriplet(visit, "(-", "", ")");         break;
      case EOpPositive:         outputTriplet(visit, "(+", "", ")");         break;
      case EOpVectorLogicalNot: outputTriplet(visit, "(!", "", ")");         break;
      case EOpLogicalNot:       outputTriplet(visit, "(!", "", ")");         break;
      case EOpBitwiseNot:       outputTriplet(visit, "(~", "", ")");         break;
      case EOpPostIncrement:    outputTriplet(visit, "(", "", "++)");        break;
      case EOpPostDecrement:    outputTriplet(visit, "(", "", "--)");        break;
      case EOpPreIncrement:     outputTriplet(visit, "(++", "", ")");        break;
      case EOpPreDecrement:     outputTriplet(visit, "(--", "", ")");        break;
      case EOpRadians:          outputTriplet(visit, "radians(", "", ")");   break;
      case EOpDegrees:          outputTriplet(visit, "degrees(", "", ")");   break;
      case EOpSin:              outputTriplet(visit, "sin(", "", ")");       break;
      case EOpCos:              outputTriplet(visit, "cos(", "", ")");       break;
      case EOpTan:              outputTriplet(visit, "tan(", "", ")");       break;
      case EOpAsin:             outputTriplet(visit, "asin(", "", ")");      break;
      case EOpAcos:             outputTriplet(visit, "acos(", "", ")");      break;
      case EOpAtan:             outputTriplet(visit, "atan(", "", ")");      break;
      case EOpSinh:             outputTriplet(visit, "sinh(", "", ")");      break;
      case EOpCosh:             outputTriplet(visit, "cosh(", "", ")");      break;
      case EOpTanh:             outputTriplet(visit, "tanh(", "", ")");      break;
      case EOpAsinh:
        ASSERT(node->getUseEmulatedFunction());
        writeEmulatedFunctionTriplet(visit, "asinh(");
        break;
      case EOpAcosh:
        ASSERT(node->getUseEmulatedFunction());
        writeEmulatedFunctionTriplet(visit, "acosh(");
        break;
      case EOpAtanh:
        ASSERT(node->getUseEmulatedFunction());
        writeEmulatedFunctionTriplet(visit, "atanh(");
        break;
      case EOpExp:              outputTriplet(visit, "exp(", "", ")");       break;
      case EOpLog:              outputTriplet(visit, "log(", "", ")");       break;
      case EOpExp2:             outputTriplet(visit, "exp2(", "", ")");      break;
      case EOpLog2:             outputTriplet(visit, "log2(", "", ")");      break;
      case EOpSqrt:             outputTriplet(visit, "sqrt(", "", ")");      break;
      case EOpInverseSqrt:      outputTriplet(visit, "rsqrt(", "", ")");     break;
      case EOpAbs:              outputTriplet(visit, "abs(", "", ")");       break;
      case EOpSign:             outputTriplet(visit, "sign(", "", ")");      break;
      case EOpFloor:            outputTriplet(visit, "floor(", "", ")");     break;
      case EOpTrunc:            outputTriplet(visit, "trunc(", "", ")");     break;
      case EOpRound:            outputTriplet(visit, "round(", "", ")");     break;
      case EOpRoundEven:
        ASSERT(node->getUseEmulatedFunction());
        writeEmulatedFunctionTriplet(visit, "roundEven(");
        break;
      case EOpCeil:             outputTriplet(visit, "ceil(", "", ")");      break;
      case EOpFract:            outputTriplet(visit, "frac(", "", ")");      break;
      case EOpIsNan:
        outputTriplet(visit, "isnan(", "", ")");
        mRequiresIEEEStrictCompiling = true;
        break;
      case EOpIsInf:            outputTriplet(visit, "isinf(", "", ")");     break;
      case EOpFloatBitsToInt:   outputTriplet(visit, "asint(", "", ")");     break;
      case EOpFloatBitsToUint:  outputTriplet(visit, "asuint(", "", ")");    break;
      case EOpIntBitsToFloat:   outputTriplet(visit, "asfloat(", "", ")");   break;
      case EOpUintBitsToFloat:  outputTriplet(visit, "asfloat(", "", ")");   break;
      case EOpPackSnorm2x16:
        ASSERT(node->getUseEmulatedFunction());
        writeEmulatedFunctionTriplet(visit, "packSnorm2x16(");
        break;
      case EOpPackUnorm2x16:
        ASSERT(node->getUseEmulatedFunction());
        writeEmulatedFunctionTriplet(visit, "packUnorm2x16(");
        break;
      case EOpPackHalf2x16:
        ASSERT(node->getUseEmulatedFunction());
        writeEmulatedFunctionTriplet(visit, "packHalf2x16(");
        break;
      case EOpUnpackSnorm2x16:
        ASSERT(node->getUseEmulatedFunction());
        writeEmulatedFunctionTriplet(visit, "unpackSnorm2x16(");
        break;
      case EOpUnpackUnorm2x16:
        ASSERT(node->getUseEmulatedFunction());
        writeEmulatedFunctionTriplet(visit, "unpackUnorm2x16(");
        break;
      case EOpUnpackHalf2x16:
        ASSERT(node->getUseEmulatedFunction());
        writeEmulatedFunctionTriplet(visit, "unpackHalf2x16(");
        break;
      case EOpLength:           outputTriplet(visit, "length(", "", ")");    break;
      case EOpNormalize:        outputTriplet(visit, "normalize(", "", ")"); break;
      case EOpDFdx:
        if(mInsideDiscontinuousLoop || mOutputLod0Function)
        {
            outputTriplet(visit, "(", "", ", 0.0)");
        }
        else
        {
            outputTriplet(visit, "ddx(", "", ")");
        }
        break;
      case EOpDFdy:
        if(mInsideDiscontinuousLoop || mOutputLod0Function)
        {
            outputTriplet(visit, "(", "", ", 0.0)");
        }
        else
        {
           outputTriplet(visit, "ddy(", "", ")");
        }
        break;
      case EOpFwidth:
        if(mInsideDiscontinuousLoop || mOutputLod0Function)
        {
            outputTriplet(visit, "(", "", ", 0.0)");
        }
        else
        {
            outputTriplet(visit, "fwidth(", "", ")");
        }
        break;
      case EOpTranspose:        outputTriplet(visit, "transpose(", "", ")");   break;
      case EOpDeterminant:      outputTriplet(visit, "determinant(transpose(", "", "))"); break;
      case EOpInverse:
        ASSERT(node->getUseEmulatedFunction());
        writeEmulatedFunctionTriplet(visit, "inverse(");
        break;

      case EOpAny:              outputTriplet(visit, "any(", "", ")");       break;
      case EOpAll:              outputTriplet(visit, "all(", "", ")");       break;
      default: UNREACHABLE();
    }

    return true;
}

bool OutputHLSL::visitAggregate(Visit visit, TIntermAggregate *node)
{
    TInfoSinkBase &out = getInfoSink();

    switch (node->getOp())
    {
      case EOpSequence:
        {
            if (mInsideFunction)
            {
                outputLineDirective(node->getLine().first_line);
                out << "{\n";
            }

            for (TIntermSequence::iterator sit = node->getSequence()->begin(); sit != node->getSequence()->end(); sit++)
            {
                outputLineDirective((*sit)->getLine().first_line);

                (*sit)->traverse(this);

                // Don't output ; after case labels, they're terminated by :
                // This is needed especially since outputting a ; after a case statement would turn empty
                // case statements into non-empty case statements, disallowing fall-through from them.
                // Also no need to output ; after selection (if) statements or sequences. This is done just
                // for code clarity.
                TIntermSelection *asSelection = (*sit)->getAsSelectionNode();
                ASSERT(asSelection == nullptr || !asSelection->usesTernaryOperator());
                if ((*sit)->getAsCaseNode() == nullptr && asSelection == nullptr && !IsSequence(*sit))
                    out << ";\n";
            }

            if (mInsideFunction)
            {
                outputLineDirective(node->getLine().last_line);
                out << "}\n";
            }

            return false;
        }
      case EOpDeclaration:
        if (visit == PreVisit)
        {
            TIntermSequence *sequence = node->getSequence();
            TIntermTyped *variable = (*sequence)[0]->getAsTyped();
            ASSERT(sequence->size() == 1);

            if (variable && (variable->getQualifier() == EvqTemporary || variable->getQualifier() == EvqGlobal))
            {
                ensureStructDefined(variable->getType());

                if (!variable->getAsSymbolNode() || variable->getAsSymbolNode()->getSymbol() != "")   // Variable declaration
                {
                    if (!mInsideFunction)
                    {
                        out << "static ";
                    }

                    out << TypeString(variable->getType()) + " ";

                    TIntermSymbol *symbol = variable->getAsSymbolNode();

                    if (symbol)
                    {
                        symbol->traverse(this);
                        out << ArrayString(symbol->getType());
                        out << " = " + initializer(symbol->getType());
                    }
                    else
                    {
                        variable->traverse(this);
                    }
                }
                else if (variable->getAsSymbolNode() && variable->getAsSymbolNode()->getSymbol() == "")   // Type (struct) declaration
                {
                    // Already added to constructor map
                }
                else UNREACHABLE();
            }
            else if (variable && IsVaryingOut(variable->getQualifier()))
            {
                for (TIntermSequence::iterator sit = sequence->begin(); sit != sequence->end(); sit++)
                {
                    TIntermSymbol *symbol = (*sit)->getAsSymbolNode();

                    if (symbol)
                    {
                        // Vertex (output) varyings which are declared but not written to should still be declared to allow successful linking
                        mReferencedVaryings[symbol->getSymbol()] = symbol;
                    }
                    else
                    {
                        (*sit)->traverse(this);
                    }
                }
            }

            return false;
        }
        else if (visit == InVisit)
        {
            out << ", ";
        }
        break;
      case EOpInvariantDeclaration:
        // Do not do any translation
        return false;
      case EOpPrototype:
        if (visit == PreVisit)
        {
            size_t index = mCallDag.findIndex(node);
            // Skip the prototype if it is not implemented (and thus not used)
            if (index == CallDAG::InvalidIndex)
            {
                return false;
            }

            TString name = DecorateFunctionIfNeeded(node->getNameObj());
            out << TypeString(node->getType()) << " " << name
                << (mOutputLod0Function ? "Lod0(" : "(");

            TIntermSequence *arguments = node->getSequence();

            for (unsigned int i = 0; i < arguments->size(); i++)
            {
                TIntermSymbol *symbol = (*arguments)[i]->getAsSymbolNode();

                if (symbol)
                {
                    out << argumentString(symbol);

                    if (i < arguments->size() - 1)
                    {
                        out << ", ";
                    }
                }
                else UNREACHABLE();
            }

            out << ");\n";

            // Also prototype the Lod0 variant if needed
            bool needsLod0 = mASTMetadataList[index].mNeedsLod0;
            if (needsLod0 && !mOutputLod0Function && mShaderType == GL_FRAGMENT_SHADER)
            {
                mOutputLod0Function = true;
                node->traverse(this);
                mOutputLod0Function = false;
            }

            return false;
        }
        break;
      case EOpComma:            outputTriplet(visit, "(", ", ", ")");                break;
      case EOpFunction:
        {
            ASSERT(mCurrentFunctionMetadata == nullptr);
            TString name = TFunction::unmangleName(node->getNameObj().getString());

            size_t index = mCallDag.findIndex(node);
            ASSERT(index != CallDAG::InvalidIndex);
            mCurrentFunctionMetadata = &mASTMetadataList[index];

            out << TypeString(node->getType()) << " ";

            if (name == "main")
            {
                out << "gl_main(";
            }
            else
            {
                out << DecorateFunctionIfNeeded(node->getNameObj())
                    << (mOutputLod0Function ? "Lod0(" : "(");
            }

            TIntermSequence *sequence = node->getSequence();
            TIntermSequence *arguments = (*sequence)[0]->getAsAggregate()->getSequence();

            for (unsigned int i = 0; i < arguments->size(); i++)
            {
                TIntermSymbol *symbol = (*arguments)[i]->getAsSymbolNode();

                if (symbol)
                {
                    ensureStructDefined(symbol->getType());

                    out << argumentString(symbol);

                    if (i < arguments->size() - 1)
                    {
                        out << ", ";
                    }
                }
                else UNREACHABLE();
            }

            out << ")\n";

            if (sequence->size() > 1)
            {
                mInsideFunction = true;
                TIntermNode *body = (*sequence)[1];
                // The function body node will output braces.
                ASSERT(IsSequence(body));
                body->traverse(this);
                mInsideFunction = false;
            }
            else
            {
                out << "{}\n";
            }

            mCurrentFunctionMetadata = nullptr;

            bool needsLod0 = mASTMetadataList[index].mNeedsLod0;
            if (needsLod0 && !mOutputLod0Function && mShaderType == GL_FRAGMENT_SHADER)
            {
                ASSERT(name != "main");
                mOutputLod0Function = true;
                node->traverse(this);
                mOutputLod0Function = false;
            }

            return false;
        }
        break;
      case EOpFunctionCall:
        {
            TIntermSequence *arguments = node->getSequence();

            bool lod0 = mInsideDiscontinuousLoop || mOutputLod0Function;
            if (node->isUserDefined())
            {
                if (node->isArray())
                {
                    UNIMPLEMENTED();
                }
                size_t index = mCallDag.findIndex(node);
                ASSERT(index != CallDAG::InvalidIndex);
                lod0 &= mASTMetadataList[index].mNeedsLod0;

                out << DecorateFunctionIfNeeded(node->getNameObj()) << (lod0 ? "Lod0(" : "(");
            }
            else
            {
                TString name           = TFunction::unmangleName(node->getNameObj().getString());
                TBasicType samplerType = (*arguments)[0]->getAsTyped()->getType().getBasicType();

                TextureFunction textureFunction;
                textureFunction.sampler = samplerType;
                textureFunction.coords = (*arguments)[1]->getAsTyped()->getNominalSize();
                textureFunction.method = TextureFunction::IMPLICIT;
                textureFunction.proj = false;
                textureFunction.offset = false;

                if (name == "texture2D" || name == "textureCube" || name == "texture")
                {
                    textureFunction.method = TextureFunction::IMPLICIT;
                }
                else if (name == "texture2DProj" || name == "textureProj")
                {
                    textureFunction.method = TextureFunction::IMPLICIT;
                    textureFunction.proj = true;
                }
                else if (name == "texture2DLod" || name == "textureCubeLod" || name == "textureLod" ||
                         name == "texture2DLodEXT" || name == "textureCubeLodEXT")
                {
                    textureFunction.method = TextureFunction::LOD;
                }
                else if (name == "texture2DProjLod" || name == "textureProjLod" || name == "texture2DProjLodEXT")
                {
                    textureFunction.method = TextureFunction::LOD;
                    textureFunction.proj = true;
                }
                else if (name == "textureSize")
                {
                    textureFunction.method = TextureFunction::SIZE;
                }
                else if (name == "textureOffset")
                {
                    textureFunction.method = TextureFunction::IMPLICIT;
                    textureFunction.offset = true;
                }
                else if (name == "textureProjOffset")
                {
                    textureFunction.method = TextureFunction::IMPLICIT;
                    textureFunction.offset = true;
                    textureFunction.proj = true;
                }
                else if (name == "textureLodOffset")
                {
                    textureFunction.method = TextureFunction::LOD;
                    textureFunction.offset = true;
                }
                else if (name == "textureProjLodOffset")
                {
                    textureFunction.method = TextureFunction::LOD;
                    textureFunction.proj = true;
                    textureFunction.offset = true;
                }
                else if (name == "texelFetch")
                {
                    textureFunction.method = TextureFunction::FETCH;
                }
                else if (name == "texelFetchOffset")
                {
                    textureFunction.method = TextureFunction::FETCH;
                    textureFunction.offset = true;
                }
                else if (name == "textureGrad" || name == "texture2DGradEXT")
                {
                    textureFunction.method = TextureFunction::GRAD;
                }
                else if (name == "textureGradOffset")
                {
                    textureFunction.method = TextureFunction::GRAD;
                    textureFunction.offset = true;
                }
                else if (name == "textureProjGrad" || name == "texture2DProjGradEXT" || name == "textureCubeGradEXT")
                {
                    textureFunction.method = TextureFunction::GRAD;
                    textureFunction.proj = true;
                }
                else if (name == "textureProjGradOffset")
                {
                    textureFunction.method = TextureFunction::GRAD;
                    textureFunction.proj = true;
                    textureFunction.offset = true;
                }
                else UNREACHABLE();

                if (textureFunction.method == TextureFunction::IMPLICIT)   // Could require lod 0 or have a bias argument
                {
                    unsigned int mandatoryArgumentCount = 2;   // All functions have sampler and coordinate arguments

                    if (textureFunction.offset)
                    {
                        mandatoryArgumentCount++;
                    }

                    bool bias = (arguments->size() > mandatoryArgumentCount);   // Bias argument is optional

                    if (lod0 || mShaderType == GL_VERTEX_SHADER)
                    {
                        if (bias)
                        {
                            textureFunction.method = TextureFunction::LOD0BIAS;
                        }
                        else
                        {
                            textureFunction.method = TextureFunction::LOD0;
                        }
                    }
                    else if (bias)
                    {
                        textureFunction.method = TextureFunction::BIAS;
                    }
                }

                mUsesTexture.insert(textureFunction);

                out << textureFunction.name();
            }

            for (TIntermSequence::iterator arg = arguments->begin(); arg != arguments->end(); arg++)
            {
                if (mOutputType == SH_HLSL11_OUTPUT && IsSampler((*arg)->getAsTyped()->getBasicType()))
                {
                    out << "texture_";
                    (*arg)->traverse(this);
                    out << ", sampler_";
                }

                (*arg)->traverse(this);

                if (arg < arguments->end() - 1)
                {
                    out << ", ";
                }
            }

            out << ")";

            return false;
        }
        break;
      case EOpParameters:       outputTriplet(visit, "(", ", ", ")\n{\n");                                break;
      case EOpConstructFloat:   outputConstructor(visit, node->getType(), "vec1", node->getSequence());  break;
      case EOpConstructVec2:    outputConstructor(visit, node->getType(), "vec2", node->getSequence());  break;
      case EOpConstructVec3:    outputConstructor(visit, node->getType(), "vec3", node->getSequence());  break;
      case EOpConstructVec4:    outputConstructor(visit, node->getType(), "vec4", node->getSequence());  break;
      case EOpConstructBool:    outputConstructor(visit, node->getType(), "bvec1", node->getSequence()); break;
      case EOpConstructBVec2:   outputConstructor(visit, node->getType(), "bvec2", node->getSequence()); break;
      case EOpConstructBVec3:   outputConstructor(visit, node->getType(), "bvec3", node->getSequence()); break;
      case EOpConstructBVec4:   outputConstructor(visit, node->getType(), "bvec4", node->getSequence()); break;
      case EOpConstructInt:     outputConstructor(visit, node->getType(), "ivec1", node->getSequence()); break;
      case EOpConstructIVec2:   outputConstructor(visit, node->getType(), "ivec2", node->getSequence()); break;
      case EOpConstructIVec3:   outputConstructor(visit, node->getType(), "ivec3", node->getSequence()); break;
      case EOpConstructIVec4:   outputConstructor(visit, node->getType(), "ivec4", node->getSequence()); break;
      case EOpConstructUInt:    outputConstructor(visit, node->getType(), "uvec1", node->getSequence()); break;
      case EOpConstructUVec2:   outputConstructor(visit, node->getType(), "uvec2", node->getSequence()); break;
      case EOpConstructUVec3:   outputConstructor(visit, node->getType(), "uvec3", node->getSequence()); break;
      case EOpConstructUVec4:   outputConstructor(visit, node->getType(), "uvec4", node->getSequence()); break;
      case EOpConstructMat2:    outputConstructor(visit, node->getType(), "mat2", node->getSequence());  break;
      case EOpConstructMat2x3:  outputConstructor(visit, node->getType(), "mat2x3", node->getSequence());  break;
      case EOpConstructMat2x4:  outputConstructor(visit, node->getType(), "mat2x4", node->getSequence());  break;
      case EOpConstructMat3x2:  outputConstructor(visit, node->getType(), "mat3x2", node->getSequence());  break;
      case EOpConstructMat3:    outputConstructor(visit, node->getType(), "mat3", node->getSequence());  break;
      case EOpConstructMat3x4:  outputConstructor(visit, node->getType(), "mat3x4", node->getSequence());  break;
      case EOpConstructMat4x2:  outputConstructor(visit, node->getType(), "mat4x2", node->getSequence());  break;
      case EOpConstructMat4x3:  outputConstructor(visit, node->getType(), "mat4x3", node->getSequence());  break;
      case EOpConstructMat4:    outputConstructor(visit, node->getType(), "mat4", node->getSequence());  break;
      case EOpConstructStruct:
        {
            if (node->getType().isArray())
            {
                UNIMPLEMENTED();
            }
            const TString &structName = StructNameString(*node->getType().getStruct());
            mStructureHLSL->addConstructor(node->getType(), structName, node->getSequence());
            outputTriplet(visit, (structName + "_ctor(").c_str(), ", ", ")");
        }
        break;
      case EOpLessThan:         outputTriplet(visit, "(", " < ", ")");                 break;
      case EOpGreaterThan:      outputTriplet(visit, "(", " > ", ")");                 break;
      case EOpLessThanEqual:    outputTriplet(visit, "(", " <= ", ")");                break;
      case EOpGreaterThanEqual: outputTriplet(visit, "(", " >= ", ")");                break;
      case EOpVectorEqual:      outputTriplet(visit, "(", " == ", ")");                break;
      case EOpVectorNotEqual:   outputTriplet(visit, "(", " != ", ")");                break;
      case EOpMod:
        ASSERT(node->getUseEmulatedFunction());
        writeEmulatedFunctionTriplet(visit, "mod(");
        break;
      case EOpModf:             outputTriplet(visit, "modf(", ", ", ")");              break;
      case EOpPow:              outputTriplet(visit, "pow(", ", ", ")");               break;
      case EOpAtan:
        ASSERT(node->getSequence()->size() == 2);   // atan(x) is a unary operator
        ASSERT(node->getUseEmulatedFunction());
        writeEmulatedFunctionTriplet(visit, "atan(");
        break;
      case EOpMin:           outputTriplet(visit, "min(", ", ", ")");           break;
      case EOpMax:           outputTriplet(visit, "max(", ", ", ")");           break;
      case EOpClamp:         outputTriplet(visit, "clamp(", ", ", ")");         break;
      case EOpMix:
        {
            TIntermTyped *lastParamNode = (*(node->getSequence()))[2]->getAsTyped();
            if (lastParamNode->getType().getBasicType() == EbtBool)
            {
                // There is no HLSL equivalent for ESSL3 built-in "genType mix (genType x, genType y, genBType a)",
                // so use emulated version.
                ASSERT(node->getUseEmulatedFunction());
                writeEmulatedFunctionTriplet(visit, "mix(");
            }
            else
            {
                outputTriplet(visit, "lerp(", ", ", ")");
            }
        }
        break;
      case EOpStep:          outputTriplet(visit, "step(", ", ", ")");          break;
      case EOpSmoothStep:    outputTriplet(visit, "smoothstep(", ", ", ")");    break;
      case EOpDistance:      outputTriplet(visit, "distance(", ", ", ")");      break;
      case EOpDot:           outputTriplet(visit, "dot(", ", ", ")");           break;
      case EOpCross:         outputTriplet(visit, "cross(", ", ", ")");         break;
      case EOpFaceForward:
        ASSERT(node->getUseEmulatedFunction());
        writeEmulatedFunctionTriplet(visit, "faceforward(");
        break;
      case EOpReflect:       outputTriplet(visit, "reflect(", ", ", ")");       break;
      case EOpRefract:       outputTriplet(visit, "refract(", ", ", ")");       break;
      case EOpOuterProduct:
        ASSERT(node->getUseEmulatedFunction());
        writeEmulatedFunctionTriplet(visit, "outerProduct(");
        break;
      case EOpMul:           outputTriplet(visit, "(", " * ", ")");             break;
      default: UNREACHABLE();
    }

    return true;
}

void OutputHLSL::writeSelection(TIntermSelection *node)
{
    TInfoSinkBase &out = getInfoSink();

    out << "if (";

    node->getCondition()->traverse(this);

    out << ")\n";

    outputLineDirective(node->getLine().first_line);

    bool discard = false;

    if (node->getTrueBlock())
    {
        // The trueBlock child node will output braces.
        ASSERT(IsSequence(node->getTrueBlock()));

        node->getTrueBlock()->traverse(this);

        // Detect true discard
        discard = (discard || FindDiscard::search(node->getTrueBlock()));
    }
    else
    {
        // TODO(oetuaho): Check if the semicolon inside is necessary.
        // It's there as a result of conservative refactoring of the output.
        out << "{;}\n";
    }

    outputLineDirective(node->getLine().first_line);

    if (node->getFalseBlock())
    {
        out << "else\n";

        outputLineDirective(node->getFalseBlock()->getLine().first_line);

        // Either this is "else if" or the falseBlock child node will output braces.
        ASSERT(IsSequence(node->getFalseBlock()) || node->getFalseBlock()->getAsSelectionNode() != nullptr);

        node->getFalseBlock()->traverse(this);

        outputLineDirective(node->getFalseBlock()->getLine().first_line);

        // Detect false discard
        discard = (discard || FindDiscard::search(node->getFalseBlock()));
    }

    // ANGLE issue 486: Detect problematic conditional discard
    if (discard)
    {
        mUsesDiscardRewriting = true;
    }
}

bool OutputHLSL::visitSelection(Visit visit, TIntermSelection *node)
{
    TInfoSinkBase &out = getInfoSink();

    ASSERT(!node->usesTernaryOperator());

    if (!mInsideFunction)
    {
        // This is part of unfolded global initialization.
        mDeferredGlobalInitializers.push_back(node);
        return false;
    }

    // D3D errors when there is a gradient operation in a loop in an unflattened if.
    if (mShaderType == GL_FRAGMENT_SHADER && mCurrentFunctionMetadata->hasGradientLoop(node))
    {
        out << "FLATTEN ";
    }

    writeSelection(node);

    return false;
}

bool OutputHLSL::visitSwitch(Visit visit, TIntermSwitch *node)
{
    if (node->getStatementList())
    {
        node->setStatementList(RemoveSwitchFallThrough::removeFallThrough(node->getStatementList()));
        outputTriplet(visit, "switch (", ") ", "");
        // The curly braces get written when visiting the statementList aggregate
    }
    else
    {
        // No statementList, so it won't output curly braces
        outputTriplet(visit, "switch (", ") {", "}\n");
    }
    return true;
}

bool OutputHLSL::visitCase(Visit visit, TIntermCase *node)
{
    if (node->hasCondition())
    {
        outputTriplet(visit, "case (", "", "):\n");
        return true;
    }
    else
    {
        TInfoSinkBase &out = getInfoSink();
        out << "default:\n";
        return false;
    }
}

void OutputHLSL::visitConstantUnion(TIntermConstantUnion *node)
{
    writeConstantUnion(node->getType(), node->getUnionArrayPointer());
}

bool OutputHLSL::visitLoop(Visit visit, TIntermLoop *node)
{
    mNestedLoopDepth++;

    bool wasDiscontinuous = mInsideDiscontinuousLoop;
    mInsideDiscontinuousLoop = mInsideDiscontinuousLoop ||
    mCurrentFunctionMetadata->mDiscontinuousLoops.count(node) > 0;

    if (mOutputType == SH_HLSL9_OUTPUT)
    {
        if (handleExcessiveLoop(node))
        {
            mInsideDiscontinuousLoop = wasDiscontinuous;
            mNestedLoopDepth--;

            return false;
        }
    }

    TInfoSinkBase &out = getInfoSink();

    const char *unroll = mCurrentFunctionMetadata->hasGradientInCallGraph(node) ? "LOOP" : "";
    if (node->getType() == ELoopDoWhile)
    {
        out << "{" << unroll << " do\n";

        outputLineDirective(node->getLine().first_line);
    }
    else
    {
        out << "{" << unroll << " for(";

        if (node->getInit())
        {
            node->getInit()->traverse(this);
        }

        out << "; ";

        if (node->getCondition())
        {
            node->getCondition()->traverse(this);
        }

        out << "; ";

        if (node->getExpression())
        {
            node->getExpression()->traverse(this);
        }

        out << ")\n";

        outputLineDirective(node->getLine().first_line);
    }

    if (node->getBody())
    {
        // The loop body node will output braces.
        ASSERT(IsSequence(node->getBody()));
        node->getBody()->traverse(this);
    }
    else
    {
        // TODO(oetuaho): Check if the semicolon inside is necessary.
        // It's there as a result of conservative refactoring of the output.
        out << "{;}\n";
    }

    outputLineDirective(node->getLine().first_line);

    if (node->getType() == ELoopDoWhile)
    {
        outputLineDirective(node->getCondition()->getLine().first_line);
        out << "while(\n";

        node->getCondition()->traverse(this);

        out << ");";
    }

    out << "}\n";

    mInsideDiscontinuousLoop = wasDiscontinuous;
    mNestedLoopDepth--;

    return false;
}

bool OutputHLSL::visitBranch(Visit visit, TIntermBranch *node)
{
    TInfoSinkBase &out = getInfoSink();

    switch (node->getFlowOp())
    {
      case EOpKill:
        outputTriplet(visit, "discard;\n", "", "");
        break;
      case EOpBreak:
        if (visit == PreVisit)
        {
            if (mNestedLoopDepth > 1)
            {
                mUsesNestedBreak = true;
            }

            if (mExcessiveLoopIndex)
            {
                out << "{Break";
                mExcessiveLoopIndex->traverse(this);
                out << " = true; break;}\n";
            }
            else
            {
                out << "break;\n";
            }
        }
        break;
      case EOpContinue: outputTriplet(visit, "continue;\n", "", ""); break;
      case EOpReturn:
        if (visit == PreVisit)
        {
            if (node->getExpression())
            {
                out << "return ";
            }
            else
            {
                out << "return;\n";
            }
        }
        else if (visit == PostVisit)
        {
            if (node->getExpression())
            {
                out << ";\n";
            }
        }
        break;
      default: UNREACHABLE();
    }

    return true;
}

bool OutputHLSL::isSingleStatement(TIntermNode *node)
{
    TIntermAggregate *aggregate = node->getAsAggregate();

    if (aggregate)
    {
        if (aggregate->getOp() == EOpSequence)
        {
            return false;
        }
        else if (aggregate->getOp() == EOpDeclaration)
        {
            // Declaring multiple comma-separated variables must be considered multiple statements
            // because each individual declaration has side effects which are visible in the next.
            return false;
        }
        else
        {
            for (TIntermSequence::iterator sit = aggregate->getSequence()->begin(); sit != aggregate->getSequence()->end(); sit++)
            {
                if (!isSingleStatement(*sit))
                {
                    return false;
                }
            }

            return true;
        }
    }

    return true;
}

// Handle loops with more than 254 iterations (unsupported by D3D9) by splitting them
// (The D3D documentation says 255 iterations, but the compiler complains at anything more than 254).
bool OutputHLSL::handleExcessiveLoop(TIntermLoop *node)
{
    const int MAX_LOOP_ITERATIONS = 254;
    TInfoSinkBase &out = getInfoSink();

    // Parse loops of the form:
    // for(int index = initial; index [comparator] limit; index += increment)
    TIntermSymbol *index = NULL;
    TOperator comparator = EOpNull;
    int initial = 0;
    int limit = 0;
    int increment = 0;

    // Parse index name and intial value
    if (node->getInit())
    {
        TIntermAggregate *init = node->getInit()->getAsAggregate();

        if (init)
        {
            TIntermSequence *sequence = init->getSequence();
            TIntermTyped *variable = (*sequence)[0]->getAsTyped();

            if (variable && variable->getQualifier() == EvqTemporary)
            {
                TIntermBinary *assign = variable->getAsBinaryNode();

                if (assign->getOp() == EOpInitialize)
                {
                    TIntermSymbol *symbol = assign->getLeft()->getAsSymbolNode();
                    TIntermConstantUnion *constant = assign->getRight()->getAsConstantUnion();

                    if (symbol && constant)
                    {
                        if (constant->getBasicType() == EbtInt && constant->isScalar())
                        {
                            index = symbol;
                            initial = constant->getIConst(0);
                        }
                    }
                }
            }
        }
    }

    // Parse comparator and limit value
    if (index != NULL && node->getCondition())
    {
        TIntermBinary *test = node->getCondition()->getAsBinaryNode();

        if (test && test->getLeft()->getAsSymbolNode()->getId() == index->getId())
        {
            TIntermConstantUnion *constant = test->getRight()->getAsConstantUnion();

            if (constant)
            {
                if (constant->getBasicType() == EbtInt && constant->isScalar())
                {
                    comparator = test->getOp();
                    limit = constant->getIConst(0);
                }
            }
        }
    }

    // Parse increment
    if (index != NULL && comparator != EOpNull && node->getExpression())
    {
        TIntermBinary *binaryTerminal = node->getExpression()->getAsBinaryNode();
        TIntermUnary *unaryTerminal = node->getExpression()->getAsUnaryNode();

        if (binaryTerminal)
        {
            TOperator op = binaryTerminal->getOp();
            TIntermConstantUnion *constant = binaryTerminal->getRight()->getAsConstantUnion();

            if (constant)
            {
                if (constant->getBasicType() == EbtInt && constant->isScalar())
                {
                    int value = constant->getIConst(0);

                    switch (op)
                    {
                      case EOpAddAssign: increment = value;  break;
                      case EOpSubAssign: increment = -value; break;
                      default: UNIMPLEMENTED();
                    }
                }
            }
        }
        else if (unaryTerminal)
        {
            TOperator op = unaryTerminal->getOp();

            switch (op)
            {
              case EOpPostIncrement: increment = 1;  break;
              case EOpPostDecrement: increment = -1; break;
              case EOpPreIncrement:  increment = 1;  break;
              case EOpPreDecrement:  increment = -1; break;
              default: UNIMPLEMENTED();
            }
        }
    }

    if (index != NULL && comparator != EOpNull && increment != 0)
    {
        if (comparator == EOpLessThanEqual)
        {
            comparator = EOpLessThan;
            limit += 1;
        }

        if (comparator == EOpLessThan)
        {
            int iterations = (limit - initial) / increment;

            if (iterations <= MAX_LOOP_ITERATIONS)
            {
                return false;   // Not an excessive loop
            }

            TIntermSymbol *restoreIndex = mExcessiveLoopIndex;
            mExcessiveLoopIndex = index;

            out << "{int ";
            index->traverse(this);
            out << ";\n"
                   "bool Break";
            index->traverse(this);
            out << " = false;\n";

            bool firstLoopFragment = true;

            while (iterations > 0)
            {
                int clampedLimit = initial + increment * std::min(MAX_LOOP_ITERATIONS, iterations);

                if (!firstLoopFragment)
                {
                    out << "if (!Break";
                    index->traverse(this);
                    out << ") {\n";
                }

                if (iterations <= MAX_LOOP_ITERATIONS)   // Last loop fragment
                {
                    mExcessiveLoopIndex = NULL;   // Stops setting the Break flag
                }

                // for(int index = initial; index < clampedLimit; index += increment)
                const char *unroll = mCurrentFunctionMetadata->hasGradientInCallGraph(node) ? "LOOP" : "";

                out << unroll << " for(";
                index->traverse(this);
                out << " = ";
                out << initial;

                out << "; ";
                index->traverse(this);
                out << " < ";
                out << clampedLimit;

                out << "; ";
                index->traverse(this);
                out << " += ";
                out << increment;
                out << ")\n";

                outputLineDirective(node->getLine().first_line);
                out << "{\n";

                if (node->getBody())
                {
                    node->getBody()->traverse(this);
                }

                outputLineDirective(node->getLine().first_line);
                out << ";}\n";

                if (!firstLoopFragment)
                {
                    out << "}\n";
                }

                firstLoopFragment = false;

                initial += MAX_LOOP_ITERATIONS * increment;
                iterations -= MAX_LOOP_ITERATIONS;
            }

            out << "}";

            mExcessiveLoopIndex = restoreIndex;

            return true;
        }
        else UNIMPLEMENTED();
    }

    return false;   // Not handled as an excessive loop
}

void OutputHLSL::outputTriplet(Visit visit, const char *preString, const char *inString, const char *postString, TInfoSinkBase &out)
{
    if (visit == PreVisit)
    {
        out << preString;
    }
    else if (visit == InVisit)
    {
        out << inString;
    }
    else if (visit == PostVisit)
    {
        out << postString;
    }
}

void OutputHLSL::outputTriplet(Visit visit, const char *preString, const char *inString, const char *postString)
{
    outputTriplet(visit, preString, inString, postString, getInfoSink());
}

void OutputHLSL::outputLineDirective(int line)
{
    if ((mCompileOptions & SH_LINE_DIRECTIVES) && (line > 0))
    {
        TInfoSinkBase &out = getInfoSink();

        out << "\n";
        out << "#line " << line;

        if (mSourcePath)
        {
            out << " \"" << mSourcePath << "\"";
        }

        out << "\n";
    }
}

TString OutputHLSL::argumentString(const TIntermSymbol *symbol)
{
    TQualifier qualifier = symbol->getQualifier();
    const TType &type    = symbol->getType();
    const TName &name    = symbol->getName();
    TString nameStr;

    if (name.getString().empty())  // HLSL demands named arguments, also for prototypes
    {
        nameStr = "x" + str(mUniqueIndex++);
    }
    else
    {
        nameStr = DecorateIfNeeded(name);
    }

    if (mOutputType == SH_HLSL11_OUTPUT && IsSampler(type.getBasicType()))
    {
        return QualifierString(qualifier) + " " + TextureString(type) + " texture_" + nameStr +
               ArrayString(type) + ", " + QualifierString(qualifier) + " " + SamplerString(type) +
               " sampler_" + nameStr + ArrayString(type);
    }

    return QualifierString(qualifier) + " " + TypeString(type) + " " + nameStr + ArrayString(type);
}

TString OutputHLSL::initializer(const TType &type)
{
    TString string;

    size_t size = type.getObjectSize();
    for (size_t component = 0; component < size; component++)
    {
        string += "0";

        if (component + 1 < size)
        {
            string += ", ";
        }
    }

    return "{" + string + "}";
}

void OutputHLSL::outputConstructor(Visit visit, const TType &type, const char *name, const TIntermSequence *parameters)
{
    if (type.isArray())
    {
        UNIMPLEMENTED();
    }
    TInfoSinkBase &out = getInfoSink();

    if (visit == PreVisit)
    {
        mStructureHLSL->addConstructor(type, name, parameters);

        out << name << "(";
    }
    else if (visit == InVisit)
    {
        out << ", ";
    }
    else if (visit == PostVisit)
    {
        out << ")";
    }
}

const TConstantUnion *OutputHLSL::writeConstantUnion(const TType &type, const TConstantUnion *constUnion)
{
    TInfoSinkBase &out = getInfoSink();

    const TStructure* structure = type.getStruct();
    if (structure)
    {
        out << StructNameString(*structure) + "_ctor(";

        const TFieldList& fields = structure->fields();

        for (size_t i = 0; i < fields.size(); i++)
        {
            const TType *fieldType = fields[i]->type();
            constUnion = writeConstantUnion(*fieldType, constUnion);

            if (i != fields.size() - 1)
            {
                out << ", ";
            }
        }

        out << ")";
    }
    else
    {
        size_t size = type.getObjectSize();
        bool writeType = size > 1;

        if (writeType)
        {
            out << TypeString(type) << "(";
        }

        for (size_t i = 0; i < size; i++, constUnion++)
        {
            switch (constUnion->getType())
            {
              case EbtFloat: out << std::min(FLT_MAX, std::max(-FLT_MAX, constUnion->getFConst())); break;
              case EbtInt:   out << constUnion->getIConst(); break;
              case EbtUInt:  out << constUnion->getUConst(); break;
              case EbtBool:  out << constUnion->getBConst(); break;
              default: UNREACHABLE();
            }

            if (i != size - 1)
            {
                out << ", ";
            }
        }

        if (writeType)
        {
            out << ")";
        }
    }

    return constUnion;
}

void OutputHLSL::writeEmulatedFunctionTriplet(Visit visit, const char *preStr)
{
    TString preString = BuiltInFunctionEmulator::GetEmulatedFunctionName(preStr);
    outputTriplet(visit, preString.c_str(), ", ", ")");
}

bool OutputHLSL::writeSameSymbolInitializer(TInfoSinkBase &out, TIntermSymbol *symbolNode, TIntermTyped *expression)
{
    sh::SearchSymbol searchSymbol(symbolNode->getSymbol());
    expression->traverse(&searchSymbol);

    if (searchSymbol.foundMatch())
    {
        // Type already printed
        out << "t" + str(mUniqueIndex) + " = ";
        expression->traverse(this);
        out << ", ";
        symbolNode->traverse(this);
        out << " = t" + str(mUniqueIndex);

        mUniqueIndex++;
        return true;
    }

    return false;
}

void OutputHLSL::writeDeferredGlobalInitializers(TInfoSinkBase &out)
{
    out << "#define ANGLE_USES_DEFERRED_INIT\n"
        << "\n"
        << "void initializeDeferredGlobals()\n"
        << "{\n";

    for (const auto &deferredGlobal : mDeferredGlobalInitializers)
    {
        TIntermBinary *binary = deferredGlobal->getAsBinaryNode();
        TIntermSelection *selection = deferredGlobal->getAsSelectionNode();
        if (binary != nullptr)
        {
            TIntermSymbol *symbol = binary->getLeft()->getAsSymbolNode();
            TIntermTyped *expression = binary->getRight();
            ASSERT(symbol);
            ASSERT(symbol->getQualifier() == EvqGlobal && expression->getQualifier() != EvqConst);

            out << "    " << Decorate(symbol->getSymbol()) << " = ";

            if (!writeSameSymbolInitializer(out, symbol, expression))
            {
                ASSERT(mInfoSinkStack.top() == &out);
                expression->traverse(this);
            }
            out << ";\n";
        }
        else if (selection != nullptr)
        {
            ASSERT(mInfoSinkStack.top() == &out);
            writeSelection(selection);
        }
        else
        {
            UNREACHABLE();
        }
    }

    out << "}\n"
        << "\n";
}

TString OutputHLSL::addStructEqualityFunction(const TStructure &structure)
{
    const TFieldList &fields = structure.fields();

    for (const auto &eqFunction : mStructEqualityFunctions)
    {
        if (eqFunction->structure == &structure)
        {
            return eqFunction->functionName;
        }
    }

    const TString &structNameString = StructNameString(structure);

    StructEqualityFunction *function = new StructEqualityFunction();
    function->structure = &structure;
    function->functionName = "angle_eq_" + structNameString;

    TInfoSinkBase fnOut;

    fnOut << "bool " << function->functionName << "(" << structNameString << " a, " << structNameString + " b)\n"
          << "{\n"
             "    return ";

    for (size_t i = 0; i < fields.size(); i++)
    {
        const TField *field = fields[i];
        const TType *fieldType = field->type();

        const TString &fieldNameA = "a." + Decorate(field->name());
        const TString &fieldNameB = "b." + Decorate(field->name());

        if (i > 0)
        {
            fnOut << " && ";
        }

        fnOut << "(";
        outputEqual(PreVisit, *fieldType, EOpEqual, fnOut);
        fnOut << fieldNameA;
        outputEqual(InVisit, *fieldType, EOpEqual, fnOut);
        fnOut << fieldNameB;
        outputEqual(PostVisit, *fieldType, EOpEqual, fnOut);
        fnOut << ")";
    }

    fnOut << ";\n" << "}\n";

    function->functionDefinition = fnOut.c_str();

    mStructEqualityFunctions.push_back(function);
    mEqualityFunctions.push_back(function);

    return function->functionName;
}

TString OutputHLSL::addArrayEqualityFunction(const TType& type)
{
    for (const auto &eqFunction : mArrayEqualityFunctions)
    {
        if (eqFunction->type == type)
        {
            return eqFunction->functionName;
        }
    }

    const TString &typeName = TypeString(type);

    ArrayHelperFunction *function = new ArrayHelperFunction();
    function->type = type;

    TInfoSinkBase fnNameOut;
    fnNameOut << "angle_eq_" << type.getArraySize() << "_" << typeName;
    function->functionName = fnNameOut.c_str();

    TType nonArrayType = type;
    nonArrayType.clearArrayness();

    TInfoSinkBase fnOut;

    fnOut << "bool " << function->functionName << "("
          << typeName << " a[" << type.getArraySize() << "], "
          << typeName << " b[" << type.getArraySize() << "])\n"
          << "{\n"
             "    for (int i = 0; i < " << type.getArraySize() << "; ++i)\n"
             "    {\n"
             "        if (";

    outputEqual(PreVisit, nonArrayType, EOpNotEqual, fnOut);
    fnOut << "a[i]";
    outputEqual(InVisit, nonArrayType, EOpNotEqual, fnOut);
    fnOut << "b[i]";
    outputEqual(PostVisit, nonArrayType, EOpNotEqual, fnOut);

    fnOut << ") { return false; }\n"
             "    }\n"
             "    return true;\n"
             "}\n";

    function->functionDefinition = fnOut.c_str();

    mArrayEqualityFunctions.push_back(function);
    mEqualityFunctions.push_back(function);

    return function->functionName;
}

TString OutputHLSL::addArrayAssignmentFunction(const TType& type)
{
    for (const auto &assignFunction : mArrayAssignmentFunctions)
    {
        if (assignFunction.type == type)
        {
            return assignFunction.functionName;
        }
    }

    const TString &typeName = TypeString(type);

    ArrayHelperFunction function;
    function.type = type;

    TInfoSinkBase fnNameOut;
    fnNameOut << "angle_assign_" << type.getArraySize() << "_" << typeName;
    function.functionName = fnNameOut.c_str();

    TInfoSinkBase fnOut;

    fnOut << "void " << function.functionName << "(out "
        << typeName << " a[" << type.getArraySize() << "], "
        << typeName << " b[" << type.getArraySize() << "])\n"
        << "{\n"
           "    for (int i = 0; i < " << type.getArraySize() << "; ++i)\n"
           "    {\n"
           "        a[i] = b[i];\n"
           "    }\n"
           "}\n";

    function.functionDefinition = fnOut.c_str();

    mArrayAssignmentFunctions.push_back(function);

    return function.functionName;
}

TString OutputHLSL::addArrayConstructIntoFunction(const TType& type)
{
    for (const auto &constructIntoFunction : mArrayConstructIntoFunctions)
    {
        if (constructIntoFunction.type == type)
        {
            return constructIntoFunction.functionName;
        }
    }

    const TString &typeName = TypeString(type);

    ArrayHelperFunction function;
    function.type = type;

    TInfoSinkBase fnNameOut;
    fnNameOut << "angle_construct_into_" << type.getArraySize() << "_" << typeName;
    function.functionName = fnNameOut.c_str();

    TInfoSinkBase fnOut;

    fnOut << "void " << function.functionName << "(out "
          << typeName << " a[" << type.getArraySize() << "]";
    for (int i = 0; i < type.getArraySize(); ++i)
    {
        fnOut << ", " << typeName << " b" << i;
    }
    fnOut << ")\n"
             "{\n";

    for (int i = 0; i < type.getArraySize(); ++i)
    {
        fnOut << "    a[" << i << "] = b" << i << ";\n";
    }
    fnOut << "}\n";

    function.functionDefinition = fnOut.c_str();

    mArrayConstructIntoFunctions.push_back(function);

    return function.functionName;
}

void OutputHLSL::ensureStructDefined(const TType &type)
{
    TStructure *structure = type.getStruct();

    if (structure)
    {
        mStructureHLSL->addConstructor(type, StructNameString(*structure), nullptr);
    }
}



}
