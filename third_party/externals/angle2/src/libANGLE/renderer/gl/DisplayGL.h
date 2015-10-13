//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayGL.h: Defines the class interface for DisplayGL.

#ifndef LIBANGLE_RENDERER_GL_DISPLAYGL_H_
#define LIBANGLE_RENDERER_GL_DISPLAYGL_H_

#include "libANGLE/renderer/DisplayImpl.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"

namespace rx
{

class RendererGL;

class DisplayGL : public DisplayImpl
{
  public:
    DisplayGL();
    ~DisplayGL() override;

    egl::Error initialize(egl::Display *display) override;
    void terminate() override;

    ImageImpl *createImage(EGLenum target,
                           egl::ImageSibling *buffer,
                           const egl::AttributeMap &attribs) override;

    egl::Error createContext(const egl::Config *config, const gl::Context *shareContext, const egl::AttributeMap &attribs,
                             gl::Context **outContext) override;

    egl::Error makeCurrent(egl::Surface *drawSurface, egl::Surface *readSurface, gl::Context *context) override;

  protected:
    RendererGL *getRenderer() const { return mRenderer; };
    const gl::Version &getMaxSupportedESVersion() const;

  private:
    virtual const FunctionsGL *getFunctionsGL() const = 0;

    RendererGL *mRenderer;
};

}

#endif // LIBANGLE_RENDERER_GL_DISPLAYGL_H_
