//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RendererGL.h: Defines the class interface for RendererGL.

#ifndef LIBANGLE_RENDERER_GL_RENDERERGL_H_
#define LIBANGLE_RENDERER_GL_RENDERERGL_H_

#include "libANGLE/Version.h"
#include "libANGLE/renderer/Renderer.h"
#include "libANGLE/renderer/gl/WorkaroundsGL.h"

namespace rx
{
class BlitGL;
class FunctionsGL;
class StateManagerGL;

class RendererGL : public Renderer
{
  public:
    RendererGL(const FunctionsGL *functions, const egl::AttributeMap &attribMap);
    ~RendererGL() override;

    gl::Error flush() override;
    gl::Error finish() override;

    gl::Error drawArrays(const gl::Data &data, GLenum mode, GLint first, GLsizei count) override;
    gl::Error drawArraysInstanced(const gl::Data &data,
                                  GLenum mode,
                                  GLint first,
                                  GLsizei count,
                                  GLsizei instanceCount) override;

    gl::Error drawElements(const gl::Data &data,
                           GLenum mode,
                           GLsizei count,
                           GLenum type,
                           const GLvoid *indices,
                           const gl::IndexRange &indexRange) override;
    gl::Error drawElementsInstanced(const gl::Data &data,
                                    GLenum mode,
                                    GLsizei count,
                                    GLenum type,
                                    const GLvoid *indices,
                                    GLsizei instances,
                                    const gl::IndexRange &indexRange) override;
    gl::Error drawRangeElements(const gl::Data &data,
                                GLenum mode,
                                GLuint start,
                                GLuint end,
                                GLsizei count,
                                GLenum type,
                                const GLvoid *indices,
                                const gl::IndexRange &indexRange) override;

    // Shader creation
    CompilerImpl *createCompiler() override;
    ShaderImpl *createShader(const gl::Shader::Data &data) override;
    ProgramImpl *createProgram(const gl::Program::Data &data) override;

    // Framebuffer creation
    FramebufferImpl *createFramebuffer(const gl::Framebuffer::Data &data) override;

    // Texture creation
    TextureImpl *createTexture(GLenum target) override;

    // Renderbuffer creation
    RenderbufferImpl *createRenderbuffer() override;

    // Buffer creation
    BufferImpl *createBuffer() override;

    // Vertex Array creation
    VertexArrayImpl *createVertexArray(const gl::VertexArray::Data &data) override;

    // Query and Fence creation
    QueryImpl *createQuery(GLenum type) override;
    FenceNVImpl *createFenceNV() override;
    FenceSyncImpl *createFenceSync() override;

    // Transform Feedback creation
    TransformFeedbackImpl *createTransformFeedback() override;

    // Sampler object creation
    SamplerImpl *createSampler() override;

    // EXT_debug_marker
    void insertEventMarker(GLsizei length, const char *marker) override;
    void pushGroupMarker(GLsizei length, const char *marker) override;
    void popGroupMarker() override;

    // lost device
    void notifyDeviceLost() override;
    bool isDeviceLost() const override;
    bool testDeviceLost() override;
    bool testDeviceResettable() override;

    VendorID getVendorId() const override;
    std::string getVendorString() const override;
    std::string getRendererDescription() const override;

    void syncState(const gl::State &state, const gl::State::DirtyBits &dirtyBits) override;

    const gl::Version &getMaxSupportedESVersion() const;
    const FunctionsGL *getFunctions() const { return mFunctions; }
    StateManagerGL *getStateManager() const { return mStateManager; }
    const WorkaroundsGL &getWorkarounds() const { return mWorkarounds; }

  private:
    void generateCaps(gl::Caps *outCaps, gl::TextureCapsMap* outTextureCaps,
                      gl::Extensions *outExtensions,
                      gl::Limitations *outLimitations) const override;

    mutable gl::Version mMaxSupportedESVersion;

    const FunctionsGL *mFunctions;
    StateManagerGL *mStateManager;

    BlitGL *mBlitter;

    WorkaroundsGL mWorkarounds;

    // For performance debugging
    bool mSkipDrawCalls;
};

}

#endif // LIBANGLE_RENDERER_GL_RENDERERGL_H_
