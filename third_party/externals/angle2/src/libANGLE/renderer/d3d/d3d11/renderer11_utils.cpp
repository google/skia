//
// Copyright (c) 2012-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// renderer11_utils.cpp: Conversion functions and other utility routines
// specific to the D3D11 renderer.

#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

#include <algorithm>

#include "common/debug.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/Program.h"
#include "libANGLE/renderer/d3d/d3d11/dxgi_support_table.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"
#include "libANGLE/renderer/d3d/FramebufferD3D.h"
#include "libANGLE/renderer/d3d/WorkaroundsD3D.h"

namespace rx
{

namespace gl_d3d11
{

D3D11_BLEND ConvertBlendFunc(GLenum glBlend, bool isAlpha)
{
    D3D11_BLEND d3dBlend = D3D11_BLEND_ZERO;

    switch (glBlend)
    {
      case GL_ZERO:                     d3dBlend = D3D11_BLEND_ZERO;                break;
      case GL_ONE:                      d3dBlend = D3D11_BLEND_ONE;                 break;
      case GL_SRC_COLOR:                d3dBlend = (isAlpha ? D3D11_BLEND_SRC_ALPHA : D3D11_BLEND_SRC_COLOR);           break;
      case GL_ONE_MINUS_SRC_COLOR:      d3dBlend = (isAlpha ? D3D11_BLEND_INV_SRC_ALPHA : D3D11_BLEND_INV_SRC_COLOR);   break;
      case GL_DST_COLOR:                d3dBlend = (isAlpha ? D3D11_BLEND_DEST_ALPHA : D3D11_BLEND_DEST_COLOR);         break;
      case GL_ONE_MINUS_DST_COLOR:      d3dBlend = (isAlpha ? D3D11_BLEND_INV_DEST_ALPHA : D3D11_BLEND_INV_DEST_COLOR); break;
      case GL_SRC_ALPHA:                d3dBlend = D3D11_BLEND_SRC_ALPHA;           break;
      case GL_ONE_MINUS_SRC_ALPHA:      d3dBlend = D3D11_BLEND_INV_SRC_ALPHA;       break;
      case GL_DST_ALPHA:                d3dBlend = D3D11_BLEND_DEST_ALPHA;          break;
      case GL_ONE_MINUS_DST_ALPHA:      d3dBlend = D3D11_BLEND_INV_DEST_ALPHA;      break;
      case GL_CONSTANT_COLOR:           d3dBlend = D3D11_BLEND_BLEND_FACTOR;        break;
      case GL_ONE_MINUS_CONSTANT_COLOR: d3dBlend = D3D11_BLEND_INV_BLEND_FACTOR;    break;
      case GL_CONSTANT_ALPHA:           d3dBlend = D3D11_BLEND_BLEND_FACTOR;        break;
      case GL_ONE_MINUS_CONSTANT_ALPHA: d3dBlend = D3D11_BLEND_INV_BLEND_FACTOR;    break;
      case GL_SRC_ALPHA_SATURATE:       d3dBlend = D3D11_BLEND_SRC_ALPHA_SAT;       break;
      default: UNREACHABLE();
    }

    return d3dBlend;
}

D3D11_BLEND_OP ConvertBlendOp(GLenum glBlendOp)
{
    D3D11_BLEND_OP d3dBlendOp = D3D11_BLEND_OP_ADD;

    switch (glBlendOp)
    {
      case GL_FUNC_ADD:              d3dBlendOp = D3D11_BLEND_OP_ADD;           break;
      case GL_FUNC_SUBTRACT:         d3dBlendOp = D3D11_BLEND_OP_SUBTRACT;      break;
      case GL_FUNC_REVERSE_SUBTRACT: d3dBlendOp = D3D11_BLEND_OP_REV_SUBTRACT;  break;
      case GL_MIN:                   d3dBlendOp = D3D11_BLEND_OP_MIN;           break;
      case GL_MAX:                   d3dBlendOp = D3D11_BLEND_OP_MAX;           break;
      default: UNREACHABLE();
    }

    return d3dBlendOp;
}

UINT8 ConvertColorMask(bool red, bool green, bool blue, bool alpha)
{
    UINT8 mask = 0;
    if (red)
    {
        mask |= D3D11_COLOR_WRITE_ENABLE_RED;
    }
    if (green)
    {
        mask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
    }
    if (blue)
    {
        mask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
    }
    if (alpha)
    {
        mask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
    }
    return mask;
}

D3D11_CULL_MODE ConvertCullMode(bool cullEnabled, GLenum cullMode)
{
    D3D11_CULL_MODE cull = D3D11_CULL_NONE;

    if (cullEnabled)
    {
        switch (cullMode)
        {
          case GL_FRONT:            cull = D3D11_CULL_FRONT;    break;
          case GL_BACK:             cull = D3D11_CULL_BACK;     break;
          case GL_FRONT_AND_BACK:   cull = D3D11_CULL_NONE;     break;
          default: UNREACHABLE();
        }
    }
    else
    {
        cull = D3D11_CULL_NONE;
    }

    return cull;
}

D3D11_COMPARISON_FUNC ConvertComparison(GLenum comparison)
{
    D3D11_COMPARISON_FUNC d3dComp = D3D11_COMPARISON_NEVER;
    switch (comparison)
    {
      case GL_NEVER:    d3dComp = D3D11_COMPARISON_NEVER;           break;
      case GL_ALWAYS:   d3dComp = D3D11_COMPARISON_ALWAYS;          break;
      case GL_LESS:     d3dComp = D3D11_COMPARISON_LESS;            break;
      case GL_LEQUAL:   d3dComp = D3D11_COMPARISON_LESS_EQUAL;      break;
      case GL_EQUAL:    d3dComp = D3D11_COMPARISON_EQUAL;           break;
      case GL_GREATER:  d3dComp = D3D11_COMPARISON_GREATER;         break;
      case GL_GEQUAL:   d3dComp = D3D11_COMPARISON_GREATER_EQUAL;   break;
      case GL_NOTEQUAL: d3dComp = D3D11_COMPARISON_NOT_EQUAL;       break;
      default: UNREACHABLE();
    }

    return d3dComp;
}

D3D11_DEPTH_WRITE_MASK ConvertDepthMask(bool depthWriteEnabled)
{
    return depthWriteEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
}

UINT8 ConvertStencilMask(GLuint stencilmask)
{
    return static_cast<UINT8>(stencilmask);
}

D3D11_STENCIL_OP ConvertStencilOp(GLenum stencilOp)
{
    D3D11_STENCIL_OP d3dStencilOp = D3D11_STENCIL_OP_KEEP;

    switch (stencilOp)
    {
      case GL_ZERO:      d3dStencilOp = D3D11_STENCIL_OP_ZERO;      break;
      case GL_KEEP:      d3dStencilOp = D3D11_STENCIL_OP_KEEP;      break;
      case GL_REPLACE:   d3dStencilOp = D3D11_STENCIL_OP_REPLACE;   break;
      case GL_INCR:      d3dStencilOp = D3D11_STENCIL_OP_INCR_SAT;  break;
      case GL_DECR:      d3dStencilOp = D3D11_STENCIL_OP_DECR_SAT;  break;
      case GL_INVERT:    d3dStencilOp = D3D11_STENCIL_OP_INVERT;    break;
      case GL_INCR_WRAP: d3dStencilOp = D3D11_STENCIL_OP_INCR;      break;
      case GL_DECR_WRAP: d3dStencilOp = D3D11_STENCIL_OP_DECR;      break;
      default: UNREACHABLE();
    }

    return d3dStencilOp;
}

D3D11_FILTER ConvertFilter(GLenum minFilter, GLenum magFilter, float maxAnisotropy, GLenum comparisonMode)
{
    bool comparison = comparisonMode != GL_NONE;

    if (maxAnisotropy > 1.0f)
    {
        return D3D11_ENCODE_ANISOTROPIC_FILTER(static_cast<D3D11_COMPARISON_FUNC>(comparison));
    }
    else
    {
        D3D11_FILTER_TYPE dxMin = D3D11_FILTER_TYPE_POINT;
        D3D11_FILTER_TYPE dxMip = D3D11_FILTER_TYPE_POINT;
        switch (minFilter)
        {
          case GL_NEAREST:                dxMin = D3D11_FILTER_TYPE_POINT;  dxMip = D3D11_FILTER_TYPE_POINT;  break;
          case GL_LINEAR:                 dxMin = D3D11_FILTER_TYPE_LINEAR; dxMip = D3D11_FILTER_TYPE_POINT;  break;
          case GL_NEAREST_MIPMAP_NEAREST: dxMin = D3D11_FILTER_TYPE_POINT;  dxMip = D3D11_FILTER_TYPE_POINT;  break;
          case GL_LINEAR_MIPMAP_NEAREST:  dxMin = D3D11_FILTER_TYPE_LINEAR; dxMip = D3D11_FILTER_TYPE_POINT;  break;
          case GL_NEAREST_MIPMAP_LINEAR:  dxMin = D3D11_FILTER_TYPE_POINT;  dxMip = D3D11_FILTER_TYPE_LINEAR; break;
          case GL_LINEAR_MIPMAP_LINEAR:   dxMin = D3D11_FILTER_TYPE_LINEAR; dxMip = D3D11_FILTER_TYPE_LINEAR; break;
          default:                        UNREACHABLE();
        }

        D3D11_FILTER_TYPE dxMag = D3D11_FILTER_TYPE_POINT;
        switch (magFilter)
        {
          case GL_NEAREST: dxMag = D3D11_FILTER_TYPE_POINT;  break;
          case GL_LINEAR:  dxMag = D3D11_FILTER_TYPE_LINEAR; break;
          default:         UNREACHABLE();
        }

        return D3D11_ENCODE_BASIC_FILTER(dxMin, dxMag, dxMip, static_cast<D3D11_COMPARISON_FUNC>(comparison));
    }
}

D3D11_TEXTURE_ADDRESS_MODE ConvertTextureWrap(GLenum wrap)
{
    switch (wrap)
    {
      case GL_REPEAT:          return D3D11_TEXTURE_ADDRESS_WRAP;
      case GL_CLAMP_TO_EDGE:   return D3D11_TEXTURE_ADDRESS_CLAMP;
      case GL_MIRRORED_REPEAT: return D3D11_TEXTURE_ADDRESS_MIRROR;
      default:                 UNREACHABLE();
    }

    return D3D11_TEXTURE_ADDRESS_WRAP;
}

D3D11_QUERY ConvertQueryType(GLenum queryType)
{
    switch (queryType)
    {
      case GL_ANY_SAMPLES_PASSED_EXT:
      case GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT:   return D3D11_QUERY_OCCLUSION;
      case GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN: return D3D11_QUERY_SO_STATISTICS;
      default: UNREACHABLE();                        return D3D11_QUERY_EVENT;
    }
}

}


namespace d3d11_gl
{

namespace
{

// Helper functor for querying DXGI support. Saves passing the parameters repeatedly.
class DXGISupportHelper : angle::NonCopyable
{
  public:
    DXGISupportHelper(ID3D11Device *device, D3D_FEATURE_LEVEL featureLevel)
        : mDevice(device),
          mFeatureLevel(featureLevel)
    {
    }

