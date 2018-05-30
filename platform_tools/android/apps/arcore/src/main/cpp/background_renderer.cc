/*
 * Copyright 2017 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// This modules handles drawing the passthrough camera image into the OpenGL
// scene.

#include <type_traits>

#include "background_renderer.h"

namespace hello_ar {
namespace {
// Positions of the quad vertices in clip space (X, Y, Z).
const GLfloat kVertices[] = {
    -1.0f, -1.0f, 0.0f, +1.0f, -1.0f, 0.0f,
    -1.0f, +1.0f, 0.0f, +1.0f, +1.0f, 0.0f,
};

// UVs of the quad vertices (S, T)
const GLfloat kUvs[] = {
    0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
};

constexpr char kVertexShader[] = R"(
    attribute vec4 vertex;
    attribute vec2 textureCoords;
    varying vec2 v_textureCoords;
    void main() {
      v_textureCoords = textureCoords;
      gl_Position = vertex;
    })";

constexpr char kFragmentShader[] = R"(
    #extension GL_OES_EGL_image_external : require
    precision mediump float;
    uniform samplerExternalOES texture;
    varying vec2 v_textureCoords;
    void main() {
      gl_FragColor = texture2D(texture, v_textureCoords);
    })";

}  // namespace

void BackgroundRenderer::InitializeGlContent() {
  shader_program_ = util::CreateProgram(kVertexShader, kFragmentShader);

  if (!shader_program_) {
    LOGE("Could not create program.");
  }

  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id_);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  uniform_texture_ = glGetUniformLocation(shader_program_, "texture");
  attribute_vertices_ = glGetAttribLocation(shader_program_, "vertex");
  attribute_uvs_ = glGetAttribLocation(shader_program_, "textureCoords");
}

void BackgroundRenderer::Draw(const ArSession* session, const ArFrame* frame) {
  static_assert(std::extent<decltype(kUvs)>::value == kNumVertices * 2,
                "Incorrect kUvs length");
  static_assert(std::extent<decltype(kVertices)>::value == kNumVertices * 3,
                "Incorrect kVertices length");

  // If display rotation changed (also includes view size change), we need to
  // re-query the uv coordinates for the on-screen portion of the camera image.
  int32_t geometry_changed = 0;
  ArFrame_getDisplayGeometryChanged(session, frame, &geometry_changed);
  if (geometry_changed != 0 || !uvs_initialized_) {
    ArFrame_transformDisplayUvCoords(session, frame, kNumVertices * 2, kUvs,
                                     transformed_uvs_);
    uvs_initialized_ = true;
  }

  glUseProgram(shader_program_);
  glDepthMask(GL_FALSE);

  glUniform1i(uniform_texture_, 1);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id_);

  glEnableVertexAttribArray(attribute_vertices_);
  glVertexAttribPointer(attribute_vertices_, 3, GL_FLOAT, GL_FALSE, 0,
                        kVertices);

  glEnableVertexAttribArray(attribute_uvs_);
  glVertexAttribPointer(attribute_uvs_, 2, GL_FLOAT, GL_FALSE, 0,
                        transformed_uvs_);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glUseProgram(0);
  glDepthMask(GL_TRUE);
  util::CheckGlError("BackgroundRenderer::Draw() error");
}

GLuint BackgroundRenderer::GetTextureId() const { return texture_id_; }

}  // namespace hello_ar
