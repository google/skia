/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureProducer_DEFINED
#define GrTextureProducer_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/private/GrResourceKey.h"
#include "include/private/SkNoncopyable.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrSamplerState.h"

class GrFragmentProcessor;
class GrRecordingContext;
class GrTexture;
class GrTextureProxy;
class SkColorSpace;
class SkMatrix;
struct SkRect;

/**
 * Different GPUs and API extensions have different requirements with respect to what texture
 * sampling parameters may be used with textures of various types. This class facilitates making
 * texture compatible with a given GrSamplerState. There are two immediate subclasses defined
 * below. One is a base class for sources that are inherently texture-backed (e.g. a texture-backed
 * SkImage). It supports subsetting the original texture. The other is for use cases where the
 * source can generate a texture that represents some content (e.g. cpu pixels, SkPicture, ...).
 */
class GrTextureProducer : public SkNoncopyable {
public:
    virtual ~GrTextureProducer() = default;


    /**
     * Returns a texture view, possibly with MIP maps. The request for MIP maps may not be honored
     * base on caps, format, and whether the texture is 1x1. A non-MIP mapped request may still
     * receive a MIP mapped texture (if that is what is available in the cache).
     */
    GrSurfaceProxyView view(GrMipmapped);

    int width() const { return fImageInfo.width(); }
    int height() const { return fImageInfo.height(); }
    SkISize dimensions() const { return fImageInfo.dimensions(); }
    GrColorType colorType() const { return fImageInfo.colorType(); }

protected:
    GrTextureProducer(GrRecordingContext* context, const GrImageInfo& imageInfo)
            : fContext(context), fImageInfo(imageInfo) {}


    GrRecordingContext* context() const { return fContext; }

private:
    virtual GrSurfaceProxyView onView(GrMipmapped) = 0;

    GrRecordingContext* fContext;
    const GrImageInfo fImageInfo;

    using INHERITED = SkNoncopyable;
};

#endif
