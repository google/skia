//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Program.cpp: Implements the gl::Program class. Implements GL program objects
// and related functionality. [OpenGL ES 2.0.24] section 2.10.3 page 28.

#include "libANGLE/Program.h"

#include <algorithm>

#include "common/BitSetIterator.h"
#include "common/debug.h"
#include "common/platform.h"
#include "common/utilities.h"
#include "common/version.h"
#include "compiler/translator/blocklayout.h"
#include "libANGLE/Data.h"
#include "libANGLE/ResourceManager.h"
#include "libANGLE/features.h"
#include "libANGLE/renderer/Renderer.h"
#include "libANGLE/renderer/ProgramImpl.h"
#include "libANGLE/queryconversions.h"

namespace gl
{
const char * const g_fakepath = "C:\\fakepath";

namespace
{

unsigned int ParseAndStripArrayIndex(std::string* name)
{
    unsigned int subscript = GL_INVALID_INDEX;

    // Strip any trailing array operator and retrieve the subscript
    size_t open = name->find_last_of('[');
    size_t close = name->find_last_of(']');
    if (open != std::string::npos && close == name->length() - 1)
    {
        subscript = atoi(name->substr(open + 1).c_str());
        name->erase(open);
    }

    return subscript;
}

void WriteShaderVar(BinaryOutputStream *stream, const sh::ShaderVariable &var)
{
    stream->writeInt(var.type);
    stream->writeInt(var.precision);
    stream->writeString(var.name);
    stream->writeString(var.mappedName);
    stream->writeInt(var.arraySize);
    stream->writeInt(var.staticUse);
    stream->writeString(var.structName);
    ASSERT(var.fields.empty());
}

void LoadShaderVar(BinaryInputStream *stream, sh::ShaderVariable *var)
{
    var->type       = stream->readInt<GLenum>();
    var->precision  = stream->readInt<GLenum>();
    var->name       = stream->readString();
    var->mappedName = stream->readString();
    var->arraySize  = stream->readInt<unsigned int>();
    var->staticUse  = stream->readBool();
    var->structName = stream->readString();
}

template <typename VarT>
void DefineUniformBlockMembers(const std::vector<VarT> &fields,
                               const std::string &prefix,
                               int blockIndex,
                               std::vector<LinkedUniform> *uniformsOut)
{
    for (const VarT &field : fields)
    {
        const std::string &fieldName = (prefix.empty() ? field.name : prefix + "." + field.name);

        if (field.isStruct())
        {
            for (unsigned int arrayElement = 0; arrayElement < field.elementCount(); arrayElement++)
            {
                const std::string uniformElementName =
                    fieldName + (field.isArray() ? ArrayString(arrayElement) : "");
                DefineUniformBlockMembers(field.fields, uniformElementName, blockIndex,
                                          uniformsOut);
            }
        }
        else
        {
            // TODO(jmadill): record row-majorness?
            // Block layout is recorded in the Impl.
            LinkedUniform newUniform(field.type, field.precision, fieldName, field.arraySize,
                                     blockIndex, sh::BlockMemberInfo::getDefaultBlockInfo());

            // Since block uniforms have no location, we don't need to store them in the uniform
            // locations list.
            uniformsOut->push_back(newUniform);
        }
    }
}

// This simplified cast function doesn't need to worry about advanced concepts like
// depth range values, or casting to bool.
template <typename DestT, typename SrcT>
DestT UniformStateQueryCast(SrcT value);

// From-Float-To-Integer Casts
template <>
GLint UniformStateQueryCast(GLfloat value)
{
    return clampCast<GLint>(roundf(value));
}

template <>
GLuint UniformStateQueryCast(GLfloat value)
{
    return clampCast<GLuint>(roundf(value));
}

// From-Integer-to-Integer Casts
template <>
GLint UniformStateQueryCast(GLuint value)
{
    return clampCast<GLint>(value);
}

template <>
GLuint UniformStateQueryCast(GLint value)
{
    return clampCast<GLuint>(value);
}

// From-Boolean-to-Anything Casts
template <>
GLfloat UniformStateQueryCast(GLboolean value)
{
    return (value == GL_TRUE ? 1.0f : 0.0f);
}

template <>
GLint UniformStateQueryCast(GLboolean value)
{
    return (value == GL_TRUE ? 1 : 0);
}

template <>
GLuint UniformStateQueryCast(GLboolean value)
{
    return (value == GL_TRUE ? 1u : 0u);
}

// Default to static_cast
template <typename DestT, typename SrcT>
DestT UniformStateQueryCast(SrcT value)
{
    return static_cast<DestT>(value);
}

template <typename SrcT, typename DestT>
void UniformStateQueryCastLoop(DestT *dataOut, const uint8_t *srcPointer, int components)
{
    for (int comp = 0; comp < components; ++comp)
    {
        // We only work with strides of 4 bytes for uniform components. (GLfloat/GLint)
        // Don't use SrcT stride directly since GLboolean has a stride of 1 byte.
        size_t offset               = comp * 4;
        const SrcT *typedSrcPointer = reinterpret_cast<const SrcT *>(&srcPointer[offset]);
        dataOut[comp]               = UniformStateQueryCast<DestT>(*typedSrcPointer);
    }
}

bool UniformInList(const std::vector<LinkedUniform> &list, const std::string &name)
{
    for (const LinkedUniform &uniform : list)
    {
        if (uniform.name == name)
            return true;
    }

    return false;
}

}  // anonymous namespace

AttributeBindings::AttributeBindings()
{
}

AttributeBindings::~AttributeBindings()
{
}

InfoLog::InfoLog()
{
}

InfoLog::~InfoLog()
{
}

size_t InfoLog::getLength() const
{
    const std::string &logString = mStream.str();
    return logString.empty() ? 0 : logString.length() + 1;
}

void InfoLog::getLog(GLsizei bufSize, GLsizei *length, char *infoLog)
{
    size_t index = 0;

    if (bufSize > 0)
    {
        const std::string str(mStream.str());

        if (!str.empty())
        {
            index = std::min(static_cast<size_t>(bufSize) - 1, str.length());
            memcpy(infoLog, str.c_str(), index);
        }

        infoLog[index] = '\0';
    }

    if (length)
    {
        *length = static_cast<GLsizei>(index);
    }
}

// append a santized message to the program info log.
// The D3D compiler includes a fake file path in some of the warning or error 
// messages, so lets remove all occurrences of this fake file path from the log.
void InfoLog::appendSanitized(const char *message)
{
    std::string msg(message);

    size_t found;
    do
    {
        found = msg.find(g_fakepath);
        if (found != std::string::npos)
        {
            msg.erase(found, strlen(g_fakepath));
        }
    }
    while (found != std::string::npos);

    mStream << message << std::endl;
}

void InfoLog::reset()
{
}

VariableLocation::VariableLocation()
    : name(),
      element(0),
      index(0)
{
}

VariableLocation::VariableLocation(const std::string &name, unsigned int element, unsigned int index)
    : name(name),
      element(element),
      index(index)
{
}

LinkedVarying::LinkedVarying()
{
}

LinkedVarying::LinkedVarying(const std::string &name, GLenum type, GLsizei size, const std::string &semanticName,
                             unsigned int semanticIndex, unsigned int semanticIndexCount)
    : name(name), type(type), size(size), semanticName(semanticName), semanticIndex(semanticIndex), semanticIndexCount(semanticIndexCount)
{
}

Program::Data::Data()
    : mAttachedFragmentShader(nullptr),
      mAttachedVertexShader(nullptr),
      mTransformFeedbackBufferMode(GL_NONE)
{
}

Program::Data::~Data()
{
    if (mAttachedVertexShader != nullptr)
    {
        mAttachedVertexShader->release();
    }

    if (mAttachedFragmentShader != nullptr)
    {
        mAttachedFragmentShader->release();
    }
}

const LinkedUniform *Program::Data::getUniformByName(const std::string &name) const
{
    for (const LinkedUniform &linkedUniform : mUniforms)
    {
        if (linkedUniform.name == name)
        {
            return &linkedUniform;
        }
    }

    return nullptr;
}

GLint Program::Data::getUniformLocation(const std::string &name) const
{
    size_t subscript     = GL_INVALID_INDEX;
    std::string baseName = gl::ParseUniformName(name, &subscript);

    for (size_t location = 0; location < mUniformLocations.size(); ++location)
    {
        const VariableLocation &uniformLocation = mUniformLocations[location];
        const LinkedUniform &uniform            = mUniforms[uniformLocation.index];

        if (uniform.name == baseName)
        {
            if ((uniform.isArray() && uniformLocation.element == subscript) ||
                (subscript == GL_INVALID_INDEX))
            {
                return static_cast<GLint>(location);
            }
        }
    }

    return -1;
}

GLuint Program::Data::getUniformIndex(const std::string &name) const
{
    size_t subscript     = GL_INVALID_INDEX;
    std::string baseName = gl::ParseUniformName(name, &subscript);

    // The app is not allowed to specify array indices other than 0 for arrays of basic types
    if (subscript != 0 && subscript != GL_INVALID_INDEX)
    {
        return GL_INVALID_INDEX;
    }

    for (size_t index = 0; index < mUniforms.size(); index++)
    {
        const LinkedUniform &uniform = mUniforms[index];
        if (uniform.name == baseName)
        {
            if (uniform.isArray() || subscript == GL_INVALID_INDEX)
            {
                return static_cast<GLuint>(index);
            }
        }
    }

    return GL_INVALID_INDEX;
}

Program::Program(rx::ImplFactory *factory, ResourceManager *manager, GLuint handle)
    : mProgram(factory->createProgram(mData)),
      mValidated(false),
      mLinked(false),
      mDeleteStatus(false),
      mRefCount(0),
      mResourceManager(manager),
      mHandle(handle),
      mSamplerUniformRange(0, 0)
{
    ASSERT(mProgram);

    resetUniformBlockBindings();
    unlink();
}

Program::~Program()
{
    unlink(true);

    SafeDelete(mProgram);
}

bool Program::attachShader(Shader *shader)
{
    if (shader->getType() == GL_VERTEX_SHADER)
    {
        if (mData.mAttachedVertexShader)
        {
            return false;
        }

        mData.mAttachedVertexShader = shader;
        mData.mAttachedVertexShader->addRef();
    }
    else if (shader->getType() == GL_FRAGMENT_SHADER)
    {
        if (mData.mAttachedFragmentShader)
        {
            return false;
        }

        mData.mAttachedFragmentShader = shader;
        mData.mAttachedFragmentShader->addRef();
    }
    else UNREACHABLE();

    return true;
}

bool Program::detachShader(Shader *shader)
{
    if (shader->getType() == GL_VERTEX_SHADER)
    {
        if (mData.mAttachedVertexShader != shader)
        {
            return false;
        }

        shader->release();
        mData.mAttachedVertexShader = nullptr;
    }
    else if (shader->getType() == GL_FRAGMENT_SHADER)
    {
        if (mData.mAttachedFragmentShader != shader)
        {
            return false;
        }

        shader->release();
        mData.mAttachedFragmentShader = nullptr;
    }
    else UNREACHABLE();

    return true;
}

int Program::getAttachedShadersCount() const
{
    return (mData.mAttachedVertexShader ? 1 : 0) + (mData.mAttachedFragmentShader ? 1 : 0);
}

void AttributeBindings::bindAttributeLocation(GLuint index, const char *name)
{
    if (index < MAX_VERTEX_ATTRIBS)
    {
        for (int i = 0; i < MAX_VERTEX_ATTRIBS; i++)
        {
            mAttributeBinding[i].erase(name);
        }

        mAttributeBinding[index].insert(name);
    }
}

void Program::bindAttributeLocation(GLuint index, const char *name)
{
    mAttributeBindings.bindAttributeLocation(index, name);
}

// Links the HLSL code of the vertex and pixel shader by matching up their varyings,
// compiling them into binaries, determining the attribute mappings, and collecting
// a list of uniforms
Error Program::link(const gl::Data &data)
{
    unlink(false);

    mInfoLog.reset();
    resetUniformBlockBindings();

    if (!mData.mAttachedFragmentShader || !mData.mAttachedFragmentShader->isCompiled())
    {
        return Error(GL_NO_ERROR);
    }
    ASSERT(mData.mAttachedFragmentShader->getType() == GL_FRAGMENT_SHADER);

    if (!mData.mAttachedVertexShader || !mData.mAttachedVertexShader->isCompiled())
    {
        return Error(GL_NO_ERROR);
    }
    ASSERT(mData.mAttachedVertexShader->getType() == GL_VERTEX_SHADER);

    if (!linkAttributes(data, mInfoLog, mAttributeBindings, mData.mAttachedVertexShader))
    {
        return Error(GL_NO_ERROR);
    }

    if (!linkVaryings(mInfoLog, mData.mAttachedVertexShader, mData.mAttachedFragmentShader))
    {
        return Error(GL_NO_ERROR);
    }

    if (!linkUniforms(mInfoLog, *data.caps))
    {
        return Error(GL_NO_ERROR);
    }

    if (!linkUniformBlocks(mInfoLog, *data.caps))
    {
        return Error(GL_NO_ERROR);
    }

    const auto &mergedVaryings = getMergedVaryings();

    if (!linkValidateTransformFeedback(mInfoLog, mergedVaryings, *data.caps))
    {
        return Error(GL_NO_ERROR);
    }

    linkOutputVariables();

    rx::LinkResult result = mProgram->link(data, mInfoLog);
    if (result.error.isError() || !result.linkSuccess)
    {
        return result.error;
    }

    gatherTransformFeedbackVaryings(mergedVaryings);
    mProgram->gatherUniformBlockInfo(&mData.mUniformBlocks, &mData.mUniforms);

    mLinked = true;
    return gl::Error(GL_NO_ERROR);
}

int AttributeBindings::getAttributeBinding(const std::string &name) const
{
    for (int location = 0; location < MAX_VERTEX_ATTRIBS; location++)
    {
        if (mAttributeBinding[location].find(name) != mAttributeBinding[location].end())
        {
            return location;
        }
    }

    return -1;
}

// Returns the program object to an unlinked state, before re-linking, or at destruction
void Program::unlink(bool destroy)
{
    if (destroy)   // Object being destructed
    {
        if (mData.mAttachedFragmentShader)
        {
            mData.mAttachedFragmentShader->release();
            mData.mAttachedFragmentShader = nullptr;
        }

        if (mData.mAttachedVertexShader)
        {
            mData.mAttachedVertexShader->release();
            mData.mAttachedVertexShader = nullptr;
        }
    }

    mData.mAttributes.clear();
    mData.mActiveAttribLocationsMask.reset();
    mData.mTransformFeedbackVaryingVars.clear();
    mData.mUniforms.clear();
    mData.mUniformLocations.clear();
    mData.mUniformBlocks.clear();
    mData.mOutputVariables.clear();

    mValidated = false;

    mLinked = false;
}

bool Program::isLinked()
{
    return mLinked;
}

Error Program::loadBinary(GLenum binaryFormat, const void *binary, GLsizei length)
{
    unlink(false);

#if ANGLE_PROGRAM_BINARY_LOAD != ANGLE_ENABLED
    return Error(GL_NO_ERROR);
#else
    ASSERT(binaryFormat == mProgram->getBinaryFormat());

    BinaryInputStream stream(binary, length);

    GLenum format = stream.readInt<GLenum>();
    if (format != mProgram->getBinaryFormat())
    {
        mInfoLog << "Invalid program binary format.";
        return Error(GL_NO_ERROR);
    }

    int majorVersion = stream.readInt<int>();
    int minorVersion = stream.readInt<int>();
    if (majorVersion != ANGLE_MAJOR_VERSION || minorVersion != ANGLE_MINOR_VERSION)
    {
        mInfoLog << "Invalid program binary version.";
        return Error(GL_NO_ERROR);
    }

    unsigned char commitString[ANGLE_COMMIT_HASH_SIZE];
    stream.readBytes(commitString, ANGLE_COMMIT_HASH_SIZE);
    if (memcmp(commitString, ANGLE_COMMIT_HASH, sizeof(unsigned char) * ANGLE_COMMIT_HASH_SIZE) != 0)
    {
        mInfoLog << "Invalid program binary version.";
        return Error(GL_NO_ERROR);
    }

    static_assert(MAX_VERTEX_ATTRIBS <= sizeof(unsigned long) * 8,
                  "Too many vertex attribs for mask");
    mData.mActiveAttribLocationsMask = stream.readInt<unsigned long>();

    unsigned int attribCount = stream.readInt<unsigned int>();
    ASSERT(mData.mAttributes.empty());
    for (unsigned int attribIndex = 0; attribIndex < attribCount; ++attribIndex)
    {
        sh::Attribute attrib;
        LoadShaderVar(&stream, &attrib);
        attrib.location = stream.readInt<int>();
        mData.mAttributes.push_back(attrib);
    }

    unsigned int uniformCount = stream.readInt<unsigned int>();
    ASSERT(mData.mUniforms.empty());
    for (unsigned int uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
    {
        LinkedUniform uniform;
        LoadShaderVar(&stream, &uniform);

        uniform.blockIndex                 = stream.readInt<int>();
        uniform.blockInfo.offset           = stream.readInt<int>();
        uniform.blockInfo.arrayStride      = stream.readInt<int>();
        uniform.blockInfo.matrixStride     = stream.readInt<int>();
        uniform.blockInfo.isRowMajorMatrix = stream.readBool();

        mData.mUniforms.push_back(uniform);
    }

    const unsigned int uniformIndexCount = stream.readInt<unsigned int>();
    ASSERT(mData.mUniformLocations.empty());
    for (unsigned int uniformIndexIndex = 0; uniformIndexIndex < uniformIndexCount;
         uniformIndexIndex++)
    {
        VariableLocation variable;
        stream.readString(&variable.name);
        stream.readInt(&variable.element);
        stream.readInt(&variable.index);

        mData.mUniformLocations.push_back(variable);
    }

    unsigned int uniformBlockCount = stream.readInt<unsigned int>();
    ASSERT(mData.mUniformBlocks.empty());
    for (unsigned int uniformBlockIndex = 0; uniformBlockIndex < uniformBlockCount;
         ++uniformBlockIndex)
    {
        UniformBlock uniformBlock;
        stream.readString(&uniformBlock.name);
        stream.readBool(&uniformBlock.isArray);
        stream.readInt(&uniformBlock.arrayElement);
        stream.readInt(&uniformBlock.dataSize);
        stream.readBool(&uniformBlock.vertexStaticUse);
        stream.readBool(&uniformBlock.fragmentStaticUse);

        unsigned int numMembers = stream.readInt<unsigned int>();
        for (unsigned int blockMemberIndex = 0; blockMemberIndex < numMembers; blockMemberIndex++)
        {
            uniformBlock.memberUniformIndexes.push_back(stream.readInt<unsigned int>());
        }

        // TODO(jmadill): Make D3D-only
        stream.readInt(&uniformBlock.psRegisterIndex);
        stream.readInt(&uniformBlock.vsRegisterIndex);

        mData.mUniformBlocks.push_back(uniformBlock);
    }

    stream.readInt(&mData.mTransformFeedbackBufferMode);

    unsigned int outputVarCount = stream.readInt<unsigned int>();
    for (unsigned int outputIndex = 0; outputIndex < outputVarCount; ++outputIndex)
    {
        int locationIndex = stream.readInt<int>();
        VariableLocation locationData;
        stream.readInt(&locationData.element);
        stream.readInt(&locationData.index);
        stream.readString(&locationData.name);
        mData.mOutputVariables[locationIndex] = locationData;
    }

    stream.readInt(&mSamplerUniformRange.start);
    stream.readInt(&mSamplerUniformRange.end);

    rx::LinkResult result = mProgram->load(mInfoLog, &stream);
    if (result.error.isError() || !result.linkSuccess)
    {
        return result.error;
    }

    mLinked = true;
    return Error(GL_NO_ERROR);
#endif // #if ANGLE_PROGRAM_BINARY_LOAD == ANGLE_ENABLED
}

Error Program::saveBinary(GLenum *binaryFormat, void *binary, GLsizei bufSize, GLsizei *length) const
{
    if (binaryFormat)
    {
        *binaryFormat = mProgram->getBinaryFormat();
    }

    BinaryOutputStream stream;

    stream.writeInt(mProgram->getBinaryFormat());
    stream.writeInt(ANGLE_MAJOR_VERSION);
    stream.writeInt(ANGLE_MINOR_VERSION);
    stream.writeBytes(reinterpret_cast<const unsigned char*>(ANGLE_COMMIT_HASH), ANGLE_COMMIT_HASH_SIZE);

    stream.writeInt(mData.mActiveAttribLocationsMask.to_ulong());

    stream.writeInt(mData.mAttributes.size());
    for (const sh::Attribute &attrib : mData.mAttributes)
    {
        WriteShaderVar(&stream, attrib);
        stream.writeInt(attrib.location);
    }

    stream.writeInt(mData.mUniforms.size());
    for (const gl::LinkedUniform &uniform : mData.mUniforms)
    {
        WriteShaderVar(&stream, uniform);

        // FIXME: referenced

        stream.writeInt(uniform.blockIndex);
        stream.writeInt(uniform.blockInfo.offset);
        stream.writeInt(uniform.blockInfo.arrayStride);
        stream.writeInt(uniform.blockInfo.matrixStride);
        stream.writeInt(uniform.blockInfo.isRowMajorMatrix);
    }

    stream.writeInt(mData.mUniformLocations.size());
    for (const auto &variable : mData.mUniformLocations)
    {
        stream.writeString(variable.name);
        stream.writeInt(variable.element);
        stream.writeInt(variable.index);
    }

    stream.writeInt(mData.mUniformBlocks.size());
    for (const UniformBlock &uniformBlock : mData.mUniformBlocks)
    {
        stream.writeString(uniformBlock.name);
        stream.writeInt(uniformBlock.isArray);
        stream.writeInt(uniformBlock.arrayElement);
        stream.writeInt(uniformBlock.dataSize);

        stream.writeInt(uniformBlock.vertexStaticUse);
        stream.writeInt(uniformBlock.fragmentStaticUse);

        stream.writeInt(uniformBlock.memberUniformIndexes.size());
        for (unsigned int memberUniformIndex : uniformBlock.memberUniformIndexes)
        {
            stream.writeInt(memberUniformIndex);
        }

        // TODO(jmadill): make D3D-only
        stream.writeInt(uniformBlock.psRegisterIndex);
        stream.writeInt(uniformBlock.vsRegisterIndex);
    }

    stream.writeInt(mData.mTransformFeedbackBufferMode);

    stream.writeInt(mData.mOutputVariables.size());
    for (const auto &outputPair : mData.mOutputVariables)
    {
        stream.writeInt(outputPair.first);
        stream.writeInt(outputPair.second.element);
        stream.writeInt(outputPair.second.index);
        stream.writeString(outputPair.second.name);
    }

    stream.writeInt(mSamplerUniformRange.start);
    stream.writeInt(mSamplerUniformRange.end);

    gl::Error error = mProgram->save(&stream);
    if (error.isError())
    {
        return error;
    }

    GLsizei streamLength   = static_cast<GLsizei>(stream.length());
    const void *streamData = stream.data();

    if (streamLength > bufSize)
    {
        if (length)
        {
            *length = 0;
        }

        // TODO: This should be moved to the validation layer but computing the size of the binary before saving
        // it causes the save to happen twice.  It may be possible to write the binary to a separate buffer, validate
        // sizes and then copy it.
        return Error(GL_INVALID_OPERATION);
    }

    if (binary)
    {
        char *ptr = reinterpret_cast<char*>(binary);

        memcpy(ptr, streamData, streamLength);
        ptr += streamLength;

        ASSERT(ptr - streamLength == binary);
    }

    if (length)
    {
        *length = streamLength;
    }

    return Error(GL_NO_ERROR);
}

GLint Program::getBinaryLength() const
{
    GLint length;
    Error error = saveBinary(nullptr, nullptr, std::numeric_limits<GLint>::max(), &length);
    if (error.isError())
    {
        return 0;
    }

    return length;
}

void Program::release()
{
    mRefCount--;

    if (mRefCount == 0 && mDeleteStatus)
    {
        mResourceManager->deleteProgram(mHandle);
    }
}

void Program::addRef()
{
    mRefCount++;
}

unsigned int Program::getRefCount() const
{
    return mRefCount;
}

int Program::getInfoLogLength() const
{
    return static_cast<int>(mInfoLog.getLength());
}

void Program::getInfoLog(GLsizei bufSize, GLsizei *length, char *infoLog)
{
    return mInfoLog.getLog(bufSize, length, infoLog);
}

void Program::getAttachedShaders(GLsizei maxCount, GLsizei *count, GLuint *shaders)
{
    int total = 0;

    if (mData.mAttachedVertexShader)
    {
        if (total < maxCount)
        {
            shaders[total] = mData.mAttachedVertexShader->getHandle();
        }

        total++;
    }

    if (mData.mAttachedFragmentShader)
    {
        if (total < maxCount)
        {
            shaders[total] = mData.mAttachedFragmentShader->getHandle();
        }

        total++;
    }

    if (count)
    {
        *count = total;
    }
}

GLuint Program::getAttributeLocation(const std::string &name)
{
    for (const sh::Attribute &attribute : mData.mAttributes)
    {
        if (attribute.name == name && attribute.staticUse)
        {
            return attribute.location;
        }
    }

    return static_cast<GLuint>(-1);
}

bool Program::isAttribLocationActive(size_t attribLocation) const
{
    ASSERT(attribLocation < mData.mActiveAttribLocationsMask.size());
    return mData.mActiveAttribLocationsMask[attribLocation];
}

void Program::getActiveAttribute(GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    if (!mLinked)
    {
        if (bufsize > 0)
        {
            name[0] = '\0';
        }

        if (length)
        {
            *length = 0;
        }

        *type = GL_NONE;
        *size = 1;
        return;
    }

    size_t attributeIndex = 0;

    for (const sh::Attribute &attribute : mData.mAttributes)
    {
        // Skip over inactive attributes
        if (attribute.staticUse)
        {
            if (static_cast<size_t>(index) == attributeIndex)
            {
                break;
            }
            attributeIndex++;
        }
    }

    ASSERT(index == attributeIndex && attributeIndex < mData.mAttributes.size());
    const sh::Attribute &attrib = mData.mAttributes[attributeIndex];

    if (bufsize > 0)
    {
        const char *string = attrib.name.c_str();

        strncpy(name, string, bufsize);
        name[bufsize - 1] = '\0';

        if (length)
        {
            *length = static_cast<GLsizei>(strlen(name));
        }
    }

    // Always a single 'type' instance
    *size = 1;
    *type = attrib.type;
}

GLint Program::getActiveAttributeCount()
{
    if (!mLinked)
    {
        return 0;
    }

    GLint count = 0;

    for (const sh::Attribute &attrib : mData.mAttributes)
    {
        count += (attrib.staticUse ? 1 : 0);
    }

    return count;
}

GLint Program::getActiveAttributeMaxLength()
{
    if (!mLinked)
    {
        return 0;
    }

    size_t maxLength = 0;

    for (const sh::Attribute &attrib : mData.mAttributes)
    {
        if (attrib.staticUse)
        {
            maxLength = std::max(attrib.name.length() + 1, maxLength);
        }
    }

    return static_cast<GLint>(maxLength);
}

GLint Program::getFragDataLocation(const std::string &name) const
{
    std::string baseName(name);
    unsigned int arrayIndex = ParseAndStripArrayIndex(&baseName);
    for (auto outputPair : mData.mOutputVariables)
    {
        const VariableLocation &outputVariable = outputPair.second;
        if (outputVariable.name == baseName && (arrayIndex == GL_INVALID_INDEX || arrayIndex == outputVariable.element))
        {
            return static_cast<GLint>(outputPair.first);
        }
    }
    return -1;
}

void Program::getActiveUniform(GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    if (mLinked)
    {
        // index must be smaller than getActiveUniformCount()
        ASSERT(index < mData.mUniforms.size());
        const LinkedUniform &uniform = mData.mUniforms[index];

        if (bufsize > 0)
        {
            std::string string = uniform.name;
            if (uniform.isArray())
            {
                string += "[0]";
            }

            strncpy(name, string.c_str(), bufsize);
            name[bufsize - 1] = '\0';

            if (length)
            {
                *length = static_cast<GLsizei>(strlen(name));
            }
        }

        *size = uniform.elementCount();
        *type = uniform.type;
    }
    else
    {
        if (bufsize > 0)
        {
            name[0] = '\0';
        }

        if (length)
        {
            *length = 0;
        }

        *size = 0;
        *type = GL_NONE;
    }
}

GLint Program::getActiveUniformCount()
{
    if (mLinked)
    {
        return static_cast<GLint>(mData.mUniforms.size());
    }
    else
    {
        return 0;
    }
}

GLint Program::getActiveUniformMaxLength()
{
    size_t maxLength = 0;

    if (mLinked)
    {
        for (const LinkedUniform &uniform : mData.mUniforms)
        {
            if (!uniform.name.empty())
            {
                size_t length = uniform.name.length() + 1u;
                if (uniform.isArray())
                {
                    length += 3;  // Counting in "[0]".
                }
                maxLength = std::max(length, maxLength);
            }
        }
    }

    return static_cast<GLint>(maxLength);
}

GLint Program::getActiveUniformi(GLuint index, GLenum pname) const
{
    ASSERT(static_cast<size_t>(index) < mData.mUniforms.size());
    const gl::LinkedUniform &uniform = mData.mUniforms[index];
    switch (pname)
    {
      case GL_UNIFORM_TYPE:         return static_cast<GLint>(uniform.type);
      case GL_UNIFORM_SIZE:         return static_cast<GLint>(uniform.elementCount());
      case GL_UNIFORM_NAME_LENGTH:  return static_cast<GLint>(uniform.name.size() + 1 + (uniform.isArray() ? 3 : 0));
      case GL_UNIFORM_BLOCK_INDEX:  return uniform.blockIndex;
      case GL_UNIFORM_OFFSET:       return uniform.blockInfo.offset;
      case GL_UNIFORM_ARRAY_STRIDE: return uniform.blockInfo.arrayStride;
      case GL_UNIFORM_MATRIX_STRIDE: return uniform.blockInfo.matrixStride;
      case GL_UNIFORM_IS_ROW_MAJOR: return static_cast<GLint>(uniform.blockInfo.isRowMajorMatrix);
      default:
        UNREACHABLE();
        break;
    }
    return 0;
}

bool Program::isValidUniformLocation(GLint location) const
{
    ASSERT(rx::IsIntegerCastSafe<GLint>(mData.mUniformLocations.size()));
    return (location >= 0 && static_cast<size_t>(location) < mData.mUniformLocations.size());
}

const LinkedUniform &Program::getUniformByLocation(GLint location) const
{
    ASSERT(location >= 0 && static_cast<size_t>(location) < mData.mUniformLocations.size());
    return mData.mUniforms[mData.mUniformLocations[location].index];
}

GLint Program::getUniformLocation(const std::string &name) const
{
    return mData.getUniformLocation(name);
}

GLuint Program::getUniformIndex(const std::string &name) const
{
    return mData.getUniformIndex(name);
}

void Program::setUniform1fv(GLint location, GLsizei count, const GLfloat *v)
{
    setUniformInternal(location, count * 1, v);
    mProgram->setUniform1fv(location, count, v);
}

void Program::setUniform2fv(GLint location, GLsizei count, const GLfloat *v)
{
    setUniformInternal(location, count * 2, v);
    mProgram->setUniform2fv(location, count, v);
}

void Program::setUniform3fv(GLint location, GLsizei count, const GLfloat *v)
{
    setUniformInternal(location, count * 3, v);
    mProgram->setUniform3fv(location, count, v);
}

void Program::setUniform4fv(GLint location, GLsizei count, const GLfloat *v)
{
    setUniformInternal(location, count * 4, v);
    mProgram->setUniform4fv(location, count, v);
}

void Program::setUniform1iv(GLint location, GLsizei count, const GLint *v)
{
    setUniformInternal(location, count * 1, v);
    mProgram->setUniform1iv(location, count, v);
}

void Program::setUniform2iv(GLint location, GLsizei count, const GLint *v)
{
    setUniformInternal(location, count * 2, v);
    mProgram->setUniform2iv(location, count, v);
}

void Program::setUniform3iv(GLint location, GLsizei count, const GLint *v)
{
    setUniformInternal(location, count * 3, v);
    mProgram->setUniform3iv(location, count, v);
}

void Program::setUniform4iv(GLint location, GLsizei count, const GLint *v)
{
    setUniformInternal(location, count * 4, v);
    mProgram->setUniform4iv(location, count, v);
}

void Program::setUniform1uiv(GLint location, GLsizei count, const GLuint *v)
{
    setUniformInternal(location, count * 1, v);
    mProgram->setUniform1uiv(location, count, v);
}

void Program::setUniform2uiv(GLint location, GLsizei count, const GLuint *v)
{
    setUniformInternal(location, count * 2, v);
    mProgram->setUniform2uiv(location, count, v);
}

void Program::setUniform3uiv(GLint location, GLsizei count, const GLuint *v)
{
    setUniformInternal(location, count * 3, v);
    mProgram->setUniform3uiv(location, count, v);
}

void Program::setUniform4uiv(GLint location, GLsizei count, const GLuint *v)
{
    setUniformInternal(location, count * 4, v);
    mProgram->setUniform4uiv(location, count, v);
}

void Program::setUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    setMatrixUniformInternal<2, 2>(location, count, transpose, v);
    mProgram->setUniformMatrix2fv(location, count, transpose, v);
}

void Program::setUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    setMatrixUniformInternal<3, 3>(location, count, transpose, v);
    mProgram->setUniformMatrix3fv(location, count, transpose, v);
}

