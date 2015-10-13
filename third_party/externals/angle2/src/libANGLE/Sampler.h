//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Sampler.h : Defines the Sampler class, which represents a GLES 3
// sampler object. Sampler objects store some state needed to sample textures.

#ifndef LIBANGLE_SAMPLER_H_
#define LIBANGLE_SAMPLER_H_

#include "libANGLE/angletypes.h"
#include "libANGLE/RefCountObject.h"

namespace rx
{
class ImplFactory;
class SamplerImpl;
}

namespace gl
{

class Sampler final : public RefCountObject
{
  public:
    Sampler(rx::ImplFactory *factory, GLuint id);
    ~Sampler() override;

    void setMinFilter(GLenum minFilter);
    GLenum getMinFilter() const;

    void setMagFilter(GLenum magFilter);
    GLenum getMagFilter() const;

    void setWrapS(GLenum wrapS);
    GLenum getWrapS() const;

    void setWrapT(GLenum wrapT);
    GLenum getWrapT() const;

    void setWrapR(GLenum wrapR);
    GLenum getWrapR() const;

    void setMaxAnisotropy(float maxAnisotropy);
    float getMaxAnisotropy() const;

    void setMinLod(GLfloat minLod);
    GLfloat getMinLod() const;

    void setMaxLod(GLfloat maxLod);
    GLfloat getMaxLod() const;

    void setCompareMode(GLenum compareMode);
    GLenum getCompareMode() const;

    void setCompareFunc(GLenum compareFunc);
    GLenum getCompareFunc() const;

    const SamplerState &getSamplerState() const;

    const rx::SamplerImpl *getImplementation() const;
    rx::SamplerImpl *getImplementation();

  private:
    rx::SamplerImpl *mImpl;

    SamplerState mSamplerState;
};

}

#endif // LIBANGLE_SAMPLER_H_
