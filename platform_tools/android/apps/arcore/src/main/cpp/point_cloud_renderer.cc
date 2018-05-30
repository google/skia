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

#include "point_cloud_renderer.h"
#include "util.h"

namespace hello_ar {
namespace {
constexpr char kVertexShader[] = R"(
    attribute vec4 vertex;
    uniform mat4 mvp;
    void main() {
      gl_PointSize = 5.0;
      // Pointcloud vertex's w component is confidence value.
      // Not used in renderer.
      gl_Position = mvp * vec4(vertex.xyz, 1.0);
    })";

constexpr char kFragmentShader[] = R"(
    precision lowp float;
    void main() {
      gl_FragColor = vec4(0.1215, 0.7372, 0.8235, 1.0);
    })";
}  // namespace

void PointCloudRenderer::InitializeGlContent() {
  shader_program_ = util::CreateProgram(kVertexShader, kFragmentShader);

  CHECK(shader_program_);

  attribute_vertices_ = glGetAttribLocation(shader_program_, "vertex");
  uniform_mvp_mat_ = glGetUniformLocation(shader_program_, "mvp");

  util::CheckGlError("point_cloud_renderer::InitializeGlContent()");
}

void PointCloudRenderer::Draw(glm::mat4 mvp_matrix, ArSession* ar_session,
                              ArPointCloud* ar_point_cloud) const {
  CHECK(shader_program_);

  glUseProgram(shader_program_);

  int32_t number_of_points = 0;
  ArPointCloud_getNumberOfPoints(ar_session, ar_point_cloud, &number_of_points);
  if (number_of_points <= 0) {
    return;
  }

  const float* point_cloud_data;
  ArPointCloud_getData(ar_session, ar_point_cloud, &point_cloud_data);

  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_matrix));

  glEnableVertexAttribArray(attribute_vertices_);
  glVertexAttribPointer(attribute_vertices_, 4, GL_FLOAT, GL_FALSE, 0,
                        point_cloud_data);

  glDrawArrays(GL_POINTS, 0, number_of_points);

  glUseProgram(0);
  util::CheckGlError("PointCloudRenderer::Draw");
}

}  // namespace hello_ar