void Program::setUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    setMatrixUniformInternal<4, 4>(location, count, transpose, v);
    mProgram->setUniformMatrix4fv(location, count, transpose, v);
}

void Program::setUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    setMatrixUniformInternal<2, 3>(location, count, transpose, v);
    mProgram->setUniformMatrix2x3fv(location, count, transpose, v);
}

void Program::setUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    setMatrixUniformInternal<2, 4>(location, count, transpose, v);
    mProgram->setUniformMatrix2x4fv(location, count, transpose, v);
}

void Program::setUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    setMatrixUniformInternal<3, 2>(location, count, transpose, v);
    mProgram->setUniformMatrix3x2fv(location, count, transpose, v);
}

void Program::setUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    setMatrixUniformInternal<3, 4>(location, count, transpose, v);
    mProgram->setUniformMatrix3x4fv(location, count, transpose, v);
}

void Program::setUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    setMatrixUniformInternal<4, 2>(location, count, transpose, v);
    mProgram->setUniformMatrix4x2fv(location, count, transpose, v);
}

void Program::setUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *v)
{
    setMatrixUniformInternal<4, 3>(location, count, transpose, v);
    mProgram->setUniformMatrix4x3fv(location, count, transpose, v);
}

void Program::getUniformfv(GLint location, GLfloat *v)
{
    getUniformInternal(location, v);
}

