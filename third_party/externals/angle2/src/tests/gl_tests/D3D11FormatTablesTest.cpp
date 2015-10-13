//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// D3D11FormatTablesTest:
//   Tests to validate our D3D11 support tables match hardware support.
//

#include "libANGLE/angletypes.h"
#include "libANGLE/Context.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"
#include "test_utils/angle_test_instantiate.h"
#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{

class D3D11FormatTablesTest : public ANGLETest
{

};

// This test enumerates all GL formats - for each, it queries the D3D support for
// using it as a texture, a render target, and sampling from it in the shader. It
// checks this against our speed-optimized baked tables, and validates they would
// give the same result.
// TODO(jmadill): Find out why in 9_3, some format queries return an error
TEST_P(D3D11FormatTablesTest, TestFormatSupport)
{
    ASSERT_EQ(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, GetParam().getRenderer());

    // Hack the angle!
    gl::Context *context = reinterpret_cast<gl::Context *>(getEGLWindow()->getContext());
    rx::Renderer11 *renderer = rx::GetAs<rx::Renderer11>(context->getRenderer());
    const auto &textureCaps = renderer->getRendererTextureCaps();

    ID3D11Device *device = renderer->getDevice();

    const gl::FormatSet &allFormats = gl::GetAllSizedInternalFormats();
    for (GLenum internalFormat : allFormats)
    {
        const rx::d3d11::TextureFormat &formatInfo =
            rx::d3d11::GetTextureFormatInfo(internalFormat, renderer->getRenderer11DeviceCaps());
        const auto &textureInfo = textureCaps.get(internalFormat);

        // Bits for texturing
        const gl::InternalFormat &internalFormatInfo = gl::GetInternalFormatInfo(internalFormat);

        UINT texSupportMask = D3D11_FORMAT_SUPPORT_TEXTURE2D;
        if (internalFormatInfo.depthBits == 0 && internalFormatInfo.stencilBits == 0)
        {
            texSupportMask |= D3D11_FORMAT_SUPPORT_TEXTURECUBE;
            if (GetParam().majorVersion > 2)
            {
                texSupportMask |= D3D11_FORMAT_SUPPORT_TEXTURE3D;
            }
        }

        UINT texSupport;
        bool texSuccess = SUCCEEDED(device->CheckFormatSupport(formatInfo.texFormat, &texSupport));
        bool textureable = texSuccess && ((texSupport & texSupportMask) == texSupportMask);
        EXPECT_EQ(textureable, textureInfo.texturable);

        // Bits for filtering
        UINT filterSupport;
        bool filterSuccess = SUCCEEDED(device->CheckFormatSupport(formatInfo.srvFormat, &filterSupport));
        bool filterable = filterSuccess && ((filterSupport & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE) != 0);
        EXPECT_EQ(filterable, textureInfo.filterable);

        // Bits for renderable
        bool renderable = false;
        if (internalFormatInfo.depthBits > 0 || internalFormatInfo.stencilBits > 0)
        {
            UINT depthSupport;
            bool depthSuccess = SUCCEEDED(device->CheckFormatSupport(formatInfo.dsvFormat, &depthSupport));
            renderable = depthSuccess && ((depthSupport & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL) != 0);
        }
        else
        {
            UINT rtSupport;
            bool rtSuccess = SUCCEEDED(device->CheckFormatSupport(formatInfo.rtvFormat, &rtSupport));
            renderable = rtSuccess && ((rtSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET) != 0);
        }
        EXPECT_EQ(renderable, textureInfo.renderable);

        // Multisample counts
        UINT renderSupport = false;
        bool renderSuccess = SUCCEEDED(device->CheckFormatSupport(formatInfo.renderFormat, &renderSupport));

        if (renderSuccess && ((renderSupport & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET) != 0))
        {
            EXPECT_TRUE(!textureInfo.sampleCounts.empty());
            for (unsigned int sampleCount = 1; sampleCount <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;
                 sampleCount *= 2)
            {
                UINT qualityCount = 0;
                bool sampleSuccess = SUCCEEDED(device->CheckMultisampleQualityLevels(formatInfo.renderFormat, sampleCount, &qualityCount));
                GLuint expectedCount = (!sampleSuccess || qualityCount == 0) ? 0 : 1;
                EXPECT_EQ(expectedCount, textureInfo.sampleCounts.count(sampleCount));
            }
        }
        else
        {
            EXPECT_TRUE(textureInfo.sampleCounts.empty());
        }
    }
}

ANGLE_INSTANTIATE_TEST(D3D11FormatTablesTest,
                       ES2_D3D11_FL9_3(),
                       ES2_D3D11_FL10_0(),
                       ES2_D3D11_FL10_1(),
                       ES2_D3D11_FL11_0());

} // anonymous namespace
