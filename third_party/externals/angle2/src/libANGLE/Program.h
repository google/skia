//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Program.h: Defines the gl::Program class. Implements GL program objects
// and related functionality. [OpenGL ES 2.0.24] section 2.10.3 page 28.

#ifndef LIBANGLE_PROGRAM_H_
#define LIBANGLE_PROGRAM_H_

#include <GLES2/gl2.h>
#include <GLSLANG/ShaderLang.h>

#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "common/angleutils.h"
#include "common/mathutil.h"
#include "common/Optional.h"

#include "libANGLE/angletypes.h"
#include "libANGLE/Constants.h"
#include "libANGLE/Error.h"
#include "libANGLE/RefCountObject.h"

namespace rx
{
class ImplFactory;
class ProgramImpl;
struct TranslatedAttribute;
}

namespace gl
{
struct Caps;
struct Data;
class ResourceManager;
class Shader;
class InfoLog;
class AttributeBindings;
class Buffer;
class Framebuffer;
struct UniformBlock;
struct LinkedUniform;

extern const char * const g_fakepath;

class AttributeBindings
{
  public:
    AttributeBindings();
    ~AttributeBindings();

    void bindAttributeLocation(GLuint index, const char *name);
    int getAttributeBinding(const std::string &name) const;

  private:
    std::set<std::string> mAttributeBinding[MAX_VERTEX_ATTRIBS];
};

class InfoLog : angle::NonCopyable
{
  public:
    InfoLog();
    ~InfoLog();

    size_t getLength() const;
    void getLog(GLsizei bufSize, GLsizei *length, char *infoLog);

    void appendSanitized(const char *message);
    void reset();

    // This helper class ensures we append a newline after writing a line.
    class StreamHelper : angle::NonCopyable
    {
      public:
        StreamHelper(StreamHelper &&rhs)
            : mStream(rhs.mStream)
        {
            rhs.mStream = nullptr;
        }

        StreamHelper &operator=(StreamHelper &&rhs)
        {
            std::swap(mStream, rhs.mStream);
            return *this;
        }

        ~StreamHelper()
        {
            // Write newline when destroyed on the stack
            if (mStream)
            {
                (*mStream) << std::endl;
            }
        }

        template <typename T>
        StreamHelper &operator<<(const T &value)
        {
            (*mStream) << value;
            return *this;
        }

      private:
        friend class InfoLog;

        StreamHelper(std::stringstream *stream)
            : mStream(stream)
        {
            ASSERT(stream);
        }

        std::stringstream *mStream;
    };

    template <typename T>
    StreamHelper operator<<(const T &value)
    {
        StreamHelper helper(&mStream);
        helper << value;
        return helper;
    }

    std::string str() const { return mStream.str(); }

  private:
    std::stringstream mStream;
};

// Struct used for correlating uniforms/elements of uniform arrays to handles
struct VariableLocation
{
    VariableLocation();
    VariableLocation(const std::string &name, unsigned int element, unsigned int index);

    std::string name;
    unsigned int element;
    unsigned int index;
};

struct LinkedVarying
{
    LinkedVarying();
    LinkedVarying(const std::string &name, GLenum type, GLsizei size, const std::string &semanticName,
        unsigned int semanticIndex, unsigned int semanticIndexCount);

    // Original GL name
    std::string name;

    GLenum type;
    GLsizei size;

    // DirectX semantic information
    std::string semanticName;
    unsigned int semanticIndex;
    unsigned int semanticIndexCount;
};

class Program : angle::NonCopyable
{
  public:
    class Data final : angle::NonCopyable
    {
      public:
        Data();
        ~Data();

        const Shader *getAttachedVertexShader() const { return mAttachedVertexShader; }
        const Shader *getAttachedFragmentShader() const { return mAttachedFragmentShader; }
        const std::vector<std::string> &getTransformFeedbackVaryingNames() const
        {
            return mTransformFeedbackVaryingNames;
        }
        GLint getTransformFeedbackBufferMode() const { return mTransformFeedbackBufferMode; }
        GLuint getUniformBlockBinding(GLuint uniformBlockIndex) const
        {
            ASSERT(uniformBlockIndex < IMPLEMENTATION_MAX_COMBINED_SHADER_UNIFORM_BUFFERS);
            return mUniformBlockBindings[uniformBlockIndex];
        }
        const std::vector<sh::Attribute> &getAttributes() const { return mAttributes; }
        const AttributesMask &getActiveAttribLocationsMask() const
        {
            return mActiveAttribLocationsMask;
        }
        const std::map<int, VariableLocation> &getOutputVariables() const
        {
            return mOutputVariables;
        }
        const std::vector<LinkedUniform> &getUniforms() const { return mUniforms; }
        const std::vector<VariableLocation> &getUniformLocations() const
        {
            return mUniformLocations;
        }
        const std::vector<UniformBlock> &getUniformBlocks() const { return mUniformBlocks; }

