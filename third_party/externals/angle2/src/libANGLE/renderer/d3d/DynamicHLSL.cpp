//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DynamicHLSL.cpp: Implementation for link and run-time HLSL generation
//

#include "libANGLE/renderer/d3d/DynamicHLSL.h"

#include "common/utilities.h"
#include "compiler/translator/blocklayoutHLSL.h"
#include "libANGLE/Program.h"
#include "libANGLE/Shader.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/ShaderD3D.h"

// For use with ArrayString, see angleutils.h
static_assert(GL_INVALID_INDEX == UINT_MAX, "GL_INVALID_INDEX must be equal to the max unsigned int.");

using namespace gl;

namespace rx
{

namespace
{

std::string HLSLComponentTypeString(GLenum componentType)
{
    switch (componentType)
    {
      case GL_UNSIGNED_INT:         return "uint";
      case GL_INT:                  return "int";
      case GL_UNSIGNED_NORMALIZED:
      case GL_SIGNED_NORMALIZED:
      case GL_FLOAT:                return "float";
      default: UNREACHABLE();       return "not-component-type";
    }
}

std::string HLSLComponentTypeString(GLenum componentType, int componentCount)
{
    return HLSLComponentTypeString(componentType) + (componentCount > 1 ? Str(componentCount) : "");
}

std::string HLSLMatrixTypeString(GLenum type)
{
    switch (type)
    {
      case GL_FLOAT_MAT2:     return "float2x2";
      case GL_FLOAT_MAT3:     return "float3x3";
      case GL_FLOAT_MAT4:     return "float4x4";
      case GL_FLOAT_MAT2x3:   return "float2x3";
      case GL_FLOAT_MAT3x2:   return "float3x2";
      case GL_FLOAT_MAT2x4:   return "float2x4";
      case GL_FLOAT_MAT4x2:   return "float4x2";
      case GL_FLOAT_MAT3x4:   return "float3x4";
      case GL_FLOAT_MAT4x3:   return "float4x3";
      default: UNREACHABLE(); return "not-matrix-type";
    }
}

std::string HLSLTypeString(GLenum type)
{
    if (gl::IsMatrixType(type))
    {
        return HLSLMatrixTypeString(type);
    }

    return HLSLComponentTypeString(gl::VariableComponentType(type), gl::VariableComponentCount(type));
}

const PixelShaderOutputVariable *FindOutputAtLocation(const std::vector<PixelShaderOutputVariable> &outputVariables,
                                                        unsigned int location)
{
    for (size_t variableIndex = 0; variableIndex < outputVariables.size(); ++variableIndex)
    {
        if (outputVariables[variableIndex].outputIndex == location)
        {
            return &outputVariables[variableIndex];
        }
    }

    return NULL;
}

typedef const PackedVarying *VaryingPacking[gl::IMPLEMENTATION_MAX_VARYING_VECTORS][4];

bool PackVarying(PackedVarying *packedVarying, const int maxVaryingVectors, VaryingPacking &packing)
{
    // Make sure we use transposed matrix types to count registers correctly.
    int registers = 0;
    int elements = 0;

    const sh::Varying &varying = *packedVarying->varying;

    if (varying.isStruct())
    {
        registers = HLSLVariableRegisterCount(varying, true) * varying.elementCount();
        elements = 4;
    }
    else
    {
        GLenum transposedType = TransposeMatrixType(varying.type);
        registers             = VariableRowCount(transposedType) * varying.elementCount();
        elements = VariableColumnCount(transposedType);
    }

    if (elements >= 2 && elements <= 4)
    {
        for (int r = 0; r <= maxVaryingVectors - registers; r++)
        {
            bool available = true;

            for (int y = 0; y < registers && available; y++)
            {
                for (int x = 0; x < elements && available; x++)
                {
                    if (packing[r + y][x])
                    {
                        available = false;
                    }
                }
            }

            if (available)
            {
                packedVarying->registerIndex = r;
                packedVarying->columnIndex   = 0;

                for (int y = 0; y < registers; y++)
                {
                    for (int x = 0; x < elements; x++)
                    {
                        packing[r + y][x] = packedVarying;
                    }
                }

                return true;
            }
        }

        if (elements == 2)
        {
            for (int r = maxVaryingVectors - registers; r >= 0; r--)
            {
                bool available = true;

                for (int y = 0; y < registers && available; y++)
                {
                    for (int x = 2; x < 4 && available; x++)
                    {
                        if (packing[r + y][x])
                        {
                            available = false;
                        }
                    }
                }

                if (available)
                {
                    packedVarying->registerIndex = r;
                    packedVarying->columnIndex   = 2;

                    for (int y = 0; y < registers; y++)
                    {
                        for (int x = 2; x < 4; x++)
                        {
                            packing[r + y][x] = packedVarying;
                        }
                    }

                    return true;
                }
            }
        }
    }
    else if (elements == 1)
    {
        int space[4] = { 0 };

        for (int y = 0; y < maxVaryingVectors; y++)
        {
            for (int x = 0; x < 4; x++)
            {
                space[x] += packing[y][x] ? 0 : 1;
            }
        }

        int column = 0;

        for (int x = 0; x < 4; x++)
        {
            if (space[x] >= registers && (space[column] < registers || space[x] < space[column]))
            {
                column = x;
            }
        }

        if (space[column] >= registers)
        {
            for (int r = 0; r < maxVaryingVectors; r++)
            {
                if (!packing[r][column])
                {
                    packedVarying->registerIndex = r;
                    packedVarying->columnIndex   = column;

                    for (int y = r; y < r + registers; y++)
                    {
                        packing[y][column] = packedVarying;
                    }

                    break;
                }
            }

            return true;
        }
    }
    else UNREACHABLE();

    return false;
}

const std::string VERTEX_ATTRIBUTE_STUB_STRING = "@@ VERTEX ATTRIBUTES @@";
const std::string PIXEL_OUTPUT_STUB_STRING     = "@@ PIXEL OUTPUT @@";
}

DynamicHLSL::DynamicHLSL(RendererD3D *const renderer) : mRenderer(renderer)
{
}

// Packs varyings into generic varying registers, using the algorithm from [OpenGL ES Shading Language 1.00 rev. 17] appendix A section 7 page 111
// Returns the number of used varying registers, or -1 if unsuccesful
int DynamicHLSL::packVaryings(InfoLog &infoLog,
                              std::vector<PackedVarying> *packedVaryings,
                              const std::vector<std::string> &transformFeedbackVaryings)
{
    // TODO (geofflang):  Use context's caps
    const int maxVaryingVectors = mRenderer->getRendererCaps().maxVaryingVectors;

    VaryingPacking packing = {};

    std::set<std::string> uniqueVaryingNames;

    for (PackedVarying &packedVarying : *packedVaryings)
    {
        const sh::Varying &varying = *packedVarying.varying;

        // Do not assign registers to built-in or unreferenced varyings
        if (varying.isBuiltIn() || !varying.staticUse)
        {
            continue;
        }

        ASSERT(uniqueVaryingNames.count(varying.name) == 0);

        if (PackVarying(&packedVarying, maxVaryingVectors, packing))
        {
            uniqueVaryingNames.insert(varying.name);
        }
        else
        {
            infoLog << "Could not pack varying " << varying.name;
            return -1;
        }
    }

    for (const std::string &transformFeedbackVaryingName : transformFeedbackVaryings)
    {
        if (transformFeedbackVaryingName == "gl_Position" ||
            transformFeedbackVaryingName == "gl_PointSize")
        {
            // do not pack builtin XFB varyings
            continue;
        }

        if (uniqueVaryingNames.count(transformFeedbackVaryingName) == 0)
        {
            bool found = false;
            for (PackedVarying &packedVarying : *packedVaryings)
            {
                const sh::Varying &varying = *packedVarying.varying;
                if (transformFeedbackVaryingName == varying.name)
                {
                    if (!PackVarying(&packedVarying, maxVaryingVectors, packing))
                    {
                        infoLog << "Could not pack varying " << varying.name;
                        return -1;
                    }

                    found = true;
                    break;
                }
            }

            if (!found)
            {
                infoLog << "Transform feedback varying " << transformFeedbackVaryingName
                        << " does not exist in the vertex shader.";
                return -1;
            }
        }
    }

    // Return the number of used registers
    int registers = 0;

    for (int r = 0; r < maxVaryingVectors; r++)
    {
        if (packing[r][0] || packing[r][1] || packing[r][2] || packing[r][3])
        {
            registers++;
        }
    }

    return registers;
}

std::string DynamicHLSL::generateVaryingHLSL(const std::vector<PackedVarying> &varyings,
                                             bool shaderUsesPointSize) const
{
    std::string varyingSemantic = getVaryingSemantic(shaderUsesPointSize);
    std::string varyingHLSL;

    for (const PackedVarying &packedVarying : varyings)
    {
        if (!packedVarying.registerAssigned())
        {
            continue;
        }

        const sh::Varying &varying = *packedVarying.varying;

        ASSERT(!varying.isBuiltIn());
        GLenum transposedType = TransposeMatrixType(varying.type);
        int variableRows      = (varying.isStruct() ? 1 : VariableRowCount(transposedType));

        for (unsigned int elementIndex = 0; elementIndex < varying.elementCount(); elementIndex++)
        {
            for (int row = 0; row < variableRows; row++)
            {
                // TODO: Add checks to ensure D3D interpolation modifiers don't result in too many
                // registers being used.
                // For example, if there are N registers, and we have N vec3 varyings and 1 float
                // varying, then D3D will pack them into N registers.
                // If the float varying has the 'nointerpolation' modifier on it then we would need
                // N + 1 registers, and D3D compilation will fail.

                switch (varying.interpolation)
                {
                    case sh::INTERPOLATION_SMOOTH:
                        varyingHLSL += "    ";
                        break;
                    case sh::INTERPOLATION_FLAT:
                        varyingHLSL += "    nointerpolation ";
                        break;
                    case sh::INTERPOLATION_CENTROID:
                        varyingHLSL += "    centroid ";
                        break;
                    default:
                        UNREACHABLE();
                }

                unsigned int semanticIndex =
                    elementIndex * variableRows +
                    packedVarying.columnIndex * mRenderer->getRendererCaps().maxVaryingVectors +
                    packedVarying.registerIndex + row;
                std::string n = Str(semanticIndex);

                std::string typeString;

                if (varying.isStruct())
                {
                    // TODO(jmadill): pass back translated name from the shader translator
                    typeString = decorateVariable(varying.structName);
                }
                else
                {
                    GLenum componentType = VariableComponentType(transposedType);
                    int columnCount      = VariableColumnCount(transposedType);
                    typeString           = HLSLComponentTypeString(componentType, columnCount);
                }
                varyingHLSL += typeString + " v" + n + " : " + varyingSemantic + n + ";\n";
            }
        }
    }

    return varyingHLSL;
}

std::string DynamicHLSL::generateVertexShaderForInputLayout(const std::string &sourceShader,
                                                            const InputLayout &inputLayout,
                                                            const std::vector<sh::Attribute> &shaderAttributes) const
{
    std::string structHLSL, initHLSL;

    int semanticIndex = 0;
    unsigned int inputIndex = 0;

    // If gl_PointSize is used in the shader then pointsprites rendering is expected.
    // If the renderer does not support Geometry shaders then Instanced PointSprite emulation
    // must be used.
    bool usesPointSize = sourceShader.find("GL_USES_POINT_SIZE") != std::string::npos;
    bool useInstancedPointSpriteEmulation = usesPointSize && mRenderer->getWorkarounds().useInstancedPointSpriteEmulation;

    // Instanced PointSprite emulation requires additional entries in the
    // VS_INPUT structure to support the vertices that make up the quad vertices.
    // These values must be in sync with the cooresponding values added during inputlayout creation
    // in InputLayoutCache::applyVertexBuffers().
    //
    // The additional entries must appear first in the VS_INPUT layout because
    // Windows Phone 8 era devices require per vertex data to physically come
    // before per instance data in the shader.
    if (useInstancedPointSpriteEmulation)
    {
        structHLSL += "    float3 spriteVertexPos : SPRITEPOSITION0;\n";
        structHLSL += "    float2 spriteTexCoord : SPRITETEXCOORD0;\n";
    }

    for (size_t attributeIndex = 0; attributeIndex < shaderAttributes.size(); ++attributeIndex)
    {
        const sh::Attribute &shaderAttribute = shaderAttributes[attributeIndex];
        if (!shaderAttribute.name.empty())
        {
            ASSERT(inputIndex < MAX_VERTEX_ATTRIBS);
            VertexFormatType vertexFormatType =
                inputIndex < inputLayout.size() ? inputLayout[inputIndex] : VERTEX_FORMAT_INVALID;

            // HLSL code for input structure
            if (IsMatrixType(shaderAttribute.type))
            {
                // Matrix types are always transposed
                structHLSL += "    " + HLSLMatrixTypeString(TransposeMatrixType(shaderAttribute.type));
            }
            else
            {
                GLenum componentType = mRenderer->getVertexComponentType(vertexFormatType);

                if (shaderAttribute.name == "gl_InstanceID")
                {
                    // The input type of the instance ID in HLSL (uint) differs from the one in ESSL (int).
                    structHLSL += " uint";
                }
                else
                {
                    structHLSL += "    " + HLSLComponentTypeString(componentType, VariableComponentCount(shaderAttribute.type));
                }
            }

            structHLSL += " " + decorateVariable(shaderAttribute.name) + " : ";

            if (shaderAttribute.name == "gl_InstanceID")
            {
                structHLSL += "SV_InstanceID";
            }
            else
            {
                structHLSL += "TEXCOORD" + Str(semanticIndex);
                semanticIndex += VariableRegisterCount(shaderAttribute.type);
            }

            structHLSL += ";\n";

            // HLSL code for initialization
            initHLSL += "    " + decorateVariable(shaderAttribute.name) + " = ";

            // Mismatched vertex attribute to vertex input may result in an undefined
            // data reinterpretation (eg for pure integer->float, float->pure integer)
            // TODO: issue warning with gl debug info extension, when supported
            if (IsMatrixType(shaderAttribute.type) ||
                (mRenderer->getVertexConversionType(vertexFormatType) & VERTEX_CONVERT_GPU) != 0)
            {
                initHLSL += generateAttributeConversionHLSL(vertexFormatType, shaderAttribute);
            }
            else
            {
                initHLSL += "input." + decorateVariable(shaderAttribute.name);
            }

            initHLSL += ";\n";

            inputIndex += VariableRowCount(TransposeMatrixType(shaderAttribute.type));
        }
    }

    std::string replacementHLSL = "struct VS_INPUT\n"
                                  "{\n" +
                                  structHLSL +
                                  "};\n"
                                  "\n"
                                  "void initAttributes(VS_INPUT input)\n"
                                  "{\n" +
                                  initHLSL +
                                  "}\n";

    std::string vertexHLSL(sourceShader);

    size_t copyInsertionPos = vertexHLSL.find(VERTEX_ATTRIBUTE_STUB_STRING);
    vertexHLSL.replace(copyInsertionPos, VERTEX_ATTRIBUTE_STUB_STRING.length(), replacementHLSL);

    return vertexHLSL;
}

std::string DynamicHLSL::generatePixelShaderForOutputSignature(const std::string &sourceShader, const std::vector<PixelShaderOutputVariable> &outputVariables,
                                                               bool usesFragDepth, const std::vector<GLenum> &outputLayout) const
{
    const int shaderModel = mRenderer->getMajorShaderModel();
    std::string targetSemantic = (shaderModel >= 4) ? "SV_TARGET" : "COLOR";
    std::string depthSemantic = (shaderModel >= 4) ? "SV_Depth" : "DEPTH";

    std::string declarationHLSL;
    std::string copyHLSL;

    for (size_t layoutIndex = 0; layoutIndex < outputLayout.size(); ++layoutIndex)
    {
        GLenum binding = outputLayout[layoutIndex];

        if (binding != GL_NONE)
        {
            unsigned int location = (binding - GL_COLOR_ATTACHMENT0);

            const PixelShaderOutputVariable *outputVariable = FindOutputAtLocation(outputVariables, location);

            // OpenGL ES 3.0 spec $4.2.1
            // If [...] not all user-defined output variables are written, the values of fragment colors
            // corresponding to unwritten variables are similarly undefined.
            if (outputVariable)
            {
                declarationHLSL += "    " + HLSLTypeString(outputVariable->type) + " " +
                                   outputVariable->name + " : " + targetSemantic +
                                   Str(static_cast<int>(layoutIndex)) + ";\n";

                copyHLSL += "    output." + outputVariable->name + " = " + outputVariable->source + ";\n";
            }
        }
    }

    if (usesFragDepth)
    {
        declarationHLSL += "    float gl_Depth : " + depthSemantic + ";\n";
        copyHLSL += "    output.gl_Depth = gl_Depth; \n";
    }

    std::string replacementHLSL = "struct PS_OUTPUT\n"
                                  "{\n" +
                                  declarationHLSL +
                                  "};\n"
                                  "\n"
                                  "PS_OUTPUT generateOutput()\n"
                                  "{\n"
                                  "    PS_OUTPUT output;\n" +
                                  copyHLSL +
                                  "    return output;\n"
                                  "}\n";

    std::string pixelHLSL(sourceShader);

    size_t outputInsertionPos = pixelHLSL.find(PIXEL_OUTPUT_STUB_STRING);
    pixelHLSL.replace(outputInsertionPos, PIXEL_OUTPUT_STUB_STRING.length(), replacementHLSL);

    return pixelHLSL;
}

std::string DynamicHLSL::getVaryingSemantic(bool pointSize) const
{
    // SM3 reserves the TEXCOORD semantic for point sprite texcoords (gl_PointCoord)
    // In D3D11 we manually compute gl_PointCoord in the GS.
    int shaderModel = mRenderer->getMajorShaderModel();
    return ((pointSize && shaderModel < 4) ? "COLOR" : "TEXCOORD");
}

struct DynamicHLSL::SemanticInfo
{
    struct BuiltinInfo
    {
        BuiltinInfo()
            : enabled(false),
              index(0),
              systemValue(false)
        {}

