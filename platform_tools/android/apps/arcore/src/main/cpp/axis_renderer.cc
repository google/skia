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

#include "axis_renderer.h"
#include "util.h"

namespace hello_ar {
	namespace {
		constexpr char kVertexShader[] = R"(
    attribute vec3 vertex;
    attribute vec3 color;
    uniform mat4 mvp;
    varying vec3 v_color;
    void main() {
      //gl_PointSize = 2;
      // Pointcloud vertex's w component is confidence value.
      // Not used in renderer.
      v_color = color;
      gl_Position = mvp * vec4(vertex.xyz, 1.0);
    })";

		constexpr char kFragmentShader[] = R"(
    precision lowp float;
    varying vec3 v_color;
    void main() {
      gl_FragColor = vec4(v_color.xyz, 1.0);
    })";
	}  // namespace

	void AxisRenderer::InitializeGlContent() {
		shader_program_ = util::CreateProgram(kVertexShader, kFragmentShader);

		if (!shader_program_) {
			LOGE("Could not create program.");
		}

		uniform_mvp_mat_ =
				glGetUniformLocation(shader_program_, "mvp");

		attri_vertices_ = glGetAttribLocation(shader_program_, "vertex");
		attri_uvs_ = glGetAttribLocation(shader_program_, "color");

		util::CreateAxes(&vertices_, &uvs_, &indices_);

		util::CheckGlError("axis_renderer::InitializeGlContent()");
	}

	void AxisRenderer::Draw(const glm::mat4 &projection_mat,
	                       const glm::mat4 &view_mat, const glm::mat4 &model_mat) const {
		if (!shader_program_) {
			LOGE("shader_program is null.");
			return;
		}

		glUseProgram(shader_program_);
		glLineWidth(2);

		glm::mat4 mvp_mat = projection_mat * view_mat * model_mat;

		glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));;

		glEnableVertexAttribArray(attri_vertices_);
		glVertexAttribPointer(attri_vertices_, 3, GL_FLOAT, GL_FALSE, 0, vertices_.data());

		glEnableVertexAttribArray(attri_uvs_);
		glVertexAttribPointer(attri_uvs_, 3, GL_FLOAT, GL_FALSE, 0,
		                      uvs_.data());

		glDrawElements(GL_LINES, indices_.size(), GL_UNSIGNED_SHORT, indices_.data());

		glDisableVertexAttribArray(attri_vertices_);
		glDisableVertexAttribArray(attri_uvs_);
		glUseProgram(0);
		util::CheckGlError("axes_renderer::Draw()");
	}

}  // namespace hello_ar
