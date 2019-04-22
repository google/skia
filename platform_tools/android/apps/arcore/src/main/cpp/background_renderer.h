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

#ifndef C_ARCORE_HELLO_AR_BACKGROUND_RENDERER_H_
#define C_ARCORE_HELLO_AR_BACKGROUND_RENDERER_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <cstdlib>

#include "arcore_c_api.h"
#include "platform_tools/android/apps/arcore/src/main/cpp/util.h"

namespace hello_ar {

// This class renders the passthrough camera image into the OpenGL frame.
    class BackgroundRenderer {
    public:
        BackgroundRenderer() = default;

        ~BackgroundRenderer() = default;

        // Sets up OpenGL state.  Must be called on the OpenGL thread and before any
        // other methods below.
        void InitializeGlContent();

        // Draws the background image.  This methods must be called for every ArFrame
        // returned by ArSession_update() to catch display geometry change events.
        void Draw(const ArSession *session, const ArFrame *frame);

        // Returns the generated texture name for the GL_TEXTURE_EXTERNAL_OES target.
        GLuint GetTextureId() const;

    private:
        static constexpr int kNumVertices = 4;

        GLuint shader_program_;
        GLuint texture_id_;

        GLuint attribute_vertices_;
        GLuint attribute_uvs_;
        GLuint uniform_texture_;

        float transformed_uvs_[kNumVertices * 2];
        bool uvs_initialized_ = false;
    };
}  // namespace hello_ar
#endif  // C_ARCORE_HELLO_AR_BACKGROUND_RENDERER_H_
