//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramD3D.cpp: Defines the rx::ProgramD3D class which implements rx::ProgramImpl.

#include "libANGLE/renderer/d3d/ProgramD3D.h"

#include "common/BitSetIterator.h"
#include "common/utilities.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/Program.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/features.h"
#include "libANGLE/renderer/d3d/DynamicHLSL.h"
#include "libANGLE/renderer/d3d/FramebufferD3D.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/ShaderD3D.h"
#include "libANGLE/renderer/d3d/ShaderExecutableD3D.h"
#include "libANGLE/renderer/d3d/VertexDataManager.h"

namespace rx
{

namespace
{

gl::InputLayout GetDefaultInputLayoutFromShader(const gl::Shader *vertexShader)
{
    gl::InputLayout defaultLayout;
    for (const sh::Attribute &shaderAttr : vertexShader->getActiveAttributes())
    {
        if (shaderAttr.type != GL_NONE)
        {
            GLenum transposedType = gl::TransposeMatrixType(shaderAttr.type);

            for (size_t rowIndex = 0;
                 static_cast<int>(rowIndex) < gl::VariableRowCount(transposedType);
                 ++rowIndex)
            {
                GLenum componentType = gl::VariableComponentType(transposedType);
                GLuint components = static_cast<GLuint>(gl::VariableColumnCount(transposedType));
                bool pureInt = (componentType != GL_FLOAT);
                gl::VertexFormatType defaultType = gl::GetVertexFormatType(
                    componentType, GL_FALSE, components, pureInt);

                defaultLayout.push_back(defaultType);
            }
        }
    }

    return defaultLayout;
}

std::vector<GLenum> GetDefaultOutputLayoutFromShader(const std::vector<PixelShaderOutputVariable> &shaderOutputVars)
{
    std::vector<GLenum> defaultPixelOutput;

    if (!shaderOutputVars.empty())
    {
        defaultPixelOutput.push_back(GL_COLOR_ATTACHMENT0 +
                                     static_cast<unsigned int>(shaderOutputVars[0].outputIndex));
    }

    return defaultPixelOutput;
}

bool IsRowMajorLayout(const sh::InterfaceBlockField &var)
{
    return var.isRowMajorLayout;
}

bool IsRowMajorLayout(const sh::ShaderVariable &var)
{
    return false;
}

struct AttributeSorter
{
    AttributeSorter(const ProgramD3D::SemanticIndexArray &semanticIndices)
        : originalIndices(&semanticIndices)
    {
    }

    bool operator()(int a, int b)
    {
        int indexA = (*originalIndices)[a];
        int indexB = (*originalIndices)[b];

        if (indexA == -1) return false;
        if (indexB == -1) return true;
        return (indexA < indexB);
    }

