//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationEGL.cpp: Validation functions for generic EGL entry point parameters

#include "libANGLE/validationEGL.h"

#include "common/utilities.h"
#include "libANGLE/Config.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Image.h"
#include "libANGLE/Surface.h"

#include <EGL/eglext.h>

namespace
{
size_t GetMaximumMipLevel(const gl::Context *context, GLenum target)
{
    const gl::Caps &caps = context->getCaps();

    size_t maxDimension = 0;
    switch (target)
    {
        case GL_TEXTURE_2D:
            maxDimension = caps.max2DTextureSize;
            break;
        case GL_TEXTURE_CUBE_MAP:
            maxDimension = caps.maxCubeMapTextureSize;
            break;
        case GL_TEXTURE_3D:
            maxDimension = caps.max3DTextureSize;
            break;
        case GL_TEXTURE_2D_ARRAY:
            maxDimension = caps.max2DTextureSize;
            break;
        default:
            UNREACHABLE();
    }

    return gl::log2(static_cast<int>(maxDimension));
}

bool TextureHasNonZeroMipLevelsSpecified(const gl::Context *context, const gl::Texture *texture)
{
    size_t maxMip = GetMaximumMipLevel(context, texture->getTarget());
    for (size_t level = 1; level < maxMip; level++)
    {
        if (texture->getTarget() == GL_TEXTURE_CUBE_MAP)
        {
            for (GLenum face = gl::FirstCubeMapTextureTarget; face <= gl::LastCubeMapTextureTarget;
                 face++)
            {
                if (texture->getInternalFormat(face, level) != GL_NONE)
                {
                    return true;
                }
            }
        }
        else
        {
            if (texture->getInternalFormat(texture->getTarget(), level) != GL_NONE)
            {
                return true;
            }
        }
    }

    return false;
}

bool CubeTextureHasUnspecifiedLevel0Face(const gl::Texture *texture)
{
    ASSERT(texture->getTarget() == GL_TEXTURE_CUBE_MAP);
    for (GLenum face = gl::FirstCubeMapTextureTarget; face <= gl::LastCubeMapTextureTarget; face++)
    {
        if (texture->getInternalFormat(face, 0) == GL_NONE)
        {
            return true;
        }
    }

    return false;
}
}

