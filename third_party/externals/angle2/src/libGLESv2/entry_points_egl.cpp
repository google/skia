//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_egl.cpp : Implements the EGL entry points.

#include "libGLESv2/entry_points_egl.h"
#include "libGLESv2/entry_points_egl_ext.h"
#include "libGLESv2/entry_points_gles_2_0.h"
#include "libGLESv2/entry_points_gles_2_0_ext.h"
#include "libGLESv2/entry_points_gles_3_0.h"
#include "libGLESv2/global_state.h"

#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Texture.h"
#include "libANGLE/Surface.h"
#include "libANGLE/validationEGL.h"

#include "common/debug.h"
#include "common/version.h"

#include <EGL/eglext.h>

namespace egl
{

// EGL 1.0
EGLint EGLAPIENTRY GetError(void)
{
    EVENT("()");

    EGLint error = GetGlobalError();
    SetGlobalError(Error(EGL_SUCCESS));
    return error;
}

EGLDisplay EGLAPIENTRY GetDisplay(EGLNativeDisplayType display_id)
{
    EVENT("(EGLNativeDisplayType display_id = 0x%0.8p)", display_id);

    return Display::getDisplay(display_id, AttributeMap());
}

EGLBoolean EGLAPIENTRY Initialize(EGLDisplay dpy, EGLint *major, EGLint *minor)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLint *major = 0x%0.8p, EGLint *minor = 0x%0.8p)",
          dpy, major, minor);

    if (dpy == EGL_NO_DISPLAY)
    {
        SetGlobalError(Error(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    Display *display = static_cast<Display*>(dpy);

    Error error = display->initialize();
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    if (major) *major = 1;
    if (minor) *minor = 4;

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean EGLAPIENTRY Terminate(EGLDisplay dpy)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p)", dpy);

    if (dpy == EGL_NO_DISPLAY)
    {
        SetGlobalError(Error(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    Display *display = static_cast<Display*>(dpy);
    gl::Context *context = GetGlobalContext();

    if (display->isValidContext(context))
    {
        SetGlobalContext(NULL);
        SetGlobalDisplay(NULL);
    }

    display->terminate();

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

const char *EGLAPIENTRY QueryString(EGLDisplay dpy, EGLint name)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLint name = %d)", dpy, name);

    Display *display = static_cast<Display*>(dpy);
    if (!(display == EGL_NO_DISPLAY && name == EGL_EXTENSIONS))
    {
        Error error = ValidateDisplay(display);
        if (error.isError())
        {
            SetGlobalError(error);
            return NULL;
        }
    }

    const char *result;
    switch (name)
    {
      case EGL_CLIENT_APIS:
        result = "OpenGL_ES";
        break;
      case EGL_EXTENSIONS:
        if (display == EGL_NO_DISPLAY)
        {
            result = Display::getClientExtensionString().c_str();
        }
        else
        {
            result = display->getExtensionString().c_str();
        }
        break;
      case EGL_VENDOR:
        result = display->getVendorString().c_str();
        break;
      case EGL_VERSION:
        result = "1.4 (ANGLE " ANGLE_VERSION_STRING ")";
        break;
      default:
        SetGlobalError(Error(EGL_BAD_PARAMETER));
        return NULL;
    }

    SetGlobalError(Error(EGL_SUCCESS));
    return result;
}

EGLBoolean EGLAPIENTRY GetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLConfig *configs = 0x%0.8p, "
          "EGLint config_size = %d, EGLint *num_config = 0x%0.8p)",
          dpy, configs, config_size, num_config);

    Display *display = static_cast<Display*>(dpy);

    Error error = ValidateDisplay(display);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    if (!num_config)
    {
        SetGlobalError(Error(EGL_BAD_PARAMETER));
        return EGL_FALSE;
    }

    std::vector<const Config*> filteredConfigs = display->getConfigs(AttributeMap());
    if (configs)
    {
        filteredConfigs.resize(std::min<size_t>(filteredConfigs.size(), config_size));
        for (size_t i = 0; i < filteredConfigs.size(); i++)
        {
            configs[i] = const_cast<Config*>(filteredConfigs[i]);
        }
    }
    *num_config = static_cast<EGLint>(filteredConfigs.size());

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean EGLAPIENTRY ChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, const EGLint *attrib_list = 0x%0.8p, "
          "EGLConfig *configs = 0x%0.8p, EGLint config_size = %d, EGLint *num_config = 0x%0.8p)",
          dpy, attrib_list, configs, config_size, num_config);

    Display *display = static_cast<Display*>(dpy);

    Error error = ValidateDisplay(display);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    if (!num_config)
    {
        SetGlobalError(Error(EGL_BAD_PARAMETER));
        return EGL_FALSE;
    }

    std::vector<const Config*> filteredConfigs = display->getConfigs(AttributeMap(attrib_list));
    if (configs)
    {
        filteredConfigs.resize(std::min<size_t>(filteredConfigs.size(), config_size));
        for (size_t i = 0; i < filteredConfigs.size(); i++)
        {
            configs[i] = const_cast<Config*>(filteredConfigs[i]);
        }
    }
    *num_config = static_cast<EGLint>(filteredConfigs.size());

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean EGLAPIENTRY GetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLConfig config = 0x%0.8p, EGLint attribute = %d, EGLint *value = 0x%0.8p)",
          dpy, config, attribute, value);

    Display *display = static_cast<Display*>(dpy);
    Config *configuration = static_cast<Config*>(config);

    Error error = ValidateConfig(display, configuration);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    if (!display->getConfigAttrib(configuration, attribute, value))
    {
        SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
        return EGL_FALSE;
    }

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLSurface EGLAPIENTRY CreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLConfig config = 0x%0.8p, EGLNativeWindowType win = 0x%0.8p, "
          "const EGLint *attrib_list = 0x%0.8p)", dpy, config, win, attrib_list);

    Display *display = static_cast<Display*>(dpy);
    Config *configuration = static_cast<Config*>(config);
    AttributeMap attributes(attrib_list);

    Error error = ValidateCreateWindowSurface(display, configuration, win, attributes);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_NO_SURFACE;
    }

    egl::Surface *surface = nullptr;
    error = display->createWindowSurface(configuration, win, attributes, &surface);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_NO_SURFACE;
    }

    return static_cast<EGLSurface>(surface);
}

EGLSurface EGLAPIENTRY CreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLConfig config = 0x%0.8p, const EGLint *attrib_list = 0x%0.8p)",
          dpy, config, attrib_list);

    Display *display = static_cast<Display*>(dpy);
    Config *configuration = static_cast<Config*>(config);
    AttributeMap attributes(attrib_list);

    Error error = ValidateCreatePbufferSurface(display, configuration, attributes);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_NO_SURFACE;
    }

    egl::Surface *surface = nullptr;
    error = display->createPbufferSurface(configuration, attributes, &surface);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_NO_SURFACE;
    }

    return static_cast<EGLSurface>(surface);
}

