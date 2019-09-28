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

#ifndef C_ARCORE_HELLOE_AR_HELLO_AR_APPLICATION_H_
#define C_ARCORE_HELLOE_AR_HELLO_AR_APPLICATION_H_

#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "modules/skottie/include/Skottie.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <jni.h>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

#include "arcore_c_api.h"
#include "platform_tools/android/apps/arcore/src/main/cpp/background_renderer.h"
#include "platform_tools/android/apps/arcore/src/main/cpp/glm.h"
#include "platform_tools/android/apps/arcore/src/main/cpp/pending_anchor.h"
#include "platform_tools/android/apps/arcore/src/main/cpp/plane_renderer.h"
#include "platform_tools/android/apps/arcore/src/main/cpp/point_cloud_renderer.h"
#include "platform_tools/android/apps/arcore/src/main/cpp/util.h"

namespace hello_ar {

// HelloArApplication handles all application logics.
    class HelloArApplication {
    public:
        // Constructor and deconstructor.
        HelloArApplication() = default;

        HelloArApplication(AAssetManager *asset_manager);

        ~HelloArApplication();

        SkMatrix SkiaRenderer(const glm::mat4 &proj, const glm::mat4 &view, const glm::mat4 &model);

        // OnPause is called on the UI thread from the Activity's onPause method.
        void OnPause();

        // OnResume is called on the UI thread from the Activity's onResume method.
        void OnResume(void *env, void *context, void *activity);

        // OnSurfaceCreated is called on the OpenGL thread when GLSurfaceView
        // is created.
        void OnSurfaceCreated();

        // OnDisplayGeometryChanged is called on the OpenGL thread when the
        // render surface size or display rotation changes.
        //
        // @param display_rotation: current display rotation.
        // @param width: width of the changed surface view.
        // @param height: height of the changed surface view.
        void OnDisplayGeometryChanged(int display_rotation, int width, int height);

        void OnObjectRotationChanged(int rotation);

        void OnAction(float value);

        // OnDrawFrame is called on the OpenGL thread to render the next frame.
        void OnDrawFrame();

        bool OnTouchedFirst(float x, float y, int drawMode);

        void OnTouchTranslate(float x, float y);

        void OnEditTouched(float x, float y);

        void OnTouchedFinal(int type);

        void RemoveAnchor(ArAnchor* anchor);

        void AddAnchor(ArAnchor* anchor, ArPlane* containingPlane);

        void UpdateMatrixMaps(ArAnchor* anchorKey, glm::mat4 aaMat, glm::mat4 caMat, glm::mat4 snapMat);

        void SetModelMatrices(glm::mat4& aaMat, glm::mat4& caMat, glm::mat4& snapMat, const glm::mat4& planeModel);

        void SetCameraAlignedMatrix(glm::mat4& caMat, glm::vec3 hitPos, glm::mat4& planeModel, const glm::mat4& initRotation);

        // Returns true if any planes have been detected.  Used for hiding the
        // "searching for planes" snackbar.
        bool HasDetectedPlanes() const { return plane_count_ > 0; }

        glm::mat4
        ComputeCameraAlignedMatrix(ArPlane *arPlane, glm::mat4 planeModel, glm::mat4 initRotation,
                                   glm::vec4 anchorPos,
                                   glm::vec3 cameraPos, glm::vec3 hitPos,
                                   float cameraDisplayOutRaw[]);

    private:
        ArSession *ar_session_ = nullptr;
        ArFrame *ar_frame_ = nullptr;

        PendingAnchor* pendingAnchor = nullptr;

        //SKIA VARS
        sk_sp<GrContext> grContext;
        sk_sp<skottie::Animation> fAnim;
        SkScalar fAnimT = 0;

        bool install_requested_ = false;
        int width_ = 1;
        int height_ = 1;
        int display_rotation_ = 0;

        int currentObjectRotation = 0;
        float currentValue = 0;

        std::vector<glm::vec3> begins;
        std::vector<glm::vec3> ends;

        AAssetManager *const asset_manager_;

        // The anchors at which we are drawing android models
        std::vector<ArAnchor *> tracked_obj_set_;

        // Stores the randomly-selected color each plane is drawn with
        std::unordered_map<ArPlane *, glm::vec3> plane_color_map_;

        std::unordered_map<ArAnchor *, SkMatrix44> anchor_skmat4_axis_aligned_map_;
        std::unordered_map<ArAnchor *, SkMatrix44> anchor_skmat4_camera_aligned_map_;
        std::unordered_map<ArAnchor *, SkMatrix44> anchor_skmat4_snap_aligned_map_;

        std::unordered_map<ArPlane *, std::vector<ArAnchor*>> plane_anchors_map_;
        std::unordered_map<ArAnchor *, ArPlane*> anchor_plane_map_;

        // The first plane is always rendered in white, if this is true then a plane
        // at some point has been found.
        bool first_plane_has_been_found_ = false;

        PointCloudRenderer point_cloud_renderer_;
        BackgroundRenderer background_renderer_;
        PlaneRenderer plane_renderer_;

        int32_t plane_count_ = 0;
    };
}  // namespace hello_ar

#endif  // C_ARCORE_HELLOE_AR_HELLO_AR_APPLICATION_H_
