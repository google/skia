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

#ifndef C_ARCORE_HELLOE_AR_PLANE_RENDERER_H_
#define C_ARCORE_HELLOE_AR_PLANE_RENDERER_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>
#include <SkColor.h>

#include "arcore_c_api.h"
#include "glm.h"

namespace hello_ar {

// PlaneRenderer renders ARCore plane type.
    class PlaneRenderer {
    public:
        PlaneRenderer() = default;

        ~PlaneRenderer() = default;

        // Loads plane texture
        void InitializeGlContent(AAssetManager *asset_manager);

        // Main function called by app to draw all planes
        void Draw(ArSession* arSession, ArFrame* arFrame, GrContext* context, SkSurface* surface,
                  SkMatrix44& vpv, int& planeCount);

    private:
        // Get plane mesh info
        void UpdateForPlane(const ArSession* arSession, ArPlane* arPlane, ArTrackable* trackable);

        // Draws one plane after calling UpdateForPlane
        void DrawSinglePlane(ArSession* arSession, ArPlane* arPlane, SkCanvas* canvas, SkPaint& paint);

        // Randomly picks a color for a plane
        void SetPlaneColor(ArPlane* arPlane, ArTrackable* trackable, SkColor& outColor);

        // True if plane is behing camera
        bool CullPlane(ArSession* arSession, ArFrame* arFrame, ArPlane* arPlane);

        // Mesh data
        std::vector<SkPoint> positions_;
        std::vector<SkColor> colors_;
        std::vector<uint16_t> triangles_;

        // Stores the randomly-selected color each plane is drawn with
        std::unordered_map<ArPlane *, SkColor> plane_color_map_;
        bool firstPlaneFound = false;

        // Shader that holds plane texture
        sk_sp<SkShader> shader_;
    };
}  // namespace hello_ar

#endif  // C_ARCORE_HELLOE_AR_PLANE_RENDERER_H_
