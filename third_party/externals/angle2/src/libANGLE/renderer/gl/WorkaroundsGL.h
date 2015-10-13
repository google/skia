//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WorkaroundsGL.h: Workarounds for GL driver bugs and other issues.

#ifndef LIBANGLE_RENDERER_GL_WORKAROUNDSGL_H_
#define LIBANGLE_RENDERER_GL_WORKAROUNDSGL_H_

namespace rx
{

struct WorkaroundsGL
{
    WorkaroundsGL()
        : avoid1BitAlphaTextureFormats(false),
          rgba4IsNotSupportedForColorRendering(false),
          doesSRGBClearsOnLinearFramebufferAttachments(false)
    {
    }

    // When writing a float to a normalized integer framebuffer, desktop OpenGL is allowed to write
    // one of the two closest normalized integer representations (although round to nearest is
    // preferred) (see section 2.3.5.2 of the GL 4.5 core specification). OpenGL ES requires that
    // round-to-nearest is used (see "Conversion from Floating-Point to Framebuffer Fixed-Point" in
    // section 2.1.2 of the OpenGL ES 2.0.25 spec).  This issue only shows up on Intel and AMD
    // drivers on framebuffer formats that have 1-bit alpha, work around this by using higher
    // precision formats instead.
    bool avoid1BitAlphaTextureFormats;

    // On some older Intel drivers, GL_RGBA4 is not color renderable, glCheckFramebufferStatus
    // returns GL_FRAMEBUFFER_UNSUPPORTED. Work around this by using a known color-renderable
    // format.
    bool rgba4IsNotSupportedForColorRendering;

    // When clearing a framebuffer on Intel or AMD drivers, when GL_FRAMEBUFFER_SRGB is enabled, the
    // driver clears to the linearized clear color despite the framebuffer not supporting SRGB
    // blending.  It only seems to do this when the framebuffer has only linear attachments, mixed
    // attachments appear to get the correct clear color.
    bool doesSRGBClearsOnLinearFramebufferAttachments;
};
}

#endif  // LIBANGLE_RENDERER_GL_WORKAROUNDSGL_H_