        bool enabled;
        std::string semantic;
        unsigned int index;
        bool systemValue;

        std::string str() const
        {
            return (systemValue ? semantic : (semantic + Str(index)));
        }

        void enableSystem(const std::string &systemValueSemantic)
        {
            enabled = true;
            semantic = systemValueSemantic;
            systemValue = true;
        }

        void enable(const std::string &semanticVal, unsigned int indexVal)
        {
            enabled = true;
            semantic = semanticVal;
            index = indexVal;
        }
    };

    BuiltinInfo dxPosition;
    BuiltinInfo glPosition;
    BuiltinInfo glFragCoord;
    BuiltinInfo glPointCoord;
    BuiltinInfo glPointSize;
};

DynamicHLSL::SemanticInfo DynamicHLSL::getSemanticInfo(int startRegisters, bool position, bool fragCoord,
                                                       bool pointCoord, bool pointSize, bool pixelShader) const
{
    SemanticInfo info;
    bool hlsl4 = (mRenderer->getMajorShaderModel() >= 4);
    const std::string &varyingSemantic = getVaryingSemantic(pointSize);

    int reservedRegisterIndex = startRegisters;

    if (hlsl4)
    {
        info.dxPosition.enableSystem("SV_Position");
    }
    else if (pixelShader)
    {
        info.dxPosition.enableSystem("VPOS");
    }
    else
    {
        info.dxPosition.enableSystem("POSITION");
    }

    if (position)
    {
        info.glPosition.enable(varyingSemantic, reservedRegisterIndex++);
    }

    if (fragCoord)
    {
        info.glFragCoord.enable(varyingSemantic, reservedRegisterIndex++);
    }

    if (pointCoord)
    {
        // SM3 reserves the TEXCOORD semantic for point sprite texcoords (gl_PointCoord)
        // In D3D11 we manually compute gl_PointCoord in the GS.
        if (hlsl4)
        {
            info.glPointCoord.enable(varyingSemantic, reservedRegisterIndex++);
        }
        else
        {
            info.glPointCoord.enable("TEXCOORD", 0);
        }
    }

    // Special case: do not include PSIZE semantic in HLSL 3 pixel shaders
    if (pointSize && (!pixelShader || hlsl4))
    {
        info.glPointSize.enableSystem("PSIZE");
    }

    return info;
}

std::string DynamicHLSL::generateVaryingLinkHLSL(const SemanticInfo &info, const std::string &varyingHLSL) const
{
    std::string linkHLSL = "{\n";

    ASSERT(info.dxPosition.enabled);
    linkHLSL += "    float4 dx_Position : " + info.dxPosition.str() + ";\n";

    if (info.glPosition.enabled)
    {
        linkHLSL += "    float4 gl_Position : " + info.glPosition.str() + ";\n";
    }

    if (info.glFragCoord.enabled)
    {
        linkHLSL += "    float4 gl_FragCoord : " + info.glFragCoord.str() + ";\n";
    }

    if (info.glPointCoord.enabled)
    {
        linkHLSL += "    float2 gl_PointCoord : " + info.glPointCoord.str() + ";\n";
    }

    if (info.glPointSize.enabled)
    {
        linkHLSL += "    float gl_PointSize : " + info.glPointSize.str() + ";\n";
    }

    // Do this after glPointSize, to potentially combine gl_PointCoord and gl_PointSize into the same register.
    linkHLSL += varyingHLSL;

    linkHLSL += "};\n";

    return linkHLSL;
}

void DynamicHLSL::storeBuiltinLinkedVaryings(const SemanticInfo &info,
                                             std::vector<LinkedVarying> *linkedVaryings) const
{
    if (info.glPosition.enabled)
    {
        linkedVaryings->push_back(LinkedVarying("gl_Position", GL_FLOAT_VEC4, 1, info.glPosition.semantic,
                                                info.glPosition.index, 1));
    }

    if (info.glFragCoord.enabled)
    {
        linkedVaryings->push_back(LinkedVarying("gl_FragCoord", GL_FLOAT_VEC4, 1, info.glFragCoord.semantic,
                                                info.glFragCoord.index, 1));
    }

    if (info.glPointSize.enabled)
    {
        linkedVaryings->push_back(LinkedVarying("gl_PointSize", GL_FLOAT, 1, "PSIZE", 0, 1));
    }
}

void DynamicHLSL::storeUserLinkedVaryings(const std::vector<PackedVarying> &packedVaryings,
                                          bool shaderUsesPointSize,
                                          std::vector<LinkedVarying> *linkedVaryings) const
{
    const std::string &varyingSemantic = getVaryingSemantic(shaderUsesPointSize);

    for (const PackedVarying &packedVarying : packedVaryings)
    {
        if (packedVarying.registerAssigned())
        {
            const sh::Varying &varying = *packedVarying.varying;

            ASSERT(!varying.isBuiltIn());
            GLenum transposedType = TransposeMatrixType(varying.type);
            int variableRows = (varying.isStruct() ? 1 : VariableRowCount(transposedType));

            linkedVaryings->push_back(
                LinkedVarying(varying.name, varying.type, varying.elementCount(), varyingSemantic,
                              packedVarying.registerIndex, variableRows * varying.elementCount()));
        }
    }
}

bool DynamicHLSL::generateShaderLinkHLSL(const gl::Data &data,
                                         const gl::Program::Data &programData,
                                         InfoLog &infoLog,
                                         int registers,
                                         std::string &pixelHLSL,
                                         std::string &vertexHLSL,
                                         const std::vector<PackedVarying> &packedVaryings,
                                         std::vector<LinkedVarying> *linkedVaryings,
                                         std::vector<PixelShaderOutputVariable> *outPixelShaderKey,
                                         bool *outUsesFragDepth) const
{
    if (pixelHLSL.empty() || vertexHLSL.empty())
    {
        return false;
    }

    const gl::Shader *vertexShaderGL   = programData.getAttachedVertexShader();
    const ShaderD3D *vertexShader      = GetImplAs<ShaderD3D>(vertexShaderGL);
    const gl::Shader *fragmentShaderGL = programData.getAttachedFragmentShader();
    const ShaderD3D *fragmentShader    = GetImplAs<ShaderD3D>(fragmentShaderGL);

    bool usesMRT = fragmentShader->mUsesMultipleRenderTargets;
    bool usesFragCoord = fragmentShader->mUsesFragCoord;
    bool usesPointCoord = fragmentShader->mUsesPointCoord;
    bool usesPointSize = vertexShader->mUsesPointSize;
    bool useInstancedPointSpriteEmulation = usesPointSize && mRenderer->getWorkarounds().useInstancedPointSpriteEmulation;

    // Validation done in the compiler
    ASSERT(!fragmentShader->mUsesFragColor || !fragmentShader->mUsesFragData);

    // Write the HLSL input/output declarations
    const int shaderModel = mRenderer->getMajorShaderModel();
    const int registersNeeded = registers + (usesFragCoord ? 1 : 0) + (usesPointCoord ? 1 : 0);

    // Two cases when writing to gl_FragColor and using ESSL 1.0:
    // - with a 3.0 context, the output color is copied to channel 0
    // - with a 2.0 context, the output color is broadcast to all channels
    const bool broadcast = (fragmentShader->mUsesFragColor && data.clientVersion < 3);
    const unsigned int numRenderTargets = (broadcast || usesMRT ? data.caps->maxDrawBuffers : 1);

    // gl_Position only needs to be outputted from the vertex shader if transform feedback is active.
    // This isn't supported on D3D11 Feature Level 9_3, so we don't output gl_Position from the vertex shader in this case.
    // This saves us 1 output vector.
    bool outputPositionFromVS = !(shaderModel >= 4 && mRenderer->getShaderModelSuffix() != "");

    int shaderVersion = vertexShaderGL->getShaderVersion();

    if (static_cast<GLuint>(registersNeeded) > data.caps->maxVaryingVectors)
    {
        infoLog << "No varying registers left to support gl_FragCoord/gl_PointCoord";
        return false;
    }

    const std::string &varyingHLSL = generateVaryingHLSL(packedVaryings, usesPointSize);

    // Instanced PointSprite emulation requires that gl_PointCoord is present in the vertex shader VS_OUTPUT
    // structure to ensure compatibility with the generated PS_INPUT of the pixel shader.
    // GeometryShader PointSprite emulation does not require this additional entry because the
    // GS_OUTPUT of the Geometry shader contains the pointCoord value and already matches the PS_INPUT of the
    // generated pixel shader.
    // The Geometry Shader point sprite implementation needs gl_PointSize to be in VS_OUTPUT and GS_INPUT.
    // Instanced point sprites doesn't need gl_PointSize in VS_OUTPUT.
    const SemanticInfo &vertexSemantics = getSemanticInfo(registers, outputPositionFromVS,
                                                          usesFragCoord, (useInstancedPointSpriteEmulation && usesPointCoord),
                                                          (!useInstancedPointSpriteEmulation && usesPointSize), false);

    storeUserLinkedVaryings(packedVaryings, usesPointSize, linkedVaryings);
    storeBuiltinLinkedVaryings(vertexSemantics, linkedVaryings);

    // Instanced PointSprite emulation requires additional entries originally generated in the
    // GeometryShader HLSL.  These include pointsize clamp values.
    if (useInstancedPointSpriteEmulation)
    {
        vertexHLSL += "static float minPointSize = " + Str((int)mRenderer->getRendererCaps().minAliasedPointSize) + ".0f;\n"
                      "static float maxPointSize = " + Str((int)mRenderer->getRendererCaps().maxAliasedPointSize) + ".0f;\n";
    }

    // Add stub string to be replaced when shader is dynamically defined by its layout
    vertexHLSL += "\n" + VERTEX_ATTRIBUTE_STUB_STRING + "\n"
                  "struct VS_OUTPUT\n" + generateVaryingLinkHLSL(vertexSemantics, varyingHLSL) + "\n"
                  "VS_OUTPUT main(VS_INPUT input)\n"
                  "{\n"
                  "    initAttributes(input);\n";

    if (vertexShader->usesDeferredInit())
    {
        vertexHLSL += "\n"
                      "    initializeDeferredGlobals();\n";
    }

    vertexHLSL += "\n"
                  "    gl_main();\n"
                  "\n"
                  "    VS_OUTPUT output;\n";

    if (outputPositionFromVS)
    {
        vertexHLSL += "    output.gl_Position = gl_Position;\n";
    }

    // On D3D9 or D3D11 Feature Level 9, we need to emulate large viewports using dx_ViewAdjust.
    if (shaderModel >= 4  && mRenderer->getShaderModelSuffix() == "")
    {
        vertexHLSL += "    output.dx_Position.x = gl_Position.x;\n"
                      "    output.dx_Position.y = -gl_Position.y;\n"
                      "    output.dx_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n"
                      "    output.dx_Position.w = gl_Position.w;\n";
    }
    else
    {
        vertexHLSL += "    output.dx_Position.x = gl_Position.x * dx_ViewAdjust.z + dx_ViewAdjust.x * gl_Position.w;\n"
                      "    output.dx_Position.y = -(gl_Position.y * dx_ViewAdjust.w + dx_ViewAdjust.y * gl_Position.w);\n"
                      "    output.dx_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n"
                      "    output.dx_Position.w = gl_Position.w;\n";
    }

    // We don't need to output gl_PointSize if we use are emulating point sprites via instancing.
    if (usesPointSize && shaderModel >= 3 && !useInstancedPointSpriteEmulation)
    {
        vertexHLSL += "    output.gl_PointSize = gl_PointSize;\n";
    }

    if (usesFragCoord)
    {
        vertexHLSL += "    output.gl_FragCoord = gl_Position;\n";
    }

    for (const PackedVarying &packedVarying : packedVaryings)
    {
        if (!packedVarying.registerAssigned())
        {
            continue;
        }

        const sh::Varying &varying = *packedVarying.varying;

        for (unsigned int elementIndex = 0; elementIndex < varying.elementCount(); elementIndex++)
        {
            int variableRows =
                (varying.isStruct() ? 1 : VariableRowCount(TransposeMatrixType(varying.type)));

            for (int row = 0; row < variableRows; row++)
            {
                int r = packedVarying.registerIndex +
                        packedVarying.columnIndex * data.caps->maxVaryingVectors +
                        elementIndex * variableRows + row;
                vertexHLSL += "    output.v" + Str(r);

                vertexHLSL += " = _" + varying.name;

                if (varying.isArray())
                {
                    vertexHLSL += ArrayString(elementIndex);
                }

                if (variableRows > 1)
                {
                    vertexHLSL += ArrayString(row);
                }

                vertexHLSL += ";\n";
            }
        }
    }

    // Instanced PointSprite emulation requires additional entries to calculate
    // the final output vertex positions of the quad that represents each sprite.
    if (useInstancedPointSpriteEmulation)
    {
        vertexHLSL += "\n"
                      "    gl_PointSize = clamp(gl_PointSize, minPointSize, maxPointSize);\n"
                      "    output.dx_Position.xyz += float3(input.spriteVertexPos.x * gl_PointSize / (dx_ViewCoords.x*2), input.spriteVertexPos.y * gl_PointSize / (dx_ViewCoords.y*2), input.spriteVertexPos.z) * output.dx_Position.w;\n";

        if (usesPointCoord)
        {
            vertexHLSL += "\n"
                          "    output.gl_PointCoord = input.spriteTexCoord;\n";
        }
    }

    vertexHLSL += "\n"
                  "    return output;\n"
                  "}\n";

    const SemanticInfo &pixelSemantics = getSemanticInfo(registers, outputPositionFromVS, usesFragCoord, usesPointCoord,
                                                         (!useInstancedPointSpriteEmulation && usesPointSize), true);

    pixelHLSL += "struct PS_INPUT\n" + generateVaryingLinkHLSL(pixelSemantics, varyingHLSL) + "\n";

    if (shaderVersion < 300)
    {
        for (unsigned int renderTargetIndex = 0; renderTargetIndex < numRenderTargets; renderTargetIndex++)
        {
            PixelShaderOutputVariable outputKeyVariable;
            outputKeyVariable.type = GL_FLOAT_VEC4;
            outputKeyVariable.name = "gl_Color" + Str(renderTargetIndex);
            outputKeyVariable.source = broadcast ? "gl_Color[0]" : "gl_Color[" + Str(renderTargetIndex) + "]";
            outputKeyVariable.outputIndex = renderTargetIndex;

            outPixelShaderKey->push_back(outputKeyVariable);
        }

        *outUsesFragDepth = fragmentShader->mUsesFragDepth;
    }
    else
    {
        const auto &shaderOutputVars = fragmentShaderGL->getActiveOutputVariables();

        for (auto outputPair : programData.getOutputVariables())
        {
            const VariableLocation &outputLocation   = outputPair.second;
            const sh::ShaderVariable &outputVariable = shaderOutputVars[outputLocation.index];
            const std::string &variableName = "out_" + outputLocation.name;
            const std::string &elementString = (outputLocation.element == GL_INVALID_INDEX ? "" : Str(outputLocation.element));

            ASSERT(outputVariable.staticUse);

            PixelShaderOutputVariable outputKeyVariable;
            outputKeyVariable.type = outputVariable.type;
            outputKeyVariable.name = variableName + elementString;
            outputKeyVariable.source = variableName + ArrayString(outputLocation.element);
            outputKeyVariable.outputIndex = outputPair.first;

            outPixelShaderKey->push_back(outputKeyVariable);
        }

        *outUsesFragDepth = false;
    }

    pixelHLSL += PIXEL_OUTPUT_STUB_STRING + "\n";

    if (fragmentShader->mUsesFrontFacing)
    {
        if (shaderModel >= 4)
        {
            pixelHLSL += "PS_OUTPUT main(PS_INPUT input, bool isFrontFace : SV_IsFrontFace)\n"
                         "{\n";
        }
        else
        {
            pixelHLSL += "PS_OUTPUT main(PS_INPUT input, float vFace : VFACE)\n"
                         "{\n";
        }
    }
    else
    {
        pixelHLSL += "PS_OUTPUT main(PS_INPUT input)\n"
                     "{\n";
    }

    if (usesFragCoord)
    {
        pixelHLSL += "    float rhw = 1.0 / input.gl_FragCoord.w;\n";

        // Certain Shader Models (4_0+ and 3_0) allow reading from dx_Position in the pixel shader.
        // Other Shader Models (4_0_level_9_3 and 2_x) don't support this, so we emulate it using dx_ViewCoords.
        if (shaderModel >= 4 && mRenderer->getShaderModelSuffix() == "")
        {
            pixelHLSL += "    gl_FragCoord.x = input.dx_Position.x;\n"
                         "    gl_FragCoord.y = input.dx_Position.y;\n";
        }
        else if (shaderModel == 3)
        {
            pixelHLSL += "    gl_FragCoord.x = input.dx_Position.x + 0.5;\n"
                         "    gl_FragCoord.y = input.dx_Position.y + 0.5;\n";
        }
        else
        {
            // dx_ViewCoords contains the viewport width/2, height/2, center.x and center.y. See Renderer::setViewport()
            pixelHLSL += "    gl_FragCoord.x = (input.gl_FragCoord.x * rhw) * dx_ViewCoords.x + dx_ViewCoords.z;\n"
                         "    gl_FragCoord.y = (input.gl_FragCoord.y * rhw) * dx_ViewCoords.y + dx_ViewCoords.w;\n";
        }

        pixelHLSL += "    gl_FragCoord.z = (input.gl_FragCoord.z * rhw) * dx_DepthFront.x + dx_DepthFront.y;\n"
                     "    gl_FragCoord.w = rhw;\n";
    }

    if (usesPointCoord && shaderModel >= 3)
    {
        pixelHLSL += "    gl_PointCoord.x = input.gl_PointCoord.x;\n";
        pixelHLSL += "    gl_PointCoord.y = 1.0 - input.gl_PointCoord.y;\n";
    }

    if (fragmentShader->mUsesFrontFacing)
    {
        if (shaderModel <= 3)
        {
            pixelHLSL += "    gl_FrontFacing = (vFace * dx_DepthFront.z >= 0.0);\n";
        }
        else
        {
            pixelHLSL += "    gl_FrontFacing = isFrontFace;\n";
        }
    }

    for (const PackedVarying &packedVarying : packedVaryings)
    {
        const sh::Varying &varying = *packedVarying.varying;

        if (!packedVarying.registerAssigned())
        {
            ASSERT(varying.isBuiltIn() || !varying.staticUse);
            continue;
        }

        // Don't reference VS-only transform feedback varyings in the PS.
        if (packedVarying.vertexOnly)
            continue;

        ASSERT(!varying.isBuiltIn());
        for (unsigned int elementIndex = 0; elementIndex < varying.elementCount(); elementIndex++)
        {
            GLenum transposedType = TransposeMatrixType(varying.type);
            int variableRows = (varying.isStruct() ? 1 : VariableRowCount(transposedType));
            for (int row = 0; row < variableRows; row++)
            {
                std::string n = Str(packedVarying.registerIndex +
                                    packedVarying.columnIndex * data.caps->maxVaryingVectors +
                                    elementIndex * variableRows + row);
                pixelHLSL += "    _" + varying.name;

                if (varying.isArray())
                {
                    pixelHLSL += ArrayString(elementIndex);
                }

                if (variableRows > 1)
                {
                    pixelHLSL += ArrayString(row);
                }

                if (varying.isStruct())
                {
                    pixelHLSL += " = input.v" + n + ";\n";
                    break;
                }
                else
                {
                    switch (VariableColumnCount(transposedType))
                    {
                        case 1:
                            pixelHLSL += " = input.v" + n + ".x;\n";
                            break;
                        case 2:
                            pixelHLSL += " = input.v" + n + ".xy;\n";
                            break;
                        case 3:
                            pixelHLSL += " = input.v" + n + ".xyz;\n";
                            break;
                        case 4:
                            pixelHLSL += " = input.v" + n + ";\n";
                            break;
                        default:
                            UNREACHABLE();
                    }
                }
            }
        }
    }

    if (fragmentShader->usesDeferredInit())
    {
        pixelHLSL += "\n"
                     "    initializeDeferredGlobals();\n";
    }

    pixelHLSL += "\n"
                 "    gl_main();\n"
                 "\n"
                 "    return generateOutput();\n"
                 "}\n";

    return true;
}

std::string DynamicHLSL::generateGeometryShaderHLSL(
    int registers,
    const ShaderD3D *fragmentShader,
    const std::vector<PackedVarying> &packedVaryings) const
{
    // for now we only handle point sprite emulation
    ASSERT(mRenderer->getMajorShaderModel() >= 4);
    return generatePointSpriteHLSL(registers, fragmentShader, packedVaryings);
}

std::string DynamicHLSL::generatePointSpriteHLSL(
    int registers,
    const ShaderD3D *fragmentShader,
    const std::vector<PackedVarying> &packedVaryings) const
{
    ASSERT(registers >= 0);
    ASSERT(mRenderer->getMajorShaderModel() >= 4);

    std::string geomHLSL;

    const SemanticInfo &inSemantics = getSemanticInfo(registers, true, fragmentShader->mUsesFragCoord,
                                                      false, true, false);
    const SemanticInfo &outSemantics = getSemanticInfo(registers, true, fragmentShader->mUsesFragCoord,
                                                       fragmentShader->mUsesPointCoord, true, false);

    // If we're generating the geometry shader, we assume the vertex shader uses point size.
    std::string varyingHLSL = generateVaryingHLSL(packedVaryings, true);
    std::string inLinkHLSL = generateVaryingLinkHLSL(inSemantics, varyingHLSL);
    std::string outLinkHLSL = generateVaryingLinkHLSL(outSemantics, varyingHLSL);

    // TODO(geofflang): use context's caps
    geomHLSL += "uniform float4 dx_ViewCoords : register(c1);\n"
                "\n"
                "struct GS_INPUT\n" + inLinkHLSL + "\n" +
                "struct GS_OUTPUT\n" + outLinkHLSL + "\n" +
                "\n"
                "static float2 pointSpriteCorners[] = \n"
                "{\n"
                "    float2( 0.5f, -0.5f),\n"
                "    float2( 0.5f,  0.5f),\n"
                "    float2(-0.5f, -0.5f),\n"
                "    float2(-0.5f,  0.5f)\n"
                "};\n"
                "\n"
                "static float2 pointSpriteTexcoords[] = \n"
                "{\n"
                "    float2(1.0f, 1.0f),\n"
                "    float2(1.0f, 0.0f),\n"
                "    float2(0.0f, 1.0f),\n"
                "    float2(0.0f, 0.0f)\n"
                "};\n"
                "\n"
                "static float minPointSize = " + Str(static_cast<int>(mRenderer->getRendererCaps().minAliasedPointSize)) + ".0f;\n"
                "static float maxPointSize = " + Str(static_cast<int>(mRenderer->getRendererCaps().maxAliasedPointSize)) + ".0f;\n"
                "\n"
                "[maxvertexcount(4)]\n"
                "void main(point GS_INPUT input[1], inout TriangleStream<GS_OUTPUT> outStream)\n"
                "{\n"
                "    GS_OUTPUT output = (GS_OUTPUT)0;\n"
                "    output.gl_Position = input[0].gl_Position;\n"
                "    output.gl_PointSize = input[0].gl_PointSize;\n";

    for (int r = 0; r < registers; r++)
    {
        geomHLSL += "    output.v" + Str(r) + " = input[0].v" + Str(r) + ";\n";
    }

    if (fragmentShader->mUsesFragCoord)
    {
        geomHLSL += "    output.gl_FragCoord = input[0].gl_FragCoord;\n";
    }

    geomHLSL += "    \n"
                "    float gl_PointSize = clamp(input[0].gl_PointSize, minPointSize, maxPointSize);\n"
                "    float4 dx_Position = input[0].dx_Position;\n"
                "    float2 viewportScale = float2(1.0f / dx_ViewCoords.x, 1.0f / dx_ViewCoords.y) * dx_Position.w;\n";

    for (int corner = 0; corner < 4; corner++)
    {
        geomHLSL += "    \n"
                    "    output.dx_Position = dx_Position + float4(pointSpriteCorners[" + Str(corner) + "] * viewportScale * gl_PointSize, 0.0f, 0.0f);\n";

        if (fragmentShader->mUsesPointCoord)
        {
            geomHLSL += "    output.gl_PointCoord = pointSpriteTexcoords[" + Str(corner) + "];\n";
        }

        geomHLSL += "    outStream.Append(output);\n";
    }

    geomHLSL += "    \n"
                "    outStream.RestartStrip();\n"
                "}\n";

    return geomHLSL;
}

// This method needs to match OutputHLSL::decorate
std::string DynamicHLSL::decorateVariable(const std::string &name)
{
    if (name.compare(0, 3, "gl_") != 0)
    {
        return "_" + name;
    }

    return name;
}

std::string DynamicHLSL::generateAttributeConversionHLSL(gl::VertexFormatType vertexFormatType,
                                                         const sh::ShaderVariable &shaderAttrib) const
{
    const gl::VertexFormat &vertexFormat = gl::GetVertexFormatFromType(vertexFormatType);
    std::string attribString = "input." + decorateVariable(shaderAttrib.name);

    // Matrix
    if (IsMatrixType(shaderAttrib.type))
    {
        return "transpose(" + attribString + ")";
    }

    GLenum shaderComponentType = VariableComponentType(shaderAttrib.type);
    int shaderComponentCount = VariableComponentCount(shaderAttrib.type);

    // Perform integer to float conversion (if necessary)
    bool requiresTypeConversion = (shaderComponentType == GL_FLOAT && vertexFormat.type != GL_FLOAT);

    if (requiresTypeConversion)
    {
        // TODO: normalization for 32-bit integer formats
        ASSERT(!vertexFormat.normalized && !vertexFormat.pureInteger);
        return "float" + Str(shaderComponentCount) + "(" + attribString + ")";
    }

    // No conversion necessary
    return attribString;
}

}