void Program::getUniformiv(GLint location, GLint *v)
{
    getUniformInternal(location, v);
}

void Program::getUniformuiv(GLint location, GLuint *v)
{
    getUniformInternal(location, v);
}

void Program::flagForDeletion()
{
    mDeleteStatus = true;
}

bool Program::isFlaggedForDeletion() const
{
    return mDeleteStatus;
}

void Program::validate(const Caps &caps)
{
    mInfoLog.reset();

    if (mLinked)
    {
        mValidated = (mProgram->validate(caps, &mInfoLog) == GL_TRUE);
    }
    else
    {
        mInfoLog << "Program has not been successfully linked.";
    }
}

bool Program::validateSamplers(InfoLog *infoLog, const Caps &caps)
{
    // Skip cache if we're using an infolog, so we get the full error.
    // Also skip the cache if the sample mapping has changed, or if we haven't ever validated.
    if (infoLog == nullptr && mCachedValidateSamplersResult.valid())
    {
        return mCachedValidateSamplersResult.value();
    }

    if (mTextureUnitTypesCache.empty())
    {
        mTextureUnitTypesCache.resize(caps.maxCombinedTextureImageUnits, GL_NONE);
    }
    else
    {
        std::fill(mTextureUnitTypesCache.begin(), mTextureUnitTypesCache.end(), GL_NONE);
    }

    // if any two active samplers in a program are of different types, but refer to the same
    // texture image unit, and this is the current program, then ValidateProgram will fail, and
    // DrawArrays and DrawElements will issue the INVALID_OPERATION error.
    for (unsigned int samplerIndex = mSamplerUniformRange.start;
         samplerIndex < mSamplerUniformRange.end; ++samplerIndex)
    {
        const LinkedUniform &uniform = mData.mUniforms[samplerIndex];
        ASSERT(uniform.isSampler());

        if (!uniform.staticUse)
            continue;

        const GLuint *dataPtr = reinterpret_cast<const GLuint *>(uniform.getDataPtrToElement(0));
        GLenum textureType    = SamplerTypeToTextureType(uniform.type);

        for (unsigned int arrayElement = 0; arrayElement < uniform.elementCount(); ++arrayElement)
        {
            GLuint textureUnit = dataPtr[arrayElement];

            if (textureUnit >= caps.maxCombinedTextureImageUnits)
            {
                if (infoLog)
                {
                    (*infoLog) << "Sampler uniform (" << textureUnit
                               << ") exceeds GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS ("
                               << caps.maxCombinedTextureImageUnits << ")";
                }

                mCachedValidateSamplersResult = false;
                return false;
            }

            if (mTextureUnitTypesCache[textureUnit] != GL_NONE)
            {
                if (textureType != mTextureUnitTypesCache[textureUnit])
                {
                    if (infoLog)
                    {
                        (*infoLog) << "Samplers of conflicting types refer to the same texture "
                                      "image unit ("
                                   << textureUnit << ").";
                    }

                    mCachedValidateSamplersResult = false;
                    return false;
                }
            }
            else
            {
                mTextureUnitTypesCache[textureUnit] = textureType;
            }
        }
    }

    mCachedValidateSamplersResult = true;
    return true;
}