    const ProgramD3D::SemanticIndexArray *originalIndices;
};

std::vector<PackedVarying> MergeVaryings(const gl::Shader &vertexShader,
                                         const gl::Shader &fragmentShader,
                                         const std::vector<std::string> &tfVaryings)
{
    std::vector<PackedVarying> packedVaryings;

    for (const sh::Varying &output : vertexShader.getVaryings())
    {
        bool packed = false;

        // Built-in varyings obey special rules
        if (output.isBuiltIn())
        {
            continue;
        }

        for (const sh::Varying &input : fragmentShader.getVaryings())
        {
            if (output.name == input.name)
            {
                packedVaryings.push_back(PackedVarying(input));
                packed = true;
                break;
            }
        }

        // Keep Transform FB varyings in the merged list always.
        if (!packed)
        {
            for (const std::string &tfVarying : tfVaryings)
            {
                if (tfVarying == output.name)
                {
                    packedVaryings.push_back(PackedVarying(output));
                    packedVaryings.back().vertexOnly = true;
                    break;
                }
            }
        }
    }

    return packedVaryings;
}

template <typename VarT>
void GetUniformBlockInfo(const std::vector<VarT> &fields,
                         const std::string &prefix,
                         sh::BlockLayoutEncoder *encoder,
                         bool inRowMajorLayout,
                         std::map<std::string, sh::BlockMemberInfo> *blockInfoOut)
{
    for (const VarT &field : fields)
    {
        const std::string &fieldName = (prefix.empty() ? field.name : prefix + "." + field.name);

        if (field.isStruct())
        {
            bool rowMajorLayout = (inRowMajorLayout || IsRowMajorLayout(field));

            for (unsigned int arrayElement = 0; arrayElement < field.elementCount(); arrayElement++)
            {
                encoder->enterAggregateType();

                const std::string uniformElementName =
                    fieldName + (field.isArray() ? ArrayString(arrayElement) : "");
                GetUniformBlockInfo(field.fields, uniformElementName, encoder, rowMajorLayout,
                                    blockInfoOut);

                encoder->exitAggregateType();
            }
        }
        else
        {
            bool isRowMajorMatrix = (gl::IsMatrixType(field.type) && inRowMajorLayout);
            (*blockInfoOut)[fieldName] =
                encoder->encodeType(field.type, field.arraySize, isRowMajorMatrix);
        }
    }
}

}  // anonymous namespace

D3DUniform::D3DUniform(GLenum typeIn,
                       const std::string &nameIn,
                       unsigned int arraySizeIn,
                       bool defaultBlock)
    : type(typeIn),
      name(nameIn),
      arraySize(arraySizeIn),
      data(nullptr),
      dirty(true),
      vsRegisterIndex(GL_INVALID_INDEX),
      psRegisterIndex(GL_INVALID_INDEX),
      registerCount(0),
      registerElement(0)
{
    // We use data storage for default block uniforms to cache values that are sent to D3D during
    // rendering
    // Uniform blocks/buffers are treated separately by the Renderer (ES3 path only)
    if (defaultBlock)
    {
        size_t bytes = gl::VariableInternalSize(type) * elementCount();
        data = new uint8_t[bytes];
        memset(data, 0, bytes);

        // TODO(jmadill): is this correct with non-square matrices?
        registerCount = gl::VariableRowCount(type) * elementCount();
    }
}

D3DUniform::~D3DUniform()
{
    SafeDeleteArray(data);
}

bool D3DUniform::isSampler() const
{
    return gl::IsSamplerType(type);
}

bool D3DUniform::isReferencedByVertexShader() const
{
    return vsRegisterIndex != GL_INVALID_INDEX;
}

bool D3DUniform::isReferencedByFragmentShader() const
{
    return psRegisterIndex != GL_INVALID_INDEX;
}

ProgramD3D::VertexExecutable::VertexExecutable(const gl::InputLayout &inputLayout,
                                               const Signature &signature,
                                               ShaderExecutableD3D *shaderExecutable)
    : mInputs(inputLayout),
      mSignature(signature),
      mShaderExecutable(shaderExecutable)
{
}

ProgramD3D::VertexExecutable::~VertexExecutable()
{
    SafeDelete(mShaderExecutable);
}

// static
void ProgramD3D::VertexExecutable::getSignature(RendererD3D *renderer,
                                                const gl::InputLayout &inputLayout,
                                                Signature *signatureOut)
{
    signatureOut->resize(inputLayout.size());

    for (size_t index = 0; index < inputLayout.size(); ++index)
    {
        gl::VertexFormatType vertexFormatType = inputLayout[index];
        bool converted = false;
        if (vertexFormatType != gl::VERTEX_FORMAT_INVALID)
        {
            VertexConversionType conversionType =
                renderer->getVertexConversionType(vertexFormatType);
            converted = ((conversionType & VERTEX_CONVERT_GPU) != 0);
        }

        (*signatureOut)[index] = converted;
    }
}

bool ProgramD3D::VertexExecutable::matchesSignature(const Signature &signature) const
{
    size_t limit = std::max(mSignature.size(), signature.size());
    for (size_t index = 0; index < limit; ++index)
    {
        // treat undefined indexes as 'not converted'
        bool a = index < signature.size() ? signature[index] : false;
        bool b = index < mSignature.size() ? mSignature[index] : false;
        if (a != b)
            return false;
    }

    return true;
}

ProgramD3D::PixelExecutable::PixelExecutable(const std::vector<GLenum> &outputSignature,
                                             ShaderExecutableD3D *shaderExecutable)
    : mOutputSignature(outputSignature),
      mShaderExecutable(shaderExecutable)
{
}

ProgramD3D::PixelExecutable::~PixelExecutable()
{
    SafeDelete(mShaderExecutable);
}

ProgramD3D::Sampler::Sampler() : active(false), logicalTextureUnit(0), textureType(GL_TEXTURE_2D)
{
}

unsigned int ProgramD3D::mCurrentSerial = 1;

ProgramD3D::ProgramD3D(const gl::Program::Data &data, RendererD3D *renderer)
    : ProgramImpl(data),
      mRenderer(renderer),
      mDynamicHLSL(NULL),
      mGeometryExecutable(NULL),
      mUsesPointSize(false),
      mVertexUniformStorage(NULL),
      mFragmentUniformStorage(NULL),
      mUsedVertexSamplerRange(0),
      mUsedPixelSamplerRange(0),
      mDirtySamplerMapping(true),
      mShaderVersion(100),
      mSerial(issueSerial())
{
    mDynamicHLSL = new DynamicHLSL(renderer);
}

ProgramD3D::~ProgramD3D()
{
    reset();
    SafeDelete(mDynamicHLSL);
}

bool ProgramD3D::usesPointSpriteEmulation() const
{
    return mUsesPointSize && mRenderer->getMajorShaderModel() >= 4;
}

bool ProgramD3D::usesGeometryShader() const
{
    return usesPointSpriteEmulation() && !usesInstancedPointSpriteEmulation();
}

bool ProgramD3D::usesInstancedPointSpriteEmulation() const
{
    return mRenderer->getWorkarounds().useInstancedPointSpriteEmulation;
}

GLint ProgramD3D::getSamplerMapping(gl::SamplerType type, unsigned int samplerIndex, const gl::Caps &caps) const
{
    GLint logicalTextureUnit = -1;

    switch (type)
    {
      case gl::SAMPLER_PIXEL:
        ASSERT(samplerIndex < caps.maxTextureImageUnits);
        if (samplerIndex < mSamplersPS.size() && mSamplersPS[samplerIndex].active)
        {
            logicalTextureUnit = mSamplersPS[samplerIndex].logicalTextureUnit;
        }
        break;
      case gl::SAMPLER_VERTEX:
        ASSERT(samplerIndex < caps.maxVertexTextureImageUnits);
        if (samplerIndex < mSamplersVS.size() && mSamplersVS[samplerIndex].active)
        {
            logicalTextureUnit = mSamplersVS[samplerIndex].logicalTextureUnit;
        }
        break;
      default: UNREACHABLE();
    }

    if (logicalTextureUnit >= 0 && logicalTextureUnit < static_cast<GLint>(caps.maxCombinedTextureImageUnits))
    {
        return logicalTextureUnit;
    }

    return -1;
}

// Returns the texture type for a given Direct3D 9 sampler type and
// index (0-15 for the pixel shader and 0-3 for the vertex shader).
GLenum ProgramD3D::getSamplerTextureType(gl::SamplerType type, unsigned int samplerIndex) const
{
    switch (type)
    {
      case gl::SAMPLER_PIXEL:
        ASSERT(samplerIndex < mSamplersPS.size());
        ASSERT(mSamplersPS[samplerIndex].active);
        return mSamplersPS[samplerIndex].textureType;
      case gl::SAMPLER_VERTEX:
        ASSERT(samplerIndex < mSamplersVS.size());
        ASSERT(mSamplersVS[samplerIndex].active);
        return mSamplersVS[samplerIndex].textureType;
      default: UNREACHABLE();
    }

    return GL_TEXTURE_2D;
}

GLint ProgramD3D::getUsedSamplerRange(gl::SamplerType type) const
{
    switch (type)
    {
      case gl::SAMPLER_PIXEL:
        return mUsedPixelSamplerRange;
      case gl::SAMPLER_VERTEX:
        return mUsedVertexSamplerRange;
      default:
        UNREACHABLE();
        return 0;
    }
}

void ProgramD3D::updateSamplerMapping()
{
    if (!mDirtySamplerMapping)
    {
        return;
    }

    mDirtySamplerMapping = false;

    // Retrieve sampler uniform values
    for (const D3DUniform *d3dUniform : mD3DUniforms)
    {
        if (!d3dUniform->dirty)
            continue;

        if (!d3dUniform->isSampler())
            continue;

        int count = d3dUniform->elementCount();
        const GLint(*v)[4] = reinterpret_cast<const GLint(*)[4]>(d3dUniform->data);

        if (d3dUniform->isReferencedByFragmentShader())
        {
            unsigned int firstIndex = d3dUniform->psRegisterIndex;

            for (int i = 0; i < count; i++)
            {
                unsigned int samplerIndex = firstIndex + i;

                if (samplerIndex < mSamplersPS.size())
                {
                    ASSERT(mSamplersPS[samplerIndex].active);
                    mSamplersPS[samplerIndex].logicalTextureUnit = v[i][0];
                }
            }
        }

        if (d3dUniform->isReferencedByVertexShader())
        {
            unsigned int firstIndex = d3dUniform->vsRegisterIndex;

            for (int i = 0; i < count; i++)
            {
                unsigned int samplerIndex = firstIndex + i;

                if (samplerIndex < mSamplersVS.size())
                {
                    ASSERT(mSamplersVS[samplerIndex].active);
                    mSamplersVS[samplerIndex].logicalTextureUnit = v[i][0];
                }
            }
        }
    }
}

LinkResult ProgramD3D::load(gl::InfoLog &infoLog, gl::BinaryInputStream *stream)
{
    reset();

    DeviceIdentifier binaryDeviceIdentifier = { 0 };
    stream->readBytes(reinterpret_cast<unsigned char*>(&binaryDeviceIdentifier), sizeof(DeviceIdentifier));

    DeviceIdentifier identifier = mRenderer->getAdapterIdentifier();
    if (memcmp(&identifier, &binaryDeviceIdentifier, sizeof(DeviceIdentifier)) != 0)
    {
        infoLog << "Invalid program binary, device configuration has changed.";
        return LinkResult(false, gl::Error(GL_NO_ERROR));
    }

    int compileFlags = stream->readInt<int>();
    if (compileFlags != ANGLE_COMPILE_OPTIMIZATION_LEVEL)
    {
        infoLog << "Mismatched compilation flags.";
        return LinkResult(false, gl::Error(GL_NO_ERROR));
    }

    stream->readInt(&mShaderVersion);

    // TODO(jmadill): replace MAX_VERTEX_ATTRIBS
    for (int i = 0; i < gl::MAX_VERTEX_ATTRIBS; ++i)
    {
        stream->readInt(&mSemanticIndexes[i]);
    }

    const unsigned int psSamplerCount = stream->readInt<unsigned int>();
    for (unsigned int i = 0; i < psSamplerCount; ++i)
    {
        Sampler sampler;
        stream->readBool(&sampler.active);
        stream->readInt(&sampler.logicalTextureUnit);
        stream->readInt(&sampler.textureType);
        mSamplersPS.push_back(sampler);
    }
    const unsigned int vsSamplerCount = stream->readInt<unsigned int>();
    for (unsigned int i = 0; i < vsSamplerCount; ++i)
    {
        Sampler sampler;
        stream->readBool(&sampler.active);
        stream->readInt(&sampler.logicalTextureUnit);
        stream->readInt(&sampler.textureType);
        mSamplersVS.push_back(sampler);
    }

    stream->readInt(&mUsedVertexSamplerRange);
    stream->readInt(&mUsedPixelSamplerRange);

    const unsigned int uniformCount = stream->readInt<unsigned int>();
    if (stream->error())
    {
        infoLog << "Invalid program binary.";
        return LinkResult(false, gl::Error(GL_NO_ERROR));
    }

    const auto &linkedUniforms = mData.getUniforms();
    ASSERT(mD3DUniforms.empty());
    for (unsigned int uniformIndex = 0; uniformIndex < uniformCount; uniformIndex++)
    {
        const gl::LinkedUniform &linkedUniform = linkedUniforms[uniformIndex];

        D3DUniform *d3dUniform =
            new D3DUniform(linkedUniform.type, linkedUniform.name, linkedUniform.arraySize,
                           linkedUniform.isInDefaultBlock());
        stream->readInt(&d3dUniform->psRegisterIndex);
        stream->readInt(&d3dUniform->vsRegisterIndex);
        stream->readInt(&d3dUniform->registerCount);
        stream->readInt(&d3dUniform->registerElement);

        mD3DUniforms.push_back(d3dUniform);
    }

    const unsigned int transformFeedbackVaryingCount = stream->readInt<unsigned int>();
    mTransformFeedbackLinkedVaryings.resize(transformFeedbackVaryingCount);
    for (unsigned int varyingIndex = 0; varyingIndex < transformFeedbackVaryingCount; varyingIndex++)
    {
        gl::LinkedVarying &varying = mTransformFeedbackLinkedVaryings[varyingIndex];

        stream->readString(&varying.name);
        stream->readInt(&varying.type);
        stream->readInt(&varying.size);
        stream->readString(&varying.semanticName);
        stream->readInt(&varying.semanticIndex);
        stream->readInt(&varying.semanticIndexCount);
    }

    stream->readString(&mVertexHLSL);
    stream->readBytes(reinterpret_cast<unsigned char*>(&mVertexWorkarounds), sizeof(D3DCompilerWorkarounds));
    stream->readString(&mPixelHLSL);
    stream->readBytes(reinterpret_cast<unsigned char*>(&mPixelWorkarounds), sizeof(D3DCompilerWorkarounds));
    stream->readBool(&mUsesFragDepth);
    stream->readBool(&mUsesPointSize);

    const size_t pixelShaderKeySize = stream->readInt<unsigned int>();
    mPixelShaderKey.resize(pixelShaderKeySize);
    for (size_t pixelShaderKeyIndex = 0; pixelShaderKeyIndex < pixelShaderKeySize; pixelShaderKeyIndex++)
    {
        stream->readInt(&mPixelShaderKey[pixelShaderKeyIndex].type);
        stream->readString(&mPixelShaderKey[pixelShaderKeyIndex].name);
        stream->readString(&mPixelShaderKey[pixelShaderKeyIndex].source);
        stream->readInt(&mPixelShaderKey[pixelShaderKeyIndex].outputIndex);
    }

    const unsigned char* binary = reinterpret_cast<const unsigned char*>(stream->data());

    const unsigned int vertexShaderCount = stream->readInt<unsigned int>();
    for (unsigned int vertexShaderIndex = 0; vertexShaderIndex < vertexShaderCount; vertexShaderIndex++)
    {
        size_t inputLayoutSize = stream->readInt<size_t>();
        gl::InputLayout inputLayout(inputLayoutSize, gl::VERTEX_FORMAT_INVALID);

        for (size_t inputIndex = 0; inputIndex < inputLayoutSize; inputIndex++)
        {
            inputLayout[inputIndex] = stream->readInt<gl::VertexFormatType>();
        }

        unsigned int vertexShaderSize = stream->readInt<unsigned int>();
        const unsigned char *vertexShaderFunction = binary + stream->offset();

        ShaderExecutableD3D *shaderExecutable = nullptr;

        gl::Error error = mRenderer->loadExecutable(
            vertexShaderFunction, vertexShaderSize, SHADER_VERTEX, mTransformFeedbackLinkedVaryings,
            (mData.getTransformFeedbackBufferMode() == GL_SEPARATE_ATTRIBS), &shaderExecutable);
        if (error.isError())
        {
            return LinkResult(false, error);
        }

        if (!shaderExecutable)
        {
            infoLog << "Could not create vertex shader.";
            return LinkResult(false, gl::Error(GL_NO_ERROR));
        }

        // generated converted input layout
        VertexExecutable::Signature signature;
        VertexExecutable::getSignature(mRenderer, inputLayout, &signature);

        // add new binary
        mVertexExecutables.push_back(new VertexExecutable(inputLayout, signature, shaderExecutable));

        stream->skip(vertexShaderSize);
    }

    const size_t pixelShaderCount = stream->readInt<unsigned int>();
    for (size_t pixelShaderIndex = 0; pixelShaderIndex < pixelShaderCount; pixelShaderIndex++)
    {
        const size_t outputCount = stream->readInt<unsigned int>();
        std::vector<GLenum> outputs(outputCount);
        for (size_t outputIndex = 0; outputIndex < outputCount; outputIndex++)
        {
            stream->readInt(&outputs[outputIndex]);
        }

        const size_t pixelShaderSize = stream->readInt<unsigned int>();
        const unsigned char *pixelShaderFunction = binary + stream->offset();
        ShaderExecutableD3D *shaderExecutable    = nullptr;

        gl::Error error = mRenderer->loadExecutable(
            pixelShaderFunction, pixelShaderSize, SHADER_PIXEL, mTransformFeedbackLinkedVaryings,
            (mData.getTransformFeedbackBufferMode() == GL_SEPARATE_ATTRIBS), &shaderExecutable);
        if (error.isError())
        {
            return LinkResult(false, error);
        }

        if (!shaderExecutable)
        {
            infoLog << "Could not create pixel shader.";
            return LinkResult(false, gl::Error(GL_NO_ERROR));
        }

        // add new binary
        mPixelExecutables.push_back(new PixelExecutable(outputs, shaderExecutable));

        stream->skip(pixelShaderSize);
    }

    unsigned int geometryShaderSize = stream->readInt<unsigned int>();

    if (geometryShaderSize > 0)
    {
        const unsigned char *geometryShaderFunction = binary + stream->offset();
        gl::Error error                             = mRenderer->loadExecutable(
            geometryShaderFunction, geometryShaderSize, SHADER_GEOMETRY,
            mTransformFeedbackLinkedVaryings,
            (mData.getTransformFeedbackBufferMode() == GL_SEPARATE_ATTRIBS), &mGeometryExecutable);
        if (error.isError())
        {
            return LinkResult(false, error);
        }

        if (!mGeometryExecutable)
        {
            infoLog << "Could not create geometry shader.";
            return LinkResult(false, gl::Error(GL_NO_ERROR));
        }
        stream->skip(geometryShaderSize);
    }

    initializeUniformStorage();
    initAttributesByLayout();

    return LinkResult(true, gl::Error(GL_NO_ERROR));
}

gl::Error ProgramD3D::save(gl::BinaryOutputStream *stream)
{
    // Output the DeviceIdentifier before we output any shader code
    // When we load the binary again later, we can validate the device identifier before trying to compile any HLSL
    DeviceIdentifier binaryIdentifier = mRenderer->getAdapterIdentifier();
    stream->writeBytes(reinterpret_cast<unsigned char*>(&binaryIdentifier), sizeof(DeviceIdentifier));

    stream->writeInt(ANGLE_COMPILE_OPTIMIZATION_LEVEL);

    stream->writeInt(mShaderVersion);

    // TODO(jmadill): replace MAX_VERTEX_ATTRIBS
    for (unsigned int i = 0; i < gl::MAX_VERTEX_ATTRIBS; ++i)
    {
        stream->writeInt(mSemanticIndexes[i]);
    }

    stream->writeInt(mSamplersPS.size());
    for (unsigned int i = 0; i < mSamplersPS.size(); ++i)
    {
        stream->writeInt(mSamplersPS[i].active);
        stream->writeInt(mSamplersPS[i].logicalTextureUnit);
        stream->writeInt(mSamplersPS[i].textureType);
    }

    stream->writeInt(mSamplersVS.size());
    for (unsigned int i = 0; i < mSamplersVS.size(); ++i)
    {
        stream->writeInt(mSamplersVS[i].active);
        stream->writeInt(mSamplersVS[i].logicalTextureUnit);
        stream->writeInt(mSamplersVS[i].textureType);
    }

    stream->writeInt(mUsedVertexSamplerRange);
    stream->writeInt(mUsedPixelSamplerRange);

    stream->writeInt(mD3DUniforms.size());
    for (size_t uniformIndex = 0; uniformIndex < mD3DUniforms.size(); ++uniformIndex)
    {
        const D3DUniform &uniform = *mD3DUniforms[uniformIndex];

        // Type, name and arraySize are redundant, so aren't stored in the binary.
        stream->writeInt(uniform.psRegisterIndex);
        stream->writeInt(uniform.vsRegisterIndex);
        stream->writeInt(uniform.registerCount);
        stream->writeInt(uniform.registerElement);
    }

    stream->writeInt(mTransformFeedbackLinkedVaryings.size());
    for (size_t i = 0; i < mTransformFeedbackLinkedVaryings.size(); i++)
    {
        const gl::LinkedVarying &varying = mTransformFeedbackLinkedVaryings[i];

        stream->writeString(varying.name);
        stream->writeInt(varying.type);
        stream->writeInt(varying.size);
        stream->writeString(varying.semanticName);
        stream->writeInt(varying.semanticIndex);
        stream->writeInt(varying.semanticIndexCount);
    }

    stream->writeString(mVertexHLSL);
    stream->writeBytes(reinterpret_cast<unsigned char*>(&mVertexWorkarounds), sizeof(D3DCompilerWorkarounds));
    stream->writeString(mPixelHLSL);
    stream->writeBytes(reinterpret_cast<unsigned char*>(&mPixelWorkarounds), sizeof(D3DCompilerWorkarounds));
    stream->writeInt(mUsesFragDepth);
    stream->writeInt(mUsesPointSize);

    const std::vector<PixelShaderOutputVariable> &pixelShaderKey = mPixelShaderKey;
    stream->writeInt(pixelShaderKey.size());
    for (size_t pixelShaderKeyIndex = 0; pixelShaderKeyIndex < pixelShaderKey.size(); pixelShaderKeyIndex++)
    {
        const PixelShaderOutputVariable &variable = pixelShaderKey[pixelShaderKeyIndex];
        stream->writeInt(variable.type);
        stream->writeString(variable.name);
        stream->writeString(variable.source);
        stream->writeInt(variable.outputIndex);
    }

    stream->writeInt(mVertexExecutables.size());
    for (size_t vertexExecutableIndex = 0; vertexExecutableIndex < mVertexExecutables.size(); vertexExecutableIndex++)
    {
        VertexExecutable *vertexExecutable = mVertexExecutables[vertexExecutableIndex];

        const auto &inputLayout = vertexExecutable->inputs();
        stream->writeInt(inputLayout.size());

        for (size_t inputIndex = 0; inputIndex < inputLayout.size(); inputIndex++)
        {
            stream->writeInt(inputLayout[inputIndex]);
        }

        size_t vertexShaderSize = vertexExecutable->shaderExecutable()->getLength();
        stream->writeInt(vertexShaderSize);

        const uint8_t *vertexBlob = vertexExecutable->shaderExecutable()->getFunction();
        stream->writeBytes(vertexBlob, vertexShaderSize);
    }

    stream->writeInt(mPixelExecutables.size());
    for (size_t pixelExecutableIndex = 0; pixelExecutableIndex < mPixelExecutables.size(); pixelExecutableIndex++)
    {
        PixelExecutable *pixelExecutable = mPixelExecutables[pixelExecutableIndex];

        const std::vector<GLenum> outputs = pixelExecutable->outputSignature();
        stream->writeInt(outputs.size());
        for (size_t outputIndex = 0; outputIndex < outputs.size(); outputIndex++)
        {
            stream->writeInt(outputs[outputIndex]);
        }

        size_t pixelShaderSize = pixelExecutable->shaderExecutable()->getLength();
        stream->writeInt(pixelShaderSize);

        const uint8_t *pixelBlob = pixelExecutable->shaderExecutable()->getFunction();
        stream->writeBytes(pixelBlob, pixelShaderSize);
    }

    size_t geometryShaderSize = (mGeometryExecutable != NULL) ? mGeometryExecutable->getLength() : 0;
    stream->writeInt(geometryShaderSize);

    if (mGeometryExecutable != NULL && geometryShaderSize > 0)
    {
        const uint8_t *geometryBlob = mGeometryExecutable->getFunction();
        stream->writeBytes(geometryBlob, geometryShaderSize);
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error ProgramD3D::getPixelExecutableForFramebuffer(const gl::Framebuffer *fbo, ShaderExecutableD3D **outExecutable)
{
    mPixelShaderOutputFormatCache.clear();

    const FramebufferD3D *fboD3D = GetImplAs<FramebufferD3D>(fbo);
    const gl::AttachmentList &colorbuffers = fboD3D->getColorAttachmentsForRender(mRenderer->getWorkarounds());

    for (size_t colorAttachment = 0; colorAttachment < colorbuffers.size(); ++colorAttachment)
    {
        const gl::FramebufferAttachment *colorbuffer = colorbuffers[colorAttachment];

        if (colorbuffer)
        {
            mPixelShaderOutputFormatCache.push_back(colorbuffer->getBinding() == GL_BACK ? GL_COLOR_ATTACHMENT0 : colorbuffer->getBinding());
        }
        else
        {
            mPixelShaderOutputFormatCache.push_back(GL_NONE);
        }
    }

    return getPixelExecutableForOutputLayout(mPixelShaderOutputFormatCache, outExecutable, nullptr);
}

gl::Error ProgramD3D::getPixelExecutableForOutputLayout(const std::vector<GLenum> &outputSignature,
                                                        ShaderExecutableD3D **outExectuable,
                                                        gl::InfoLog *infoLog)
{
    for (size_t executableIndex = 0; executableIndex < mPixelExecutables.size(); executableIndex++)
    {
        if (mPixelExecutables[executableIndex]->matchesSignature(outputSignature))
        {
            *outExectuable = mPixelExecutables[executableIndex]->shaderExecutable();
            return gl::Error(GL_NO_ERROR);
        }
    }

    std::string finalPixelHLSL = mDynamicHLSL->generatePixelShaderForOutputSignature(mPixelHLSL, mPixelShaderKey, mUsesFragDepth,
                                                                                     outputSignature);

    // Generate new pixel executable
    ShaderExecutableD3D *pixelExecutable = NULL;

    gl::InfoLog tempInfoLog;
    gl::InfoLog *currentInfoLog = infoLog ? infoLog : &tempInfoLog;

    gl::Error error = mRenderer->compileToExecutable(
        *currentInfoLog, finalPixelHLSL, SHADER_PIXEL, mTransformFeedbackLinkedVaryings,
        (mData.getTransformFeedbackBufferMode() == GL_SEPARATE_ATTRIBS), mPixelWorkarounds,
        &pixelExecutable);
    if (error.isError())
    {
        return error;
    }

    if (pixelExecutable)
    {
        mPixelExecutables.push_back(new PixelExecutable(outputSignature, pixelExecutable));
    }
    else if (!infoLog)
    {
        std::vector<char> tempCharBuffer(tempInfoLog.getLength() + 3);
        tempInfoLog.getLog(static_cast<GLsizei>(tempInfoLog.getLength()), NULL, &tempCharBuffer[0]);
        ERR("Error compiling dynamic pixel executable:\n%s\n", &tempCharBuffer[0]);
    }

    *outExectuable = pixelExecutable;
    return gl::Error(GL_NO_ERROR);
}

gl::Error ProgramD3D::getVertexExecutableForInputLayout(const gl::InputLayout &inputLayout,
                                                        ShaderExecutableD3D **outExectuable,
                                                        gl::InfoLog *infoLog)
{
    VertexExecutable::getSignature(mRenderer, inputLayout, &mCachedVertexSignature);

    for (size_t executableIndex = 0; executableIndex < mVertexExecutables.size(); executableIndex++)
    {
        if (mVertexExecutables[executableIndex]->matchesSignature(mCachedVertexSignature))
        {
            *outExectuable = mVertexExecutables[executableIndex]->shaderExecutable();
            return gl::Error(GL_NO_ERROR);
        }
    }

    // Generate new dynamic layout with attribute conversions
    std::string finalVertexHLSL = mDynamicHLSL->generateVertexShaderForInputLayout(
        mVertexHLSL, inputLayout, mData.getAttributes());

    // Generate new vertex executable
    ShaderExecutableD3D *vertexExecutable = NULL;

    gl::InfoLog tempInfoLog;
    gl::InfoLog *currentInfoLog = infoLog ? infoLog : &tempInfoLog;

    gl::Error error = mRenderer->compileToExecutable(
        *currentInfoLog, finalVertexHLSL, SHADER_VERTEX, mTransformFeedbackLinkedVaryings,
        (mData.getTransformFeedbackBufferMode() == GL_SEPARATE_ATTRIBS), mVertexWorkarounds,
        &vertexExecutable);
    if (error.isError())
    {
        return error;
    }

    if (vertexExecutable)
    {
        mVertexExecutables.push_back(new VertexExecutable(inputLayout, mCachedVertexSignature, vertexExecutable));
    }
    else if (!infoLog)
    {
        std::vector<char> tempCharBuffer(tempInfoLog.getLength() + 3);
        tempInfoLog.getLog(static_cast<GLsizei>(tempInfoLog.getLength()), NULL, &tempCharBuffer[0]);
        ERR("Error compiling dynamic vertex executable:\n%s\n", &tempCharBuffer[0]);
    }

    *outExectuable = vertexExecutable;
    return gl::Error(GL_NO_ERROR);
}

LinkResult ProgramD3D::compileProgramExecutables(gl::InfoLog &infoLog,
                                                 int registers,
                                                 const std::vector<PackedVarying> &packedVaryings)
{
    const ShaderD3D *fragmentShaderD3D = GetImplAs<ShaderD3D>(mData.getAttachedFragmentShader());

    const gl::InputLayout &defaultInputLayout =
        GetDefaultInputLayoutFromShader(mData.getAttachedVertexShader());
    ShaderExecutableD3D *defaultVertexExecutable = NULL;
    gl::Error error = getVertexExecutableForInputLayout(defaultInputLayout, &defaultVertexExecutable, &infoLog);
    if (error.isError())
    {
        return LinkResult(false, error);
    }

    std::vector<GLenum> defaultPixelOutput = GetDefaultOutputLayoutFromShader(getPixelShaderKey());
    ShaderExecutableD3D *defaultPixelExecutable = NULL;
    error = getPixelExecutableForOutputLayout(defaultPixelOutput, &defaultPixelExecutable, &infoLog);
    if (error.isError())
    {
        return LinkResult(false, error);
    }

    if (usesGeometryShader())
    {
        std::string geometryHLSL =
            mDynamicHLSL->generateGeometryShaderHLSL(registers, fragmentShaderD3D, packedVaryings);

        error = mRenderer->compileToExecutable(
            infoLog, geometryHLSL, SHADER_GEOMETRY, mTransformFeedbackLinkedVaryings,
            (mData.getTransformFeedbackBufferMode() == GL_SEPARATE_ATTRIBS),
            D3DCompilerWorkarounds(), &mGeometryExecutable);
        if (error.isError())
        {
            return LinkResult(false, error);
        }
    }

#if ANGLE_SHADER_DEBUG_INFO == ANGLE_ENABLED
    const ShaderD3D *vertexShaderD3D = GetImplAs<ShaderD3D>(mData.getAttachedVertexShader());

    if (usesGeometryShader() && mGeometryExecutable)
    {
        // Geometry shaders are currently only used internally, so there is no corresponding shader object at the interface level
        // For now the geometry shader debug info is pre-pended to the vertex shader, this is a bit of a clutch
        vertexShaderD3D->appendDebugInfo("// GEOMETRY SHADER BEGIN\n\n");
        vertexShaderD3D->appendDebugInfo(mGeometryExecutable->getDebugInfo());
        vertexShaderD3D->appendDebugInfo("\nGEOMETRY SHADER END\n\n\n");
    }

    if (defaultVertexExecutable)
    {
        vertexShaderD3D->appendDebugInfo(defaultVertexExecutable->getDebugInfo());
    }

    if (defaultPixelExecutable)
    {
        fragmentShaderD3D->appendDebugInfo(defaultPixelExecutable->getDebugInfo());
    }
#endif

    bool linkSuccess = (defaultVertexExecutable && defaultPixelExecutable && (!usesGeometryShader() || mGeometryExecutable));
    return LinkResult(linkSuccess, gl::Error(GL_NO_ERROR));
}

LinkResult ProgramD3D::link(const gl::Data &data, gl::InfoLog &infoLog)
{
    reset();

    // TODO(jmadill): structures containing samplers
    for (const gl::LinkedUniform &linkedUniform : mData.getUniforms())
    {
        if (linkedUniform.isSampler() && linkedUniform.isField())
        {
            infoLog << "Structures containing samplers not currently supported in D3D.";
            return LinkResult(false, gl::Error(GL_NO_ERROR));
        }
    }

    const gl::Shader *vertexShader   = mData.getAttachedVertexShader();
    const gl::Shader *fragmentShader = mData.getAttachedFragmentShader();

    const ShaderD3D *vertexShaderD3D   = GetImplAs<ShaderD3D>(vertexShader);
    const ShaderD3D *fragmentShaderD3D = GetImplAs<ShaderD3D>(fragmentShader);

    mSamplersVS.resize(data.caps->maxVertexTextureImageUnits);
    mSamplersPS.resize(data.caps->maxTextureImageUnits);

    mVertexHLSL = vertexShader->getTranslatedSource();
    vertexShaderD3D->generateWorkarounds(&mVertexWorkarounds);
    mShaderVersion = vertexShader->getShaderVersion();

    mPixelHLSL = fragmentShader->getTranslatedSource();
    fragmentShaderD3D->generateWorkarounds(&mPixelWorkarounds);

    if (mRenderer->getRendererLimitations().noFrontFacingSupport)
    {
        if (fragmentShaderD3D->usesFrontFacing())
        {
            infoLog << "The current renderer doesn't support gl_FrontFacing";
            return LinkResult(false, gl::Error(GL_NO_ERROR));
        }
    }

    std::vector<PackedVarying> packedVaryings =
        MergeVaryings(*vertexShader, *fragmentShader, mData.getTransformFeedbackVaryingNames());

    // Map the varyings to the register file
    int registers = mDynamicHLSL->packVaryings(infoLog, &packedVaryings,
                                               mData.getTransformFeedbackVaryingNames());

    if (registers < 0)
    {
        return LinkResult(false, gl::Error(GL_NO_ERROR));
    }

    std::vector<gl::LinkedVarying> linkedVaryings;
    if (!mDynamicHLSL->generateShaderLinkHLSL(data, mData, infoLog, registers, mPixelHLSL,
                                              mVertexHLSL, packedVaryings, &linkedVaryings,
                                              &mPixelShaderKey, &mUsesFragDepth))
    {
        return LinkResult(false, gl::Error(GL_NO_ERROR));
    }

    mUsesPointSize = vertexShaderD3D->usesPointSize();

    initSemanticIndex();

    defineUniformsAndAssignRegisters();

    gatherTransformFeedbackVaryings(linkedVaryings);

    LinkResult result = compileProgramExecutables(infoLog, registers, packedVaryings);
    if (result.error.isError() || !result.linkSuccess)
    {
        infoLog << "Failed to create D3D shaders.";
        return result;
    }

    return LinkResult(true, gl::Error(GL_NO_ERROR));
}

GLboolean ProgramD3D::validate(const gl::Caps & /*caps*/, gl::InfoLog * /*infoLog*/)
{
    // TODO(jmadill): Do something useful here?
    return GL_TRUE;
}

void ProgramD3D::gatherUniformBlockInfo(std::vector<gl::UniformBlock> *uniformBlocks,
                                        std::vector<gl::LinkedUniform> *uniforms)
{
    const gl::Shader *vertexShader = mData.getAttachedVertexShader();

    BlockInfoMap blockInfo;
    std::map<std::string, size_t> blockDataSizes;

    for (const sh::InterfaceBlock &vertexBlock : vertexShader->getInterfaceBlocks())
    {
        if (!vertexBlock.staticUse && vertexBlock.layout == sh::BLOCKLAYOUT_PACKED)
            continue;

        if (blockDataSizes.count(vertexBlock.name) > 0)
            continue;

        size_t dataSize                  = defineUniformBlock(vertexBlock, &blockInfo);
        blockDataSizes[vertexBlock.name] = dataSize;
    }

    const gl::Shader *fragmentShader = mData.getAttachedFragmentShader();

    for (const sh::InterfaceBlock &fragmentBlock : fragmentShader->getInterfaceBlocks())
    {
        if (!fragmentBlock.staticUse && fragmentBlock.layout == sh::BLOCKLAYOUT_PACKED)
            continue;

        if (blockDataSizes.count(fragmentBlock.name) > 0)
            continue;

        size_t dataSize                    = defineUniformBlock(fragmentBlock, &blockInfo);
        blockDataSizes[fragmentBlock.name] = dataSize;
    }

    // Copy block info out to uniforms.
    for (gl::LinkedUniform &linkedUniform : *uniforms)
    {
        const auto &infoEntry = blockInfo.find(linkedUniform.name);

        if (infoEntry != blockInfo.end())
        {
            linkedUniform.blockInfo = infoEntry->second;
        }
    }

    // Assign registers and update sizes.
    const ShaderD3D *vertexShaderD3D   = GetImplAs<ShaderD3D>(vertexShader);
    const ShaderD3D *fragmentShaderD3D = GetImplAs<ShaderD3D>(fragmentShader);

    for (gl::UniformBlock &uniformBlock : *uniformBlocks)
    {
        unsigned int uniformBlockElement = uniformBlock.isArray ? uniformBlock.arrayElement : 0;

        if (uniformBlock.vertexStaticUse)
        {
            unsigned int baseRegister =
                vertexShaderD3D->getInterfaceBlockRegister(uniformBlock.name);
            uniformBlock.vsRegisterIndex = baseRegister + uniformBlockElement;
        }

        if (uniformBlock.fragmentStaticUse)
        {
            unsigned int baseRegister =
                fragmentShaderD3D->getInterfaceBlockRegister(uniformBlock.name);
            uniformBlock.psRegisterIndex = baseRegister + uniformBlockElement;
        }

        ASSERT(blockDataSizes.count(uniformBlock.name) == 1);
        uniformBlock.dataSize = static_cast<unsigned int>(blockDataSizes[uniformBlock.name]);
    }
}

void ProgramD3D::initializeUniformStorage()
{
    // Compute total default block size
    unsigned int vertexRegisters = 0;
    unsigned int fragmentRegisters = 0;
    for (const D3DUniform *d3dUniform : mD3DUniforms)
    {
        if (!d3dUniform->isSampler())
        {
            if (d3dUniform->isReferencedByVertexShader())
            {
                vertexRegisters = std::max(vertexRegisters,
                                           d3dUniform->vsRegisterIndex + d3dUniform->registerCount);
            }
            if (d3dUniform->isReferencedByFragmentShader())
            {
                fragmentRegisters = std::max(
                    fragmentRegisters, d3dUniform->psRegisterIndex + d3dUniform->registerCount);
            }
        }
    }

    mVertexUniformStorage = mRenderer->createUniformStorage(vertexRegisters * 16u);
    mFragmentUniformStorage = mRenderer->createUniformStorage(fragmentRegisters * 16u);
}

gl::Error ProgramD3D::applyUniforms()
{
    updateSamplerMapping();

    gl::Error error = mRenderer->applyUniforms(*this, mD3DUniforms);
    if (error.isError())
    {
        return error;
    }

    for (D3DUniform *d3dUniform : mD3DUniforms)
    {
        d3dUniform->dirty = false;
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error ProgramD3D::applyUniformBuffers(const gl::Data &data)
{
    mVertexUBOCache.clear();
    mFragmentUBOCache.clear();

    const unsigned int reservedBuffersInVS = mRenderer->getReservedVertexUniformBuffers();
    const unsigned int reservedBuffersInFS = mRenderer->getReservedFragmentUniformBuffers();

    const auto &uniformBlocks = mData.getUniformBlocks();
    for (unsigned int uniformBlockIndex = 0; uniformBlockIndex < uniformBlocks.size();
         uniformBlockIndex++)
    {
        const gl::UniformBlock &uniformBlock = uniformBlocks[uniformBlockIndex];
        GLuint blockBinding            = mData.getUniformBlockBinding(uniformBlockIndex);

        // Unnecessary to apply an unreferenced standard or shared UBO
        if (!uniformBlock.vertexStaticUse && !uniformBlock.fragmentStaticUse)
        {
            continue;
        }

        if (uniformBlock.vertexStaticUse)
        {
            unsigned int registerIndex = uniformBlock.vsRegisterIndex - reservedBuffersInVS;
            ASSERT(registerIndex < data.caps->maxVertexUniformBlocks);

            if (mVertexUBOCache.size() <= registerIndex)
            {
                mVertexUBOCache.resize(registerIndex + 1, -1);
            }

            ASSERT(mVertexUBOCache[registerIndex] == -1);
            mVertexUBOCache[registerIndex] = blockBinding;
        }

        if (uniformBlock.fragmentStaticUse)
        {
            unsigned int registerIndex = uniformBlock.psRegisterIndex - reservedBuffersInFS;
            ASSERT(registerIndex < data.caps->maxFragmentUniformBlocks);

            if (mFragmentUBOCache.size() <= registerIndex)
            {
                mFragmentUBOCache.resize(registerIndex + 1, -1);
            }

            ASSERT(mFragmentUBOCache[registerIndex] == -1);
            mFragmentUBOCache[registerIndex] = blockBinding;
        }
    }

    return mRenderer->setUniformBuffers(data, mVertexUBOCache, mFragmentUBOCache);
}

void ProgramD3D::dirtyAllUniforms()
{
    for (D3DUniform *d3dUniform : mD3DUniforms)
    {
        d3dUniform->dirty = true;
    }
}

void ProgramD3D::setUniform1fv(GLint location, GLsizei count, const GLfloat* v)
{
    setUniform(location, count, v, GL_FLOAT);
}

void ProgramD3D::setUniform2fv(GLint location, GLsizei count, const GLfloat *v)
{
    setUniform(location, count, v, GL_FLOAT_VEC2);
}

void ProgramD3D::setUniform3fv(GLint location, GLsizei count, const GLfloat *v)
{
    setUniform(location, count, v, GL_FLOAT_VEC3);
}

void ProgramD3D::setUniform4fv(GLint location, GLsizei count, const GLfloat *v)
{
    setUniform(location, count, v, GL_FLOAT_VEC4);
}

void ProgramD3D::setUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    setUniformMatrixfv<2, 2>(location, count, transpose, value, GL_FLOAT_MAT2);
}

void ProgramD3D::setUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    setUniformMatrixfv<3, 3>(location, count, transpose, value, GL_FLOAT_MAT3);
}

void ProgramD3D::setUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    setUniformMatrixfv<4, 4>(location, count, transpose, value, GL_FLOAT_MAT4);
}

void ProgramD3D::setUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    setUniformMatrixfv<2, 3>(location, count, transpose, value, GL_FLOAT_MAT2x3);
}

void ProgramD3D::setUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    setUniformMatrixfv<3, 2>(location, count, transpose, value, GL_FLOAT_MAT3x2);
}