EGLSurface EGLAPIENTRY CreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLConfig config = 0x%0.8p, EGLNativePixmapType pixmap = 0x%0.8p, "
          "const EGLint *attrib_list = 0x%0.8p)", dpy, config, pixmap, attrib_list);

    Display *display = static_cast<Display*>(dpy);
    Config *configuration = static_cast<Config*>(config);

    Error error = ValidateConfig(display, configuration);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_NO_SURFACE;
    }

    UNIMPLEMENTED();   // FIXME

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_NO_SURFACE;
}

EGLBoolean EGLAPIENTRY DestroySurface(EGLDisplay dpy, EGLSurface surface)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p)", dpy, surface);

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = static_cast<Surface*>(surface);

    Error error = ValidateSurface(display, eglSurface);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    if (surface == EGL_NO_SURFACE)
    {
        SetGlobalError(Error(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    display->destroySurface((Surface*)surface);

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean EGLAPIENTRY QuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p, EGLint attribute = %d, EGLint *value = 0x%0.8p)",
          dpy, surface, attribute, value);

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = (Surface*)surface;

    Error error = ValidateSurface(display, eglSurface);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    if (surface == EGL_NO_SURFACE)
    {
        SetGlobalError(Error(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    switch (attribute)
    {
      case EGL_VG_ALPHA_FORMAT:
        UNIMPLEMENTED();   // FIXME
        break;
      case EGL_VG_COLORSPACE:
        UNIMPLEMENTED();   // FIXME
        break;
      case EGL_CONFIG_ID:
        *value = eglSurface->getConfig()->configID;
        break;
      case EGL_HEIGHT:
        *value = eglSurface->getHeight();
        break;
      case EGL_HORIZONTAL_RESOLUTION:
        UNIMPLEMENTED();   // FIXME
        break;
      case EGL_LARGEST_PBUFFER:
        UNIMPLEMENTED();   // FIXME
        break;
      case EGL_MIPMAP_TEXTURE:
        UNIMPLEMENTED();   // FIXME
        break;
      case EGL_MIPMAP_LEVEL:
        UNIMPLEMENTED();   // FIXME
        break;
      case EGL_MULTISAMPLE_RESOLVE:
        UNIMPLEMENTED();   // FIXME
        break;
      case EGL_PIXEL_ASPECT_RATIO:
        *value = eglSurface->getPixelAspectRatio();
        break;
      case EGL_RENDER_BUFFER:
        *value = eglSurface->getRenderBuffer();
        break;
      case EGL_SWAP_BEHAVIOR:
        *value = eglSurface->getSwapBehavior();
        break;
      case EGL_TEXTURE_FORMAT:
        *value = eglSurface->getTextureFormat();
        break;
      case EGL_TEXTURE_TARGET:
        *value = eglSurface->getTextureTarget();
        break;
      case EGL_VERTICAL_RESOLUTION:
        UNIMPLEMENTED();   // FIXME
        break;
      case EGL_WIDTH:
        *value = eglSurface->getWidth();
        break;
      case EGL_POST_SUB_BUFFER_SUPPORTED_NV:
        if (!display->getExtensions().postSubBuffer)
        {
            SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
            return EGL_FALSE;
        }
        *value = eglSurface->isPostSubBufferSupported();
        break;
      case EGL_FIXED_SIZE_ANGLE:
        if (!display->getExtensions().windowFixedSize)
        {
            SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
            return EGL_FALSE;
        }
        *value = eglSurface->isFixedSize();
        break;
      default:
        SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
        return EGL_FALSE;
    }

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLContext EGLAPIENTRY CreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLConfig config = 0x%0.8p, EGLContext share_context = 0x%0.8p, "
          "const EGLint *attrib_list = 0x%0.8p)", dpy, config, share_context, attrib_list);

    Display *display = static_cast<Display*>(dpy);
    Config *configuration = static_cast<Config*>(config);
    gl::Context* sharedGLContext = static_cast<gl::Context*>(share_context);
    AttributeMap attributes(attrib_list);

    Error error = ValidateCreateContext(display, configuration, sharedGLContext, attributes);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_NO_CONTEXT;
    }

    gl::Context *context = nullptr;
    error = display->createContext(configuration, sharedGLContext, attributes, &context);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_NO_CONTEXT;
    }

    SetGlobalError(Error(EGL_SUCCESS));
    return static_cast<EGLContext>(context);
}

