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
#include "util.h"

#include <unistd.h>
#include <sstream>
#include <string>

#include "jni_interface.h"

namespace hello_ar {
namespace util {

void CheckGlError(const char* operation) {
  bool anyError = false;
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGE("after %s() glError (0x%x)\n", operation, error);
    anyError = true;
  }
  if (anyError) {
    abort();
  }
}

// Convenience function used in CreateProgram below.
static GLuint LoadShader(GLenum shader_type, const char* shader_source) {
  GLuint shader = glCreateShader(shader_type);
  if (!shader) {
    return shader;
  }

  glShaderSource(shader, 1, &shader_source, nullptr);
  glCompileShader(shader);
  GLint compiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

  if (!compiled) {
    GLint info_len = 0;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
    if (!info_len) {
      return shader;
    }

    char* buf = reinterpret_cast<char*>(malloc(info_len));
    if (!buf) {
      return shader;
    }

    glGetShaderInfoLog(shader, info_len, nullptr, buf);
    LOGE("hello_ar::util::Could not compile shader %d:\n%s\n", shader_type,
         buf);
    free(buf);
    glDeleteShader(shader);
    shader = 0;
  }

  return shader;
}

GLuint CreateProgram(const char* vertex_source, const char* fragment_source) {
  GLuint vertexShader = LoadShader(GL_VERTEX_SHADER, vertex_source);
  if (!vertexShader) {
    return 0;
  }

  GLuint fragment_shader = LoadShader(GL_FRAGMENT_SHADER, fragment_source);
  if (!fragment_shader) {
    return 0;
  }

  GLuint program = glCreateProgram();
  if (program) {
    glAttachShader(program, vertexShader);
    CheckGlError("hello_ar::util::glAttachShader");
    glAttachShader(program, fragment_shader);
    CheckGlError("hello_ar::util::glAttachShader");
    glLinkProgram(program);
    GLint link_status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if (link_status != GL_TRUE) {
      GLint buf_length = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &buf_length);
      if (buf_length) {
        char* buf = reinterpret_cast<char*>(malloc(buf_length));
        if (buf) {
          glGetProgramInfoLog(program, buf_length, nullptr, buf);
          LOGE("hello_ar::util::Could not link program:\n%s\n", buf);
          free(buf);
        }
      }
      glDeleteProgram(program);
      program = 0;
    }
  }
  return program;
}

bool LoadPngFromAssetManager(int target, const std::string& path) {
  JNIEnv* env = GetJniEnv();

  // Put all the JNI values in a structure that is statically initalized on the
  // first call to this method.  This makes it thread safe in the unlikely case
  // of multiple threads calling this method.
  static struct JNIData {
    jclass helper_class;
    jmethodID load_image_method;
    jmethodID load_texture_method;
  } jniIds = [env]() -> JNIData {
    constexpr char kHelperClassName[] =
        "org/skia/viewer/JniInterface";
    constexpr char kLoadImageMethodName[] = "loadImage";
    constexpr char kLoadImageMethodSignature[] =
        "(Ljava/lang/String;)Landroid/graphics/Bitmap;";
    constexpr char kLoadTextureMethodName[] = "loadTexture";
    constexpr char kLoadTextureMethodSignature[] =
        "(ILandroid/graphics/Bitmap;)V";
    jclass helper_class = FindClass(kHelperClassName);
    if (helper_class) {
      helper_class = static_cast<jclass>(env->NewGlobalRef(helper_class));
      jmethodID load_image_method = env->GetStaticMethodID(
          helper_class, kLoadImageMethodName, kLoadImageMethodSignature);
      jmethodID load_texture_method = env->GetStaticMethodID(
          helper_class, kLoadTextureMethodName, kLoadTextureMethodSignature);
      return {helper_class, load_image_method, load_texture_method};
    }
    LOGE("hello_ar::util::Could not find Java helper class %s",
         kHelperClassName);
    return {};
  }();

  if (!jniIds.helper_class) {
    return false;
  }

  jstring j_path = env->NewStringUTF(path.c_str());

  jobject image_obj = env->CallStaticObjectMethod(
      jniIds.helper_class, jniIds.load_image_method, j_path);

  if (j_path) {
    env->DeleteLocalRef(j_path);
  }

  env->CallStaticVoidMethod(jniIds.helper_class, jniIds.load_texture_method,
                            target, image_obj);
  return true;
}

