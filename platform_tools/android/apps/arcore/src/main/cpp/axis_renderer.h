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

#ifndef C_ARCORE_HELLOE_AR_AXIS_RENDERER_
#define C_ARCORE_HELLOE_AR_AXIS_RENDERER_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "arcore_c_api.h"
#include "glm.h"

namespace hello_ar {

// PlaneRenderer renders ARCore plane type.
	class AxisRenderer {
	public:
	    AxisRenderer() = default;

		~AxisRenderer() = default;

		// Loads the OBJ file and texture and sets up OpenGL resources used to draw
		// the model.  Must be called on the OpenGL thread prior to any other calls.
		void InitializeGlContent();


		// Draws the model.
		void Draw(const glm::mat4 &projection_mat, const glm::mat4 &view_mat,
		          const glm::mat4 &model_mat) const;

	private:
		// Model attribute arrays
		std::vector<GLfloat> vertices_;
		std::vector<GLfloat> uvs_;

		// Model line indices
		std::vector<GLushort> indices_;

		// Shader program details
		GLuint shader_program_;
		GLuint attri_vertices_;
		GLuint attri_uvs_;
		GLint uniform_mvp_mat_;
	};
}  // namespace hello_ar

#endif  // C_ARCORE_HELLOE_AR_AXIS_RENDERER_