bool Program::isValidated() const
{
    return mValidated;
}

GLuint Program::getActiveUniformBlockCount()
{
    return static_cast<GLuint>(mData.mUniformBlocks.size());
}

void Program::getActiveUniformBlockName(GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName) const
{
    ASSERT(uniformBlockIndex <
           mData.mUniformBlocks.size());  // index must be smaller than getActiveUniformBlockCount()

    const UniformBlock &uniformBlock = mData.mUniformBlocks[uniformBlockIndex];

    if (bufSize > 0)
    {
        std::string string = uniformBlock.name;

        if (uniformBlock.isArray)
        {
            string += ArrayString(uniformBlock.arrayElement);
        }

        strncpy(uniformBlockName, string.c_str(), bufSize);
        uniformBlockName[bufSize - 1] = '\0';

        if (length)
        {
            *length = static_cast<GLsizei>(strlen(uniformBlockName));
        }
    }
}

void Program::getActiveUniformBlockiv(GLuint uniformBlockIndex, GLenum pname, GLint *params) const
{
    ASSERT(uniformBlockIndex <
           mData.mUniformBlocks.size());  // index must be smaller than getActiveUniformBlockCount()

    const UniformBlock &uniformBlock = mData.mUniformBlocks[uniformBlockIndex];

    switch (pname)
    {
      case GL_UNIFORM_BLOCK_DATA_SIZE:
        *params = static_cast<GLint>(uniformBlock.dataSize);
        break;
      case GL_UNIFORM_BLOCK_NAME_LENGTH:
          *params =
              static_cast<GLint>(uniformBlock.name.size() + 1 + (uniformBlock.isArray ? 3 : 0));
        break;
      case GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS:
        *params = static_cast<GLint>(uniformBlock.memberUniformIndexes.size());
        break;
      case GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES:
        {
            for (unsigned int blockMemberIndex = 0; blockMemberIndex < uniformBlock.memberUniformIndexes.size(); blockMemberIndex++)
            {
                params[blockMemberIndex] = static_cast<GLint>(uniformBlock.memberUniformIndexes[blockMemberIndex]);
            }
        }
        break;
      case GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER:
          *params = static_cast<GLint>(uniformBlock.vertexStaticUse);
        break;
      case GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER:
          *params = static_cast<GLint>(uniformBlock.fragmentStaticUse);
        break;
      default: UNREACHABLE();
    }
}

