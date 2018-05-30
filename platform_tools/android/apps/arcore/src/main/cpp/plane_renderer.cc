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

#include "plane_renderer.h"
#include "util.h"

namespace hello_ar {
namespace {
constexpr char kVertexShader[] = R"(
    precision highp float;
    precision highp int;
    attribute vec3 vertex;
    varying vec2 v_textureCoords;
    varying float v_alpha;

    uniform mat4 mvp;
    uniform mat4 model_mat;
    uniform vec3 normal;

    void main() {
      // Vertex Z value is used as the alpha in this shader.
      v_alpha = vertex.z;

      vec4 local_pos = vec4(vertex.x, 0.0, vertex.y, 1.0);
      gl_Position = mvp * local_pos;
      vec4 world_pos = model_mat * local_pos;

      // Construct two vectors that are orthogonal to the normal.
      // This arbitrary choice is not co-linear with either horizontal
      // or vertical plane normals.
      const vec3 arbitrary = vec3(1.0, 1.0, 0.0);
      vec3 vec_u = normalize(cross(normal, arbitrary));
      vec3 vec_v = normalize(cross(normal, vec_u));

      // Project vertices in world frame onto vec_u and vec_v.
      v_textureCoords = vec2(
         dot(world_pos.xyz, vec_u), dot(world_pos.xyz, vec_v));
    })";

constexpr char kFragmentShader[] = R"(
    precision highp float;
    precision highp int;
    uniform sampler2D texture;
    uniform vec3 color;
    varying vec2 v_textureCoords;
    varying float v_alpha;
    void main() {
      float r = texture2D(texture, v_textureCoords).r;
      gl_FragColor = vec4(color.xyz, r * v_alpha);
    })";
}  // namespace

void PlaneRenderer::InitializeGlContent(AAssetManager* asset_manager) {
  shader_program_ = util::CreateProgram(kVertexShader, kFragmentShader);

  if (!shader_program_) {
    LOGE("Could not create program.");
  }

  uniform_mvp_mat_ = glGetUniformLocation(shader_program_, "mvp");
  uniform_texture_ = glGetUniformLocation(shader_program_, "texture");
  uniform_model_mat_ = glGetUniformLocation(shader_program_, "model_mat");
  uniform_normal_vec_ = glGetUniformLocation(shader_program_, "normal");
  uniform_color_ = glGetUniformLocation(shader_program_, "color");
  attri_vertices_ = glGetAttribLocation(shader_program_, "vertex");

  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  if (!util::LoadPngFromAssetManager(GL_TEXTURE_2D, "models/trigrid.png")) {
    LOGE("Could not load png texture for planes.");
  }

  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);

  util::CheckGlError("plane_renderer::InitializeGlContent()");
}

void PlaneRenderer::Draw(const glm::mat4& projection_mat,
                         const glm::mat4& view_mat, const ArSession* ar_session,
                         const ArPlane* ar_plane, const glm::vec3& color) {
  if (!shader_program_) {
    LOGE("shader_program is null.");
    return;
  }

  UpdateForPlane(ar_session, ar_plane);

  glUseProgram(shader_program_);
  glDepthMask(GL_FALSE);

  glActiveTexture(GL_TEXTURE0);
  glUniform1i(uniform_texture_, 0);
  glBindTexture(GL_TEXTURE_2D, texture_id_);

  // Compose final mvp matrix for this plane renderer.
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE,
                     glm::value_ptr(projection_mat * view_mat * model_mat_));

  glUniformMatrix4fv(uniform_model_mat_, 1, GL_FALSE,
                     glm::value_ptr(model_mat_));
  glUniform3f(uniform_normal_vec_, normal_vec_.x, normal_vec_.y, normal_vec_.z);
  glUniform3f(uniform_color_, color.x, color.y, color.z);

  glEnableVertexAttribArray(attri_vertices_);
  glVertexAttribPointer(attri_vertices_, 3, GL_FLOAT, GL_FALSE, 0,
                        vertices_.data());

  glDrawElements(GL_TRIANGLES, triangles_.size(), GL_UNSIGNED_SHORT,
                 triangles_.data());

  glUseProgram(0);
  glDepthMask(GL_TRUE);
  util::CheckGlError("plane_renderer::Draw()");
}