namespace egl
{

Error ValidateDisplay(const Display *display)
{
    if (display == EGL_NO_DISPLAY)
    {
        return Error(EGL_BAD_DISPLAY, "display is EGL_NO_DISPLAY.");
    }

    if (!Display::isValidDisplay(display))
    {
        return Error(EGL_BAD_DISPLAY, "display is not a valid display.");
    }

    if (!display->isInitialized())
    {
        return Error(EGL_NOT_INITIALIZED, "display is not initialized.");
    }

    return Error(EGL_SUCCESS);
}

Error ValidateSurface(const Display *display, Surface *surface)
{
    Error error = ValidateDisplay(display);
    if (error.isError())
    {
        return error;
    }

    if (!display->isValidSurface(surface))
    {
        return Error(EGL_BAD_SURFACE);
    }

    return Error(EGL_SUCCESS);
}

Error ValidateConfig(const Display *display, const Config *config)
{
    Error error = ValidateDisplay(display);
    if (error.isError())
    {
        return error;
    }

    if (!display->isValidConfig(config))
    {
        return Error(EGL_BAD_CONFIG);
    }

    return Error(EGL_SUCCESS);
}

Error ValidateContext(const Display *display, gl::Context *context)
{
    Error error = ValidateDisplay(display);
    if (error.isError())
    {
        return error;
    }

    if (!display->isValidContext(context))
    {
        return Error(EGL_BAD_CONTEXT);
    }

    return Error(EGL_SUCCESS);
}

Error ValidateImage(const Display *display, const Image *image)
{
    Error error = ValidateDisplay(display);
    if (error.isError())
    {
        return error;
    }

    if (!display->isValidImage(image))
    {
        return Error(EGL_BAD_PARAMETER, "image is not valid.");
    }

    return Error(EGL_SUCCESS);
}

Error ValidateCreateContext(Display *display, Config *configuration, gl::Context *shareContext,
                            const AttributeMap& attributes)
{
    Error error = ValidateConfig(display, configuration);
    if (error.isError())
    {
        return error;
    }

    // Get the requested client version (default is 1) and check it is 2 or 3.
    EGLint clientMajorVersion = 1;
    EGLint clientMinorVersion = 0;
    EGLint contextFlags = 0;
    bool resetNotification = false;
    bool robustAccess = false;
    for (AttributeMap::const_iterator attributeIter = attributes.begin(); attributeIter != attributes.end(); attributeIter++)
    {
        EGLint attribute = attributeIter->first;
        EGLint value = attributeIter->second;

        switch (attribute)
        {
          case EGL_CONTEXT_CLIENT_VERSION:
            clientMajorVersion = value;
            break;

          case EGL_CONTEXT_MINOR_VERSION:
            clientMinorVersion = value;
            break;

          case EGL_CONTEXT_FLAGS_KHR:
            contextFlags = value;
            break;

          case EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR:
            // Only valid for OpenGL (non-ES) contexts
            return Error(EGL_BAD_ATTRIBUTE);

          case EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT:
            if (!display->getExtensions().createContextRobustness)
            {
                return Error(EGL_BAD_ATTRIBUTE);
            }
            if (value != EGL_TRUE && value != EGL_FALSE)
            {
                return Error(EGL_BAD_ATTRIBUTE);
            }
            robustAccess = (value == EGL_TRUE);
            break;

          case EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR:
            static_assert(EGL_LOSE_CONTEXT_ON_RESET_EXT == EGL_LOSE_CONTEXT_ON_RESET_KHR, "EGL extension enums not equal.");
            static_assert(EGL_NO_RESET_NOTIFICATION_EXT == EGL_NO_RESET_NOTIFICATION_KHR, "EGL extension enums not equal.");
            // same as EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT, fall through
          case EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT:
            if (!display->getExtensions().createContextRobustness)
            {
                return Error(EGL_BAD_ATTRIBUTE);
            }
            if (value == EGL_LOSE_CONTEXT_ON_RESET_EXT)
            {
                resetNotification = true;
            }
            else if (value != EGL_NO_RESET_NOTIFICATION_EXT)
            {
                return Error(EGL_BAD_ATTRIBUTE);
            }
            break;

          default:
            return Error(EGL_BAD_ATTRIBUTE);
        }
    }

    if ((clientMajorVersion != 2 && clientMajorVersion != 3) || clientMinorVersion != 0)
    {
        return Error(EGL_BAD_CONFIG);
    }

    if (clientMajorVersion == 3 && !(configuration->conformant & EGL_OPENGL_ES3_BIT_KHR))
    {
        return Error(EGL_BAD_CONFIG);
    }

    // Note: EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR does not apply to ES
    const EGLint validContextFlags = (EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR |
                                      EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR);
    if ((contextFlags & ~validContextFlags) != 0)
    {
        return Error(EGL_BAD_ATTRIBUTE);
    }

    if ((contextFlags & EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR) > 0)
    {
        robustAccess = true;
    }

    if (robustAccess)
    {
        // Unimplemented
        return Error(EGL_BAD_CONFIG);
    }

    if (shareContext)
    {
        // Shared context is invalid or is owned by another display
        if (!display->isValidContext(shareContext))
        {
            return Error(EGL_BAD_MATCH);
        }

        if (shareContext->isResetNotificationEnabled() != resetNotification)
        {
            return Error(EGL_BAD_MATCH);
        }

        if (shareContext->getClientVersion() != clientMajorVersion)
        {
            return Error(EGL_BAD_CONTEXT);
        }
    }

    return Error(EGL_SUCCESS);
}

Error ValidateCreateWindowSurface(Display *display, Config *config, EGLNativeWindowType window,
                                  const AttributeMap& attributes)
{
    Error error = ValidateConfig(display, config);
    if (error.isError())
    {
        return error;
    }

    if (!display->isValidNativeWindow(window))
    {
        return Error(EGL_BAD_NATIVE_WINDOW);
    }

    const DisplayExtensions &displayExtensions = display->getExtensions();

    for (AttributeMap::const_iterator attributeIter = attributes.begin(); attributeIter != attributes.end(); attributeIter++)
    {
        EGLint attribute = attributeIter->first;
        EGLint value = attributeIter->second;

        switch (attribute)
        {
          case EGL_RENDER_BUFFER:
            switch (value)
            {
              case EGL_BACK_BUFFER:
                break;
              case EGL_SINGLE_BUFFER:
                return Error(EGL_BAD_MATCH);   // Rendering directly to front buffer not supported
              default:
                return Error(EGL_BAD_ATTRIBUTE);
            }
            break;

          case EGL_POST_SUB_BUFFER_SUPPORTED_NV:
            if (!displayExtensions.postSubBuffer)
            {
                return Error(EGL_BAD_ATTRIBUTE);
            }
            break;

          case EGL_WIDTH:
          case EGL_HEIGHT:
            if (!displayExtensions.windowFixedSize)
            {
                return Error(EGL_BAD_ATTRIBUTE);
            }
            if (value < 0)
            {
                return Error(EGL_BAD_PARAMETER);
            }
            break;

          case EGL_FIXED_SIZE_ANGLE:
            if (!displayExtensions.windowFixedSize)
            {
                return Error(EGL_BAD_ATTRIBUTE);
            }
            break;

          case EGL_VG_COLORSPACE:
            return Error(EGL_BAD_MATCH);

          case EGL_VG_ALPHA_FORMAT:
            return Error(EGL_BAD_MATCH);

          default:
            return Error(EGL_BAD_ATTRIBUTE);
        }
    }

    if (Display::hasExistingWindowSurface(window))
    {
        return Error(EGL_BAD_ALLOC);
    }

    return Error(EGL_SUCCESS);
}

Error ValidateCreatePbufferSurface(Display *display, Config *config, const AttributeMap& attributes)
{
    Error error = ValidateConfig(display, config);
    if (error.isError())
    {
        return error;
    }
    
    for (AttributeMap::const_iterator attributeIter = attributes.begin(); attributeIter != attributes.end(); attributeIter++)
    {
        EGLint attribute = attributeIter->first;
        EGLint value = attributeIter->second;

        switch (attribute)
        {
          case EGL_WIDTH:
          case EGL_HEIGHT:
            if (value < 0)
            {
                return Error(EGL_BAD_PARAMETER);
            }
            break;

          case EGL_LARGEST_PBUFFER:
            break;

          case EGL_TEXTURE_FORMAT:
            switch (value)
            {
              case EGL_NO_TEXTURE:
              case EGL_TEXTURE_RGB:
              case EGL_TEXTURE_RGBA:
                break;
              default:
                return Error(EGL_BAD_ATTRIBUTE);
            }
            break;

          case EGL_TEXTURE_TARGET:
            switch (value)
            {
              case EGL_NO_TEXTURE:
              case EGL_TEXTURE_2D:
                break;
              default:
                return Error(EGL_BAD_ATTRIBUTE);
            }
            break;

          case EGL_MIPMAP_TEXTURE:
            break;

          case EGL_VG_COLORSPACE:
            break;

          case EGL_VG_ALPHA_FORMAT:
            break;

          default:
            return Error(EGL_BAD_ATTRIBUTE);
        }
    }

    if (!(config->surfaceType & EGL_PBUFFER_BIT))
    {
        return Error(EGL_BAD_MATCH);
    }

    const Caps &caps = display->getCaps();

    EGLenum textureFormat = attributes.get(EGL_TEXTURE_FORMAT, EGL_NO_TEXTURE);
    EGLenum textureTarget = attributes.get(EGL_TEXTURE_TARGET, EGL_NO_TEXTURE);

    if ((textureFormat != EGL_NO_TEXTURE && textureTarget == EGL_NO_TEXTURE) ||
        (textureFormat == EGL_NO_TEXTURE && textureTarget != EGL_NO_TEXTURE))
    {
        return Error(EGL_BAD_MATCH);
    }

    if ((textureFormat == EGL_TEXTURE_RGB  && config->bindToTextureRGB != EGL_TRUE) ||
        (textureFormat == EGL_TEXTURE_RGBA && config->bindToTextureRGBA != EGL_TRUE))
    {
        return Error(EGL_BAD_ATTRIBUTE);
    }

    EGLint width = attributes.get(EGL_WIDTH, 0);
    EGLint height = attributes.get(EGL_HEIGHT, 0);
    if (textureFormat != EGL_NO_TEXTURE && !caps.textureNPOT && (!gl::isPow2(width) || !gl::isPow2(height)))
    {
        return Error(EGL_BAD_MATCH);
    }

    return Error(EGL_SUCCESS);
}

Error ValidateCreatePbufferFromClientBuffer(Display *display, EGLenum buftype, EGLClientBuffer buffer,
                                            Config *config, const AttributeMap& attributes)
{
    Error error = ValidateConfig(display, config);
    if (error.isError())
    {
        return error;
    }

    const DisplayExtensions &displayExtensions = display->getExtensions();

    switch (buftype)
    {
      case EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE:
        if (!displayExtensions.d3dShareHandleClientBuffer)
        {
            return Error(EGL_BAD_PARAMETER);
        }
        if (buffer == nullptr)
        {
            return Error(EGL_BAD_PARAMETER);
        }
        break;

      default:
        return Error(EGL_BAD_PARAMETER);
    }

    for (AttributeMap::const_iterator attributeIter = attributes.begin(); attributeIter != attributes.end(); attributeIter++)
    {
        EGLint attribute = attributeIter->first;
        EGLint value = attributeIter->second;

        switch (attribute)
        {
          case EGL_WIDTH:
          case EGL_HEIGHT:
            if (!displayExtensions.d3dShareHandleClientBuffer)
            {
                return Error(EGL_BAD_PARAMETER);
            }
            if (value < 0)
            {
                return Error(EGL_BAD_PARAMETER);
            }
            break;

          case EGL_TEXTURE_FORMAT:
            switch (value)
            {
              case EGL_NO_TEXTURE:
              case EGL_TEXTURE_RGB:
              case EGL_TEXTURE_RGBA:
                break;
              default:
                return Error(EGL_BAD_ATTRIBUTE);
            }
            break;

          case EGL_TEXTURE_TARGET:
            switch (value)
            {
              case EGL_NO_TEXTURE:
              case EGL_TEXTURE_2D:
                break;
              default:
                return Error(EGL_BAD_ATTRIBUTE);
            }
            break;

          case EGL_MIPMAP_TEXTURE:
            break;

          default:
            return Error(EGL_BAD_ATTRIBUTE);
        }
    }

    if (!(config->surfaceType & EGL_PBUFFER_BIT))
    {
        return Error(EGL_BAD_MATCH);
    }

    EGLenum textureFormat = attributes.get(EGL_TEXTURE_FORMAT, EGL_NO_TEXTURE);
    EGLenum textureTarget = attributes.get(EGL_TEXTURE_TARGET, EGL_NO_TEXTURE);
    if ((textureFormat != EGL_NO_TEXTURE && textureTarget == EGL_NO_TEXTURE) ||
        (textureFormat == EGL_NO_TEXTURE && textureTarget != EGL_NO_TEXTURE))
    {
        return Error(EGL_BAD_MATCH);
    }

    if ((textureFormat == EGL_TEXTURE_RGB  && config->bindToTextureRGB  != EGL_TRUE) ||
        (textureFormat == EGL_TEXTURE_RGBA && config->bindToTextureRGBA != EGL_TRUE))
    {
        return Error(EGL_BAD_ATTRIBUTE);
    }

    if (buftype == EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE)
    {
        EGLint width = attributes.get(EGL_WIDTH, 0);
        EGLint height = attributes.get(EGL_HEIGHT, 0);

        if (width == 0 || height == 0)
        {
            return Error(EGL_BAD_ATTRIBUTE);
        }

        const Caps &caps = display->getCaps();
        if (textureFormat != EGL_NO_TEXTURE && !caps.textureNPOT && (!gl::isPow2(width) || !gl::isPow2(height)))
        {
            return Error(EGL_BAD_MATCH);
        }
    }

    return Error(EGL_SUCCESS);
}

Error ValidateCompatibleConfigs(const Config *config1, const Config *config2, EGLint surfaceType)
{
    // Config compatibility is defined in section 2.2 of the EGL 1.5 spec

    bool colorBufferCompat = config1->colorBufferType == config2->colorBufferType;
    if (!colorBufferCompat)
    {
        return Error(EGL_BAD_MATCH, "Color buffer types are not compatible.");
    }

    bool colorCompat = config1->redSize == config2->redSize && config1->greenSize == config2->greenSize &&
                       config1->blueSize == config2->blueSize && config1->alphaSize == config2->alphaSize &&
                       config1->luminanceSize == config2->luminanceSize;
    if (!colorCompat)
    {
        return Error(EGL_BAD_MATCH, "Color buffer sizes are not compatible.");
    }

    bool dsCompat = config1->depthSize == config2->depthSize && config1->stencilSize == config2->stencilSize;
    if (!dsCompat)
    {
        return Error(EGL_BAD_MATCH, "Depth-stencil buffer types are not compatible.");
    }

    bool surfaceTypeCompat = (config1->surfaceType & config2->surfaceType & surfaceType) != 0;
    if (!surfaceTypeCompat)
    {
        return Error(EGL_BAD_MATCH, "Surface types are not compatible.");
    }

    return Error(EGL_SUCCESS);
}

Error ValidateCreateImageKHR(const Display *display,
                             gl::Context *context,
                             EGLenum target,
                             EGLClientBuffer buffer,
                             const AttributeMap &attributes)
{
    Error error = ValidateContext(display, context);
    if (error.isError())
    {
        return error;
    }

    const DisplayExtensions &displayExtensions = display->getExtensions();

    if (!displayExtensions.imageBase && !displayExtensions.image)
    {
        // It is out of spec what happens when calling an extension function when the extension is
        // not available.
        // EGL_BAD_DISPLAY seems like a reasonable error.
        return Error(EGL_BAD_DISPLAY, "EGL_KHR_image not supported.");
    }

    // TODO(geofflang): Complete validation from EGL_KHR_image_base:
    // If the resource specified by <dpy>, <ctx>, <target>, <buffer> and <attrib_list> is itself an
    // EGLImage sibling, the error EGL_BAD_ACCESS is generated.

    for (AttributeMap::const_iterator attributeIter = attributes.begin();
         attributeIter != attributes.end(); attributeIter++)
    {
        EGLint attribute = attributeIter->first;
        EGLint value     = attributeIter->second;

        switch (attribute)
        {
            case EGL_IMAGE_PRESERVED_KHR:
                switch (value)
                {
                    case EGL_TRUE:
                    case EGL_FALSE:
                        break;

                    default:
                        return Error(EGL_BAD_PARAMETER,
                                     "EGL_IMAGE_PRESERVED_KHR must be EGL_TRUE or EGL_FALSE.");
                }
                break;

            case EGL_GL_TEXTURE_LEVEL_KHR:
                if (!displayExtensions.glTexture2DImage &&
                    !displayExtensions.glTextureCubemapImage && !displayExtensions.glTexture3DImage)
                {
                    return Error(EGL_BAD_PARAMETER,
                                 "EGL_GL_TEXTURE_LEVEL_KHR cannot be used without "
                                 "KHR_gl_texture_*_image support.");
                }

                if (value < 0)
                {
                    return Error(EGL_BAD_PARAMETER, "EGL_GL_TEXTURE_LEVEL_KHR cannot be negative.");
                }
                break;

            case EGL_GL_TEXTURE_ZOFFSET_KHR:
                if (!displayExtensions.glTexture3DImage)
                {
                    return Error(EGL_BAD_PARAMETER,
                                 "EGL_GL_TEXTURE_ZOFFSET_KHR cannot be used without "
                                 "KHR_gl_texture_3D_image support.");
                }
                break;

            default:
                return Error(EGL_BAD_PARAMETER, "invalid attribute: 0x%X", attribute);
        }
    }

    switch (target)
    {
        case EGL_GL_TEXTURE_2D_KHR:
        {
            if (!displayExtensions.glTexture2DImage)
            {
                return Error(EGL_BAD_PARAMETER, "KHR_gl_texture_2D_image not supported.");
            }

            if (buffer == 0)
            {
                return Error(EGL_BAD_PARAMETER,
                             "buffer cannot reference a 2D texture with the name 0.");
            }

            const gl::Texture *texture =
                context->getTexture(egl_gl::EGLClientBufferToGLObjectHandle(buffer));
            if (texture == nullptr || texture->getTarget() != GL_TEXTURE_2D)
            {
                return Error(EGL_BAD_PARAMETER, "target is not a 2D texture.");
            }

            if (texture->getBoundSurface() != nullptr)
            {
                return Error(EGL_BAD_ACCESS, "texture has a surface bound to it.");
            }

            EGLint level = attributes.get(EGL_GL_TEXTURE_LEVEL_KHR, 0);
            if (texture->getWidth(GL_TEXTURE_2D, static_cast<size_t>(level)) == 0 ||
                texture->getHeight(GL_TEXTURE_2D, static_cast<size_t>(level)) == 0)
            {
                return Error(EGL_BAD_PARAMETER,
                             "target 2D texture does not have a valid size at specified level.");
            }

            if (level > 0 && (!texture->isMipmapComplete() ||
                              static_cast<size_t>(level) >= texture->getMipCompleteLevels()))
            {
                return Error(EGL_BAD_PARAMETER, "texture must be complete if level is non-zero.");
            }

            if (level == 0 && !texture->isMipmapComplete() &&
                TextureHasNonZeroMipLevelsSpecified(context, texture))
            {
                return Error(EGL_BAD_PARAMETER,
                             "if level is zero and the texture is incomplete, it must have no mip "
                             "levels specified except zero.");
            }
        }
        break;

        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR:
        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR:
        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR:
        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR:
        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR:
        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR:
        {
            if (!displayExtensions.glTextureCubemapImage)
            {
                return Error(EGL_BAD_PARAMETER, "KHR_gl_texture_cubemap_image not supported.");
            }

            if (buffer == 0)
            {
                return Error(EGL_BAD_PARAMETER,
                             "buffer cannot reference a cubemap texture with the name 0.");
            }

            const gl::Texture *texture =
                context->getTexture(egl_gl::EGLClientBufferToGLObjectHandle(buffer));
            if (texture == nullptr || texture->getTarget() != GL_TEXTURE_CUBE_MAP)
            {
                return Error(EGL_BAD_PARAMETER, "target is not a cubemap texture.");
            }

            if (texture->getBoundSurface() != nullptr)
            {
                return Error(EGL_BAD_ACCESS, "texture has a surface bound to it.");
            }

            EGLint level       = attributes.get(EGL_GL_TEXTURE_LEVEL_KHR, 0);
            GLenum cubeMapFace = egl_gl::EGLCubeMapTargetToGLCubeMapTarget(target);
            if (texture->getWidth(cubeMapFace, static_cast<size_t>(level)) == 0 ||
                texture->getHeight(cubeMapFace, static_cast<size_t>(level)) == 0)
            {
                return Error(EGL_BAD_PARAMETER,
                             "target cubemap texture does not have a valid size at specified level "
                             "and face.");
            }

            if (level > 0 && (!texture->isMipmapComplete() ||
                              static_cast<size_t>(level) >= texture->getMipCompleteLevels()))
            {
                return Error(EGL_BAD_PARAMETER, "texture must be complete if level is non-zero.");
            }

            if (level == 0 && !texture->isMipmapComplete() &&
                TextureHasNonZeroMipLevelsSpecified(context, texture))
            {
                return Error(EGL_BAD_PARAMETER,
                             "if level is zero and the texture is incomplete, it must have no mip "
                             "levels specified except zero.");
            }

            if (level == 0 && !texture->isMipmapComplete() &&
                CubeTextureHasUnspecifiedLevel0Face(texture))
            {
                return Error(EGL_BAD_PARAMETER,
                             "if level is zero and the texture is incomplete, it must have all of "
                             "its faces specified at level zero.");
            }
        }
        break;

        case EGL_GL_TEXTURE_3D_KHR:
        {
            if (!displayExtensions.glTexture3DImage)
            {
                return Error(EGL_BAD_PARAMETER, "KHR_gl_texture_3D_image not supported.");
            }

            if (buffer == 0)
            {
                return Error(EGL_BAD_PARAMETER,
                             "buffer cannot reference a 3D texture with the name 0.");
            }

            const gl::Texture *texture =
                context->getTexture(egl_gl::EGLClientBufferToGLObjectHandle(buffer));
            if (texture == nullptr || texture->getTarget() != GL_TEXTURE_3D)
            {
                return Error(EGL_BAD_PARAMETER, "target is not a 3D texture.");
            }

            if (texture->getBoundSurface() != nullptr)
            {
                return Error(EGL_BAD_ACCESS, "texture has a surface bound to it.");
            }

            EGLint level   = attributes.get(EGL_GL_TEXTURE_LEVEL_KHR, 0);
            EGLint zOffset = attributes.get(EGL_GL_TEXTURE_ZOFFSET_KHR, 0);
            if (texture->getWidth(GL_TEXTURE_3D, static_cast<size_t>(level)) == 0 ||
                texture->getHeight(GL_TEXTURE_3D, static_cast<size_t>(level)) == 0 ||
                texture->getDepth(GL_TEXTURE_3D, static_cast<size_t>(level)) == 0)
            {
                return Error(EGL_BAD_PARAMETER,
                             "target 3D texture does not have a valid size at specified level.");
            }

            if (static_cast<size_t>(zOffset) >=
                texture->getDepth(GL_TEXTURE_3D, static_cast<size_t>(level)))
            {
                return Error(EGL_BAD_PARAMETER,
                             "target 3D texture does not have enough layers for the specified Z "
                             "offset at the specified level.");
            }

            if (level > 0 && (!texture->isMipmapComplete() ||
                              static_cast<size_t>(level) >= texture->getMipCompleteLevels()))
            {
                return Error(EGL_BAD_PARAMETER, "texture must be complete if level is non-zero.");
            }

            if (level == 0 && !texture->isMipmapComplete() &&
                TextureHasNonZeroMipLevelsSpecified(context, texture))
            {
                return Error(EGL_BAD_PARAMETER,
                             "if level is zero and the texture is incomplete, it must have no mip "
                             "levels specified except zero.");
            }
        }
        break;

        case EGL_GL_RENDERBUFFER_KHR:
        {
            if (!displayExtensions.glRenderbufferImage)
            {
                return Error(EGL_BAD_PARAMETER, "KHR_gl_renderbuffer_image not supported.");
            }

            if (attributes.contains(EGL_GL_TEXTURE_LEVEL_KHR))
            {
                return Error(EGL_BAD_PARAMETER,
                             "EGL_GL_TEXTURE_LEVEL_KHR cannot be used in conjunction with a "
                             "renderbuffer target.");
            }

            if (buffer == 0)
            {
                return Error(EGL_BAD_PARAMETER,
                             "buffer cannot reference a renderbuffer with the name 0.");
            }

            const gl::Renderbuffer *renderbuffer =
                context->getRenderbuffer(egl_gl::EGLClientBufferToGLObjectHandle(buffer));
            if (renderbuffer == nullptr)
            {
                return Error(EGL_BAD_PARAMETER, "target is not a renderbuffer.");
            }

            if (renderbuffer->getSamples() > 0)
            {
                return Error(EGL_BAD_PARAMETER, "target renderbuffer cannot be multisampled.");
            }
        }
        break;

        default:
            return Error(EGL_BAD_PARAMETER, "invalid target: 0x%X", target);
    }

    return Error(EGL_SUCCESS);
}

Error ValidateDestroyImageKHR(const Display *display, const Image *image)
{
    Error error = ValidateImage(display, image);
    if (error.isError())
    {
        return error;
    }

    if (!display->getExtensions().imageBase && !display->getExtensions().image)
    {
        // It is out of spec what happens when calling an extension function when the extension is
        // not available.
        // EGL_BAD_DISPLAY seems like a reasonable error.
        return Error(EGL_BAD_DISPLAY);
    }

    return Error(EGL_SUCCESS);
}
}