GLint Program::getActiveUniformBlockMaxLength()
{
    int maxLength = 0;

    if (mLinked)
    {
        unsigned int numUniformBlocks = static_cast<unsigned int>(mData.mUniformBlocks.size());
        for (unsigned int uniformBlockIndex = 0; uniformBlockIndex < numUniformBlocks; uniformBlockIndex++)
        {
            const UniformBlock &uniformBlock = mData.mUniformBlocks[uniformBlockIndex];
            if (!uniformBlock.name.empty())
            {
                const int length = static_cast<int>(uniformBlock.name.length()) + 1;

                // Counting in "[0]".
                const int arrayLength = (uniformBlock.isArray ? 3 : 0);

                maxLength = std::max(length + arrayLength, maxLength);
            }
        }
    }

    return maxLength;
}

GLuint Program::getUniformBlockIndex(const std::string &name)
{
    size_t subscript     = GL_INVALID_INDEX;
    std::string baseName = gl::ParseUniformName(name, &subscript);

    unsigned int numUniformBlocks = static_cast<unsigned int>(mData.mUniformBlocks.size());
    for (unsigned int blockIndex = 0; blockIndex < numUniformBlocks; blockIndex++)
    {
        const gl::UniformBlock &uniformBlock = mData.mUniformBlocks[blockIndex];
        if (uniformBlock.name == baseName)
        {
            const bool arrayElementZero =
                (subscript == GL_INVALID_INDEX &&
                 (!uniformBlock.isArray || uniformBlock.arrayElement == 0));
            if (subscript == uniformBlock.arrayElement || arrayElementZero)
            {
                return blockIndex;
            }
        }
    }

    return GL_INVALID_INDEX;
}

const UniformBlock &Program::getUniformBlockByIndex(GLuint index) const
{
    ASSERT(index < static_cast<GLuint>(mData.mUniformBlocks.size()));
    return mData.mUniformBlocks[index];
}

void Program::bindUniformBlock(GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    mData.mUniformBlockBindings[uniformBlockIndex] = uniformBlockBinding;
}

GLuint Program::getUniformBlockBinding(GLuint uniformBlockIndex) const
{
    return mData.getUniformBlockBinding(uniformBlockIndex);
}

void Program::resetUniformBlockBindings()
{
    for (unsigned int blockId = 0; blockId < IMPLEMENTATION_MAX_COMBINED_SHADER_UNIFORM_BUFFERS; blockId++)
    {
        mData.mUniformBlockBindings[blockId] = 0;
    }
}

void Program::setTransformFeedbackVaryings(GLsizei count, const GLchar *const *varyings, GLenum bufferMode)
{
    mData.mTransformFeedbackVaryingNames.resize(count);
    for (GLsizei i = 0; i < count; i++)
    {
        mData.mTransformFeedbackVaryingNames[i] = varyings[i];
    }

    mData.mTransformFeedbackBufferMode = bufferMode;
}

void Program::getTransformFeedbackVarying(GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name) const
{
    if (mLinked)
    {
        ASSERT(index < mData.mTransformFeedbackVaryingVars.size());
        const sh::Varying &varying = mData.mTransformFeedbackVaryingVars[index];
        GLsizei lastNameIdx = std::min(bufSize - 1, static_cast<GLsizei>(varying.name.length()));
        if (length)
        {
            *length = lastNameIdx;
        }
        if (size)
        {
            *size = varying.elementCount();
        }
        if (type)
        {
            *type = varying.type;
        }
        if (name)
        {
            memcpy(name, varying.name.c_str(), lastNameIdx);
            name[lastNameIdx] = '\0';
        }
    }
}

GLsizei Program::getTransformFeedbackVaryingCount() const
{
    if (mLinked)
    {
        return static_cast<GLsizei>(mData.mTransformFeedbackVaryingVars.size());
    }
    else
    {
        return 0;
    }
}

GLsizei Program::getTransformFeedbackVaryingMaxLength() const
{
    if (mLinked)
    {
        GLsizei maxSize = 0;
        for (const sh::Varying &varying : mData.mTransformFeedbackVaryingVars)
        {
            maxSize = std::max(maxSize, static_cast<GLsizei>(varying.name.length() + 1));
        }

        return maxSize;
    }
    else
    {
        return 0;
    }
}

GLenum Program::getTransformFeedbackBufferMode() const
{
    return mData.mTransformFeedbackBufferMode;
}

// static
bool Program::linkVaryings(InfoLog &infoLog,
                           const Shader *vertexShader,
                           const Shader *fragmentShader)
{
    const std::vector<sh::Varying> &vertexVaryings   = vertexShader->getVaryings();
    const std::vector<sh::Varying> &fragmentVaryings = fragmentShader->getVaryings();

    for (const sh::Varying &output : fragmentVaryings)
    {
        bool matched = false;

        // Built-in varyings obey special rules
        if (output.isBuiltIn())
        {
            continue;
        }

        for (const sh::Varying &input : vertexVaryings)
        {
            if (output.name == input.name)
            {
                ASSERT(!input.isBuiltIn());
                if (!linkValidateVaryings(infoLog, output.name, input, output))
                {
                    return false;
                }

                matched = true;
                break;
            }
        }

        // We permit unmatched, unreferenced varyings
        if (!matched && output.staticUse)
        {
            infoLog << "Fragment varying " << output.name << " does not match any vertex varying";
            return false;
        }
    }

    // TODO(jmadill): verify no unmatched vertex varyings?

    return true;
}

bool Program::linkUniforms(gl::InfoLog &infoLog, const gl::Caps &caps)
{
    const std::vector<sh::Uniform> &vertexUniforms   = mData.mAttachedVertexShader->getUniforms();
    const std::vector<sh::Uniform> &fragmentUniforms = mData.mAttachedFragmentShader->getUniforms();

    // Check that uniforms defined in the vertex and fragment shaders are identical
    std::map<std::string, LinkedUniform> linkedUniforms;

    for (const sh::Uniform &vertexUniform : vertexUniforms)
    {
        linkedUniforms[vertexUniform.name] = LinkedUniform(vertexUniform);
    }

    for (const sh::Uniform &fragmentUniform : fragmentUniforms)
    {
        auto entry = linkedUniforms.find(fragmentUniform.name);
        if (entry != linkedUniforms.end())
        {
            LinkedUniform *vertexUniform   = &entry->second;
            const std::string &uniformName = "uniform '" + vertexUniform->name + "'";
            if (!linkValidateUniforms(infoLog, uniformName, *vertexUniform, fragmentUniform))
            {
                return false;
            }
        }
    }

    // Flatten the uniforms list (nested fields) into a simple list (no nesting).
    // Also check the maximum uniform vector and sampler counts.
    if (!flattenUniformsAndCheckCaps(caps, infoLog))
    {
        return false;
    }

    indexUniforms();

    return true;
}

