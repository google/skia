//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramGL.cpp: Implements the class methods for ProgramGL.

#include "libANGLE/renderer/gl/ProgramGL.h"

#include "common/debug.h"
#include "common/utilities.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/ShaderGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

namespace rx
{

ProgramGL::ProgramGL(const gl::Program::Data &data,
                     const FunctionsGL *functions,
                     StateManagerGL *stateManager)
    : ProgramImpl(data), mFunctions(functions), mStateManager(stateManager), mProgramID(0)
{
    ASSERT(mFunctions);
    ASSERT(mStateManager);

    mProgramID = mFunctions->createProgram();
}

ProgramGL::~ProgramGL()
{
    mFunctions->deleteProgram(mProgramID);
    mProgramID = 0;
}

int ProgramGL::getShaderVersion() const
{
    UNIMPLEMENTED();
    return int();
}

GLenum ProgramGL::getBinaryFormat()
{
    UNIMPLEMENTED();
    return GLenum();
}

LinkResult ProgramGL::load(gl::InfoLog &infoLog, gl::BinaryInputStream *stream)
{
    UNIMPLEMENTED();
    return LinkResult(false, gl::Error(GL_INVALID_OPERATION));
}

gl::Error ProgramGL::save(gl::BinaryOutputStream *stream)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

LinkResult ProgramGL::link(const gl::Data &data, gl::InfoLog &infoLog)
{
    // Reset the program state, delete the current program if one exists
    reset();

    const gl::Shader *vertexShader   = mData.getAttachedVertexShader();
    const gl::Shader *fragmentShader = mData.getAttachedFragmentShader();

    const ShaderGL *vertexShaderGL   = GetImplAs<ShaderGL>(vertexShader);
    const ShaderGL *fragmentShaderGL = GetImplAs<ShaderGL>(fragmentShader);

    // Attach the shaders
    mFunctions->attachShader(mProgramID, vertexShaderGL->getShaderID());
    mFunctions->attachShader(mProgramID, fragmentShaderGL->getShaderID());

    // Bind attribute locations to match the GL layer.
    for (const sh::Attribute &attribute : mData.getAttributes())
    {
        if (!attribute.staticUse)
        {
            continue;
        }

        mFunctions->bindAttribLocation(mProgramID, attribute.location, attribute.name.c_str());
    }

    // Link and verify
    mFunctions->linkProgram(mProgramID);

    // Detach the shaders
    mFunctions->detachShader(mProgramID, vertexShaderGL->getShaderID());
    mFunctions->detachShader(mProgramID, fragmentShaderGL->getShaderID());

    // Verify the link
    GLint linkStatus = GL_FALSE;
    mFunctions->getProgramiv(mProgramID, GL_LINK_STATUS, &linkStatus);
    ASSERT(linkStatus == GL_TRUE);
    if (linkStatus == GL_FALSE)
    {
        // Linking failed, put the error into the info log
        GLint infoLogLength = 0;
        mFunctions->getProgramiv(mProgramID, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<char> buf(infoLogLength);
        mFunctions->getProgramInfoLog(mProgramID, infoLogLength, nullptr, &buf[0]);

        mFunctions->deleteProgram(mProgramID);
        mProgramID = 0;

        infoLog << &buf[0];
        TRACE("\n%s", &buf[0]);

        // TODO, return GL_OUT_OF_MEMORY or just fail the link? This is an unexpected case
        return LinkResult(false, gl::Error(GL_NO_ERROR));
    }

    // Query the uniform information
    ASSERT(mUniformRealLocationMap.empty());
    const auto &uniforms = mData.getUniforms();
    for (const gl::VariableLocation &entry : mData.getUniformLocations())
    {
        // From the spec:
        // "Locations for sequential array indices are not required to be sequential."
        const gl::LinkedUniform &uniform = uniforms[entry.index];
        std::stringstream fullNameStr;
        fullNameStr << uniform.name;
        if (uniform.isArray())
        {
            fullNameStr << "[" << entry.element << "]";
        }
        const std::string &fullName = fullNameStr.str();

        GLint realLocation = mFunctions->getUniformLocation(mProgramID, fullName.c_str());
        mUniformRealLocationMap.push_back(realLocation);
    }

    mUniformIndexToSamplerIndex.resize(mData.getUniforms().size(), GL_INVALID_INDEX);

    for (size_t uniformId = 0; uniformId < uniforms.size(); ++uniformId)
    {
        const gl::LinkedUniform &linkedUniform = uniforms[uniformId];

        if (!linkedUniform.isSampler() || !linkedUniform.staticUse)
            continue;

        mUniformIndexToSamplerIndex[uniformId] = mSamplerBindings.size();

        // If uniform is a sampler type, insert it into the mSamplerBindings array
        SamplerBindingGL samplerBinding;
        samplerBinding.textureType = gl::SamplerTypeToTextureType(linkedUniform.type);
        samplerBinding.boundTextureUnits.resize(linkedUniform.elementCount(), 0);
        mSamplerBindings.push_back(samplerBinding);
    }

    return LinkResult(true, gl::Error(GL_NO_ERROR));
}

GLboolean ProgramGL::validate(const gl::Caps & /*caps*/, gl::InfoLog * /*infoLog*/)
{
    // TODO(jmadill): implement validate
    return true;
}

void ProgramGL::setUniform1fv(GLint location, GLsizei count, const GLfloat *v)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniform1fv(uniLoc(location), count, v);
}

void ProgramGL::setUniform2fv(GLint location, GLsizei count, const GLfloat *v)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniform2fv(uniLoc(location), count, v);
}