EGLBoolean EGLAPIENTRY DestroyContext(EGLDisplay dpy, EGLContext ctx)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLContext ctx = 0x%0.8p)", dpy, ctx);

    Display *display = static_cast<Display*>(dpy);
    gl::Context *context = static_cast<gl::Context*>(ctx);

    Error error = ValidateContext(display, context);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    if (ctx == EGL_NO_CONTEXT)
    {
        SetGlobalError(Error(EGL_BAD_CONTEXT));
        return EGL_FALSE;
    }

    if (context == GetGlobalContext())
    {
        SetGlobalDisplay(NULL);
        SetGlobalContext(NULL);
    }

    display->destroyContext(context);

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean EGLAPIENTRY MakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface draw = 0x%0.8p, EGLSurface read = 0x%0.8p, EGLContext ctx = 0x%0.8p)",
          dpy, draw, read, ctx);

    Display *display = static_cast<Display*>(dpy);
    gl::Context *context = static_cast<gl::Context*>(ctx);

    bool noContext = (ctx == EGL_NO_CONTEXT);
    bool noSurface = (draw == EGL_NO_SURFACE || read == EGL_NO_SURFACE);
    if (noContext != noSurface)
    {
        SetGlobalError(Error(EGL_BAD_MATCH));
        return EGL_FALSE;
    }

    if (dpy == EGL_NO_DISPLAY)
    {
        SetGlobalError(Error(EGL_BAD_DISPLAY, "'dpy' not a valid EGLDisplay handle"));
        return EGL_FALSE;
    }

    // EGL 1.5 spec: dpy can be uninitialized if all other parameters are null
    if (!display->isInitialized() && (ctx != EGL_NO_CONTEXT || draw != EGL_NO_SURFACE || read != EGL_NO_SURFACE))
    {
        SetGlobalError(Error(EGL_NOT_INITIALIZED, "'dpy' not initialized"));
        return EGL_FALSE;
    }

    if (ctx != EGL_NO_CONTEXT)
    {
        Error error = ValidateContext(display, context);
        if (error.isError())
        {
            SetGlobalError(error);
            return EGL_FALSE;
        }
    }

    if (display->isInitialized())
    {
        if (display->testDeviceLost())
        {
            display->notifyDeviceLost();
            return EGL_FALSE;
        }

        if (display->isDeviceLost())
        {
            SetGlobalError(Error(EGL_CONTEXT_LOST));
            return EGL_FALSE;
        }
    }

    Surface *drawSurface = static_cast<Surface*>(draw);
    if (draw != EGL_NO_SURFACE)
    {
        Error error = ValidateSurface(display, drawSurface);
        if (error.isError())
        {
            SetGlobalError(error);
            return EGL_FALSE;
        }
    }

    Surface *readSurface = static_cast<Surface*>(read);
    if (read != EGL_NO_SURFACE)
    {
        Error error = ValidateSurface(display, readSurface);
        if (error.isError())
        {
            SetGlobalError(error);
            return EGL_FALSE;
        }
    }

    if (readSurface)
    {
        Error readCompatError = ValidateCompatibleConfigs(readSurface->getConfig(), context->getConfig(), readSurface->getType());
        if (readCompatError.isError())
        {
            SetGlobalError(readCompatError);
            return EGL_FALSE;
        }
    }

    if (draw != read)
    {
        UNIMPLEMENTED();   // FIXME

        if (drawSurface)
        {
            Error drawCompatError = ValidateCompatibleConfigs(drawSurface->getConfig(), context->getConfig(), drawSurface->getType());
            if (drawCompatError.isError())
            {
                SetGlobalError(drawCompatError);
                return EGL_FALSE;
            }
        }
    }

    Error makeCurrentError = display->makeCurrent(drawSurface, readSurface, context);
    if (makeCurrentError.isError())
    {
        SetGlobalError(makeCurrentError);
        return EGL_FALSE;
    }

    gl::Context *previousContext = GetGlobalContext();

    SetGlobalDisplay(display);
    SetGlobalDrawSurface(drawSurface);
    SetGlobalReadSurface(readSurface);
    SetGlobalContext(context);

    // Release the surface from the previously-current context, to allow
    // destroyed surfaces to delete themselves.
    if (previousContext != nullptr && context != previousContext)
    {
        previousContext->releaseSurface();
    }

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLSurface EGLAPIENTRY GetCurrentSurface(EGLint readdraw)
{
    EVENT("(EGLint readdraw = %d)", readdraw);

    if (readdraw == EGL_READ)
    {
        SetGlobalError(Error(EGL_SUCCESS));
        return GetGlobalReadSurface();
    }
    else if (readdraw == EGL_DRAW)
    {
        SetGlobalError(Error(EGL_SUCCESS));
        return GetGlobalDrawSurface();
    }
    else
    {
        SetGlobalError(Error(EGL_BAD_PARAMETER));
        return EGL_NO_SURFACE;
    }
}

EGLDisplay EGLAPIENTRY GetCurrentDisplay(void)
{
    EVENT("()");

    EGLDisplay dpy = GetGlobalDisplay();

    SetGlobalError(Error(EGL_SUCCESS));
    return dpy;
}

EGLBoolean EGLAPIENTRY QueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLContext ctx = 0x%0.8p, EGLint attribute = %d, EGLint *value = 0x%0.8p)",
          dpy, ctx, attribute, value);

    Display *display = static_cast<Display*>(dpy);
    gl::Context *context = static_cast<gl::Context*>(ctx);

    Error error = ValidateContext(display, context);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    switch (attribute)
    {
      case EGL_CONFIG_ID:
        *value = context->getConfig()->configID;
        break;
      case EGL_CONTEXT_CLIENT_TYPE:
        *value = context->getClientType();
        break;
      case EGL_CONTEXT_CLIENT_VERSION:
        *value = context->getClientVersion();
        break;
      case EGL_RENDER_BUFFER:
        *value = context->getRenderBuffer();
        break;
      default:
        SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
        return EGL_FALSE;
    }

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean EGLAPIENTRY WaitGL(void)
{
    EVENT("()");

    UNIMPLEMENTED();   // FIXME

    SetGlobalError(Error(EGL_SUCCESS));
    return 0;
}

EGLBoolean EGLAPIENTRY WaitNative(EGLint engine)
{
    EVENT("(EGLint engine = %d)", engine);

    UNIMPLEMENTED();   // FIXME

    SetGlobalError(Error(EGL_SUCCESS));
    return 0;
}