void ProgramD3D::setUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    setUniformMatrixfv<2, 4>(location, count, transpose, value, GL_FLOAT_MAT2x4);
}

void ProgramD3D::setUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    setUniformMatrixfv<4, 2>(location, count, transpose, value, GL_FLOAT_MAT4x2);
}

void ProgramD3D::setUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    setUniformMatrixfv<3, 4>(location, count, transpose, value, GL_FLOAT_MAT3x4);
}

void ProgramD3D::setUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    setUniformMatrixfv<4, 3>(location, count, transpose, value, GL_FLOAT_MAT4x3);
}

void ProgramD3D::setUniform1iv(GLint location, GLsizei count, const GLint *v)
{
    setUniform(location, count, v, GL_INT);
}

void ProgramD3D::setUniform2iv(GLint location, GLsizei count, const GLint *v)
{
    setUniform(location, count, v, GL_INT_VEC2);
}

void ProgramD3D::setUniform3iv(GLint location, GLsizei count, const GLint *v)
{
    setUniform(location, count, v, GL_INT_VEC3);
}

void ProgramD3D::setUniform4iv(GLint location, GLsizei count, const GLint *v)
{
    setUniform(location, count, v, GL_INT_VEC4);
}

void ProgramD3D::setUniform1uiv(GLint location, GLsizei count, const GLuint *v)
{
    setUniform(location, count, v, GL_UNSIGNED_INT);
}