    bool query(DXGI_FORMAT dxgiFormat, UINT supportMask)
    {
        if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
            return false;

        auto dxgiSupport = d3d11::GetDXGISupport(dxgiFormat, mFeatureLevel);

        UINT supportedBits = dxgiSupport.alwaysSupportedFlags;

        if ((dxgiSupport.optionallySupportedFlags & supportMask) != 0)
        {
            UINT formatSupport;
            if (SUCCEEDED(mDevice->CheckFormatSupport(dxgiFormat, &formatSupport)))
            {
                supportedBits |= (formatSupport & supportMask);
            }
            else
            {
                // TODO(jmadill): find out why we fail this call sometimes in FL9_3
                // ERR("Error checking format support for format 0x%x", dxgiFormat);
            }
        }

        return ((supportedBits & supportMask) == supportMask);
    }

  private:
    ID3D11Device *mDevice;
    D3D_FEATURE_LEVEL mFeatureLevel;
};

} // anonymous namespace

unsigned int GetReservedVertexUniformVectors(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return 0;

        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 2;  // dx_ViewAdjust and dx_ViewCoords

        default:
            UNREACHABLE();
            return 0;
    }
}

unsigned int GetReservedFragmentUniformVectors(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return 0;

        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 2;

        default:
            UNREACHABLE();
            return 0;
    }
}

