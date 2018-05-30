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

#ifndef C_ARCORE_HELLOE_AR_OBJ_RENDERER_
#define C_ARCORE_HELLOE_AR_OBJ_RENDERER_
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
class ObjRenderer {
 public:
  ObjRenderer() = default;
  ~ObjRenderer() = default;

  // Loads the OBJ file and texture and sets up OpenGL resources used to draw
  // the model.  Must be called on the OpenGL thread prior to any other calls.
  void InitializeGlContent(AAssetManager* asset_manager,
                           const std::string& obj_file_name,
                           const std::string& png_file_name);

  // Sets the surface's lighting reflectace properties.  Diffuse is modulated by
  // the texture's color.
  void SetMaterialProperty(float ambient, float diffuse, float specular,
                           float specular_power);

  // Draws the model.
  void Draw(const glm::mat4& projection_mat, const glm::mat4& view_mat,
            const glm::mat4& model_mat, const float* color_correction4) const;

 private:
  // Shader material lighting pateremrs
  float ambient_ = 0.0f;
  float diffuse_ = 2.0f;
  float specular_ = 0.5f;
  float specular_power_ = 6.0f;

  // Model attribute arrays
  std::vector<GLfloat> vertices_;
  std::vector<GLfloat> uvs_;
  std::vector<GLfloat> normals_;

  // Model triangle indices
  std::vector<GLushort> indices_;

  // Loaded TEXTURE_2D object name
  GLuint texture_id_;

  // Shader program details
  GLuint shader_program_;
  GLint attri_vertices_;
  GLint attri_uvs_;
  GLint attri_normals_;
  GLint uniform_mvp_mat_;
  GLint uniform_mv_mat_;
  GLint uniform_texture_;
  GLint uniform_lighting_param_;
  GLint uniform_material_param_;
  GLint uniform_color_correction_param_;
};
}  // namespace hello_ar

#endif  // C_ARCORE_HELLOE_AR_OBJ_RENDERER_