void ProgramD3D::setUniform2uiv(GLint location, GLsizei count, const GLuint *v)
{
    setUniform(location, count, v, GL_UNSIGNED_INT_VEC2);
}

void ProgramD3D::setUniform3uiv(GLint location, GLsizei count, const GLuint *v)
{
    setUniform(location, count, v, GL_UNSIGNED_INT_VEC3);
}

void ProgramD3D::setUniform4uiv(GLint location, GLsizei count, const GLuint *v)
{
    setUniform(location, count, v, GL_UNSIGNED_INT_VEC4);
}

void ProgramD3D::defineUniformsAndAssignRegisters()
{
    D3DUniformMap uniformMap;
    const gl::Shader *vertexShader   = mData.getAttachedVertexShader();
    for (const sh::Uniform &vertexUniform : vertexShader->getUniforms())

    {
        if (vertexUniform.staticUse)
        {
            defineUniformBase(vertexShader, vertexUniform, &uniformMap);
        }
    }

    const gl::Shader *fragmentShader   = mData.getAttachedFragmentShader();
    for (const sh::Uniform &fragmentUniform : fragmentShader->getUniforms())
    {
        if (fragmentUniform.staticUse)
        {
            defineUniformBase(fragmentShader, fragmentUniform, &uniformMap);
        }
    }

    // Initialize the D3DUniform list to mirror the indexing of the GL layer.
    for (const gl::LinkedUniform &glUniform : mData.getUniforms())
    {
        if (!glUniform.isInDefaultBlock())
            continue;

        auto mapEntry = uniformMap.find(glUniform.name);
        ASSERT(mapEntry != uniformMap.end());
        mD3DUniforms.push_back(mapEntry->second);
    }

    assignAllSamplerRegisters();
    initializeUniformStorage();
}