void ProgramGL::setUniform3fv(GLint location, GLsizei count, const GLfloat *v)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniform3fv(uniLoc(location), count, v);
}

void ProgramGL::setUniform4fv(GLint location, GLsizei count, const GLfloat *v)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniform4fv(uniLoc(location), count, v);
}

void ProgramGL::setUniform1iv(GLint location, GLsizei count, const GLint *v)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniform1iv(uniLoc(location), count, v);

    const gl::VariableLocation &locationEntry = mData.getUniformLocations()[location];

    size_t samplerIndex = mUniformIndexToSamplerIndex[locationEntry.index];
    if (samplerIndex != GL_INVALID_INDEX)
    {
        std::vector<GLuint> &boundTextureUnits = mSamplerBindings[samplerIndex].boundTextureUnits;

        size_t copyCount =
            std::max<size_t>(count, boundTextureUnits.size() - locationEntry.element);
        std::copy(v, v + copyCount, boundTextureUnits.begin() + locationEntry.element);
    }
}

void ProgramGL::setUniform2iv(GLint location, GLsizei count, const GLint *v)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniform2iv(uniLoc(location), count, v);
}

void ProgramGL::setUniform3iv(GLint location, GLsizei count, const GLint *v)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniform3iv(uniLoc(location), count, v);
}

void ProgramGL::setUniform4iv(GLint location, GLsizei count, const GLint *v)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniform4iv(uniLoc(location), count, v);
}

void ProgramGL::setUniform1uiv(GLint location, GLsizei count, const GLuint *v)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniform1uiv(location, count, v);
}

void ProgramGL::setUniform2uiv(GLint location, GLsizei count, const GLuint *v)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniform2uiv(uniLoc(location), count, v);
}

void ProgramGL::setUniform3uiv(GLint location, GLsizei count, const GLuint *v)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniform3uiv(uniLoc(location), count, v);
}

void ProgramGL::setUniform4uiv(GLint location, GLsizei count, const GLuint *v)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniform4uiv(uniLoc(location), count, v);
}

void ProgramGL::setUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniformMatrix2fv(uniLoc(location), count, transpose, value);
}

void ProgramGL::setUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniformMatrix3fv(uniLoc(location), count, transpose, value);
}

void ProgramGL::setUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniformMatrix4fv(uniLoc(location), count, transpose, value);
}

void ProgramGL::setUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniformMatrix2x3fv(uniLoc(location), count, transpose, value);
}

void ProgramGL::setUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniformMatrix3x2fv(uniLoc(location), count, transpose, value);
}

void ProgramGL::setUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniformMatrix2x4fv(uniLoc(location), count, transpose, value);
}

void ProgramGL::setUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniformMatrix4x2fv(uniLoc(location), count, transpose, value);
}

void ProgramGL::setUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniformMatrix3x4fv(uniLoc(location), count, transpose, value);
}

void ProgramGL::setUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    mFunctions->uniformMatrix4x3fv(uniLoc(location), count, transpose, value);
}

void ProgramGL::reset()
{
    mUniformRealLocationMap.clear();
    mSamplerBindings.clear();
    mUniformIndexToSamplerIndex.clear();
}

GLuint ProgramGL::getProgramID() const
{
    return mProgramID;
}

const std::vector<SamplerBindingGL> &ProgramGL::getAppliedSamplerUniforms() const
{
    return mSamplerBindings;
}

void ProgramGL::gatherUniformBlockInfo(std::vector<gl::UniformBlock> * /*uniformBlocks*/,
                                       std::vector<gl::LinkedUniform> * /*uniforms*/)
{
    // TODO(jmadill): Gather uniform block layout info, and data sizes.
}
}
