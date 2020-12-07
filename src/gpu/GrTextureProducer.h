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
     * Helper for creating a fragment processor to sample the texture with a given filtering mode.
     * Attempts to avoid unnecessary copies (e.g. for planar sources or subsets) by generating more
     * complex shader code.
     *
     * @param textureMatrix                    Matrix used to access the texture. It is applied to
     *                                         the local coords. The post-transformed coords should
     *                                         be in texel units (rather than normalized) with
     *                                         respect to this Producer's bounds (width()/height()).
     * @param subset                           If not null, a subset of the texture to restrict
     *                                         sampling to. The wrap modes apply to this subset.
     * @param domain                           If not null, a known limit on the texture coordinates
     *                                         that will be accessed. Applies after textureMatrix.
     * @param sampler                          Sampler state. Wrap modes applies to subset if not
     *                                         null, otherwise to the entire source.
     **/
    virtual std::unique_ptr<GrFragmentProcessor> createFragmentProcessor(
            const SkMatrix& textureMatrix,
            const SkRect* subset,
            const SkRect* domain,
            GrSamplerState sampler) = 0;

    /**
     * Similar createFragmentProcessor but produces a fragment processor that does bicubic
     * interpolation of the source. Attempts to avoid unnecessary copies (e.g. for planar sources or
     * subsets) by generating more complex shader code.
     *
     * @param textureMatrix                    Matrix used to access the texture. It is applied to
     *                                         the local coords. The post-transformed coords should
     *                                         be in texel units (rather than normalized) with
     *                                         respect to this Producer's bounds (width()/height()).
     * @param subset                           If not null, a subset of the texture to restrict
     *                                         sampling to. The wrap modes apply to this subset.
     * @param domain                           If not null, a known limit on the texture coordinates
     *                                         that will be accessed. Applies after textureMatrix.
     * @param wrapX                            Wrap mode on x axis. Applied to subset if not null,
     *                                         otherwise to the entire source.
     * @param wrapY                            Wrap mode on y axis. Applied to subset if not null,
     *                                         otherwise to the entire source.
     */
    virtual std::unique_ptr<GrFragmentProcessor> createBicubicFragmentProcessor(
            const SkMatrix& textureMatrix,
            const SkRect* subset,
            const SkRect* domain,
            GrSamplerState::WrapMode wrapX,
            GrSamplerState::WrapMode wrapY,
            SkImage::CubicResampler kernel) = 0;

    /**
     * Returns a texture view, possibly with MIP maps. The request for MIP maps may not be honored
     * base on caps, format, and whether the texture is 1x1. A non-MIP mapped request may still
     * receive a MIP mapped texture (if that is what is available in the cache).
     */
    GrSurfaceProxyView view(GrMipmapped);

    const GrColorInfo& colorInfo() const { return fImageInfo.colorInfo(); }
    int width() const { return fImageInfo.width(); }
    int height() const { return fImageInfo.height(); }
    SkISize dimensions() const { return fImageInfo.dimensions(); }
    GrColorType colorType() const { return fImageInfo.colorType(); }
    SkAlphaType alphaType() const { return fImageInfo.alphaType(); }
    SkColorSpace* colorSpace() const { return fImageInfo.colorSpace(); }
    bool isAlphaOnly() const { return GrColorTypeIsAlphaOnly(fImageInfo.colorType()); }
    /* Is it a planar image consisting of multiple textures that may have different resolutions? */
    virtual bool isPlanar() const { return false; }

protected:
    GrTextureProducer(GrRecordingContext* context, const GrImageInfo& imageInfo)
            : fContext(context), fImageInfo(imageInfo) {}

    // Helper for making a texture effect from a single proxy view.
    std::unique_ptr<GrFragmentProcessor> createFragmentProcessorForView(
            GrSurfaceProxyView view,
            const SkMatrix& textureMatrix,
            const SkRect* subset,
            const SkRect* domain,
            GrSamplerState sampler);

    // Helper for making a bicubic effect from a single proxy view.
    std::unique_ptr<GrFragmentProcessor> createBicubicFragmentProcessorForView(
            GrSurfaceProxyView view,
            const SkMatrix& textureMatrix,
            const SkRect* subset,
            const SkRect* domain,
            GrSamplerState::WrapMode wrapX,
            GrSamplerState::WrapMode wrapY,
            SkImage::CubicResampler kernel);

    GrRecordingContext* context() const { return fContext; }

private:
    virtual GrSurfaceProxyView onView(GrMipmapped) = 0;

    GrRecordingContext* fContext;
    const GrImageInfo fImageInfo;

    using INHERITED = SkNoncopyable;
};

#endif
