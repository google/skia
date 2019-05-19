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
#include "platform_tools/android/apps/arcore/src/main/cpp/util.h"

#include "include/core/SkMatrix44.h"
#include <gtx/string_cast.inl>
#include <sstream>
#include <string>
#include <unistd.h>

#include "platform_tools/android/apps/arcore/src/main/cpp/jni_interface.h"

namespace hello_ar {
    namespace util {

        void CheckGlError(const char *operation) {
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
        static GLuint LoadShader(GLenum shader_type, const char *shader_source) {
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

                char *buf = reinterpret_cast<char *>(malloc(info_len));
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

        GLuint CreateProgram(const char *vertex_source, const char *fragment_source) {
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
                        char *buf = reinterpret_cast<char *>(malloc(buf_length));
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

        bool LoadPngFromAssetManager(int target, const std::string &path) {
            JNIEnv *env = GetJniEnv();

            // Put all the JNI values in a structure that is statically initalized on the
            // first call to this method.  This makes it thread safe in the unlikely case
            // of multiple threads calling this method.
            static struct JNIData {
                jclass helper_class;
                jmethodID load_image_method;
                jmethodID load_texture_method;
            } jniIds = [env]() -> JNIData {
                constexpr char kHelperClassName[] =
                        "org/skia/arcore/JniInterface";
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

        void GetTransformMatrixFromPose(ArSession *ar_session,
                                        const ArPose *ar_pose,
                                        glm::mat4 *out_model_mat) {
            if (out_model_mat == nullptr) {
                LOGE("util::GetTransformMatrixFromPose model_mat is null.");
                return;
            }
            ArPose_getMatrix(ar_session, ar_pose,
                             glm::value_ptr(*out_model_mat));
        }

        glm::vec3 GetPlaneNormal(const ArSession *ar_session,
                                 const ArPose &plane_pose) {
            float plane_pose_raw[7] = {0.f};
            ArPose_getPoseRaw(ar_session, &plane_pose, plane_pose_raw);
            glm::quat plane_quaternion(plane_pose_raw[3], plane_pose_raw[0],
                                       plane_pose_raw[1], plane_pose_raw[2]);
            // Get normal vector, normal is defined to be positive Y-position in local
            // frame.
            return glm::rotate(plane_quaternion, glm::vec3(0., 1.f, 0.));
        }

        float CalculateDistanceToPlane(const ArSession *ar_session,
                                       const ArPose &plane_pose,
                                       const ArPose &camera_pose) {
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

        glm::mat4 GetCameraRotationMatrix(float cameraOutRaw[]) {
            glm::mat4 cameraRotation(1);
            glm::quat cameraQuat = glm::quat(cameraOutRaw[0], cameraOutRaw[1], cameraOutRaw[2],
                                             cameraOutRaw[3]);
            cameraRotation = glm::toMat4(cameraQuat);
            glm::vec4 temp = cameraRotation[0];
            cameraRotation[0] = cameraRotation[2];
            cameraRotation[2] = temp;
            return cameraRotation;
        }

        void GetCameraInfo(ArSession* arSession, ArFrame* arFrame, glm::vec3& cameraPos, glm::mat4& cameraRotation) {
            //Acquire camera
            ArCamera *ar_camera;
            ArFrame_acquireCamera(arSession, arFrame, &ar_camera);

            //Get camera pose
            ArPose *camera_pose = nullptr;
            ArPose_create(arSession, nullptr, &camera_pose);
            ArCamera_getDisplayOrientedPose(arSession, ar_camera, camera_pose);

            //Get camera raw info
            float outCameraRaw[] = {0, 0, 0, 0, 0, 0, 0};
            ArPose_getPoseRaw(arSession, camera_pose, outCameraRaw);
            ArPose_destroy(camera_pose);

            //Write to out variables
            cameraPos = glm::vec3(outCameraRaw[4], outCameraRaw[5], outCameraRaw[6]);
            cameraRotation = util::GetCameraRotationMatrix(outCameraRaw);

            //Release camera
            ArCamera_release(ar_camera);
        }

        SkMatrix44 GlmMatToSkMat(const glm::mat4 m) {
            SkMatrix44 skMat = SkMatrix44::kIdentity_Constructor;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    skMat.set(j, i, m[i][j]);
                }
            }
            return skMat;
        }

        glm::mat4 SkMatToGlmMat(const SkMatrix44 m) {
            glm::mat4 glmMat(1);
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    glmMat[i][j] = m.get(j, i);
                }
            }
            return glmMat;
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

        void LogGlmMat(glm::mat4 m, char *type) {
            std::string str = glm::to_string(m);
            LOGE("glm Matrix - %s: %s\n", type, str.c_str());
        }

        void LogSkMat44(SkMatrix44 m, char *type) {
            LOGE("SkMatrix - %s: [%g, %g, %g, %g] || [%g, %g, %g, %g] || [%g, %g, %g, %g] || [%g, %g, %g, %g] \n",
                 type,
                 m.get(0, 0), m.get(1, 0), m.get(2, 0), m.get(3, 0),
                 m.get(0, 1), m.get(1, 1), m.get(2, 1), m.get(3, 1),
                 m.get(0, 2), m.get(1, 2), m.get(2, 2), m.get(3, 2),
                 m.get(0, 3), m.get(1, 3), m.get(2, 3), m.get(3, 3)
            );
        }

        void LogSkMat(SkMatrix m, char *type) {
            LOGE("SkMatrix - %s: [%g, %g, %g] || [%g, %g, %g] || [%g, %g, %g] \n", type,
                 m.get(0), m.get(3), m.get(6),
                 m.get(1), m.get(4), m.get(7),
                 m.get(2), m.get(5), m.get(8)
            );
        }

        void LogOrientation(float rotationDirection, float angleRad, char *type) {
            LOGI("Plane orientation: %s", type);
            LOGI("Cross dotted with zDir:", rotationDirection);
            if (rotationDirection == -1) {
                LOGI("Counter Clockwise %.6f degrees rotation: ", glm::degrees(angleRad));
            } else {
                LOGI("Clockwise %.6f degrees rotation: ", glm::degrees(angleRad));
            }
        }

        float Dot(glm::vec3 u, glm::vec3 v) {
            float result = u.x * v.x + u.y * v.y + u.z * v.z;
            return result;
        }

        float Magnitude(glm::vec3 u) {
            float result = u.x * u.x + u.y * u.y + u.z * u.z;
            return sqrt(result);
        }

        float AngleRad(glm::vec3 u, glm::vec3 v) {
            float dot = util::Dot(u, v);
            float scale = (util::Magnitude(u) * util::Magnitude(v));
            float cosine = dot / scale;
            float acosine = acos(cosine);
            return acosine;
        }

        glm::vec3 ProjectOntoPlane(glm::vec3 in, glm::vec3 normal) {
            float dot = util::Dot(in, normal);
            float multiplier = dot / (util::Magnitude(normal) * util::Magnitude(normal));
            glm::vec3 out = in - multiplier * normal;
            return out;
        }

    }  // namespace util
}  // namespace hello_ar
