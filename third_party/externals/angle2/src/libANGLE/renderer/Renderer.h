//
// Copyright (c) 2012-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Renderer.h: Defines a back-end specific class that hides the details of the
// implementation-specific renderer.

#ifndef LIBANGLE_RENDERER_RENDERER_H_
#define LIBANGLE_RENDERER_RENDERER_H_

#include "libANGLE/Caps.h"
#include "libANGLE/Error.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/State.h"
#include "libANGLE/Uniform.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/ImplFactory.h"
#include "common/mathutil.h"

#include <stdint.h>

#include <EGL/egl.h>

namespace egl
{
class AttributeMap;
class Display;
class Surface;
}

namespace rx
{
struct TranslatedIndexData;
struct SourceIndexData;
struct WorkaroundsD3D;
class DisplayImpl;

class Renderer : public ImplFactory
{
  public:
    Renderer();
    virtual ~Renderer();

    virtual gl::Error flush() = 0;
    virtual gl::Error finish() = 0;

    virtual gl::Error drawArrays(const gl::Data &data, GLenum mode, GLint first, GLsizei count) = 0;
    virtual gl::Error drawArraysInstanced(const gl::Data &data,
                                          GLenum mode,
                                          GLint first,
                                          GLsizei count,
                                          GLsizei instanceCount) = 0;

    virtual gl::Error drawElements(const gl::Data &data,
                                   GLenum mode,
                                   GLsizei count,
                                   GLenum type,
                                   const GLvoid *indices,
                                   const gl::IndexRange &indexRange) = 0;
    virtual gl::Error drawElementsInstanced(const gl::Data &data,
                                            GLenum mode,
                                            GLsizei count,
                                            GLenum type,
                                            const GLvoid *indices,
                                            GLsizei instances,
                                            const gl::IndexRange &indexRange) = 0;
    virtual gl::Error drawRangeElements(const gl::Data &data,
                                        GLenum mode,
                                        GLuint start,
                                        GLuint end,
                                        GLsizei count,
                                        GLenum type,
                                        const GLvoid *indices,
                                        const gl::IndexRange &indexRange) = 0;

    // lost device
    //TODO(jmadill): investigate if this stuff is necessary in GL
    virtual void notifyDeviceLost() = 0;
    virtual bool isDeviceLost() const = 0;
    virtual bool testDeviceLost() = 0;
    virtual bool testDeviceResettable() = 0;

    virtual VendorID getVendorId() const = 0;
    virtual std::string getVendorString() const = 0;
    virtual std::string getRendererDescription() const = 0;

    virtual void insertEventMarker(GLsizei length, const char *marker) = 0;
    virtual void pushGroupMarker(GLsizei length, const char *marker) = 0;
    virtual void popGroupMarker() = 0;

    virtual void syncState(const gl::State &state, const gl::State::DirtyBits &dirtyBits) = 0;

    // Renderer capabilities
    const gl::Caps &getRendererCaps() const;
    const gl::TextureCapsMap &getRendererTextureCaps() const;
    const gl::Extensions &getRendererExtensions() const;
    const gl::Limitations &getRendererLimitations() const;

  private:
    void ensureCapsInitialized() const;
    virtual void generateCaps(gl::Caps *outCaps, gl::TextureCapsMap* outTextureCaps,
                              gl::Extensions *outExtensions,
                              gl::Limitations *outLimitations) const = 0;

    mutable bool mCapsInitialized;
    mutable gl::Caps mCaps;
    mutable gl::TextureCapsMap mTextureCaps;
    mutable gl::Extensions mExtensions;
    mutable gl::Limitations mLimitations;
};

}
#endif // LIBANGLE_RENDERER_RENDERER_H_
