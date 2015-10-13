//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ImageIndex.h: A helper struct for indexing into an Image array

#ifndef LIBANGLE_IMAGE_INDEX_H_
#define LIBANGLE_IMAGE_INDEX_H_

#include "common/mathutil.h"

#include "angle_gl.h"

namespace gl
{

class ImageIndexIterator;

struct ImageIndex
{
    GLenum type;
    GLint mipIndex;
    GLint layerIndex;

    ImageIndex(const ImageIndex &other);
    ImageIndex &operator=(const ImageIndex &other);

    bool hasLayer() const { return layerIndex != ENTIRE_LEVEL; }

    static ImageIndex Make2D(GLint mipIndex);
    static ImageIndex MakeCube(GLenum target, GLint mipIndex);
    static ImageIndex Make2DArray(GLint mipIndex, GLint layerIndex);
    static ImageIndex Make3D(GLint mipIndex, GLint layerIndex = ENTIRE_LEVEL);
    static ImageIndex MakeGeneric(GLenum target, GLint mipIndex);

    static ImageIndex MakeInvalid();

    static const GLint ENTIRE_LEVEL = static_cast<GLint>(-1);

    bool operator<(const ImageIndex &other) const;

  private:
    friend class ImageIndexIterator;

    ImageIndex(GLenum typeIn, GLint mipIndexIn, GLint layerIndexIn);
};

class ImageIndexIterator
{
  public:
    static ImageIndexIterator Make2D(GLint minMip, GLint maxMip);
    static ImageIndexIterator MakeCube(GLint minMip, GLint maxMip);
    static ImageIndexIterator Make3D(GLint minMip, GLint maxMip, GLint minLayer, GLint maxLayer);
    static ImageIndexIterator Make2DArray(GLint minMip, GLint maxMip, const GLsizei *layerCounts);

    ImageIndex next();
    ImageIndex current() const;
    bool hasNext() const;

  private:

    ImageIndexIterator(GLenum type, const Range<GLint> &mipRange,
                       const Range<GLint> &layerRange, const GLsizei *layerCounts);

    GLint maxLayer() const;

    GLenum mType;
    Range<GLint> mMipRange;
    Range<GLint> mLayerRange;
    const GLsizei *mLayerCounts;
    GLint mCurrentMip;
    GLint mCurrentLayer;
};

}

#endif // LIBANGLE_IMAGE_INDEX_H_
