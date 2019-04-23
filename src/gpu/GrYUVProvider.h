/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrYUVProvider_DEFINED
#define GrYUVProvider_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkYUVAIndex.h"
#include "include/core/SkYUVASizeInfo.h"
#include "include/gpu/GrTypes.h"

class GrBackendFormat;
class GrRecordingContext;
struct GrSurfaceDesc;
class GrTexture;
class GrTextureProxy;
class SkCachedData;

/**
 *  There are at least 2 different ways to extract/retrieve YUV planar data...
 *  - SkPixelRef
 *  - SkImageGenerator
 *
 *  To share common functionality around using the planar data, we use this abstract base-class
 *  to represent accessing that data.
 */
class GrYUVProvider {
public:
    virtual ~GrYUVProvider() {}

    /**
     *  On success, this returns a texture proxy that has converted the YUV data from the provider
     *  into a form that is supported by the GPU (typically transformed into RGB). The texture will
     *  automatically have a key added, so it can be retrieved from the cache (assuming it is
     *  requested by a provider w/ the same genID). If srcColorSpace and dstColorSpace are
     *  specified, then a color conversion from src to dst will be applied to the pixels.
     *
     *  On failure (e.g. the provider had no data), this returns NULL.
     */
    sk_sp<GrTextureProxy> refAsTextureProxy(GrRecordingContext*,
                                            const GrBackendFormat&,
                                            const GrSurfaceDesc&,
                                            SkColorSpace* srcColorSpace,
                                            SkColorSpace* dstColorSpace);

    sk_sp<SkCachedData> getPlanes(SkYUVASizeInfo*, SkYUVAIndex[SkYUVAIndex::kIndexCount],
                                  SkYUVColorSpace*, const void* planes[SkYUVASizeInfo::kMaxCount]);

private:
    virtual uint32_t onGetID() const = 0;

    // These are not meant to be called by a client, only by the implementation

    /**
     *  If decoding to YUV is supported, this returns true.  Otherwise, this
     *  returns false and does not modify any of the parameters.
     *
     *  @param sizeInfo    Output parameter indicating the sizes and required
     *                     allocation widths of the Y, U, V, and A planes.
     *  @param yuvaIndices How the YUVA planes are used/organized
     *  @param colorSpace  Output parameter.
     */
    virtual bool onQueryYUVA8(SkYUVASizeInfo* sizeInfo,
                              SkYUVAIndex yuvaIndices[SkYUVAIndex::kIndexCount],
                              SkYUVColorSpace* colorSpace) const = 0;

    /**
     *  Returns true on success and false on failure.
     *  This always attempts to perform a full decode.  If the client only
     *  wants size, it should call onQueryYUVA8().
     *
     *  @param sizeInfo    Needs to exactly match the values returned by the
     *                     query, except the WidthBytes may be larger than the
     *                     recommendation (but not smaller).
     *  @param yuvaIndices How the YUVA planes are used/organized
     *  @param planes      Memory for each of the Y, U, V, and A planes.
     */
    virtual bool onGetYUVA8Planes(const SkYUVASizeInfo& sizeInfo,
                                  const SkYUVAIndex yuvaIndices[SkYUVAIndex::kIndexCount],
                                  void* planes[]) = 0;

    // This is used as release callback for the YUV data that we capture in an SkImage when
    // uploading to a gpu. When the upload is complete and we release the SkImage this callback will
    // release the underlying data.
    static void YUVGen_DataReleaseProc(const void*, void* data);
};

#endif