void Program::indexUniforms()
{
    for (size_t uniformIndex = 0; uniformIndex < mData.mUniforms.size(); uniformIndex++)
    {
        const gl::LinkedUniform &uniform = mData.mUniforms[uniformIndex];

        for (unsigned int arrayIndex = 0; arrayIndex < uniform.elementCount(); arrayIndex++)
        {
            if (!uniform.isBuiltIn())
            {
                // Assign in-order uniform locations
                mData.mUniformLocations.push_back(gl::VariableLocation(
                    uniform.name, arrayIndex, static_cast<unsigned int>(uniformIndex)));
            }
        }
    }
}

bool Program::linkValidateInterfaceBlockFields(InfoLog &infoLog, const std::string &uniformName, const sh::InterfaceBlockField &vertexUniform, const sh::InterfaceBlockField &fragmentUniform)
{
    if (!linkValidateVariablesBase(infoLog, uniformName, vertexUniform, fragmentUniform, true))
    {
        return false;
    }

    if (vertexUniform.isRowMajorLayout != fragmentUniform.isRowMajorLayout)
    {
        infoLog << "Matrix packings for " << uniformName << " differ between vertex and fragment shaders";
        return false;
    }

    return true;
}

// Determines the mapping between GL attributes and Direct3D 9 vertex stream usage indices
bool Program::linkAttributes(const gl::Data &data,
                             InfoLog &infoLog,
                             const AttributeBindings &attributeBindings,
                             const Shader *vertexShader)
{
    unsigned int usedLocations = 0;
    mData.mAttributes          = vertexShader->getActiveAttributes();
    GLuint maxAttribs = data.caps->maxVertexAttributes;

    // TODO(jmadill): handle aliasing robustly
    if (mData.mAttributes.size() > maxAttribs)
    {
        infoLog << "Too many vertex attributes.";
        return false;
    }

    std::vector<sh::Attribute *> usedAttribMap(data.caps->maxVertexAttributes, nullptr);

    // Link attributes that have a binding location
    for (sh::Attribute &attribute : mData.mAttributes)
    {
        // TODO(jmadill): do staticUse filtering step here, or not at all
        ASSERT(attribute.staticUse);

        int bindingLocation = attributeBindings.getAttributeBinding(attribute.name);
        if (attribute.location == -1 && bindingLocation != -1)
        {
            attribute.location = bindingLocation;
        }

        if (attribute.location != -1)
        {
            // Location is set by glBindAttribLocation or by location layout qualifier
            const int regs = VariableRegisterCount(attribute.type);

            if (static_cast<GLuint>(regs + attribute.location) > maxAttribs)
            {
                infoLog << "Active attribute (" << attribute.name << ") at location "
                        << attribute.location << " is too big to fit";

                return false;
            }

            for (int reg = 0; reg < regs; reg++)
            {
                const int regLocation               = attribute.location + reg;
                sh::ShaderVariable *linkedAttribute = usedAttribMap[regLocation];

                // In GLSL 3.00, attribute aliasing produces a link error
                // In GLSL 1.00, attribute aliasing is allowed, but ANGLE currently has a bug
                if (linkedAttribute)
                {
                    // TODO(jmadill): fix aliasing on ES2
                    // if (mProgram->getShaderVersion() >= 300)
                    {
                        infoLog << "Attribute '" << attribute.name << "' aliases attribute '"
                                << linkedAttribute->name << "' at location " << regLocation;
                        return false;
                    }
                }
                else
                {
                    usedAttribMap[regLocation] = &attribute;
                }

                usedLocations |= 1 << regLocation;
            }
        }
    }

    // Link attributes that don't have a binding location
    for (sh::Attribute &attribute : mData.mAttributes)
    {
        ASSERT(attribute.staticUse);

        // Not set by glBindAttribLocation or by location layout qualifier
        if (attribute.location == -1)
        {
            int regs           = VariableRegisterCount(attribute.type);
            int availableIndex = AllocateFirstFreeBits(&usedLocations, regs, maxAttribs);

            if (availableIndex == -1 || static_cast<GLuint>(availableIndex + regs) > maxAttribs)
            {
                infoLog << "Too many active attributes (" << attribute.name << ")";
                return false;
            }

            attribute.location = availableIndex;
        }
    }

    for (const sh::Attribute &attribute : mData.mAttributes)
    {
        ASSERT(attribute.staticUse);
        ASSERT(attribute.location != -1);
        int regs = VariableRegisterCount(attribute.type);

        for (int r = 0; r < regs; r++)
        {
            mData.mActiveAttribLocationsMask.set(attribute.location + r);
        }
    }

    return true;
}

bool Program::linkUniformBlocks(InfoLog &infoLog, const Caps &caps)
{
    const Shader &vertexShader   = *mData.mAttachedVertexShader;
    const Shader &fragmentShader = *mData.mAttachedFragmentShader;

    const std::vector<sh::InterfaceBlock> &vertexInterfaceBlocks = vertexShader.getInterfaceBlocks();
    const std::vector<sh::InterfaceBlock> &fragmentInterfaceBlocks = fragmentShader.getInterfaceBlocks();

    // Check that interface blocks defined in the vertex and fragment shaders are identical
    typedef std::map<std::string, const sh::InterfaceBlock*> UniformBlockMap;
    UniformBlockMap linkedUniformBlocks;

    GLuint vertexBlockCount = 0;
    for (const sh::InterfaceBlock &vertexInterfaceBlock : vertexInterfaceBlocks)
    {
        linkedUniformBlocks[vertexInterfaceBlock.name] = &vertexInterfaceBlock;

        // Note: shared and std140 layouts are always considered active
        if (vertexInterfaceBlock.staticUse || vertexInterfaceBlock.layout != sh::BLOCKLAYOUT_PACKED)
        {
            if (++vertexBlockCount > caps.maxVertexUniformBlocks)
            {
                infoLog << "Vertex shader uniform block count exceed GL_MAX_VERTEX_UNIFORM_BLOCKS ("
                        << caps.maxVertexUniformBlocks << ")";
                return false;
            }
        }
    }

    GLuint fragmentBlockCount = 0;
    for (const sh::InterfaceBlock &fragmentInterfaceBlock : fragmentInterfaceBlocks)
    {
        auto entry = linkedUniformBlocks.find(fragmentInterfaceBlock.name);
        if (entry != linkedUniformBlocks.end())
        {
            const sh::InterfaceBlock &vertexInterfaceBlock = *entry->second;
            if (!areMatchingInterfaceBlocks(infoLog, vertexInterfaceBlock, fragmentInterfaceBlock))
            {
                return false;
            }
        }

        // Note: shared and std140 layouts are always considered active
        if (fragmentInterfaceBlock.staticUse ||
            fragmentInterfaceBlock.layout != sh::BLOCKLAYOUT_PACKED)
        {
            if (++fragmentBlockCount > caps.maxFragmentUniformBlocks)
            {
                infoLog
                    << "Fragment shader uniform block count exceed GL_MAX_FRAGMENT_UNIFORM_BLOCKS ("
                    << caps.maxFragmentUniformBlocks << ")";
                return false;
            }
        }
    }

    gatherInterfaceBlockInfo();

    return true;
}

bool Program::areMatchingInterfaceBlocks(gl::InfoLog &infoLog, const sh::InterfaceBlock &vertexInterfaceBlock,
                                         const sh::InterfaceBlock &fragmentInterfaceBlock)
{
    const char* blockName = vertexInterfaceBlock.name.c_str();
    // validate blocks for the same member types
    if (vertexInterfaceBlock.fields.size() != fragmentInterfaceBlock.fields.size())
    {
        infoLog << "Types for interface block '" << blockName
                << "' differ between vertex and fragment shaders";
        return false;
    }
    if (vertexInterfaceBlock.arraySize != fragmentInterfaceBlock.arraySize)
    {
        infoLog << "Array sizes differ for interface block '" << blockName
                << "' between vertex and fragment shaders";
        return false;
    }
    if (vertexInterfaceBlock.layout != fragmentInterfaceBlock.layout || vertexInterfaceBlock.isRowMajorLayout != fragmentInterfaceBlock.isRowMajorLayout)
    {
        infoLog << "Layout qualifiers differ for interface block '" << blockName
                << "' between vertex and fragment shaders";
        return false;
    }
    const unsigned int numBlockMembers =
        static_cast<unsigned int>(vertexInterfaceBlock.fields.size());
    for (unsigned int blockMemberIndex = 0; blockMemberIndex < numBlockMembers; blockMemberIndex++)
    {
        const sh::InterfaceBlockField &vertexMember = vertexInterfaceBlock.fields[blockMemberIndex];
        const sh::InterfaceBlockField &fragmentMember = fragmentInterfaceBlock.fields[blockMemberIndex];
        if (vertexMember.name != fragmentMember.name)
        {
            infoLog << "Name mismatch for field " << blockMemberIndex
                    << " of interface block '" << blockName
                    << "': (in vertex: '" << vertexMember.name
                    << "', in fragment: '" << fragmentMember.name << "')";
            return false;
        }
        std::string memberName = "interface block '" + vertexInterfaceBlock.name + "' member '" + vertexMember.name + "'";
        if (!linkValidateInterfaceBlockFields(infoLog, memberName, vertexMember, fragmentMember))
        {
            return false;
        }
    }
    return true;
}