EGLBoolean EGLAPIENTRY SwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p)", dpy, surface);

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = (Surface*)surface;

    Error error = ValidateSurface(display, eglSurface);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    if (display->isDeviceLost())
    {
        SetGlobalError(Error(EGL_CONTEXT_LOST));
        return EGL_FALSE;
    }

    if (surface == EGL_NO_SURFACE)
    {
        SetGlobalError(Error(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    error = eglSurface->swap();
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean EGLAPIENTRY CopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p, EGLNativePixmapType target = 0x%0.8p)", dpy, surface, target);

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = static_cast<Surface*>(surface);

    Error error = ValidateSurface(display, eglSurface);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    if (display->isDeviceLost())
    {
        SetGlobalError(Error(EGL_CONTEXT_LOST));
        return EGL_FALSE;
    }

    UNIMPLEMENTED();   // FIXME

    SetGlobalError(Error(EGL_SUCCESS));
    return 0;
}

// EGL 1.1
EGLBoolean EGLAPIENTRY BindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p, EGLint buffer = %d)", dpy, surface, buffer);

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = static_cast<Surface*>(surface);

    Error error = ValidateSurface(display, eglSurface);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    if (buffer != EGL_BACK_BUFFER)
    {
        SetGlobalError(Error(EGL_BAD_PARAMETER));
        return EGL_FALSE;
    }

    if (surface == EGL_NO_SURFACE || eglSurface->getType() == EGL_WINDOW_BIT)
    {
        SetGlobalError(Error(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    if (eglSurface->getBoundTexture())
    {
        SetGlobalError(Error(EGL_BAD_ACCESS));
        return EGL_FALSE;
    }

    if (eglSurface->getTextureFormat() == EGL_NO_TEXTURE)
    {
        SetGlobalError(Error(EGL_BAD_MATCH));
        return EGL_FALSE;
    }

    gl::Context *context = GetGlobalContext();
    if (context)
    {
        gl::Texture *textureObject = context->getTargetTexture(GL_TEXTURE_2D);
        ASSERT(textureObject != NULL);

        if (textureObject->getImmutableFormat())
        {
            SetGlobalError(Error(EGL_BAD_MATCH));
            return EGL_FALSE;
        }

        error = eglSurface->bindTexImage(textureObject, buffer);
        if (error.isError())
        {
            SetGlobalError(error);
            return EGL_FALSE;
        }
    }

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean EGLAPIENTRY SurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p, EGLint attribute = %d, EGLint value = %d)",
        dpy, surface, attribute, value);

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = static_cast<Surface*>(surface);

    Error error = ValidateSurface(display, eglSurface);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    UNIMPLEMENTED();   // FIXME

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean EGLAPIENTRY ReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p, EGLint buffer = %d)", dpy, surface, buffer);

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = static_cast<Surface*>(surface);

    Error error = ValidateSurface(display, eglSurface);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    if (buffer != EGL_BACK_BUFFER)
    {
        SetGlobalError(Error(EGL_BAD_PARAMETER));
        return EGL_FALSE;
    }

    if (surface == EGL_NO_SURFACE || eglSurface->getType() == EGL_WINDOW_BIT)
    {
        SetGlobalError(Error(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    if (eglSurface->getTextureFormat() == EGL_NO_TEXTURE)
    {
        SetGlobalError(Error(EGL_BAD_MATCH));
        return EGL_FALSE;
    }

    gl::Texture *texture = eglSurface->getBoundTexture();

    if (texture)
    {
        error = eglSurface->releaseTexImage(buffer);
        if (error.isError())
        {
            SetGlobalError(error);
            return EGL_FALSE;
        }
    }

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean EGLAPIENTRY SwapInterval(EGLDisplay dpy, EGLint interval)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLint interval = %d)", dpy, interval);

    Display *display = static_cast<Display*>(dpy);

    Error error = ValidateDisplay(display);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    Surface *draw_surface = static_cast<Surface*>(GetGlobalDrawSurface());

    if (draw_surface == NULL)
    {
        SetGlobalError(Error(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    const egl::Config *surfaceConfig = draw_surface->getConfig();
    EGLint clampedInterval = std::min(std::max(interval, surfaceConfig->minSwapInterval), surfaceConfig->maxSwapInterval);

    draw_surface->setSwapInterval(clampedInterval);

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}


// EGL 1.2
EGLBoolean EGLAPIENTRY BindAPI(EGLenum api)
{
    EVENT("(EGLenum api = 0x%X)", api);

    switch (api)
    {
      case EGL_OPENGL_API:
      case EGL_OPENVG_API:
        SetGlobalError(Error(EGL_BAD_PARAMETER));
        return EGL_FALSE;   // Not supported by this implementation
      case EGL_OPENGL_ES_API:
        break;
      default:
        SetGlobalError(Error(EGL_BAD_PARAMETER));
        return EGL_FALSE;
    }

    SetGlobalAPI(api);

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLenum EGLAPIENTRY QueryAPI(void)
{
    EVENT("()");

    EGLenum API = GetGlobalAPI();

    SetGlobalError(Error(EGL_SUCCESS));
    return API;
}

EGLSurface EGLAPIENTRY CreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLenum buftype = 0x%X, EGLClientBuffer buffer = 0x%0.8p, "
          "EGLConfig config = 0x%0.8p, const EGLint *attrib_list = 0x%0.8p)",
          dpy, buftype, buffer, config, attrib_list);

    Display *display = static_cast<Display*>(dpy);
    Config *configuration = static_cast<Config*>(config);
    AttributeMap attributes(attrib_list);

    Error error = ValidateCreatePbufferFromClientBuffer(display, buftype, buffer, configuration, attributes);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_NO_SURFACE;
    }

    egl::Surface *surface = nullptr;
    error = display->createPbufferFromClientBuffer(configuration, buffer, attributes, &surface);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_NO_SURFACE;
    }

    return static_cast<EGLSurface>(surface);
}

EGLBoolean EGLAPIENTRY ReleaseThread(void)
{
    EVENT("()");

    MakeCurrent(EGL_NO_DISPLAY, EGL_NO_CONTEXT, EGL_NO_SURFACE, EGL_NO_SURFACE);

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean EGLAPIENTRY WaitClient(void)
{
    EVENT("()");

    UNIMPLEMENTED();   // FIXME

    SetGlobalError(Error(EGL_SUCCESS));
    return 0;
}

// EGL 1.4
EGLContext EGLAPIENTRY GetCurrentContext(void)
{
    EVENT("()");

    gl::Context *context = GetGlobalContext();

    SetGlobalError(Error(EGL_SUCCESS));
    return static_cast<EGLContext>(context);
}

// EGL 1.5
EGLSync EGLAPIENTRY CreateSync(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLenum type = 0x%X, const EGLint* attrib_list = 0x%0.8p)", dpy, type, attrib_list);

    UNIMPLEMENTED();
    return EGL_NO_SYNC;
}

EGLBoolean EGLAPIENTRY DestroySync(EGLDisplay dpy, EGLSync sync)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSync sync = 0x%0.8p)", dpy, sync);

    UNIMPLEMENTED();
    return EGL_FALSE;
}

EGLint EGLAPIENTRY ClientWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSync sync = 0x%0.8p, EGLint flags = 0x%X, EGLTime timeout = %d)", dpy, sync, flags, timeout);

    UNIMPLEMENTED();
    return 0;
}

EGLBoolean EGLAPIENTRY GetSyncAttrib(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib *value)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSync sync = 0x%0.8p, EGLint attribute = 0x%X, EGLAttrib *value = 0x%0.8p)", dpy, sync, attribute, value);

    UNIMPLEMENTED();
    return EGL_FALSE;
}

EGLImage EGLAPIENTRY CreateImage(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib *attrib_list)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLContext ctx = 0x%0.8p, EGLenum target = 0x%X, "
          "EGLClientBuffer buffer = 0x%0.8p, const EGLAttrib *attrib_list = 0x%0.8p)",
          dpy, ctx, target, buffer, attrib_list);

    UNIMPLEMENTED();
    return EGL_NO_IMAGE;
}

EGLBoolean EGLAPIENTRY DestroyImage(EGLDisplay dpy, EGLImage image)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLImage image = 0x%0.8p)", dpy, image);

    UNIMPLEMENTED();
    return EGL_FALSE;
}

EGLDisplay EGLAPIENTRY GetPlatformDisplay(EGLenum platform, void *native_display, const EGLAttrib *attrib_list)
{
    EVENT("(EGLenum platform = %d, void* native_display = 0x%0.8p, const EGLint* attrib_list = 0x%0.8p)",
          platform, native_display, attrib_list);

    UNIMPLEMENTED();
    return EGL_NO_DISPLAY;
}

EGLSurface EGLAPIENTRY CreatePlatformWindowSurface(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLAttrib *attrib_list)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLConfig config = 0x%0.8p, void* native_window = 0x%0.8p, const EGLint* attrib_list = 0x%0.8p)",
          dpy, config, native_window, attrib_list);

    UNIMPLEMENTED();
    return EGL_NO_SURFACE;
}

EGLSurface EGLAPIENTRY CreatePlatformPixmapSurface(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLAttrib *attrib_list)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLConfig config = 0x%0.8p, void* native_pixmap = 0x%0.8p, const EGLint* attrib_list = 0x%0.8p)",
          dpy, config, native_pixmap, attrib_list);

    UNIMPLEMENTED();
    return EGL_NO_SURFACE;
}

EGLBoolean EGLAPIENTRY WaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSync sync = 0x%0.8p, EGLint flags = 0x%X)", dpy, sync, flags);

    UNIMPLEMENTED();
    return EGL_FALSE;
}