        const LinkedUniform *getUniformByName(const std::string &name) const;
        GLint getUniformLocation(const std::string &name) const;
        GLuint getUniformIndex(const std::string &name) const;

      private:
        friend class Program;

        Shader *mAttachedFragmentShader;
        Shader *mAttachedVertexShader;

        std::vector<std::string> mTransformFeedbackVaryingNames;
        std::vector<sh::Varying> mTransformFeedbackVaryingVars;
        GLenum mTransformFeedbackBufferMode;

        GLuint mUniformBlockBindings[IMPLEMENTATION_MAX_COMBINED_SHADER_UNIFORM_BUFFERS];

        std::vector<sh::Attribute> mAttributes;
        std::bitset<MAX_VERTEX_ATTRIBS> mActiveAttribLocationsMask;

        // Uniforms are sorted in order:
        //  1. Non-sampler uniforms
        //  2. Sampler uniforms
        //  3. Uniform block uniforms
        // This makes sampler validation easier, since we don't need a separate list.
        std::vector<LinkedUniform> mUniforms;
        std::vector<VariableLocation> mUniformLocations;
        std::vector<UniformBlock> mUniformBlocks;

        // TODO(jmadill): use unordered/hash map when available
        std::map<int, VariableLocation> mOutputVariables;
    };

    Program(rx::ImplFactory *factory, ResourceManager *manager, GLuint handle);
    ~Program();

    GLuint id() const { return mHandle; }

    rx::ProgramImpl *getImplementation() { return mProgram; }
    const rx::ProgramImpl *getImplementation() const { return mProgram; }

    bool attachShader(Shader *shader);
    bool detachShader(Shader *shader);
    int getAttachedShadersCount() const;

    void bindAttributeLocation(GLuint index, const char *name);

    Error link(const gl::Data &data);
    bool isLinked();

    Error loadBinary(GLenum binaryFormat, const void *binary, GLsizei length);
    Error saveBinary(GLenum *binaryFormat, void *binary, GLsizei bufSize, GLsizei *length) const;
    GLint getBinaryLength() const;

    int getInfoLogLength() const;
    void getInfoLog(GLsizei bufSize, GLsizei *length, char *infoLog);
    void getAttachedShaders(GLsizei maxCount, GLsizei *count, GLuint *shaders);

    GLuint getAttributeLocation(const std::string &name);
    bool isAttribLocationActive(size_t attribLocation) const;