bool Program::linkValidateVariablesBase(InfoLog &infoLog, const std::string &variableName, const sh::ShaderVariable &vertexVariable,
                                              const sh::ShaderVariable &fragmentVariable, bool validatePrecision)
{
    if (vertexVariable.type != fragmentVariable.type)
    {
        infoLog << "Types for " << variableName << " differ between vertex and fragment shaders";
        return false;
    }
    if (vertexVariable.arraySize != fragmentVariable.arraySize)
    {
        infoLog << "Array sizes for " << variableName << " differ between vertex and fragment shaders";
        return false;
    }
    if (validatePrecision && vertexVariable.precision != fragmentVariable.precision)
    {
        infoLog << "Precisions for " << variableName << " differ between vertex and fragment shaders";
        return false;
    }

    if (vertexVariable.fields.size() != fragmentVariable.fields.size())
    {
        infoLog << "Structure lengths for " << variableName << " differ between vertex and fragment shaders";
        return false;
    }
    const unsigned int numMembers = static_cast<unsigned int>(vertexVariable.fields.size());
    for (unsigned int memberIndex = 0; memberIndex < numMembers; memberIndex++)
    {
        const sh::ShaderVariable &vertexMember = vertexVariable.fields[memberIndex];
        const sh::ShaderVariable &fragmentMember = fragmentVariable.fields[memberIndex];

        if (vertexMember.name != fragmentMember.name)
        {
            infoLog << "Name mismatch for field '" << memberIndex
                    << "' of " << variableName
                    << ": (in vertex: '" << vertexMember.name
                    << "', in fragment: '" << fragmentMember.name << "')";
            return false;
        }

        const std::string memberName = variableName.substr(0, variableName.length() - 1) + "." +
                                       vertexMember.name + "'";

        if (!linkValidateVariablesBase(infoLog, vertexMember.name, vertexMember, fragmentMember, validatePrecision))
        {
            return false;
        }
    }

    return true;
}

bool Program::linkValidateUniforms(InfoLog &infoLog, const std::string &uniformName, const sh::Uniform &vertexUniform, const sh::Uniform &fragmentUniform)
{
#if ANGLE_PROGRAM_LINK_VALIDATE_UNIFORM_PRECISION == ANGLE_ENABLED
    const bool validatePrecision = true;
#else
    const bool validatePrecision = false;
#endif

    if (!linkValidateVariablesBase(infoLog, uniformName, vertexUniform, fragmentUniform, validatePrecision))
    {
        return false;
    }

    return true;
}

bool Program::linkValidateVaryings(InfoLog &infoLog, const std::string &varyingName, const sh::Varying &vertexVarying, const sh::Varying &fragmentVarying)
{
    if (!linkValidateVariablesBase(infoLog, varyingName, vertexVarying, fragmentVarying, false))
    {
        return false;
    }

    if (!sh::InterpolationTypesMatch(vertexVarying.interpolation, fragmentVarying.interpolation))
    {
        infoLog << "Interpolation types for " << varyingName << " differ between vertex and fragment shaders";
        return false;
    }

    return true;
}

bool Program::linkValidateTransformFeedback(InfoLog &infoLog,
                                            const std::vector<const sh::Varying *> &varyings,
                                            const Caps &caps) const
{
    size_t totalComponents = 0;

    std::set<std::string> uniqueNames;

    for (const std::string &tfVaryingName : mData.mTransformFeedbackVaryingNames)
    {
        bool found = false;
        for (const sh::Varying *varying : varyings)
        {
            if (tfVaryingName == varying->name)
            {
                if (uniqueNames.count(tfVaryingName) > 0)
                {
                    infoLog << "Two transform feedback varyings specify the same output variable ("
                            << tfVaryingName << ").";
                    return false;
                }
                uniqueNames.insert(tfVaryingName);

                // TODO(jmadill): Investigate implementation limits on D3D11
                size_t componentCount = gl::VariableComponentCount(varying->type);
                if (mData.mTransformFeedbackBufferMode == GL_SEPARATE_ATTRIBS &&
                    componentCount > caps.maxTransformFeedbackSeparateComponents)
                {
                    infoLog << "Transform feedback varying's " << varying->name << " components ("
                            << componentCount << ") exceed the maximum separate components ("
                            << caps.maxTransformFeedbackSeparateComponents << ").";
                    return false;
                }

                totalComponents += componentCount;
                found = true;
                break;
            }
        }

        // TODO(jmadill): investigate if we can support capturing array elements.
        if (tfVaryingName.find('[') != std::string::npos)
        {
            infoLog << "Capture of array elements not currently supported.";
            return false;
        }

        // All transform feedback varyings are expected to exist since packVaryings checks for them.
        ASSERT(found);
        UNUSED_ASSERTION_VARIABLE(found);
    }

    if (mData.mTransformFeedbackBufferMode == GL_INTERLEAVED_ATTRIBS &&
        totalComponents > caps.maxTransformFeedbackInterleavedComponents)
    {
        infoLog << "Transform feedback varying total components (" << totalComponents
                << ") exceed the maximum interleaved components ("
                << caps.maxTransformFeedbackInterleavedComponents << ").";
        return false;
    }

    return true;
}

void Program::gatherTransformFeedbackVaryings(const std::vector<const sh::Varying *> &varyings)
{
    // Gather the linked varyings that are used for transform feedback, they should all exist.
    mData.mTransformFeedbackVaryingVars.clear();
    for (const std::string &tfVaryingName : mData.mTransformFeedbackVaryingNames)
    {
        for (const sh::Varying *varying : varyings)
        {
            if (tfVaryingName == varying->name)
            {
                mData.mTransformFeedbackVaryingVars.push_back(*varying);
                break;
            }
        }
    }
}

std::vector<const sh::Varying *> Program::getMergedVaryings() const
{
    std::set<std::string> uniqueNames;
    std::vector<const sh::Varying *> varyings;

    for (const sh::Varying &varying : mData.mAttachedVertexShader->getVaryings())
    {
        if (uniqueNames.count(varying.name) == 0)
        {
            uniqueNames.insert(varying.name);
            varyings.push_back(&varying);
        }
    }

    for (const sh::Varying &varying : mData.mAttachedFragmentShader->getVaryings())
    {
        if (uniqueNames.count(varying.name) == 0)
        {
            uniqueNames.insert(varying.name);
            varyings.push_back(&varying);
        }
    }

    return varyings;
}

void Program::linkOutputVariables()
{
    const Shader *fragmentShader = mData.mAttachedFragmentShader;
    ASSERT(fragmentShader != nullptr);

    // Skip this step for GLES2 shaders.
    if (fragmentShader->getShaderVersion() == 100)
        return;

    const auto &shaderOutputVars = fragmentShader->getActiveOutputVariables();

    // TODO(jmadill): any caps validation here?

    for (unsigned int outputVariableIndex = 0; outputVariableIndex < shaderOutputVars.size();
         outputVariableIndex++)
    {
        const sh::OutputVariable &outputVariable = shaderOutputVars[outputVariableIndex];

        // Don't store outputs for gl_FragDepth, gl_FragColor, etc.
        if (outputVariable.isBuiltIn())
            continue;

        // Since multiple output locations must be specified, use 0 for non-specified locations.
        int baseLocation = (outputVariable.location == -1 ? 0 : outputVariable.location);

        ASSERT(outputVariable.staticUse);

        for (unsigned int elementIndex = 0; elementIndex < outputVariable.elementCount();
             elementIndex++)
        {
            const int location = baseLocation + elementIndex;
            ASSERT(mData.mOutputVariables.count(location) == 0);
            unsigned int element = outputVariable.isArray() ? elementIndex : GL_INVALID_INDEX;
            mData.mOutputVariables[location] =
                VariableLocation(outputVariable.name, element, outputVariableIndex);
        }
    }
}

bool Program::flattenUniformsAndCheckCaps(const Caps &caps, InfoLog &infoLog)
{
    const gl::Shader *vertexShader = mData.getAttachedVertexShader();
    VectorAndSamplerCount vsCounts;

    std::vector<LinkedUniform> samplerUniforms;

    for (const sh::Uniform &uniform : vertexShader->getUniforms())
    {
        if (uniform.staticUse)
        {
            vsCounts += flattenUniform(uniform, uniform.name, &samplerUniforms);
        }
    }

    if (vsCounts.vectorCount > caps.maxVertexUniformVectors)
    {
        infoLog << "Vertex shader active uniforms exceed MAX_VERTEX_UNIFORM_VECTORS ("
                << caps.maxVertexUniformVectors << ").";
        return false;
    }

    if (vsCounts.samplerCount > caps.maxVertexTextureImageUnits)
    {
        infoLog << "Vertex shader sampler count exceeds MAX_VERTEX_TEXTURE_IMAGE_UNITS ("
                << caps.maxVertexTextureImageUnits << ").";
        return false;
    }

    const gl::Shader *fragmentShader = mData.getAttachedFragmentShader();
    VectorAndSamplerCount fsCounts;

    for (const sh::Uniform &uniform : fragmentShader->getUniforms())
    {
        if (uniform.staticUse)
        {
            fsCounts += flattenUniform(uniform, uniform.name, &samplerUniforms);
        }
    }

    if (fsCounts.vectorCount > caps.maxFragmentUniformVectors)
    {
        infoLog << "Fragment shader active uniforms exceed MAX_FRAGMENT_UNIFORM_VECTORS ("
                << caps.maxFragmentUniformVectors << ").";
        return false;
    }

    if (fsCounts.samplerCount > caps.maxTextureImageUnits)
    {
        infoLog << "Fragment shader sampler count exceeds MAX_TEXTURE_IMAGE_UNITS ("
                << caps.maxTextureImageUnits << ").";
        return false;
    }

    mSamplerUniformRange.start = static_cast<unsigned int>(mData.mUniforms.size());
    mSamplerUniformRange.end =
        mSamplerUniformRange.start + static_cast<unsigned int>(samplerUniforms.size());

    mData.mUniforms.insert(mData.mUniforms.end(), samplerUniforms.begin(), samplerUniforms.end());

    return true;
}

