//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// angle_unittests_utils.h:
//   Helpers for mocking and unit testing.

#ifndef TESTS_ANGLE_UNITTESTS_UTILS_H_
#define TESTS_ANGLE_UNITTESTS_UTILS_H_

#include "libANGLE/renderer/ImplFactory.h"

namespace rx
{

// Useful when mocking a part of the ImplFactory class
class NullFactory : public ImplFactory
{
  public:
    NullFactory() {}

    // Shader creation
    CompilerImpl *createCompiler() override { return nullptr; }
    ShaderImpl *createShader(const gl::Shader::Data &data) override { return nullptr; }
    ProgramImpl *createProgram(const gl::Program::Data &data) override { return nullptr; }

    // Framebuffer creation
    FramebufferImpl *createFramebuffer(const gl::Framebuffer::Data &data) override { return nullptr; }

    // Texture creation
    TextureImpl *createTexture(GLenum target) override { return nullptr; }

    // Renderbuffer creation
    RenderbufferImpl *createRenderbuffer() override { return nullptr; }

    // Buffer creation
    BufferImpl *createBuffer() override { return nullptr; }

    // Vertex Array creation
    VertexArrayImpl *createVertexArray(const gl::VertexArray::Data &data) override { return nullptr; }

    // Query and Fence creation
    QueryImpl *createQuery(GLenum type) override { return nullptr; }
    FenceNVImpl *createFenceNV() override { return nullptr; }
    FenceSyncImpl *createFenceSync() override { return nullptr; }

    // Transform Feedback creation
    TransformFeedbackImpl *createTransformFeedback() override { return nullptr; }

    // Sampler object creation
    SamplerImpl *createSampler() override { return nullptr; }
};

}

#endif // TESTS_ANGLE_UNITTESTS_UTILS_H_
