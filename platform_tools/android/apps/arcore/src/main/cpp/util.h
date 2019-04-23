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

#ifndef C_ARCORE_HELLOE_AR_UTIL_H_
#define C_ARCORE_HELLOE_AR_UTIL_H_

#include "SkMatrix44.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <android/log.h>
#include <cstdint>
#include <cstdlib>
#include <errno.h>
#include <jni.h>
#include <vector>

#include "arcore_c_api.h"
#include "glm.h"

#ifndef LOGI
#define LOGI(...) \
  __android_log_print(ANDROID_LOG_INFO, "hello_ar_example_c", __VA_ARGS__)
#endif  // LOGI

#ifndef LOGE
#define LOGE(...) \
  __android_log_print(ANDROID_LOG_ERROR, "hello_ar_example_c", __VA_ARGS__)
#endif  // LOGE

#ifndef CHECK
#define CHECK(condition)                                                   \
  if (!(condition)) {                                                      \
    LOGE("*** CHECK FAILED at %s:%d: %s", __FILE__, __LINE__, #condition); \
    abort();                                                               \
  }
#endif  // CHECK

namespace hello_ar {
    // Utilities
    namespace util {

        // Provides a scoped allocated instance of Anchor.
        // Can be treated as an ArAnchor*.
        class ScopedArPose {
        public:
            explicit ScopedArPose(const ArSession *session) {
                ArPose_create(session, nullptr, &pose_);
            }

            ~ScopedArPose() { ArPose_destroy(pose_); }

            ArPose *GetArPose() { return pose_; }

            // Delete copy constructors.
            ScopedArPose(const ScopedArPose &) = delete;

            void operator=(const ScopedArPose &) = delete;

        private:
            ArPose *pose_;
        };

        /* GL Utils */
        // Check GL error, and abort if an error is encountered.
        //
        // @param operation, the name of the GL function call.
        void CheckGlError(const char *operation);

        // Create a shader program ID.
        //
        // @param vertex_source, the vertex shader source.
        // @param fragment_source, the fragment shader source.
        // @return
        GLuint CreateProgram(const char *vertex_source, const char *fragment_source);

        // Load png file from assets folder and then assign it to the OpenGL target.
        // This method must be called from the renderer thread since it will result in
        // OpenGL calls to assign the image to the texture target.
        //
        // @param target, openGL texture target to load the image into.
        // @param path, path to the file, relative to the assets folder.
        // @return true if png is loaded correctly, otherwise false.
        bool LoadPngFromAssetManager(int target, const std::string &path);


        /* ARCore utils */
        void GetTransformMatrixFromPose(ArSession *ar_session, const ArPose *ar_pose, glm::mat4 *out_model_mat);

        // Get the plane's normal from center pose.
        glm::vec3 GetPlaneNormal(const ArSession *ar_session, const ArPose &plane_pose);

        // Calculate the normal distance to plane from cameraPose, the given planePose
        // should have y axis parallel to plane's normal, for example plane's center
        // pose or hit test pose.
        float CalculateDistanceToPlane(const ArSession *ar_session, const ArPose &plane_pose, const ArPose &camera_pose);

        // Outputs the camera rotation using display orientation
        glm::mat4 GetCameraRotationMatrix(float cameraOutRaw[]);

        // Computes camera position and orientation (using GetCameraRotationMatrix)
        void GetCameraInfo(ArSession* arSession, ArFrame* arFrame, glm::vec3& cameraPos, glm::mat4& cameraRotation);

        /* Matrix conversion */
        SkMatrix44 GlmMatToSkMat(const glm::mat4 m);
        glm::mat4 SkMatToGlmMat(const SkMatrix44 m);

        /* Logging utils */
        //Row major output
        void Log4x4Matrix(float raw_matrix[16]);

        //Column major output
        void LogGlmMat(glm::mat4 m, char *type);
        void LogSkMat44(SkMatrix44 m, char *type);
        void LogSkMat(SkMatrix m, char *type);
        void LogOrientation(float rotationDirection, float angleRad, char *type);

        /* Vector ops */
        float Dot(glm::vec3 u, glm::vec3 v);
        float Magnitude(glm::vec3 u);
        float AngleRad(glm::vec3 u, glm::vec3 v);
        glm::vec3 ProjectOntoPlane(glm::vec3 in, glm::vec3 normal);
    }  // namespace util
}  // namespace hello_ar

#endif  // C_ARCORE_HELLOE_AR_UTIL_H_