Program::VectorAndSamplerCount Program::flattenUniform(const sh::ShaderVariable &uniform,
                                                       const std::string &fullName,
                                                       std::vector<LinkedUniform> *samplerUniforms)
{
    VectorAndSamplerCount vectorAndSamplerCount;

    if (uniform.isStruct())
    {
        for (unsigned int elementIndex = 0; elementIndex < uniform.elementCount(); elementIndex++)
        {
            const std::string &elementString = (uniform.isArray() ? ArrayString(elementIndex) : "");

            for (size_t fieldIndex = 0; fieldIndex < uniform.fields.size(); fieldIndex++)
            {
                const sh::ShaderVariable &field  = uniform.fields[fieldIndex];
                const std::string &fieldFullName = (fullName + elementString + "." + field.name);

                vectorAndSamplerCount += flattenUniform(field, fieldFullName, samplerUniforms);
            }
        }

        return vectorAndSamplerCount;
    }

    // Not a struct
    bool isSampler = IsSamplerType(uniform.type);
    if (!UniformInList(mData.getUniforms(), fullName) && !UniformInList(*samplerUniforms, fullName))
    {
        gl::LinkedUniform linkedUniform(uniform.type, uniform.precision, fullName,
                                        uniform.arraySize, -1,
                                        sh::BlockMemberInfo::getDefaultBlockInfo());
        linkedUniform.staticUse = true;

        // Store sampler uniforms separately, so we'll append them to the end of the list.
        if (isSampler)
        {
            samplerUniforms->push_back(linkedUniform);
        }
        else
        {
            mData.mUniforms.push_back(linkedUniform);
        }
    }

    unsigned int elementCount          = uniform.elementCount();
    vectorAndSamplerCount.vectorCount  = (VariableRegisterCount(uniform.type) * elementCount);
    vectorAndSamplerCount.samplerCount = (isSampler ? elementCount : 0);

    return vectorAndSamplerCount;
}

void Program::gatherInterfaceBlockInfo()
{
    std::set<std::string> visitedList;

    const gl::Shader *vertexShader = mData.getAttachedVertexShader();

    ASSERT(mData.mUniformBlocks.empty());
    for (const sh::InterfaceBlock &vertexBlock : vertexShader->getInterfaceBlocks())
    {
        // Only 'packed' blocks are allowed to be considered inacive.
        if (!vertexBlock.staticUse && vertexBlock.layout == sh::BLOCKLAYOUT_PACKED)
            continue;

        if (visitedList.count(vertexBlock.name) > 0)
            continue;

        defineUniformBlock(vertexBlock, GL_VERTEX_SHADER);
        visitedList.insert(vertexBlock.name);
    }

    const gl::Shader *fragmentShader = mData.getAttachedFragmentShader();

    for (const sh::InterfaceBlock &fragmentBlock : fragmentShader->getInterfaceBlocks())
    {
        // Only 'packed' blocks are allowed to be considered inacive.
        if (!fragmentBlock.staticUse && fragmentBlock.layout == sh::BLOCKLAYOUT_PACKED)
            continue;

        if (visitedList.count(fragmentBlock.name) > 0)
        {
            for (gl::UniformBlock &block : mData.mUniformBlocks)
            {
                if (block.name == fragmentBlock.name)
                {
                    block.fragmentStaticUse = fragmentBlock.staticUse;
                }
            }

            continue;
        }

        defineUniformBlock(fragmentBlock, GL_FRAGMENT_SHADER);
        visitedList.insert(fragmentBlock.name);
    }
}

void Program::defineUniformBlock(const sh::InterfaceBlock &interfaceBlock, GLenum shaderType)
{
    int blockIndex                = static_cast<int>(mData.mUniformBlocks.size());
    size_t firstBlockUniformIndex = mData.mUniforms.size();
    DefineUniformBlockMembers(interfaceBlock.fields, "", blockIndex, &mData.mUniforms);
    size_t lastBlockUniformIndex = mData.mUniforms.size();

    std::vector<unsigned int> blockUniformIndexes;
    for (size_t blockUniformIndex = firstBlockUniformIndex;
         blockUniformIndex < lastBlockUniformIndex; ++blockUniformIndex)
    {
        blockUniformIndexes.push_back(static_cast<unsigned int>(blockUniformIndex));
    }

    if (interfaceBlock.arraySize > 0)
    {
        for (unsigned int arrayElement = 0; arrayElement < interfaceBlock.arraySize; ++arrayElement)
        {
            UniformBlock block(interfaceBlock.name, true, arrayElement);
            block.memberUniformIndexes = blockUniformIndexes;

            if (shaderType == GL_VERTEX_SHADER)
            {
                block.vertexStaticUse = interfaceBlock.staticUse;
            }
            else
            {
                ASSERT(shaderType == GL_FRAGMENT_SHADER);
                block.fragmentStaticUse = interfaceBlock.staticUse;
            }

            mData.mUniformBlocks.push_back(block);
        }
    }
    else
    {
        UniformBlock block(interfaceBlock.name, false, 0);
        block.memberUniformIndexes = blockUniformIndexes;

        if (shaderType == GL_VERTEX_SHADER)
        {
            block.vertexStaticUse = interfaceBlock.staticUse;
        }
        else
        {
            ASSERT(shaderType == GL_FRAGMENT_SHADER);
            block.fragmentStaticUse = interfaceBlock.staticUse;
        }

        mData.mUniformBlocks.push_back(block);
    }
}

template <typename T>
void Program::setUniformInternal(GLint location, GLsizei count, const T *v)
{
    const VariableLocation &locationInfo = mData.mUniformLocations[location];
    LinkedUniform *linkedUniform         = &mData.mUniforms[locationInfo.index];
    uint8_t *destPointer                 = linkedUniform->getDataPtrToElement(locationInfo.element);

    if (VariableComponentType(linkedUniform->type) == GL_BOOL)
    {
        // Do a cast conversion for boolean types. From the spec:
        // "The uniform is set to FALSE if the input value is 0 or 0.0f, and set to TRUE otherwise."
        GLint *destAsInt = reinterpret_cast<GLint *>(destPointer);
        for (GLsizei component = 0; component < count; ++component)
        {
            destAsInt[component] = (v[component] != static_cast<T>(0) ? GL_TRUE : GL_FALSE);
        }
    }
    else
    {
        // Invalide the validation cache if we modify the sampler data.
        if (linkedUniform->isSampler() && memcmp(destPointer, v, sizeof(T) * count) != 0)
        {
            mCachedValidateSamplersResult.reset();
        }

        memcpy(destPointer, v, sizeof(T) * count);
    }
}

template <size_t cols, size_t rows, typename T>
void Program::setMatrixUniformInternal(GLint location,
                                       GLsizei count,
                                       GLboolean transpose,
                                       const T *v)
{
    if (!transpose)
    {
        setUniformInternal(location, count * cols * rows, v);
        return;
    }

    // Perform a transposing copy.
    const VariableLocation &locationInfo = mData.mUniformLocations[location];
    LinkedUniform *linkedUniform         = &mData.mUniforms[locationInfo.index];
    T *destPtr = reinterpret_cast<T *>(linkedUniform->getDataPtrToElement(locationInfo.element));
    for (GLsizei element = 0; element < count; ++element)
    {
        size_t elementOffset = element * rows * cols;

        for (size_t row = 0; row < rows; ++row)
        {
            for (size_t col = 0; col < cols; ++col)
            {
                destPtr[col * rows + row + elementOffset] = v[row * cols + col + elementOffset];
            }
        }
    }
}

template <typename DestT>
void Program::getUniformInternal(GLint location, DestT *dataOut) const
{
    const VariableLocation &locationInfo = mData.mUniformLocations[location];
    const LinkedUniform &uniform         = mData.mUniforms[locationInfo.index];

    const uint8_t *srcPointer = uniform.getDataPtrToElement(locationInfo.element);

    GLenum componentType = VariableComponentType(uniform.type);
    if (componentType == GLTypeToGLenum<DestT>::value)
    {
        memcpy(dataOut, srcPointer, uniform.getElementSize());
        return;
    }

    int components = VariableComponentCount(uniform.type) * uniform.elementCount();

    switch (componentType)
    {
        case GL_INT:
            UniformStateQueryCastLoop<GLint>(dataOut, srcPointer, components);
            break;
        case GL_UNSIGNED_INT:
            UniformStateQueryCastLoop<GLuint>(dataOut, srcPointer, components);
            break;
        case GL_BOOL:
            UniformStateQueryCastLoop<GLboolean>(dataOut, srcPointer, components);
            break;
        case GL_FLOAT:
            UniformStateQueryCastLoop<GLfloat>(dataOut, srcPointer, components);
            break;
        default:
            UNREACHABLE();
    }
}
}