void ProgramD3D::defineUniformBase(const gl::Shader *shader,
                                   const sh::Uniform &uniform,
                                   D3DUniformMap *uniformMap)
{
    if (uniform.isBuiltIn())
    {
        defineUniform(shader->getType(), uniform, uniform.name, nullptr, uniformMap);
        return;
    }

    const ShaderD3D *shaderD3D = GetImplAs<ShaderD3D>(shader);

    unsigned int startRegister = shaderD3D->getUniformRegister(uniform.name);
    ShShaderOutput outputType = shaderD3D->getCompilerOutputType();
    sh::HLSLBlockEncoder encoder(sh::HLSLBlockEncoder::GetStrategyFor(outputType));
    encoder.skipRegisters(startRegister);

    defineUniform(shader->getType(), uniform, uniform.name, &encoder, uniformMap);
}

D3DUniform *ProgramD3D::getD3DUniformByName(const std::string &name)
{
    for (D3DUniform *d3dUniform : mD3DUniforms)
    {
        if (d3dUniform->name == name)
        {
            return d3dUniform;
        }
    }

    return nullptr;
}

void ProgramD3D::defineUniform(GLenum shaderType,
                               const sh::ShaderVariable &uniform,
                               const std::string &fullName,
                               sh::HLSLBlockEncoder *encoder,
                               D3DUniformMap *uniformMap)
{
    if (uniform.isStruct())
    {
        for (unsigned int elementIndex = 0; elementIndex < uniform.elementCount(); elementIndex++)
        {
            const std::string &elementString = (uniform.isArray() ? ArrayString(elementIndex) : "");

            if (encoder)
                encoder->enterAggregateType();

            for (size_t fieldIndex = 0; fieldIndex < uniform.fields.size(); fieldIndex++)
            {
                const sh::ShaderVariable &field = uniform.fields[fieldIndex];
                const std::string &fieldFullName = (fullName + elementString + "." + field.name);

                defineUniform(shaderType, field, fieldFullName, encoder, uniformMap);
            }

            if (encoder)
                encoder->exitAggregateType();
        }
        return;
    }

    // Not a struct. Arrays are treated as aggregate types.
    if (uniform.isArray() && encoder)
    {
        encoder->enterAggregateType();
    }

    // Advance the uniform offset, to track registers allocation for structs
    sh::BlockMemberInfo blockInfo =
        encoder ? encoder->encodeType(uniform.type, uniform.arraySize, false)
                : sh::BlockMemberInfo::getDefaultBlockInfo();

    auto uniformMapEntry   = uniformMap->find(fullName);
    D3DUniform *d3dUniform = nullptr;

    if (uniformMapEntry != uniformMap->end())
    {
        d3dUniform = uniformMapEntry->second;
    }
    else
    {
        d3dUniform = new D3DUniform(uniform.type, fullName, uniform.arraySize, true);
        (*uniformMap)[fullName] = d3dUniform;
    }

    if (encoder)
    {
        d3dUniform->registerElement =
            static_cast<unsigned int>(sh::HLSLBlockEncoder::getBlockRegisterElement(blockInfo));
        unsigned int reg =
            static_cast<unsigned int>(sh::HLSLBlockEncoder::getBlockRegister(blockInfo));
        if (shaderType == GL_FRAGMENT_SHADER)
        {
            d3dUniform->psRegisterIndex = reg;
        }
        else
        {
            ASSERT(shaderType == GL_VERTEX_SHADER);
            d3dUniform->vsRegisterIndex = reg;
        }

        // Arrays are treated as aggregate types
        if (uniform.isArray())
        {
            encoder->exitAggregateType();
        }
    }
}