bool LoadObjFile(AAssetManager* mgr, const std::string& file_name,
                 std::vector<GLfloat>* out_vertices,
                 std::vector<GLfloat>* out_normals,
                 std::vector<GLfloat>* out_uv,
                 std::vector<GLushort>* out_indices) {
  std::vector<GLfloat> temp_positions;
  std::vector<GLfloat> temp_normals;
  std::vector<GLfloat> temp_uvs;
  std::vector<GLushort> vertex_indices;
  std::vector<GLushort> normal_indices;
  std::vector<GLushort> uv_indices;

  // If the file hasn't been uncompressed, load it to the internal storage.
  // Note that AAsset_openFileDescriptor doesn't support compressed
  // files (.obj).
  AAsset* asset =
      AAssetManager_open(mgr, file_name.c_str(), AASSET_MODE_STREAMING);
  if (asset == nullptr) {
    LOGE("Error opening asset %s", file_name.c_str());
    return false;
  }

  off_t file_size = AAsset_getLength(asset);
  std::string file_buffer;
  file_buffer.resize(file_size);
  int ret = AAsset_read(asset, &file_buffer.front(), file_size);

  if (ret < 0 || ret == EOF) {
    LOGE("Failed to open file: %s", file_name.c_str());
    AAsset_close(asset);
    return false;
  }

  AAsset_close(asset);
  std::stringstream file_string_stream(file_buffer);

  while (!file_string_stream.eof()) {
    char line_header[128];
    file_string_stream.getline(line_header, 128);

    if (line_header[0] == 'v' && line_header[1] == 'n') {
      // Parse vertex normal.
      GLfloat normal[3];
      int matches = sscanf(line_header, "vn %f %f %f\n", &normal[0], &normal[1],
                           &normal[2]);
      if (matches != 3) {
        LOGE("Format of 'vn float float float' required for each normal line");
        return false;
      }

      temp_normals.push_back(normal[0]);
      temp_normals.push_back(normal[1]);
      temp_normals.push_back(normal[2]);
    } else if (line_header[0] == 'v' && line_header[1] == 't') {
      // Parse texture uv.
      GLfloat uv[2];
      int matches = sscanf(line_header, "vt %f %f\n", &uv[0], &uv[1]);
      if (matches != 2) {
        LOGE("Format of 'vt float float' required for each texture uv line");
        return false;
      }

      temp_uvs.push_back(uv[0]);
      temp_uvs.push_back(uv[1]);
    } else if (line_header[0] == 'v') {
      // Parse vertex.
      GLfloat vertex[3];
      int matches = sscanf(line_header, "v %f %f %f\n", &vertex[0], &vertex[1],
                           &vertex[2]);
      if (matches != 3) {
        LOGE("Format of 'v float float float' required for each vertice line");
        return false;
      }

      temp_positions.push_back(vertex[0]);
      temp_positions.push_back(vertex[1]);
      temp_positions.push_back(vertex[2]);
    } else if (line_header[0] == 'f') {
      // Actual faces information starts from the second character.
      char* face_line = &line_header[1];

      unsigned int vertex_index[4];
      unsigned int normal_index[4];
      unsigned int texture_index[4];

      std::vector<char*> per_vert_info_list;
      char* per_vert_info_list_c_str;
      char* face_line_iter = face_line;
      while ((per_vert_info_list_c_str =
                  strtok_r(face_line_iter, " ", &face_line_iter))) {
        // Divide each faces information into individual positions.
        per_vert_info_list.push_back(per_vert_info_list_c_str);
      }

      bool is_normal_available = false;
      bool is_uv_available = false;
      for (int i = 0; i < per_vert_info_list.size(); ++i) {
        char* per_vert_info;
        int per_vert_infor_count = 0;

        bool is_vertex_normal_only_face =
            (strstr(per_vert_info_list[i], "//") != nullptr);

        char* per_vert_info_iter = per_vert_info_list[i];
        while ((per_vert_info =
                    strtok_r(per_vert_info_iter, "/", &per_vert_info_iter))) {
          // write only normal and vert values.
          switch (per_vert_infor_count) {
            case 0:
              // Write to vertex indices.
              vertex_index[i] = atoi(per_vert_info);  // NOLINT
              break;
            case 1:
              // Write to texture indices.
              if (is_vertex_normal_only_face) {
                normal_index[i] = atoi(per_vert_info);  // NOLINT
                is_normal_available = true;
              } else {
                texture_index[i] = atoi(per_vert_info);  // NOLINT
                is_uv_available = true;
              }
              break;
            case 2:
              // Write to normal indices.
              if (!is_vertex_normal_only_face) {
                normal_index[i] = atoi(per_vert_info);  // NOLINT
                is_normal_available = true;
                break;
              }
              [[clang::fallthrough]];
            // Intentionally falling to default error case because vertex
            // normal face only has two values.
            default:
              // Error formatting.
              LOGE(
                  "Format of 'f int/int/int int/int/int int/int/int "
                  "(int/int/int)' "
                  "or 'f int//int int//int int//int (int//int)' required for "
                  "each face");
              return false;
          }
          per_vert_infor_count++;
        }
      }

      int vertices_count = per_vert_info_list.size();
      for (int i = 2; i < vertices_count; ++i) {
        vertex_indices.push_back(vertex_index[0] - 1);
        vertex_indices.push_back(vertex_index[i - 1] - 1);
        vertex_indices.push_back(vertex_index[i] - 1);

        if (is_normal_available) {
          normal_indices.push_back(normal_index[0] - 1);
          normal_indices.push_back(normal_index[i - 1] - 1);
          normal_indices.push_back(normal_index[i] - 1);
        }

        if (is_uv_available) {
          uv_indices.push_back(texture_index[0] - 1);
          uv_indices.push_back(texture_index[i - 1] - 1);
          uv_indices.push_back(texture_index[i] - 1);
        }
      }
    }
  }

  bool is_normal_available = (!normal_indices.empty());
  bool is_uv_available = (!uv_indices.empty());

  if (is_normal_available && normal_indices.size() != vertex_indices.size()) {
    LOGE("Obj normal indices does not equal to vertex indices.");
    return false;
  }

  if (is_uv_available && uv_indices.size() != vertex_indices.size()) {
    LOGE("Obj UV indices does not equal to vertex indices.");
    return false;
  }

  for (unsigned int i = 0; i < vertex_indices.size(); i++) {
    unsigned int vertex_index = vertex_indices[i];
    out_vertices->push_back(temp_positions[vertex_index * 3]);
    out_vertices->push_back(temp_positions[vertex_index * 3 + 1]);
    out_vertices->push_back(temp_positions[vertex_index * 3 + 2]);
    out_indices->push_back(i);

    if (is_normal_available) {
      unsigned int normal_index = normal_indices[i];
      out_normals->push_back(temp_normals[normal_index * 3]);
      out_normals->push_back(temp_normals[normal_index * 3 + 1]);
      out_normals->push_back(temp_normals[normal_index * 3 + 2]);
    }

    if (is_uv_available) {
      unsigned int uv_index = uv_indices[i];
      out_uv->push_back(temp_uvs[uv_index * 2]);
      out_uv->push_back(temp_uvs[uv_index * 2 + 1]);
    }
  }

  return true;
}

