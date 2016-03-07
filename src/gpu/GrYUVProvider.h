/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrYUVProvider_DEFINED
#define GrYUVProvider_DEFINED

#include "GrTypes.h"
#include "SkImageInfo.h"

class GrContext;
class GrTexture;

/**
 *  There are at least 2 different ways to extract/retrieve YUV planar data...
 *  - SkPixelRef
 *  - SkImageGeneartor
 *
 *  To share common functionality around using the planar data, we use this abstract base-class
 *  to represent accessing that data.
 */
class GrYUVProvider {
public:
    virtual ~GrYUVProvider() {}

    /**
     *  On success, this returns a texture that has converted the YUV data from the provider
     *  into a form that is supported by the GPU (typically transformed into RGB). If useCache
     *  is true, then the texture will automatically have a key added, so it can be retrieved
     *  from the cache (assuming it is requested by a provider w/ the same genID).
     *
     *  On failure (e.g. the provider had no data), this returns NULL.
     */
    GrTexture* refAsTexture(GrContext*, const GrSurfaceDesc&, bool useCache);

    virtual uint32_t onGetID() = 0;

    enum {
        kY_Index = 0,
        kU_Index = 1,
        kV_Index = 2,

        kPlaneCount = 3
    };

    // These are not meant to be called by a client, only by the implementation

    /**
     *  Return the 3 dimensions for each plane and return true. On failure, return false and
     *  ignore the sizes parameter. Typical failure is that the provider does not contain YUV
     *  data, and may just be an RGB src.
     */
    virtual bool onGetYUVSizes(SkISize sizes[kPlaneCount]) = 0;

    /**
     *  On success, return true, and set sizes, rowbytes and colorspace to the appropriate values.
     *  planes[] will have already been allocated by the client (based on the worst-case sizes
     *  returned by onGetYUVSizes(). This method copies its planar data into those buffers.
     *
     *  On failure, return false and ignore other parameters.
     */
    virtual bool onGetYUVPlanes(SkISize sizes[kPlaneCount], void* planes[kPlaneCount],
                                size_t rowBytes[kPlaneCount], SkYUVColorSpace*) = 0;
};

#endif