template <typename T>
static inline void SetIfDirty(T *dest, const T& source, bool *dirtyFlag)
{
    ASSERT(dest != NULL);
    ASSERT(dirtyFlag != NULL);

    *dirtyFlag = *dirtyFlag || (memcmp(dest, &source, sizeof(T)) != 0);
    *dest = source;
}

template <typename T>
void ProgramD3D::setUniform(GLint location, GLsizei countIn, const T *v, GLenum targetUniformType)
{
    const int components = gl::VariableComponentCount(targetUniformType);
    const GLenum targetBoolType = gl::VariableBoolVectorType(targetUniformType);

    D3DUniform *targetUniform = getD3DUniformFromLocation(location);

    unsigned int elementCount = targetUniform->elementCount();
    unsigned int arrayElement = mData.getUniformLocations()[location].element;
    unsigned int count        = std::min(elementCount - arrayElement, static_cast<unsigned int>(countIn));

    if (targetUniform->type == targetUniformType)
    {
        T *target = reinterpret_cast<T *>(targetUniform->data) + arrayElement * 4;

        for (unsigned int i = 0; i < count; i++)
        {
            T *dest = target + (i * 4);
            const T *source = v + (i * components);

            for (int c = 0; c < components; c++)
            {
                SetIfDirty(dest + c, source[c], &targetUniform->dirty);
            }
            for (int c = components; c < 4; c++)
            {
                SetIfDirty(dest + c, T(0), &targetUniform->dirty);
            }
        }
    }
    else if (targetUniform->type == targetBoolType)
    {
        GLint *boolParams = reinterpret_cast<GLint *>(targetUniform->data) + arrayElement * 4;

        for (unsigned int i = 0; i < count; i++)
        {
            GLint *dest = boolParams + (i * 4);
            const T *source = v + (i * components);

            for (int c = 0; c < components; c++)
            {
                SetIfDirty(dest + c, (source[c] == static_cast<T>(0)) ? GL_FALSE : GL_TRUE, &targetUniform->dirty);
            }
            for (int c = components; c < 4; c++)
            {
                SetIfDirty(dest + c, GL_FALSE, &targetUniform->dirty);
            }
        }
    }
    else if (targetUniform->isSampler())
    {
        ASSERT(targetUniformType == GL_INT);

        GLint *target = reinterpret_cast<GLint *>(targetUniform->data) + arrayElement * 4;

        bool wasDirty = targetUniform->dirty;

        for (unsigned int i = 0; i < count; i++)
        {
            GLint *dest = target + (i * 4);
            const GLint *source = reinterpret_cast<const GLint*>(v) + (i * components);

            SetIfDirty(dest + 0, source[0], &targetUniform->dirty);
            SetIfDirty(dest + 1, 0, &targetUniform->dirty);
            SetIfDirty(dest + 2, 0, &targetUniform->dirty);
            SetIfDirty(dest + 3, 0, &targetUniform->dirty);
        }

        if (!wasDirty && targetUniform->dirty)
        {
            mDirtySamplerMapping = true;
        }
    }
    else UNREACHABLE();
}