void Log4x4Matrix(float raw_matrix[16]) {
  LOGI(
      "%f, %f, %f, %f\n"
      "%f, %f, %f, %f\n"
      "%f, %f, %f, %f\n"
      "%f, %f, %f, %f\n",
      raw_matrix[0], raw_matrix[1], raw_matrix[2], raw_matrix[3], raw_matrix[4],
      raw_matrix[5], raw_matrix[6], raw_matrix[7], raw_matrix[8], raw_matrix[9],
      raw_matrix[10], raw_matrix[11], raw_matrix[12], raw_matrix[13],
      raw_matrix[14], raw_matrix[15]);
}

void GetTransformMatrixFromAnchor(ArSession* ar_session,
                                  const ArAnchor* ar_anchor,
                                  glm::mat4* out_model_mat) {
  if (out_model_mat == nullptr) {
    LOGE("util::GetTransformMatrixFromAnchor model_mat is null.");
    return;
  }
  util::ScopedArPose pose(ar_session);
  ArAnchor_getPose(ar_session, ar_anchor, pose.GetArPose());
  ArPose_getMatrix(ar_session, pose.GetArPose(),
                   glm::value_ptr(*out_model_mat));
}

glm::vec3 GetPlaneNormal(const ArSession* ar_session,
                         const ArPose& plane_pose) {
  float plane_pose_raw[7] = {0.f};
  ArPose_getPoseRaw(ar_session, &plane_pose, plane_pose_raw);
  glm::quat plane_quaternion(plane_pose_raw[3], plane_pose_raw[0],
                             plane_pose_raw[1], plane_pose_raw[2]);
  // Get normal vector, normal is defined to be positive Y-position in local
  // frame.
  return glm::rotate(plane_quaternion, glm::vec3(0., 1.f, 0.));
}

float CalculateDistanceToPlane(const ArSession* ar_session,
                               const ArPose& plane_pose,
                               const ArPose& camera_pose) {
  float plane_pose_raw[7] = {0.f};
  ArPose_getPoseRaw(ar_session, &plane_pose, plane_pose_raw);
  glm::vec3 plane_position(plane_pose_raw[4], plane_pose_raw[5],
                           plane_pose_raw[6]);
  glm::vec3 normal = GetPlaneNormal(ar_session, plane_pose);

  float camera_pose_raw[7] = {0.f};
  ArPose_getPoseRaw(ar_session, &camera_pose, camera_pose_raw);
  glm::vec3 camera_P_plane(camera_pose_raw[4] - plane_position.x,
                           camera_pose_raw[5] - plane_position.y,
                           camera_pose_raw[6] - plane_position.z);
  return glm::dot(normal, camera_P_plane);
}

}  // namespace util
}  // namespace hello_ar