__eglMustCastToProperFunctionPointerType EGLAPIENTRY GetProcAddress(const char *procname)
{
    EVENT("(const char *procname = \"%s\")", procname);

    typedef std::map<std::string, __eglMustCastToProperFunctionPointerType> ProcAddressMap;
    auto generateProcAddressMap = []()
    {
        ProcAddressMap map;
#define INSERT_PROC_ADDRESS(ns, proc) \
    map[#ns #proc] = reinterpret_cast<__eglMustCastToProperFunctionPointerType>(ns::proc)

        // GLES2 core
        INSERT_PROC_ADDRESS(gl, ActiveTexture);
        INSERT_PROC_ADDRESS(gl, AttachShader);
        INSERT_PROC_ADDRESS(gl, BindAttribLocation);
        INSERT_PROC_ADDRESS(gl, BindBuffer);
        INSERT_PROC_ADDRESS(gl, BindFramebuffer);
        INSERT_PROC_ADDRESS(gl, BindRenderbuffer);
        INSERT_PROC_ADDRESS(gl, BindTexture);
        INSERT_PROC_ADDRESS(gl, BlendColor);
        INSERT_PROC_ADDRESS(gl, BlendEquation);
        INSERT_PROC_ADDRESS(gl, BlendEquationSeparate);
        INSERT_PROC_ADDRESS(gl, BlendFunc);
        INSERT_PROC_ADDRESS(gl, BlendFuncSeparate);
        INSERT_PROC_ADDRESS(gl, BufferData);
        INSERT_PROC_ADDRESS(gl, BufferSubData);
        INSERT_PROC_ADDRESS(gl, CheckFramebufferStatus);
        INSERT_PROC_ADDRESS(gl, Clear);
        INSERT_PROC_ADDRESS(gl, ClearColor);
        INSERT_PROC_ADDRESS(gl, ClearDepthf);
        INSERT_PROC_ADDRESS(gl, ClearStencil);
        INSERT_PROC_ADDRESS(gl, ColorMask);
        INSERT_PROC_ADDRESS(gl, CompileShader);
        INSERT_PROC_ADDRESS(gl, CompressedTexImage2D);
        INSERT_PROC_ADDRESS(gl, CompressedTexSubImage2D);
        INSERT_PROC_ADDRESS(gl, CopyTexImage2D);
        INSERT_PROC_ADDRESS(gl, CopyTexSubImage2D);
        INSERT_PROC_ADDRESS(gl, CreateProgram);
        INSERT_PROC_ADDRESS(gl, CreateShader);
        INSERT_PROC_ADDRESS(gl, CullFace);
        INSERT_PROC_ADDRESS(gl, DeleteBuffers);
        INSERT_PROC_ADDRESS(gl, DeleteFramebuffers);
        INSERT_PROC_ADDRESS(gl, DeleteProgram);
        INSERT_PROC_ADDRESS(gl, DeleteRenderbuffers);
        INSERT_PROC_ADDRESS(gl, DeleteShader);
        INSERT_PROC_ADDRESS(gl, DeleteTextures);
        INSERT_PROC_ADDRESS(gl, DepthFunc);
        INSERT_PROC_ADDRESS(gl, DepthMask);
        INSERT_PROC_ADDRESS(gl, DepthRangef);
        INSERT_PROC_ADDRESS(gl, DetachShader);
        INSERT_PROC_ADDRESS(gl, Disable);
        INSERT_PROC_ADDRESS(gl, DisableVertexAttribArray);
        INSERT_PROC_ADDRESS(gl, DrawArrays);
        INSERT_PROC_ADDRESS(gl, DrawElements);
        INSERT_PROC_ADDRESS(gl, Enable);
        INSERT_PROC_ADDRESS(gl, EnableVertexAttribArray);
        INSERT_PROC_ADDRESS(gl, Finish);
        INSERT_PROC_ADDRESS(gl, Flush);
        INSERT_PROC_ADDRESS(gl, FramebufferRenderbuffer);
        INSERT_PROC_ADDRESS(gl, FramebufferTexture2D);
        INSERT_PROC_ADDRESS(gl, FrontFace);
        INSERT_PROC_ADDRESS(gl, GenBuffers);
        INSERT_PROC_ADDRESS(gl, GenerateMipmap);
        INSERT_PROC_ADDRESS(gl, GenFramebuffers);
        INSERT_PROC_ADDRESS(gl, GenRenderbuffers);
        INSERT_PROC_ADDRESS(gl, GenTextures);
        INSERT_PROC_ADDRESS(gl, GetActiveAttrib);
        INSERT_PROC_ADDRESS(gl, GetActiveUniform);
        INSERT_PROC_ADDRESS(gl, GetAttachedShaders);
        INSERT_PROC_ADDRESS(gl, GetAttribLocation);
        INSERT_PROC_ADDRESS(gl, GetBooleanv);
        INSERT_PROC_ADDRESS(gl, GetBufferParameteriv);
        INSERT_PROC_ADDRESS(gl, GetError);
        INSERT_PROC_ADDRESS(gl, GetFloatv);
        INSERT_PROC_ADDRESS(gl, GetFramebufferAttachmentParameteriv);
        INSERT_PROC_ADDRESS(gl, GetIntegerv);
        INSERT_PROC_ADDRESS(gl, GetProgramiv);
        INSERT_PROC_ADDRESS(gl, GetProgramInfoLog);
        INSERT_PROC_ADDRESS(gl, GetRenderbufferParameteriv);
        INSERT_PROC_ADDRESS(gl, GetShaderiv);
        INSERT_PROC_ADDRESS(gl, GetShaderInfoLog);
        INSERT_PROC_ADDRESS(gl, GetShaderPrecisionFormat);
        INSERT_PROC_ADDRESS(gl, GetShaderSource);
        INSERT_PROC_ADDRESS(gl, GetString);
        INSERT_PROC_ADDRESS(gl, GetTexParameterfv);
        INSERT_PROC_ADDRESS(gl, GetTexParameteriv);
        INSERT_PROC_ADDRESS(gl, GetUniformfv);
        INSERT_PROC_ADDRESS(gl, GetUniformiv);
        INSERT_PROC_ADDRESS(gl, GetUniformLocation);
        INSERT_PROC_ADDRESS(gl, GetVertexAttribfv);
        INSERT_PROC_ADDRESS(gl, GetVertexAttribiv);
        INSERT_PROC_ADDRESS(gl, GetVertexAttribPointerv);
        INSERT_PROC_ADDRESS(gl, Hint);
        INSERT_PROC_ADDRESS(gl, IsBuffer);
        INSERT_PROC_ADDRESS(gl, IsEnabled);
        INSERT_PROC_ADDRESS(gl, IsFramebuffer);
        INSERT_PROC_ADDRESS(gl, IsProgram);
        INSERT_PROC_ADDRESS(gl, IsRenderbuffer);
        INSERT_PROC_ADDRESS(gl, IsShader);
        INSERT_PROC_ADDRESS(gl, IsTexture);
        INSERT_PROC_ADDRESS(gl, LineWidth);
        INSERT_PROC_ADDRESS(gl, LinkProgram);
        INSERT_PROC_ADDRESS(gl, PixelStorei);
        INSERT_PROC_ADDRESS(gl, PolygonOffset);
        INSERT_PROC_ADDRESS(gl, ReadPixels);
        INSERT_PROC_ADDRESS(gl, ReleaseShaderCompiler);
        INSERT_PROC_ADDRESS(gl, RenderbufferStorage);
        INSERT_PROC_ADDRESS(gl, SampleCoverage);
        INSERT_PROC_ADDRESS(gl, Scissor);
        INSERT_PROC_ADDRESS(gl, ShaderBinary);
        INSERT_PROC_ADDRESS(gl, ShaderSource);
        INSERT_PROC_ADDRESS(gl, StencilFunc);
        INSERT_PROC_ADDRESS(gl, StencilFuncSeparate);
        INSERT_PROC_ADDRESS(gl, StencilMask);
        INSERT_PROC_ADDRESS(gl, StencilMaskSeparate);
        INSERT_PROC_ADDRESS(gl, StencilOp);
        INSERT_PROC_ADDRESS(gl, StencilOpSeparate);
        INSERT_PROC_ADDRESS(gl, TexImage2D);
        INSERT_PROC_ADDRESS(gl, TexParameterf);
        INSERT_PROC_ADDRESS(gl, TexParameterfv);
        INSERT_PROC_ADDRESS(gl, TexParameteri);
        INSERT_PROC_ADDRESS(gl, TexParameteriv);
        INSERT_PROC_ADDRESS(gl, TexSubImage2D);
        INSERT_PROC_ADDRESS(gl, Uniform1f);
        INSERT_PROC_ADDRESS(gl, Uniform1fv);
        INSERT_PROC_ADDRESS(gl, Uniform1i);
        INSERT_PROC_ADDRESS(gl, Uniform1iv);
        INSERT_PROC_ADDRESS(gl, Uniform2f);
        INSERT_PROC_ADDRESS(gl, Uniform2fv);
        INSERT_PROC_ADDRESS(gl, Uniform2i);
        INSERT_PROC_ADDRESS(gl, Uniform2iv);
        INSERT_PROC_ADDRESS(gl, Uniform3f);
        INSERT_PROC_ADDRESS(gl, Uniform3fv);
        INSERT_PROC_ADDRESS(gl, Uniform3i);
        INSERT_PROC_ADDRESS(gl, Uniform3iv);
        INSERT_PROC_ADDRESS(gl, Uniform4f);
        INSERT_PROC_ADDRESS(gl, Uniform4fv);
        INSERT_PROC_ADDRESS(gl, Uniform4i);
        INSERT_PROC_ADDRESS(gl, Uniform4iv);
        INSERT_PROC_ADDRESS(gl, UniformMatrix2fv);
        INSERT_PROC_ADDRESS(gl, UniformMatrix3fv);
        INSERT_PROC_ADDRESS(gl, UniformMatrix4fv);
        INSERT_PROC_ADDRESS(gl, UseProgram);
        INSERT_PROC_ADDRESS(gl, ValidateProgram);
        INSERT_PROC_ADDRESS(gl, VertexAttrib1f);
        INSERT_PROC_ADDRESS(gl, VertexAttrib1fv);
        INSERT_PROC_ADDRESS(gl, VertexAttrib2f);
        INSERT_PROC_ADDRESS(gl, VertexAttrib2fv);
        INSERT_PROC_ADDRESS(gl, VertexAttrib3f);
        INSERT_PROC_ADDRESS(gl, VertexAttrib3fv);
        INSERT_PROC_ADDRESS(gl, VertexAttrib4f);
        INSERT_PROC_ADDRESS(gl, VertexAttrib4fv);
        INSERT_PROC_ADDRESS(gl, VertexAttribPointer);
        INSERT_PROC_ADDRESS(gl, Viewport);

        // GL_ANGLE_framebuffer_blit
        INSERT_PROC_ADDRESS(gl, BlitFramebufferANGLE);

        // GL_ANGLE_framebuffer_multisample
        INSERT_PROC_ADDRESS(gl, RenderbufferStorageMultisampleANGLE);

        // GL_EXT_discard_framebuffer
        INSERT_PROC_ADDRESS(gl, DiscardFramebufferEXT);

        // GL_NV_fence
        INSERT_PROC_ADDRESS(gl, DeleteFencesNV);
        INSERT_PROC_ADDRESS(gl, GenFencesNV);
        INSERT_PROC_ADDRESS(gl, IsFenceNV);
        INSERT_PROC_ADDRESS(gl, TestFenceNV);
        INSERT_PROC_ADDRESS(gl, GetFenceivNV);
        INSERT_PROC_ADDRESS(gl, FinishFenceNV);
        INSERT_PROC_ADDRESS(gl, SetFenceNV);

        // GL_ANGLE_translated_shader_source
        INSERT_PROC_ADDRESS(gl, GetTranslatedShaderSourceANGLE);

        // GL_EXT_texture_storage
        INSERT_PROC_ADDRESS(gl, TexStorage2DEXT);

        // GL_EXT_robustness
        INSERT_PROC_ADDRESS(gl, GetGraphicsResetStatusEXT);
        INSERT_PROC_ADDRESS(gl, ReadnPixelsEXT);
        INSERT_PROC_ADDRESS(gl, GetnUniformfvEXT);
        INSERT_PROC_ADDRESS(gl, GetnUniformivEXT);

        // GL_EXT_occlusion_query_boolean
        INSERT_PROC_ADDRESS(gl, GenQueriesEXT);
        INSERT_PROC_ADDRESS(gl, DeleteQueriesEXT);
        INSERT_PROC_ADDRESS(gl, IsQueryEXT);
        INSERT_PROC_ADDRESS(gl, BeginQueryEXT);
        INSERT_PROC_ADDRESS(gl, EndQueryEXT);
        INSERT_PROC_ADDRESS(gl, GetQueryivEXT);
        INSERT_PROC_ADDRESS(gl, GetQueryObjectuivEXT);

        // GL_EXT_draw_buffers
        INSERT_PROC_ADDRESS(gl, DrawBuffersEXT);

        // GL_ANGLE_instanced_arrays
        INSERT_PROC_ADDRESS(gl, DrawArraysInstancedANGLE);
        INSERT_PROC_ADDRESS(gl, DrawElementsInstancedANGLE);
        INSERT_PROC_ADDRESS(gl, VertexAttribDivisorANGLE);

        // GL_OES_get_program_binary
        INSERT_PROC_ADDRESS(gl, GetProgramBinaryOES);
        INSERT_PROC_ADDRESS(gl, ProgramBinaryOES);

        // GL_OES_mapbuffer
        INSERT_PROC_ADDRESS(gl, MapBufferOES);
        INSERT_PROC_ADDRESS(gl, UnmapBufferOES);
        INSERT_PROC_ADDRESS(gl, GetBufferPointervOES);

        // GL_EXT_map_buffer_range
        INSERT_PROC_ADDRESS(gl, MapBufferRangeEXT);
        INSERT_PROC_ADDRESS(gl, FlushMappedBufferRangeEXT);

        // GL_EXT_debug_marker
        INSERT_PROC_ADDRESS(gl, InsertEventMarkerEXT);
        INSERT_PROC_ADDRESS(gl, PushGroupMarkerEXT);
        INSERT_PROC_ADDRESS(gl, PopGroupMarkerEXT);

        // GL_OES_EGL_image
        INSERT_PROC_ADDRESS(gl, EGLImageTargetTexture2DOES);
        INSERT_PROC_ADDRESS(gl, EGLImageTargetRenderbufferStorageOES);

        // GLES3 core
        INSERT_PROC_ADDRESS(gl, ReadBuffer);
        INSERT_PROC_ADDRESS(gl, DrawRangeElements);
        INSERT_PROC_ADDRESS(gl, TexImage3D);
        INSERT_PROC_ADDRESS(gl, TexSubImage3D);
        INSERT_PROC_ADDRESS(gl, CopyTexSubImage3D);
        INSERT_PROC_ADDRESS(gl, CompressedTexImage3D);
        INSERT_PROC_ADDRESS(gl, CompressedTexSubImage3D);
        INSERT_PROC_ADDRESS(gl, GenQueries);
        INSERT_PROC_ADDRESS(gl, DeleteQueries);
        INSERT_PROC_ADDRESS(gl, IsQuery);
        INSERT_PROC_ADDRESS(gl, BeginQuery);
        INSERT_PROC_ADDRESS(gl, EndQuery);
        INSERT_PROC_ADDRESS(gl, GetQueryiv);
        INSERT_PROC_ADDRESS(gl, GetQueryObjectuiv);
        INSERT_PROC_ADDRESS(gl, UnmapBuffer);
        INSERT_PROC_ADDRESS(gl, GetBufferPointerv);
        INSERT_PROC_ADDRESS(gl, DrawBuffers);
        INSERT_PROC_ADDRESS(gl, UniformMatrix2x3fv);
        INSERT_PROC_ADDRESS(gl, UniformMatrix3x2fv);
        INSERT_PROC_ADDRESS(gl, UniformMatrix2x4fv);
        INSERT_PROC_ADDRESS(gl, UniformMatrix4x2fv);
        INSERT_PROC_ADDRESS(gl, UniformMatrix3x4fv);
        INSERT_PROC_ADDRESS(gl, UniformMatrix4x3fv);
        INSERT_PROC_ADDRESS(gl, BlitFramebuffer);
        INSERT_PROC_ADDRESS(gl, RenderbufferStorageMultisample);
        INSERT_PROC_ADDRESS(gl, FramebufferTextureLayer);
        INSERT_PROC_ADDRESS(gl, MapBufferRange);
        INSERT_PROC_ADDRESS(gl, FlushMappedBufferRange);
        INSERT_PROC_ADDRESS(gl, BindVertexArray);
        INSERT_PROC_ADDRESS(gl, DeleteVertexArrays);
        INSERT_PROC_ADDRESS(gl, GenVertexArrays);
        INSERT_PROC_ADDRESS(gl, IsVertexArray);
        INSERT_PROC_ADDRESS(gl, GetIntegeri_v);
        INSERT_PROC_ADDRESS(gl, BeginTransformFeedback);
        INSERT_PROC_ADDRESS(gl, EndTransformFeedback);
        INSERT_PROC_ADDRESS(gl, BindBufferRange);
        INSERT_PROC_ADDRESS(gl, BindBufferBase);
        INSERT_PROC_ADDRESS(gl, TransformFeedbackVaryings);
        INSERT_PROC_ADDRESS(gl, GetTransformFeedbackVarying);
        INSERT_PROC_ADDRESS(gl, VertexAttribIPointer);
        INSERT_PROC_ADDRESS(gl, GetVertexAttribIiv);
        INSERT_PROC_ADDRESS(gl, GetVertexAttribIuiv);
        INSERT_PROC_ADDRESS(gl, VertexAttribI4i);
        INSERT_PROC_ADDRESS(gl, VertexAttribI4ui);
        INSERT_PROC_ADDRESS(gl, VertexAttribI4iv);
        INSERT_PROC_ADDRESS(gl, VertexAttribI4uiv);
        INSERT_PROC_ADDRESS(gl, GetUniformuiv);
        INSERT_PROC_ADDRESS(gl, GetFragDataLocation);
        INSERT_PROC_ADDRESS(gl, Uniform1ui);
        INSERT_PROC_ADDRESS(gl, Uniform2ui);
        INSERT_PROC_ADDRESS(gl, Uniform3ui);
        INSERT_PROC_ADDRESS(gl, Uniform4ui);
        INSERT_PROC_ADDRESS(gl, Uniform1uiv);
        INSERT_PROC_ADDRESS(gl, Uniform2uiv);
        INSERT_PROC_ADDRESS(gl, Uniform3uiv);
        INSERT_PROC_ADDRESS(gl, Uniform4uiv);
        INSERT_PROC_ADDRESS(gl, ClearBufferiv);
        INSERT_PROC_ADDRESS(gl, ClearBufferuiv);
        INSERT_PROC_ADDRESS(gl, ClearBufferfv);
        INSERT_PROC_ADDRESS(gl, ClearBufferfi);
        INSERT_PROC_ADDRESS(gl, GetStringi);
        INSERT_PROC_ADDRESS(gl, CopyBufferSubData);
        INSERT_PROC_ADDRESS(gl, GetUniformIndices);
        INSERT_PROC_ADDRESS(gl, GetActiveUniformsiv);
        INSERT_PROC_ADDRESS(gl, GetUniformBlockIndex);
        INSERT_PROC_ADDRESS(gl, GetActiveUniformBlockiv);
        INSERT_PROC_ADDRESS(gl, GetActiveUniformBlockName);
        INSERT_PROC_ADDRESS(gl, UniformBlockBinding);
        INSERT_PROC_ADDRESS(gl, DrawArraysInstanced);
        INSERT_PROC_ADDRESS(gl, DrawElementsInstanced);
        map["glFenceSync"] =
            reinterpret_cast<__eglMustCastToProperFunctionPointerType>(gl::FenceSync_);
        INSERT_PROC_ADDRESS(gl, IsSync);
        INSERT_PROC_ADDRESS(gl, DeleteSync);
        INSERT_PROC_ADDRESS(gl, ClientWaitSync);
        INSERT_PROC_ADDRESS(gl, WaitSync);
        INSERT_PROC_ADDRESS(gl, GetInteger64v);
        INSERT_PROC_ADDRESS(gl, GetSynciv);
        INSERT_PROC_ADDRESS(gl, GetInteger64i_v);
        INSERT_PROC_ADDRESS(gl, GetBufferParameteri64v);
        INSERT_PROC_ADDRESS(gl, GenSamplers);
        INSERT_PROC_ADDRESS(gl, DeleteSamplers);
        INSERT_PROC_ADDRESS(gl, IsSampler);
        INSERT_PROC_ADDRESS(gl, BindSampler);
        INSERT_PROC_ADDRESS(gl, SamplerParameteri);
        INSERT_PROC_ADDRESS(gl, SamplerParameteriv);
        INSERT_PROC_ADDRESS(gl, SamplerParameterf);
        INSERT_PROC_ADDRESS(gl, SamplerParameterfv);
        INSERT_PROC_ADDRESS(gl, GetSamplerParameteriv);
        INSERT_PROC_ADDRESS(gl, GetSamplerParameterfv);
        INSERT_PROC_ADDRESS(gl, VertexAttribDivisor);
        INSERT_PROC_ADDRESS(gl, BindTransformFeedback);
        INSERT_PROC_ADDRESS(gl, DeleteTransformFeedbacks);
        INSERT_PROC_ADDRESS(gl, GenTransformFeedbacks);
        INSERT_PROC_ADDRESS(gl, IsTransformFeedback);
        INSERT_PROC_ADDRESS(gl, PauseTransformFeedback);
        INSERT_PROC_ADDRESS(gl, ResumeTransformFeedback);
        INSERT_PROC_ADDRESS(gl, GetProgramBinary);
        INSERT_PROC_ADDRESS(gl, ProgramBinary);
        INSERT_PROC_ADDRESS(gl, ProgramParameteri);
        INSERT_PROC_ADDRESS(gl, InvalidateFramebuffer);
        INSERT_PROC_ADDRESS(gl, InvalidateSubFramebuffer);
        INSERT_PROC_ADDRESS(gl, TexStorage2D);
        INSERT_PROC_ADDRESS(gl, TexStorage3D);
        INSERT_PROC_ADDRESS(gl, GetInternalformativ);

        // EGL 1.0
        INSERT_PROC_ADDRESS(egl, ChooseConfig);
        INSERT_PROC_ADDRESS(egl, CopyBuffers);
        INSERT_PROC_ADDRESS(egl, CreateContext);
        INSERT_PROC_ADDRESS(egl, CreatePbufferSurface);
        INSERT_PROC_ADDRESS(egl, CreatePixmapSurface);
        INSERT_PROC_ADDRESS(egl, CreateWindowSurface);
        INSERT_PROC_ADDRESS(egl, DestroyContext);
        INSERT_PROC_ADDRESS(egl, DestroySurface);
        INSERT_PROC_ADDRESS(egl, GetConfigAttrib);
        INSERT_PROC_ADDRESS(egl, GetConfigs);
        INSERT_PROC_ADDRESS(egl, GetCurrentDisplay);
        INSERT_PROC_ADDRESS(egl, GetCurrentSurface);
        INSERT_PROC_ADDRESS(egl, GetDisplay);
        INSERT_PROC_ADDRESS(egl, GetError);
        INSERT_PROC_ADDRESS(egl, GetProcAddress);
        INSERT_PROC_ADDRESS(egl, Initialize);
        INSERT_PROC_ADDRESS(egl, MakeCurrent);
        INSERT_PROC_ADDRESS(egl, QueryContext);
        INSERT_PROC_ADDRESS(egl, QueryString);
        INSERT_PROC_ADDRESS(egl, QuerySurface);
        INSERT_PROC_ADDRESS(egl, SwapBuffers);
        INSERT_PROC_ADDRESS(egl, Terminate);
        INSERT_PROC_ADDRESS(egl, WaitGL);
        INSERT_PROC_ADDRESS(egl, WaitNative);

        // EGL 1.1
        INSERT_PROC_ADDRESS(egl, BindTexImage);
        INSERT_PROC_ADDRESS(egl, ReleaseTexImage);
        INSERT_PROC_ADDRESS(egl, SurfaceAttrib);
        INSERT_PROC_ADDRESS(egl, SwapInterval);

        // EGL 1.2
        INSERT_PROC_ADDRESS(egl, BindAPI);
        INSERT_PROC_ADDRESS(egl, QueryAPI);
        INSERT_PROC_ADDRESS(egl, CreatePbufferFromClientBuffer);
        INSERT_PROC_ADDRESS(egl, ReleaseThread);
        INSERT_PROC_ADDRESS(egl, WaitClient);

        // EGL 1.4
        INSERT_PROC_ADDRESS(egl, GetCurrentContext);

        // EGL 1.5
        INSERT_PROC_ADDRESS(egl, CreateSync);
        INSERT_PROC_ADDRESS(egl, DestroySync);
        INSERT_PROC_ADDRESS(egl, ClientWaitSync);
        INSERT_PROC_ADDRESS(egl, GetSyncAttrib);
        INSERT_PROC_ADDRESS(egl, CreateImage);
        INSERT_PROC_ADDRESS(egl, DestroyImage);
        INSERT_PROC_ADDRESS(egl, GetPlatformDisplay);
        INSERT_PROC_ADDRESS(egl, CreatePlatformWindowSurface);
        INSERT_PROC_ADDRESS(egl, CreatePlatformPixmapSurface);
        INSERT_PROC_ADDRESS(egl, WaitSync);

        // EGL_ANGLE_query_surface_pointer
        INSERT_PROC_ADDRESS(egl, QuerySurfacePointerANGLE);

        // EGL_NV_post_sub_buffer
        INSERT_PROC_ADDRESS(egl, PostSubBufferNV);

        // EGL_EXT_platform_base
        INSERT_PROC_ADDRESS(egl, GetPlatformDisplayEXT);

        // EGL_EXT_device_query
        INSERT_PROC_ADDRESS(egl, QueryDisplayAttribEXT);
        INSERT_PROC_ADDRESS(egl, QueryDeviceAttribEXT);
        INSERT_PROC_ADDRESS(egl, QueryDeviceStringEXT);

        // EGL_KHR_image_base/EGL_KHR_image
        INSERT_PROC_ADDRESS(egl, CreateImageKHR);
        INSERT_PROC_ADDRESS(egl, DestroyImageKHR);

#undef INSERT_PROC_ADDRESS
        return map;
    };

    static const ProcAddressMap procAddressMap = generateProcAddressMap();

    auto iter = procAddressMap.find(procname);
    if (iter != procAddressMap.end())
    {
        return iter->second;
    }
    else
    {
        return nullptr;
    }
}

}