template<typename T>
bool transposeMatrix(T *target, const GLfloat *value, int targetWidth, int targetHeight, int srcWidth, int srcHeight)
{
    bool dirty = false;
    int copyWidth = std::min(targetHeight, srcWidth);
    int copyHeight = std::min(targetWidth, srcHeight);

    for (int x = 0; x < copyWidth; x++)
    {
        for (int y = 0; y < copyHeight; y++)
        {
            SetIfDirty(target + (x * targetWidth + y), static_cast<T>(value[y * srcWidth + x]), &dirty);
        }
    }
    // clear unfilled right side
    for (int y = 0; y < copyWidth; y++)
    {
        for (int x = copyHeight; x < targetWidth; x++)
        {
            SetIfDirty(target + (y * targetWidth + x), static_cast<T>(0), &dirty);
        }
    }
    // clear unfilled bottom.
    for (int y = copyWidth; y < targetHeight; y++)
    {
        for (int x = 0; x < targetWidth; x++)
        {
            SetIfDirty(target + (y * targetWidth + x), static_cast<T>(0), &dirty);
        }
    }

    return dirty;
}

template<typename T>
bool expandMatrix(T *target, const GLfloat *value, int targetWidth, int targetHeight, int srcWidth, int srcHeight)
{
    bool dirty = false;
    int copyWidth = std::min(targetWidth, srcWidth);
    int copyHeight = std::min(targetHeight, srcHeight);

    for (int y = 0; y < copyHeight; y++)
    {
        for (int x = 0; x < copyWidth; x++)
        {
            SetIfDirty(target + (y * targetWidth + x), static_cast<T>(value[y * srcWidth + x]), &dirty);
        }
    }
    // clear unfilled right side
    for (int y = 0; y < copyHeight; y++)
    {
        for (int x = copyWidth; x < targetWidth; x++)
        {
            SetIfDirty(target + (y * targetWidth + x), static_cast<T>(0), &dirty);
        }
    }
    // clear unfilled bottom.
    for (int y = copyHeight; y < targetHeight; y++)
    {
        for (int x = 0; x < targetWidth; x++)
        {
            SetIfDirty(target + (y * targetWidth + x), static_cast<T>(0), &dirty);
        }
    }

    return dirty;
}