GLint GetMaximumClientVersion(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0:
      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return 3;

      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return 2;

      default: UNREACHABLE();      return 0;
    }
}

static gl::TextureCaps GenerateTextureFormatCaps(GLint maxClientVersion, GLenum internalFormat, ID3D11Device *device, const Renderer11DeviceCaps &renderer11DeviceCaps)
{
    gl::TextureCaps textureCaps;

    DXGISupportHelper support(device, renderer11DeviceCaps.featureLevel);
    const d3d11::TextureFormat &formatInfo =
        d3d11::GetTextureFormatInfo(internalFormat, renderer11DeviceCaps);

    const gl::InternalFormat &internalFormatInfo = gl::GetInternalFormatInfo(internalFormat);

    UINT texSupportMask = D3D11_FORMAT_SUPPORT_TEXTURE2D;
    if (internalFormatInfo.depthBits == 0 && internalFormatInfo.stencilBits == 0)
    {
        texSupportMask |= D3D11_FORMAT_SUPPORT_TEXTURECUBE;
        if (maxClientVersion > 2)
        {
            texSupportMask |= D3D11_FORMAT_SUPPORT_TEXTURE3D;
        }
    }

    textureCaps.texturable = support.query(formatInfo.texFormat, texSupportMask);
    textureCaps.filterable = support.query(formatInfo.srvFormat, D3D11_FORMAT_SUPPORT_SHADER_SAMPLE);
    textureCaps.renderable = (support.query(formatInfo.rtvFormat, D3D11_FORMAT_SUPPORT_RENDER_TARGET)) ||
                             (support.query(formatInfo.dsvFormat, D3D11_FORMAT_SUPPORT_DEPTH_STENCIL));

    if (support.query(formatInfo.renderFormat, D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET))
    {
        // Assume 1x
        textureCaps.sampleCounts.insert(1);

        for (unsigned int sampleCount = 2; sampleCount <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;
             sampleCount *= 2)
        {
            UINT qualityCount = 0;
            if (SUCCEEDED(device->CheckMultisampleQualityLevels(formatInfo.renderFormat, sampleCount, &qualityCount)))
            {
                // Assume we always support lower sample counts
                if (qualityCount == 0)
                {
                    break;
                }
                textureCaps.sampleCounts.insert(sampleCount);
            }
        }
    }

    return textureCaps;
}

static bool GetNPOTTextureSupport(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0:
      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return true;

        // From http://msdn.microsoft.com/en-us/library/windows/desktop/ff476876.aspx
      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return false;

      default: UNREACHABLE();      return false;
    }
}

static float GetMaximumAnisotropy(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_MAX_MAXANISOTROPY;

      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return D3D10_MAX_MAXANISOTROPY;

        // From http://msdn.microsoft.com/en-us/library/windows/desktop/ff476876.aspx
      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:  return 16;

      case D3D_FEATURE_LEVEL_9_1:  return D3D_FL9_1_DEFAULT_MAX_ANISOTROPY;

      default: UNREACHABLE();      return 0;
    }
}

static bool GetOcclusionQuerySupport(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0:
      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return true;

        // From http://msdn.microsoft.com/en-us/library/windows/desktop/ff476150.aspx ID3D11Device::CreateQuery
      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:  return true;
      case D3D_FEATURE_LEVEL_9_1:  return false;

      default: UNREACHABLE();      return false;
    }
}

static bool GetEventQuerySupport(D3D_FEATURE_LEVEL featureLevel)
{
    // From http://msdn.microsoft.com/en-us/library/windows/desktop/ff476150.aspx ID3D11Device::CreateQuery

    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0:
      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0:
      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return true;

      default: UNREACHABLE();      return false;
    }
}

static bool GetInstancingSupport(D3D_FEATURE_LEVEL featureLevel)
{
    // From http://msdn.microsoft.com/en-us/library/windows/desktop/ff476150.aspx ID3D11Device::CreateInputLayout

    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0:
      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return true;

      // Feature Level 9_3 supports instancing, but slot 0 in the input layout must not be instanced.
      // D3D9 has a similar restriction, where stream 0 must not be instanced.
      // This restriction can be worked around by remapping any non-instanced slot to slot 0.
      // This works because HLSL uses shader semantics to match the vertex inputs to the elements in the input layout, rather than the slots.
      // Note that we only support instancing via ANGLE_instanced_array on 9_3, since 9_3 doesn't support OpenGL ES 3.0
      case D3D_FEATURE_LEVEL_9_3:  return true;

      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return false;

      default: UNREACHABLE();      return false;
    }
}