    void getActiveAttribute(GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
    GLint getActiveAttributeCount();
    GLint getActiveAttributeMaxLength();
    const std::vector<sh::Attribute> &getAttributes() const { return mData.mAttributes; }

    GLint getFragDataLocation(const std::string &name) const;

    void getActiveUniform(GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
    GLint getActiveUniformCount();
    GLint getActiveUniformMaxLength();
    GLint getActiveUniformi(GLuint index, GLenum pname) const;
    bool isValidUniformLocation(GLint location) const;
    const LinkedUniform &getUniformByLocation(GLint location) const;

    GLint getUniformLocation(const std::string &name) const;
    GLuint getUniformIndex(const std::string &name) const;
    void setUniform1fv(GLint location, GLsizei count, const GLfloat *v);
    void setUniform2fv(GLint location, GLsizei count, const GLfloat *v);
    void setUniform3fv(GLint location, GLsizei count, const GLfloat *v);
    void setUniform4fv(GLint location, GLsizei count, const GLfloat *v);
    void setUniform1iv(GLint location, GLsizei count, const GLint *v);
    void setUniform2iv(GLint location, GLsizei count, const GLint *v);
    void setUniform3iv(GLint location, GLsizei count, const GLint *v);
    void setUniform4iv(GLint location, GLsizei count, const GLint *v);
    void setUniform1uiv(GLint location, GLsizei count, const GLuint *v);
    void setUniform2uiv(GLint location, GLsizei count, const GLuint *v);
    void setUniform3uiv(GLint location, GLsizei count, const GLuint *v);
    void setUniform4uiv(GLint location, GLsizei count, const GLuint *v);
    void setUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void setUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void setUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void setUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void setUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void setUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void setUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void setUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void setUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

    void getUniformfv(GLint location, GLfloat *params);
    void getUniformiv(GLint location, GLint *params);
    void getUniformuiv(GLint location, GLuint *params);

    void getActiveUniformBlockName(GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName) const;
    void getActiveUniformBlockiv(GLuint uniformBlockIndex, GLenum pname, GLint *params) const;
    GLuint getActiveUniformBlockCount();
    GLint getActiveUniformBlockMaxLength();

    GLuint getUniformBlockIndex(const std::string &name);

    void bindUniformBlock(GLuint uniformBlockIndex, GLuint uniformBlockBinding);
    GLuint getUniformBlockBinding(GLuint uniformBlockIndex) const;

    const UniformBlock &getUniformBlockByIndex(GLuint index) const;

    void setTransformFeedbackVaryings(GLsizei count, const GLchar *const *varyings, GLenum bufferMode);
    void getTransformFeedbackVarying(GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name) const;
    GLsizei getTransformFeedbackVaryingCount() const;
    GLsizei getTransformFeedbackVaryingMaxLength() const;
    GLenum getTransformFeedbackBufferMode() const;

    static bool linkValidateUniforms(InfoLog &infoLog, const std::string &uniformName, const sh::Uniform &vertexUniform, const sh::Uniform &fragmentUniform);
    static bool linkValidateInterfaceBlockFields(InfoLog &infoLog, const std::string &uniformName, const sh::InterfaceBlockField &vertexUniform, const sh::InterfaceBlockField &fragmentUniform);

    void addRef();
    void release();
    unsigned int getRefCount() const;
    void flagForDeletion();
    bool isFlaggedForDeletion() const;

    void validate(const Caps &caps);
    bool validateSamplers(InfoLog *infoLog, const Caps &caps);
    bool isValidated() const;

    const AttributesMask &getActiveAttribLocationsMask() const
    {
        return mData.mActiveAttribLocationsMask;
    }

  private:
    void unlink(bool destroy = false);
    void resetUniformBlockBindings();

    bool linkAttributes(const gl::Data &data,
                        InfoLog &infoLog,
                        const AttributeBindings &attributeBindings,
                        const Shader *vertexShader);
    bool linkUniformBlocks(InfoLog &infoLog, const Caps &caps);
    static bool linkVaryings(InfoLog &infoLog,
                             const Shader *vertexShader,
                             const Shader *fragmentShader);
    bool linkUniforms(gl::InfoLog &infoLog, const gl::Caps &caps);
    void indexUniforms();
    bool areMatchingInterfaceBlocks(gl::InfoLog &infoLog, const sh::InterfaceBlock &vertexInterfaceBlock,
                                    const sh::InterfaceBlock &fragmentInterfaceBlock);

    static bool linkValidateVariablesBase(InfoLog &infoLog,
                                          const std::string &variableName,
                                          const sh::ShaderVariable &vertexVariable,
                                          const sh::ShaderVariable &fragmentVariable,
                                          bool validatePrecision);

    static bool linkValidateVaryings(InfoLog &infoLog, const std::string &varyingName, const sh::Varying &vertexVarying, const sh::Varying &fragmentVarying);
    bool linkValidateTransformFeedback(InfoLog &infoLog,
                                       const std::vector<const sh::Varying *> &linkedVaryings,
                                       const Caps &caps) const;

    void gatherTransformFeedbackVaryings(const std::vector<const sh::Varying *> &varyings);
    bool assignUniformBlockRegister(InfoLog &infoLog, UniformBlock *uniformBlock, GLenum shader, unsigned int registerIndex, const Caps &caps);
    void defineOutputVariables(Shader *fragmentShader);

    std::vector<const sh::Varying *> getMergedVaryings() const;
    void linkOutputVariables();

    bool flattenUniformsAndCheckCaps(const Caps &caps, InfoLog &infoLog);

    struct VectorAndSamplerCount
    {
        VectorAndSamplerCount() : vectorCount(0), samplerCount(0) {}
        VectorAndSamplerCount(const VectorAndSamplerCount &other) = default;
        VectorAndSamplerCount &operator=(const VectorAndSamplerCount &other) = default;

        VectorAndSamplerCount &operator+=(const VectorAndSamplerCount &other)
        {
            vectorCount += other.vectorCount;
            samplerCount += other.samplerCount;
            return *this;
        }

        unsigned int vectorCount;
        unsigned int samplerCount;
    };

    VectorAndSamplerCount flattenUniform(const sh::ShaderVariable &uniform,
                                         const std::string &fullName,
                                         std::vector<LinkedUniform> *samplerUniforms);

    void gatherInterfaceBlockInfo();
    void defineUniformBlock(const sh::InterfaceBlock &interfaceBlock, GLenum shaderType);

    template <typename T>
    void setUniformInternal(GLint location, GLsizei count, const T *v);

    template <size_t cols, size_t rows, typename T>
    void setMatrixUniformInternal(GLint location, GLsizei count, GLboolean transpose, const T *v);

    template <typename DestT>
    void getUniformInternal(GLint location, DestT *dataOut) const;

    Data mData;
    rx::ProgramImpl *mProgram;

    bool mValidated;

    AttributeBindings mAttributeBindings;

    bool mLinked;
    bool mDeleteStatus;   // Flag to indicate that the program can be deleted when no longer in use

    unsigned int mRefCount;

    ResourceManager *mResourceManager;
    const GLuint mHandle;

    InfoLog mInfoLog;

    // Cache for sampler validation
    Optional<bool> mCachedValidateSamplersResult;
    std::vector<GLenum> mTextureUnitTypesCache;
    RangeUI mSamplerUniformRange;
};
}

#endif   // LIBANGLE_PROGRAM_H_
