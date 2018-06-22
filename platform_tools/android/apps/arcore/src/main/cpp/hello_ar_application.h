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

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <jni.h>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <GrContext.h>
#include <gl/GrGLTypes.h>
#include <GrBackendSurface.h>
#include <SkSurface.h>
#include <Skottie.h>
#include "arcore_c_api.h"
#include "background_renderer.h"
#include "glm.h"
#include "plane_renderer.h"
#include "point_cloud_renderer.h"
#include "util.h"
#include "pending_anchor.h"
#include "anchor_wrapper.h"

namespace hello_ar {

// HelloArApplication handles all application logics.
    class HelloArApplication {
    public:
        // Constructor and deconstructor.
        HelloArApplication() = default;

        HelloArApplication(AAssetManager *asset_manager);

        ~HelloArApplication();

        // Returns true if any planes have been detected.  Used for hiding the
        // "searching for planes" snackbar.
        bool HasDetectedPlanes() const { return plane_count_ > 0; }

        /* Application life cycle functions */
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

        /* OnTouch functions */
        // Handles every touch: every touch gestures goes through here first to determine if
        // we are adding a new anchor or if we are editing an old one
        bool OnTouchedFirst(float x, float y, int drawMode);

        // Handles translating a pre-existing anchor
        void OnTouchTranslate(float x, float y);

        // Handles scaling a pre-existing anchor
        void OnTouchScale(float scale);

        // Handles scaling a pre-existing anchor
        void OnTouchRotate(float angle);

        // Handles adding a new anchor & computing its model matrices
        void OnTouchedFinal(int type);

    private:
        /* Model matrix computations */
        // Updates model matrix maps with new anchor-matrix key-value pairs
        void AddModelMatrices(AnchorWrapper* anchorWrapperKey, glm::mat4 aaMat, glm::mat4 caMat, glm::mat4 snapMat);

        // Updates a pre-existing anchor wrapper key in the model matrix maps
        void UpdateModelMatrices(AnchorWrapper* anchorWrapperKey, glm::mat4 aaMat, glm::mat4 caMat, glm::mat4 snapMat);

        // Handles creating all model matrices for a given anchor
        void SetModelMatrices(glm::mat4& aaMat, glm::mat4& caMat, glm::mat4& snapMat, util::MatrixComputationInfo* info);

        // Called by SetModelMatrices to set the camera-aligned matrix
        void SetCameraAlignedMatrix(glm::mat4& caMat, glm::vec3 hitPos, const util::MatrixComputationInfo* info);

        // Return a MatrixComputationInfo given the finalized pending anchor
        util::MatrixComputationInfo* GetMatrixComputationInfo(ArPlane* containingPlane);

        /* Hit Test Helpers */
        // Checks if the hit location corresponds to a neighboring anchor on the same hit plane
        bool CheckNeighborAnchors(ArPlane* containingPlane, const glm::vec4& hitPosition);

        // Traverses a list of hitResults at an index and a given pose associated with the result.
        // @return: true if traversal must continue, false otherwise.
        // If traversal must end/break, then pendingAnchor edit mode has been updated to true or false
        bool TraverseHitResultList(ArHitResultList* hitResultList, int index,
                                   ArPose** outHitPose, ArHitResult** outHitResult, ArPlane** outHitPlane);

        bool TraverseHitResultListOnTranslate(ArHitResultList* hitResultList, int index,
                                                ArPose** outHitPose, ArHitResult** outHitResult,
                                                ArPlane** outHitPlane, glm::mat4& outTranslate);

        /* Anchor management */
        // Handles all operations associated with deleting an anchor wrapper (memory & state management)
        void RemoveAnchorWrapper(AnchorWrapper* anchorW);

        // Handles all operations associated with adding an anchor wrapper (memory & state management)
        void AddAnchorWrapper(AnchorWrapper* anchorW, ArPlane* containingPlane);

        // Computes the state of the new anchor wrapper after a translation happens on edit
        void ComputeAnchorWrapperPostTranslate(AnchorWrapper* oldWrapper, AnchorWrapper* newWrapper, ArAnchor* newAnchor);

        // Checks if the pending anchor is valid (not null & is stored in anchor map) on any edit call
        bool CheckPendingAnchorOnEdit();

        // Resets pending anchor to nullptr (releases memory)
        void ReleasePendingAnchor();

        ArSession *ar_session_ = nullptr;
        ArFrame *ar_frame_ = nullptr;
        AAssetManager *const asset_manager_;
        PendingAnchor* pendingAnchor = nullptr;

        //Skia state
        sk_sp<GrContext> grContext;

        // The anchors at which we are drawing android models
        std::vector<AnchorWrapper *> tracked_obj_set_;

        // Stores the randomly-selected color each plane is drawn with
        std::unordered_map<ArPlane *, glm::vec3> plane_color_map_;

        // Maps from ArAnchor to model matrix
        std::unordered_map<AnchorWrapper*, SkMatrix44> anchor_skmat4_axis_aligned_map_;
        std::unordered_map<AnchorWrapper*, SkMatrix44> anchor_skmat4_camera_aligned_map_;
        std::unordered_map<AnchorWrapper*, SkMatrix44> anchor_skmat4_snap_aligned_map_;

        // Maps from ArPlane to set of contained ArAnchors, and from ArAnchor to containing ArPlane
        std::unordered_map<ArPlane *, std::vector<AnchorWrapper*>> plane_anchors_map_;
        std::unordered_map<AnchorWrapper *, ArPlane*> anchor_plane_map_;

        PointCloudRenderer point_cloud_renderer_;
        BackgroundRenderer background_renderer_;
        PlaneRenderer plane_renderer_;

        glm::vec3 kek;

        bool install_requested_ = false;
        int width_ = 1;
        int height_ = 1;
        int display_rotation_ = 0;
        // The first plane is always rendered in white, if this is true then a plane
        // at some point has been found.
        bool first_plane_has_been_found_ = false;
        int32_t plane_count_ = 0;

        int currentObjectRotation = 0;
        float currentValue = 0;
        std::vector<glm::vec3> begins;
        std::vector<glm::vec3> ends;
    };
}  // namespace hello_ar

#endif  // C_ARCORE_HELLOE_AR_HELLO_AR_APPLICATION_H_