void PlaneRenderer::UpdateForPlane(const ArSession* ar_session,
                                   const ArPlane* ar_plane) {
  // The following code generates a triangle mesh filling a convex polygon,
  // including a feathered edge for blending.
  //
  // The indices shown in the diagram are used in comments below.
  // _______________     0_______________1
  // |             |      |4___________5|
  // |             |      | |         | |
  // |             | =>   | |         | |
  // |             |      | |         | |
  // |             |      |7-----------6|
  // ---------------     3---------------2

  vertices_.clear();
  triangles_.clear();

  int32_t polygon_length;
  ArPlane_getPolygonSize(ar_session, ar_plane, &polygon_length);

  if (polygon_length == 0) {
    LOGE("PlaneRenderer::UpdatePlane, no valid plane polygon is found");
    return;
  }

  const int32_t vertices_size = polygon_length / 2;
  std::vector<glm::vec2> raw_vertices(vertices_size);
  ArPlane_getPolygon(ar_session, ar_plane,
                     glm::value_ptr(raw_vertices.front()));

  // Fill vertex 0 to 3. Note that the vertex.xy are used for x and z
  // position. vertex.z is used for alpha. The outter polygon's alpha
  // is 0.
  for (int32_t i = 0; i < vertices_size; ++i) {
    vertices_.push_back(glm::vec3(raw_vertices[i].x, raw_vertices[i].y, 0.0f));
  }

  util::ScopedArPose scopedArPose(ar_session);
  ArPlane_getCenterPose(ar_session, ar_plane, scopedArPose.GetArPose());
  ArPose_getMatrix(ar_session, scopedArPose.GetArPose(),
                   glm::value_ptr(model_mat_));
  normal_vec_ = util::GetPlaneNormal(ar_session, *scopedArPose.GetArPose());

  // Feather distance 0.2 meters.
  const float kFeatherLength = 0.2f;
  // Feather scale over the distance between plane center and vertices.
  const float kFeatherScale = 0.2f;

  // Fill vertex 4 to 7, with alpha set to 1.
  for (int32_t i = 0; i < vertices_size; ++i) {
    // Vector from plane center to current point.
    glm::vec2 v = raw_vertices[i];
    const float scale =
        1.0f - std::min((kFeatherLength / glm::length(v)), kFeatherScale);
    const glm::vec2 result_v = scale * v;

    vertices_.push_back(glm::vec3(result_v.x, result_v.y, 1.0f));
  }

  const int32_t vertices_length = vertices_.size();
  const int32_t half_vertices_length = vertices_length / 2;

  // Generate triangle (4, 5, 6) and (4, 6, 7).
  for (int i = half_vertices_length + 1; i < vertices_length - 1; ++i) {
    triangles_.push_back(half_vertices_length);
    triangles_.push_back(i);
    triangles_.push_back(i + 1);
  }

  // Generate triangle (0, 1, 4), (4, 1, 5), (5, 1, 2), (5, 2, 6),
  // (6, 2, 3), (6, 3, 7), (7, 3, 0), (7, 0, 4)
  for (int i = 0; i < half_vertices_length; ++i) {
    triangles_.push_back(i);
    triangles_.push_back((i + 1) % half_vertices_length);
    triangles_.push_back(i + half_vertices_length);

    triangles_.push_back(i + half_vertices_length);
    triangles_.push_back((i + 1) % half_vertices_length);
    triangles_.push_back((i + half_vertices_length + 1) % half_vertices_length +
                         half_vertices_length);
  }
}

}  // namespace hello_ar