template <int cols, int rows>
void ProgramD3D::setUniformMatrixfv(GLint location,
                                    GLsizei countIn,
                                    GLboolean transpose,
                                    const GLfloat *value,
                                    GLenum targetUniformType)
{
    D3DUniform *targetUniform = getD3DUniformFromLocation(location);

    unsigned int elementCount = targetUniform->elementCount();
    unsigned int arrayElement = mData.getUniformLocations()[location].element;
    unsigned int count        = std::min(elementCount - arrayElement, static_cast<unsigned int>(countIn));

    const unsigned int targetMatrixStride = (4 * rows);
    GLfloat *target =
        (GLfloat *)(targetUniform->data + arrayElement * sizeof(GLfloat) * targetMatrixStride);

    for (unsigned int i = 0; i < count; i++)
    {
        // Internally store matrices as transposed versions to accomodate HLSL matrix indexing
        if (transpose == GL_FALSE)
        {
            targetUniform->dirty = transposeMatrix<GLfloat>(target, value, 4, rows, rows, cols) || targetUniform->dirty;
        }
        else
        {
            targetUniform->dirty = expandMatrix<GLfloat>(target, value, 4, rows, cols, rows) || targetUniform->dirty;
        }
        target += targetMatrixStride;
        value += cols * rows;
    }
}

size_t ProgramD3D::defineUniformBlock(const sh::InterfaceBlock &interfaceBlock,
                                      BlockInfoMap *blockInfoOut)
{
    ASSERT(interfaceBlock.staticUse || interfaceBlock.layout != sh::BLOCKLAYOUT_PACKED);

    // define member uniforms
    sh::Std140BlockEncoder std140Encoder;
    sh::HLSLBlockEncoder hlslEncoder(sh::HLSLBlockEncoder::ENCODE_PACKED);
    sh::BlockLayoutEncoder *encoder = nullptr;

    if (interfaceBlock.layout == sh::BLOCKLAYOUT_STANDARD)
    {
        encoder = &std140Encoder;
    }
    else
    {
        encoder = &hlslEncoder;
    }

    GetUniformBlockInfo(interfaceBlock.fields, "", encoder, interfaceBlock.isRowMajorLayout,
                        blockInfoOut);

    return encoder->getBlockSize();
}

void ProgramD3D::assignAllSamplerRegisters()
{
    for (const D3DUniform *d3dUniform : mD3DUniforms)
    {
        if (d3dUniform->isSampler())
        {
            assignSamplerRegisters(d3dUniform);
        }
    }
}

void ProgramD3D::assignSamplerRegisters(const D3DUniform *d3dUniform)
{
    ASSERT(d3dUniform->isSampler());
    ASSERT(d3dUniform->vsRegisterIndex != GL_INVALID_INDEX ||
           d3dUniform->psRegisterIndex != GL_INVALID_INDEX);

    if (d3dUniform->vsRegisterIndex != GL_INVALID_INDEX)
    {
        AssignSamplers(d3dUniform->vsRegisterIndex, d3dUniform->type, d3dUniform->arraySize,
                       mSamplersVS, &mUsedVertexSamplerRange);
    }

    if (d3dUniform->psRegisterIndex != GL_INVALID_INDEX)
    {
        AssignSamplers(d3dUniform->psRegisterIndex, d3dUniform->type, d3dUniform->arraySize,
                       mSamplersPS, &mUsedPixelSamplerRange);
    }
}

// static
void ProgramD3D::AssignSamplers(unsigned int startSamplerIndex,
                                GLenum samplerType,
                                unsigned int samplerCount,
                                std::vector<Sampler> &outSamplers,
                                GLuint *outUsedRange)
{
    unsigned int samplerIndex = startSamplerIndex;

    do
    {
        ASSERT(samplerIndex < outSamplers.size());
        Sampler *sampler            = &outSamplers[samplerIndex];
        sampler->active             = true;
        sampler->textureType        = gl::SamplerTypeToTextureType(samplerType);
        sampler->logicalTextureUnit = 0;
        *outUsedRange               = std::max(samplerIndex + 1, *outUsedRange);
        samplerIndex++;
    } while (samplerIndex < startSamplerIndex + samplerCount);
}

void ProgramD3D::reset()
{
    SafeDeleteContainer(mVertexExecutables);
    SafeDeleteContainer(mPixelExecutables);
    SafeDelete(mGeometryExecutable);

    mVertexHLSL.clear();
    mVertexWorkarounds = D3DCompilerWorkarounds();
    mShaderVersion = 100;

    mPixelHLSL.clear();
    mPixelWorkarounds = D3DCompilerWorkarounds();
    mUsesFragDepth = false;
    mPixelShaderKey.clear();
    mUsesPointSize = false;

    SafeDeleteContainer(mD3DUniforms);

    SafeDelete(mVertexUniformStorage);
    SafeDelete(mFragmentUniformStorage);

    mSamplersPS.clear();
    mSamplersVS.clear();

    mUsedVertexSamplerRange = 0;
    mUsedPixelSamplerRange = 0;
    mDirtySamplerMapping = true;

    std::fill(mSemanticIndexes, mSemanticIndexes + ArraySize(mSemanticIndexes), -1);
    std::fill(mAttributesByLayout, mAttributesByLayout + ArraySize(mAttributesByLayout), -1);

    mTransformFeedbackLinkedVaryings.clear();
}

unsigned int ProgramD3D::getSerial() const
{
    return mSerial;
}

unsigned int ProgramD3D::issueSerial()
{
    return mCurrentSerial++;
}

void ProgramD3D::initSemanticIndex()
{
    const gl::Shader *vertexShader = mData.getAttachedVertexShader();
    ASSERT(vertexShader != nullptr);

    // Init semantic index
    for (const sh::Attribute &attribute : mData.getAttributes())
    {
        int attributeIndex = attribute.location;
        int index          = vertexShader->getSemanticIndex(attribute.name);
        int regs           = gl::VariableRegisterCount(attribute.type);

        for (int reg = 0; reg < regs; ++reg)
        {
            mSemanticIndexes[attributeIndex + reg] = index + reg;
        }
    }

    initAttributesByLayout();
}

void ProgramD3D::initAttributesByLayout()
{
    for (int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
    {
        mAttributesByLayout[i] = i;
    }

    std::sort(&mAttributesByLayout[0], &mAttributesByLayout[gl::MAX_VERTEX_ATTRIBS],
              AttributeSorter(mSemanticIndexes));
}

void ProgramD3D::sortAttributesByLayout(const std::vector<TranslatedAttribute> &unsortedAttributes,
                                        int sortedSemanticIndicesOut[gl::MAX_VERTEX_ATTRIBS],
                                        const rx::TranslatedAttribute *sortedAttributesOut[gl::MAX_VERTEX_ATTRIBS]) const
{
    for (size_t attribIndex = 0; attribIndex < unsortedAttributes.size(); ++attribIndex)
    {
        int oldIndex = mAttributesByLayout[attribIndex];
        sortedSemanticIndicesOut[attribIndex] = mSemanticIndexes[oldIndex];
        sortedAttributesOut[attribIndex] = &unsortedAttributes[oldIndex];
    }
}

void ProgramD3D::updateCachedInputLayout(const gl::State &state)
{
    mCachedInputLayout.clear();
    const auto &vertexAttributes = state.getVertexArray()->getVertexAttributes();

    for (unsigned int attributeIndex : angle::IterateBitSet(mData.getActiveAttribLocationsMask()))
    {
        int semanticIndex = mSemanticIndexes[attributeIndex];

        if (semanticIndex != -1)
        {
            if (mCachedInputLayout.size() < static_cast<size_t>(semanticIndex + 1))
            {
                mCachedInputLayout.resize(semanticIndex + 1, gl::VERTEX_FORMAT_INVALID);
            }
            mCachedInputLayout[semanticIndex] =
                GetVertexFormatType(vertexAttributes[attributeIndex],
                                    state.getVertexAttribCurrentValue(attributeIndex).Type);
        }
    }
}

void ProgramD3D::gatherTransformFeedbackVaryings(
    const std::vector<gl::LinkedVarying> &linkedVaryings)
{
    // Gather the linked varyings that are used for transform feedback, they should all exist.
    mTransformFeedbackLinkedVaryings.clear();
    for (const std::string &tfVaryingName : mData.getTransformFeedbackVaryingNames())
    {
        for (const gl::LinkedVarying &linkedVarying : linkedVaryings)
        {
            if (tfVaryingName == linkedVarying.name)
            {
                mTransformFeedbackLinkedVaryings.push_back(linkedVarying);
                break;
            }
        }
    }
}

D3DUniform *ProgramD3D::getD3DUniformFromLocation(GLint location)
{
    return mD3DUniforms[mData.getUniformLocations()[location].index];
}
}