static bool GetFramebufferMultisampleSupport(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0:
      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return true;

      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return false;

      default: UNREACHABLE();      return false;
    }
}

static bool GetFramebufferBlitSupport(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0:
      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return true;

      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return false;

      default: UNREACHABLE();      return false;
    }
}

static bool GetDerivativeInstructionSupport(D3D_FEATURE_LEVEL featureLevel)
{
    // http://msdn.microsoft.com/en-us/library/windows/desktop/bb509588.aspx states that shader model
    // ps_2_x is required for the ddx (and other derivative functions).

    // http://msdn.microsoft.com/en-us/library/windows/desktop/ff476876.aspx states that feature level
    // 9.3 supports shader model ps_2_x.

    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0:
      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0:
      case D3D_FEATURE_LEVEL_9_3:  return true;
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return false;

      default: UNREACHABLE();      return false;
    }
}

static bool GetShaderTextureLODSupport(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0:
      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return true;

      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return false;

      default: UNREACHABLE();      return false;
    }
}

static size_t GetMaximumSimultaneousRenderTargets(D3D_FEATURE_LEVEL featureLevel)
{
    // From http://msdn.microsoft.com/en-us/library/windows/desktop/ff476150.aspx ID3D11Device::CreateInputLayout

    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;

      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT;

      case D3D_FEATURE_LEVEL_9_3:  return D3D_FL9_3_SIMULTANEOUS_RENDER_TARGET_COUNT;
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return D3D_FL9_1_SIMULTANEOUS_RENDER_TARGET_COUNT;

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetMaximum2DTextureSize(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;

      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;

      case D3D_FEATURE_LEVEL_9_3:  return D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION;
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION;

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetMaximumCubeMapTextureSize(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_REQ_TEXTURECUBE_DIMENSION;

      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return D3D10_REQ_TEXTURECUBE_DIMENSION;

      case D3D_FEATURE_LEVEL_9_3:  return D3D_FL9_3_REQ_TEXTURECUBE_DIMENSION;
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return D3D_FL9_1_REQ_TEXTURECUBE_DIMENSION;

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetMaximum2DTextureArraySize(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;

      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return D3D10_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;

      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return 0;

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetMaximum3DTextureSize(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;

      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return D3D10_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;

      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return D3D_FL9_1_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetMaximumViewportSize(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_VIEWPORT_BOUNDS_MAX;

      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return D3D10_VIEWPORT_BOUNDS_MAX;

      // No constants for D3D11 Feature Level 9 viewport size limits, use the maximum texture sizes
      case D3D_FEATURE_LEVEL_9_3:  return D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION;
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION;

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetMaximumDrawIndexedIndexCount(D3D_FEATURE_LEVEL featureLevel)
{
    // D3D11 allows up to 2^32 elements, but we report max signed int for convenience since that's what's
    // returned from glGetInteger
    static_assert(D3D11_REQ_DRAWINDEXED_INDEX_COUNT_2_TO_EXP == 32, "Unexpected D3D11 constant value.");
    static_assert(D3D10_REQ_DRAWINDEXED_INDEX_COUNT_2_TO_EXP == 32, "Unexpected D3D11 constant value.");

    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0:
      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return std::numeric_limits<GLint>::max();

      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:  return D3D_FL9_2_IA_PRIMITIVE_MAX_COUNT;
      case D3D_FEATURE_LEVEL_9_1:  return D3D_FL9_1_IA_PRIMITIVE_MAX_COUNT;

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetMaximumDrawVertexCount(D3D_FEATURE_LEVEL featureLevel)
{
    // D3D11 allows up to 2^32 elements, but we report max signed int for convenience since that's what's
    // returned from glGetInteger
    static_assert(D3D11_REQ_DRAW_VERTEX_COUNT_2_TO_EXP == 32, "Unexpected D3D11 constant value.");
    static_assert(D3D10_REQ_DRAW_VERTEX_COUNT_2_TO_EXP == 32, "Unexpected D3D11 constant value.");

    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0:
      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return std::numeric_limits<GLint>::max();

      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:  return D3D_FL9_2_IA_PRIMITIVE_MAX_COUNT;
      case D3D_FEATURE_LEVEL_9_1:  return D3D_FL9_1_IA_PRIMITIVE_MAX_COUNT;

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetMaximumVertexInputSlots(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_STANDARD_VERTEX_ELEMENT_COUNT;

      case D3D_FEATURE_LEVEL_10_1: return D3D10_1_STANDARD_VERTEX_ELEMENT_COUNT;
      case D3D_FEATURE_LEVEL_10_0: return D3D10_STANDARD_VERTEX_ELEMENT_COUNT;

      // From http://http://msdn.microsoft.com/en-us/library/windows/desktop/ff476876.aspx "Max Input Slots"
      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return 16;

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetMaximumVertexUniformVectors(D3D_FEATURE_LEVEL featureLevel)
{
    // TODO(geofflang): Remove hard-coded limit once the gl-uniform-arrays test can pass
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return 1024; // D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT;

      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return 1024; // D3D10_REQ_CONSTANT_BUFFER_ELEMENT_COUNT;

      // From http://msdn.microsoft.com/en-us/library/windows/desktop/ff476149.aspx ID3D11DeviceContext::VSSetConstantBuffers
      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:
          return 255 - d3d11_gl::GetReservedVertexUniformVectors(featureLevel);

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetReservedVertexUniformBuffers()
{
    // Reserve one buffer for the application uniforms, and one for driver uniforms
    return 2;
}

static size_t GetMaximumVertexUniformBlocks(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - GetReservedVertexUniformBuffers();

      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return D3D10_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - GetReservedVertexUniformBuffers();

      // Uniform blocks not supported on D3D11 Feature Level 9
      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return 0;

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetReservedVertexOutputVectors(D3D_FEATURE_LEVEL featureLevel)
{
    // According to The OpenGL ES Shading Language specifications 
    // (Language Version 1.00 section 10.16, Language Version 3.10 section 12.21)
    // built-in special variables (e.g. gl_FragCoord, or gl_PointCoord)
    // which are statically used in the shader should be included in the variable packing algorithm.
    // Therefore, we should not reserve output vectors for them.

    switch (featureLevel)
    {
      // We must reserve one output vector for dx_Position.
      // We also reserve one for gl_Position, which we unconditionally output on Feature Levels 10_0+,
      // even if it's unused in the shader (e.g. for transform feedback). TODO: This could be improved.
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0:
      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return 2;

      // Just reserve dx_Position on Feature Level 9, since we don't ever need to output gl_Position.
      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return 1;

      default: UNREACHABLE();      return 0;
    }

    return 1;
}

static size_t GetMaximumVertexOutputVectors(D3D_FEATURE_LEVEL featureLevel)
{
    static_assert(gl::IMPLEMENTATION_MAX_VARYING_VECTORS == D3D11_VS_OUTPUT_REGISTER_COUNT, "Unexpected D3D11 constant value.");

    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_VS_OUTPUT_REGISTER_COUNT - GetReservedVertexOutputVectors(featureLevel);

      case D3D_FEATURE_LEVEL_10_1: return D3D10_1_VS_OUTPUT_REGISTER_COUNT - GetReservedVertexOutputVectors(featureLevel);
      case D3D_FEATURE_LEVEL_10_0: return D3D10_VS_OUTPUT_REGISTER_COUNT - GetReservedVertexOutputVectors(featureLevel);

      // Use Shader Model 2.X limits
      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return 8 - GetReservedVertexOutputVectors(featureLevel);

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetMaximumVertexTextureUnits(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;

      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT;

      // Vertex textures not supported on D3D11 Feature Level 9 according to
      // http://msdn.microsoft.com/en-us/library/windows/desktop/ff476149.aspx
      // ID3D11DeviceContext::VSSetSamplers and ID3D11DeviceContext::VSSetShaderResources
      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return 0;

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetMaximumPixelUniformVectors(D3D_FEATURE_LEVEL featureLevel)
{
    // TODO(geofflang): Remove hard-coded limit once the gl-uniform-arrays test can pass
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return 1024; // D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT;

      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return 1024; // D3D10_REQ_CONSTANT_BUFFER_ELEMENT_COUNT;

      // From http://msdn.microsoft.com/en-us/library/windows/desktop/ff476149.aspx ID3D11DeviceContext::PSSetConstantBuffers
      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:
          return 32 - d3d11_gl::GetReservedFragmentUniformVectors(featureLevel);

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetReservedPixelUniformBuffers()
{
    // Reserve one buffer for the application uniforms, and one for driver uniforms
    return 2;
}

static size_t GetMaximumPixelUniformBlocks(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - GetReservedPixelUniformBuffers();

      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return D3D10_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - GetReservedPixelUniformBuffers();

      // Uniform blocks not supported on D3D11 Feature Level 9
      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return 0;

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetMaximumPixelInputVectors(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_PS_INPUT_REGISTER_COUNT - GetReservedVertexOutputVectors(featureLevel);

      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return D3D10_PS_INPUT_REGISTER_COUNT - GetReservedVertexOutputVectors(featureLevel);

      // Use Shader Model 2.X limits
      case D3D_FEATURE_LEVEL_9_3:  return 8 - GetReservedVertexOutputVectors(featureLevel);
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return 8 - GetReservedVertexOutputVectors(featureLevel);

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetMaximumPixelTextureUnits(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;

      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT;

      // http://msdn.microsoft.com/en-us/library/windows/desktop/ff476149.aspx ID3D11DeviceContext::PSSetShaderResources
      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return 16;

      default: UNREACHABLE();      return 0;
    }
}

static int GetMinimumTexelOffset(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_COMMONSHADER_TEXEL_OFFSET_MAX_NEGATIVE;

      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return D3D10_COMMONSHADER_TEXEL_OFFSET_MAX_NEGATIVE;

      // Sampling functions with offsets are not available below shader model 4.0.
      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return 0;

      default: UNREACHABLE();      return 0;
    }
}

static int GetMaximumTexelOffset(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_COMMONSHADER_TEXEL_OFFSET_MAX_POSITIVE;
      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return D3D11_COMMONSHADER_TEXEL_OFFSET_MAX_POSITIVE;

      // Sampling functions with offsets are not available below shader model 4.0.
      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return 0;

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetMaximumConstantBufferSize(D3D_FEATURE_LEVEL featureLevel)
{
    // Returns a size_t despite the limit being a GLuint64 because size_t is the maximum size of
    // any buffer that could be allocated.

    const size_t bytesPerComponent = 4 * sizeof(float);

    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * bytesPerComponent;

      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return D3D10_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * bytesPerComponent;

      // Limits from http://msdn.microsoft.com/en-us/library/windows/desktop/ff476501.aspx remarks section
      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return 4096 * bytesPerComponent;

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetMaximumStreamOutputBuffers(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return D3D11_SO_BUFFER_SLOT_COUNT;

      case D3D_FEATURE_LEVEL_10_1: return D3D10_1_SO_BUFFER_SLOT_COUNT;
      case D3D_FEATURE_LEVEL_10_0: return D3D10_SO_BUFFER_SLOT_COUNT;

      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return 0;

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetMaximumStreamOutputInterleavedComponents(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0:

      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return GetMaximumVertexOutputVectors(featureLevel) * 4;

      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return 0;

      default: UNREACHABLE();      return 0;
    }
}

static size_t GetMaximumStreamOutputSeparateComponents(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
      case D3D_FEATURE_LEVEL_11_1:
      case D3D_FEATURE_LEVEL_11_0: return GetMaximumStreamOutputInterleavedComponents(featureLevel) /
                                          GetMaximumStreamOutputBuffers(featureLevel);


      // D3D 10 and 10.1 only allow one output per output slot if an output slot other than zero is used.
      case D3D_FEATURE_LEVEL_10_1:
      case D3D_FEATURE_LEVEL_10_0: return 4;

      case D3D_FEATURE_LEVEL_9_3:
      case D3D_FEATURE_LEVEL_9_2:
      case D3D_FEATURE_LEVEL_9_1:  return 0;

      default: UNREACHABLE();      return 0;
    }
}

void GenerateCaps(ID3D11Device *device, ID3D11DeviceContext *deviceContext, const Renderer11DeviceCaps &renderer11DeviceCaps, gl::Caps *caps,
                  gl::TextureCapsMap *textureCapsMap, gl::Extensions *extensions, gl::Limitations *limitations)
{
    GLuint maxSamples = 0;
    D3D_FEATURE_LEVEL featureLevel = renderer11DeviceCaps.featureLevel;
    const gl::FormatSet &allFormats = gl::GetAllSizedInternalFormats();
    for (gl::FormatSet::const_iterator internalFormat = allFormats.begin(); internalFormat != allFormats.end(); ++internalFormat)
    {
        gl::TextureCaps textureCaps = GenerateTextureFormatCaps(GetMaximumClientVersion(featureLevel), *internalFormat, device, renderer11DeviceCaps);
        textureCapsMap->insert(*internalFormat, textureCaps);

        maxSamples = std::max(maxSamples, textureCaps.getMaxSamples());

        if (gl::GetInternalFormatInfo(*internalFormat).compressed)
        {
            caps->compressedTextureFormats.push_back(*internalFormat);
        }
    }

    // GL core feature limits
    caps->maxElementIndex = static_cast<GLint64>(std::numeric_limits<unsigned int>::max());
    caps->max3DTextureSize      = static_cast<GLuint>(GetMaximum3DTextureSize(featureLevel));
    caps->max2DTextureSize      = static_cast<GLuint>(GetMaximum2DTextureSize(featureLevel));
    caps->maxCubeMapTextureSize = static_cast<GLuint>(GetMaximumCubeMapTextureSize(featureLevel));
    caps->maxArrayTextureLayers = static_cast<GLuint>(GetMaximum2DTextureArraySize(featureLevel));

    // Unimplemented, set to minimum required
    caps->maxLODBias = 2.0f;

    // No specific limits on render target size, maximum 2D texture size is equivalent
    caps->maxRenderbufferSize = caps->max2DTextureSize;

    // Maximum draw buffers and color attachments are the same, max color attachments could eventually be
    // increased to 16
    caps->maxDrawBuffers = static_cast<GLuint>(GetMaximumSimultaneousRenderTargets(featureLevel));
    caps->maxColorAttachments =
        static_cast<GLuint>(GetMaximumSimultaneousRenderTargets(featureLevel));

    // D3D11 has the same limit for viewport width and height
    caps->maxViewportWidth  = static_cast<GLuint>(GetMaximumViewportSize(featureLevel));
    caps->maxViewportHeight = caps->maxViewportWidth;

    // Choose a reasonable maximum, enforced in the shader.
    caps->minAliasedPointSize = 1.0f;
    caps->maxAliasedPointSize = 1024.0f;

    // Wide lines not supported
    caps->minAliasedLineWidth = 1.0f;
    caps->maxAliasedLineWidth = 1.0f;

    // Primitive count limits
    caps->maxElementsIndices  = static_cast<GLuint>(GetMaximumDrawIndexedIndexCount(featureLevel));
    caps->maxElementsVertices = static_cast<GLuint>(GetMaximumDrawVertexCount(featureLevel));

    // Program and shader binary formats (no supported shader binary formats)
    caps->programBinaryFormats.push_back(GL_PROGRAM_BINARY_ANGLE);

    caps->vertexHighpFloat.setIEEEFloat();
    caps->vertexMediumpFloat.setIEEEFloat();
    caps->vertexLowpFloat.setIEEEFloat();
    caps->fragmentHighpFloat.setIEEEFloat();
    caps->fragmentMediumpFloat.setIEEEFloat();
    caps->fragmentLowpFloat.setIEEEFloat();

    // 32-bit integers are natively supported
    caps->vertexHighpInt.setTwosComplementInt(32);
    caps->vertexMediumpInt.setTwosComplementInt(32);
    caps->vertexLowpInt.setTwosComplementInt(32);
    caps->fragmentHighpInt.setTwosComplementInt(32);
    caps->fragmentMediumpInt.setTwosComplementInt(32);
    caps->fragmentLowpInt.setTwosComplementInt(32);

    // We do not wait for server fence objects internally, so report a max timeout of zero.
    caps->maxServerWaitTimeout = 0;

    // Vertex shader limits
    caps->maxVertexAttributes = static_cast<GLuint>(GetMaximumVertexInputSlots(featureLevel));
    caps->maxVertexUniformComponents =
        static_cast<GLuint>(GetMaximumVertexUniformVectors(featureLevel)) * 4;
    caps->maxVertexUniformVectors =
        static_cast<GLuint>(GetMaximumVertexUniformVectors(featureLevel));
    caps->maxVertexUniformBlocks = static_cast<GLuint>(GetMaximumVertexUniformBlocks(featureLevel));
    caps->maxVertexOutputComponents =
        static_cast<GLuint>(GetMaximumVertexOutputVectors(featureLevel)) * 4;
    caps->maxVertexTextureImageUnits =
        static_cast<GLuint>(GetMaximumVertexTextureUnits(featureLevel));

    // Fragment shader limits
    caps->maxFragmentUniformComponents =
        static_cast<GLuint>(GetMaximumPixelUniformVectors(featureLevel)) * 4;
    caps->maxFragmentUniformVectors =
        static_cast<GLuint>(GetMaximumPixelUniformVectors(featureLevel));
    caps->maxFragmentUniformBlocks =
        static_cast<GLuint>(GetMaximumPixelUniformBlocks(featureLevel));
    caps->maxFragmentInputComponents =
        static_cast<GLuint>(GetMaximumPixelInputVectors(featureLevel)) * 4;
    caps->maxTextureImageUnits  = static_cast<GLuint>(GetMaximumPixelTextureUnits(featureLevel));
    caps->minProgramTexelOffset = GetMinimumTexelOffset(featureLevel);
    caps->maxProgramTexelOffset = GetMaximumTexelOffset(featureLevel);

    // Aggregate shader limits
    caps->maxUniformBufferBindings = caps->maxVertexUniformBlocks + caps->maxFragmentUniformBlocks;
    caps->maxUniformBlockSize = GetMaximumConstantBufferSize(featureLevel);

    // With DirectX 11.1, constant buffer offset and size must be a multiple of 16 constants of 16 bytes each.
    // https://msdn.microsoft.com/en-us/library/windows/desktop/hh404649%28v=vs.85%29.aspx
    // With DirectX 11.0, we emulate UBO offsets using copies of ranges of the UBO however
    // we still keep the same alignment as 11.1 for consistency.
    caps->uniformBufferOffsetAlignment = 256;

    caps->maxCombinedUniformBlocks = caps->maxVertexUniformBlocks + caps->maxFragmentUniformBlocks;
    caps->maxCombinedVertexUniformComponents = (static_cast<GLint64>(caps->maxVertexUniformBlocks) * static_cast<GLint64>(caps->maxUniformBlockSize / 4)) +
                                               static_cast<GLint64>(caps->maxVertexUniformComponents);
    caps->maxCombinedFragmentUniformComponents = (static_cast<GLint64>(caps->maxFragmentUniformBlocks) * static_cast<GLint64>(caps->maxUniformBlockSize / 4)) +
                                                 static_cast<GLint64>(caps->maxFragmentUniformComponents);
    caps->maxVaryingComponents =
        static_cast<GLuint>(GetMaximumVertexOutputVectors(featureLevel)) * 4;
    caps->maxVaryingVectors            = static_cast<GLuint>(GetMaximumVertexOutputVectors(featureLevel));
    caps->maxCombinedTextureImageUnits = caps->maxVertexTextureImageUnits + caps->maxTextureImageUnits;

    // Transform feedback limits
    caps->maxTransformFeedbackInterleavedComponents =
        static_cast<GLuint>(GetMaximumStreamOutputInterleavedComponents(featureLevel));
    caps->maxTransformFeedbackSeparateAttributes =
        static_cast<GLuint>(GetMaximumStreamOutputBuffers(featureLevel));
    caps->maxTransformFeedbackSeparateComponents =
        static_cast<GLuint>(GetMaximumStreamOutputSeparateComponents(featureLevel));

    // Multisample limits
    caps->maxSamples = maxSamples;

    // GL extension support
    extensions->setTextureExtensionSupport(*textureCapsMap);
    extensions->elementIndexUint = true;
    extensions->getProgramBinary = true;
    extensions->rgb8rgba8 = true;
    extensions->readFormatBGRA = true;
    extensions->pixelBufferObject = true;
    extensions->mapBuffer = true;
    extensions->mapBufferRange = true;
    extensions->textureNPOT = GetNPOTTextureSupport(featureLevel);
    extensions->drawBuffers = GetMaximumSimultaneousRenderTargets(featureLevel) > 1;
    extensions->textureStorage = true;
    extensions->textureFilterAnisotropic = true;
    extensions->maxTextureAnisotropy = GetMaximumAnisotropy(featureLevel);
    extensions->occlusionQueryBoolean = GetOcclusionQuerySupport(featureLevel);
    extensions->fence = GetEventQuerySupport(featureLevel);
    extensions->timerQuery = false; // Unimplemented
    extensions->robustness = true;
    extensions->blendMinMax = true;
    extensions->framebufferBlit = GetFramebufferBlitSupport(featureLevel);
    extensions->framebufferMultisample = GetFramebufferMultisampleSupport(featureLevel);
    extensions->instancedArrays = GetInstancingSupport(featureLevel);
    extensions->packReverseRowOrder = true;
    extensions->standardDerivatives = GetDerivativeInstructionSupport(featureLevel);
    extensions->shaderTextureLOD = GetShaderTextureLODSupport(featureLevel);
    extensions->fragDepth = true;
    extensions->textureUsage = true; // This could be false since it has no effect in D3D11
    extensions->discardFramebuffer = false; // TODO: enable this once BUG:497445 is fixed (Chrome WebGL video tests fail with this extension active)
    extensions->translatedShaderSource = true;
    extensions->fboRenderMipmap = false;
    extensions->debugMarker = true;
    extensions->eglImage                 = true;

    // D3D11 Feature Level 10_0+ uses SV_IsFrontFace in HLSL to emulate gl_FrontFacing.
    // D3D11 Feature Level 9_3 doesn't support SV_IsFrontFace, and has no equivalent, so can't support gl_FrontFacing.
    limitations->noFrontFacingSupport = (renderer11DeviceCaps.featureLevel <= D3D_FEATURE_LEVEL_9_3);

    // D3D11 Feature Level 9_3 doesn't support alpha-to-coverage
    limitations->noSampleAlphaToCoverageSupport = (renderer11DeviceCaps.featureLevel <= D3D_FEATURE_LEVEL_9_3);

    // D3D11 Feature Levels 9_3 and below do not support non-constant loop indexing and require
    // additional
    // pre-validation of the shader at compile time to produce a better error message.
    limitations->shadersRequireIndexedLoopValidation =
        (renderer11DeviceCaps.featureLevel <= D3D_FEATURE_LEVEL_9_3);

    // D3D11 has no concept of separate masks and refs for front and back faces in the depth stencil
    // state.
    limitations->noSeparateStencilRefsAndMasks = true;

    // D3D11 cannot support constant color and alpha blend funcs together
    limitations->noSimultaneousConstantColorAndAlphaBlendFunc = true;

#ifdef ANGLE_ENABLE_WINDOWS_STORE
    // Setting a non-zero divisor on attribute zero doesn't work on certain Windows Phone 8-era devices.
    // We should prevent developers from doing this on ALL Windows Store devices. This will maintain consistency across all Windows devices.
    // We allow non-zero divisors on attribute zero if the Client Version >= 3, since devices affected by this issue don't support ES3+.
    limitations->attributeZeroRequiresZeroDivisorInEXT = true;
#endif
}

}

namespace d3d11
{

void MakeValidSize(bool isImage, DXGI_FORMAT format, GLsizei *requestWidth, GLsizei *requestHeight, int *levelOffset)
{
    const DXGIFormat &dxgiFormatInfo = d3d11::GetDXGIFormatInfo(format);

    int upsampleCount = 0;
    // Don't expand the size of full textures that are at least (blockWidth x blockHeight) already.
    if (isImage || *requestWidth  < static_cast<GLsizei>(dxgiFormatInfo.blockWidth) ||
                   *requestHeight < static_cast<GLsizei>(dxgiFormatInfo.blockHeight))
    {
        while (*requestWidth % dxgiFormatInfo.blockWidth != 0 || *requestHeight % dxgiFormatInfo.blockHeight != 0)
        {
            *requestWidth <<= 1;
            *requestHeight <<= 1;
            upsampleCount++;
        }
    }
    *levelOffset = upsampleCount;
}

void GenerateInitialTextureData(GLint internalFormat, const Renderer11DeviceCaps &renderer11DeviceCaps, GLuint width, GLuint height, GLuint depth,
                                GLuint mipLevels, std::vector<D3D11_SUBRESOURCE_DATA> *outSubresourceData,
                                std::vector< std::vector<BYTE> > *outData)
{
    const d3d11::TextureFormat &d3dFormatInfo = d3d11::GetTextureFormatInfo(internalFormat, renderer11DeviceCaps);
    ASSERT(d3dFormatInfo.dataInitializerFunction != NULL);

    const d3d11::DXGIFormat &dxgiFormatInfo = d3d11::GetDXGIFormatInfo(d3dFormatInfo.texFormat);

    outSubresourceData->resize(mipLevels);
    outData->resize(mipLevels);

    for (unsigned int i = 0; i < mipLevels; i++)
    {
        unsigned int mipWidth = std::max(width >> i, 1U);
        unsigned int mipHeight = std::max(height >> i, 1U);
        unsigned int mipDepth = std::max(depth >> i, 1U);

        unsigned int rowWidth = dxgiFormatInfo.pixelBytes * mipWidth;
        unsigned int imageSize = rowWidth * height;

        outData->at(i).resize(rowWidth * mipHeight * mipDepth);
        d3dFormatInfo.dataInitializerFunction(mipWidth, mipHeight, mipDepth, outData->at(i).data(), rowWidth, imageSize);

        outSubresourceData->at(i).pSysMem = outData->at(i).data();
        outSubresourceData->at(i).SysMemPitch = rowWidth;
        outSubresourceData->at(i).SysMemSlicePitch = imageSize;
    }
}

void SetPositionTexCoordVertex(PositionTexCoordVertex* vertex, float x, float y, float u, float v)
{
    vertex->x = x;
    vertex->y = y;
    vertex->u = u;
    vertex->v = v;
}

void SetPositionLayerTexCoord3DVertex(PositionLayerTexCoord3DVertex* vertex, float x, float y,
                                      unsigned int layer, float u, float v, float s)
{
    vertex->x = x;
    vertex->y = y;
    vertex->l = layer;
    vertex->u = u;
    vertex->v = v;
    vertex->s = s;
}

HRESULT SetDebugName(ID3D11DeviceChild *resource, const char *name)
{
#if defined(_DEBUG)
    return resource->SetPrivateData(WKPDID_D3DDebugObjectName,
                                    static_cast<unsigned int>(strlen(name)), name);
#else
    return S_OK;
#endif
}

WorkaroundsD3D GenerateWorkarounds(D3D_FEATURE_LEVEL featureLevel)
{
    WorkaroundsD3D workarounds;
    workarounds.mrtPerfWorkaround = true;
    workarounds.setDataFasterThanImageUpload = true;
    workarounds.zeroMaxLodWorkaround = (featureLevel <= D3D_FEATURE_LEVEL_9_3);
    workarounds.useInstancedPointSpriteEmulation = (featureLevel <= D3D_FEATURE_LEVEL_9_3);
    return workarounds;
}

}

}
