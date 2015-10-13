//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Shader.h: Defines the abstract gl::Shader class and its concrete derived
// classes VertexShader and FragmentShader. Implements GL shader objects and
// related functionality. [OpenGL ES 2.0.24] section 2.10 page 24 and section
// 3.8 page 84.

#ifndef LIBANGLE_SHADER_H_
#define LIBANGLE_SHADER_H_

#include <string>
#include <list>
#include <vector>

#include "angle_gl.h"
#include <GLSLANG/ShaderLang.h>

#include "common/angleutils.h"
#include "libANGLE/angletypes.h"

namespace rx
{
class ImplFactory;
class ShaderImpl;
class ShaderSh;
}

namespace gl
{
class Compiler;
struct Limitations;
class ResourceManager;
struct Data;

class Shader : angle::NonCopyable
{
  public:
    class Data final : angle::NonCopyable
    {
      public:
        Data(GLenum shaderType);
        ~Data();

        const std::string &getSource() const { return mSource; }
        const std::string &getTranslatedSource() const { return mTranslatedSource; }

        GLenum getShaderType() const { return mShaderType; }
        int getShaderVersion() const { return mShaderVersion; }

        const std::vector<sh::Varying> &getVaryings() const { return mVaryings; }
        const std::vector<sh::Uniform> &getUniforms() const { return mUniforms; }
        const std::vector<sh::InterfaceBlock> &getInterfaceBlocks() const
        {
            return mInterfaceBlocks;
        }
        const std::vector<sh::Attribute> &getActiveAttributes() const { return mActiveAttributes; }
        const std::vector<sh::OutputVariable> &getActiveOutputVariables() const
        {
            return mActiveOutputVariables;
        }

      private:
        friend class Shader;

        GLenum mShaderType;
        int mShaderVersion;
        std::string mTranslatedSource;
        std::string mSource;

        std::vector<sh::Varying> mVaryings;
        std::vector<sh::Uniform> mUniforms;
        std::vector<sh::InterfaceBlock> mInterfaceBlocks;
        std::vector<sh::Attribute> mActiveAttributes;
        std::vector<sh::OutputVariable> mActiveOutputVariables;
    };

    Shader(ResourceManager *manager,
           rx::ImplFactory *implFactory,
           const gl::Limitations &rendererLimitations,
           GLenum type,
           GLuint handle);

    virtual ~Shader();

    GLenum getType() const { return mType; }
    GLuint getHandle() const;

    const rx::ShaderImpl *getImplementation() const { return mImplementation; }

    void deleteSource();
    void setSource(GLsizei count, const char *const *string, const GLint *length);
    int getInfoLogLength() const;
    void getInfoLog(GLsizei bufSize, GLsizei *length, char *infoLog) const;
    int getSourceLength() const;
    void getSource(GLsizei bufSize, GLsizei *length, char *buffer) const;
    int getTranslatedSourceLength() const;
    const std::string &getTranslatedSource() const { return mData.getTranslatedSource(); }
    void getTranslatedSource(GLsizei bufSize, GLsizei *length, char *buffer) const;
    void getTranslatedSourceWithDebugInfo(GLsizei bufSize, GLsizei *length, char *buffer) const;

    void compile(Compiler *compiler);
    bool isCompiled() const { return mCompiled; }

    void addRef();
    void release();
    unsigned int getRefCount() const;
    bool isFlaggedForDeletion() const;
    void flagForDeletion();

    int getShaderVersion() const;

    const std::vector<sh::Varying> &getVaryings() const;
    const std::vector<sh::Uniform> &getUniforms() const;
    const std::vector<sh::InterfaceBlock> &getInterfaceBlocks() const;
    const std::vector<sh::Attribute> &getActiveAttributes() const;
    const std::vector<sh::OutputVariable> &getActiveOutputVariables() const;

    int getSemanticIndex(const std::string &attributeName) const;

  private:
    static void getSourceImpl(const std::string &source, GLsizei bufSize, GLsizei *length, char *buffer);

    Data mData;
    rx::ShaderImpl *mImplementation;
    const gl::Limitations &mRendererLimitations;
    const GLuint mHandle;
    const GLenum mType;
    unsigned int mRefCount;     // Number of program objects this shader is attached to
    bool mDeleteStatus;         // Flag to indicate that the shader can be deleted when no longer in use
    bool mCompiled;             // Indicates if this shader has been successfully compiled
    std::string mInfoLog;

    ResourceManager *mResourceManager;
};

}

#endif   // LIBANGLE_SHADER_H_
