/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureMaker_DEFINED
#define GrTextureMaker_DEFINED

#include "GrTextureParams.h"
#include "GrResourceKey.h"
#include "SkTLazy.h"

class GrContext;
class GrTexture;
class GrTextureParams;
class GrUniqueKey;
class SkBitmap;

/**
 * Different GPUs and API extensions have different requirements with respect to what texture
 * sampling parameters may be used with textures of various types. This class facilitates making
 * texture compatible with a given GrTextureParams. There are two immediate subclasses defined
 * below. One is a base class for sources that are inherently texture-backed (e.g. a texture-backed
 * SkImage). It supports subsetting the original texture. The other is for use cases where the
 * source can generate a texture that represents some content (e.g. cpu pixels, SkPicture, ...).
 */
class GrTextureProducer : public SkNoncopyable {
public:
    struct CopyParams {
        GrTextureParams::FilterMode fFilter;
        int                         fWidth;
        int                         fHeight;
    };

    virtual ~GrTextureProducer() {}

protected:
    /** Helper for creating a key for a copy from an original key. */
    static void MakeCopyKeyFromOrigKey(const GrUniqueKey& origKey,
                                       const CopyParams& copyParams,
                                       GrUniqueKey* copyKey) {
        SkASSERT(!copyKey->isValid());
        if (origKey.isValid()) {
            static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
            GrUniqueKey::Builder builder(copyKey, origKey, kDomain, 3);
            builder[0] = copyParams.fFilter;
            builder[1] = copyParams.fWidth;
            builder[2] = copyParams.fHeight;
        }
    }

    /**
    *  If we need to make a copy in order to be compatible with GrTextureParams producer is asked to
    *  return a key that identifies its original content + the CopyParms parameter. If the producer
    *  does not want to cache the stretched version (e.g. the producer is volatile), this should
    *  simply return without initializing the copyKey.
    */
    virtual void makeCopyKey(const CopyParams&, GrUniqueKey* copyKey) = 0;

    /**
    *  If a stretched version of the texture is generated, it may be cached (assuming that
    *  makeCopyKey() returns true). In that case, the maker is notified in case it
    *  wants to note that for when the maker is destroyed.
    */
    virtual void didCacheCopy(const GrUniqueKey& copyKey) = 0;

    typedef SkNoncopyable INHERITED;
};

/** Base class for sources that start out as textures */
class GrTextureAdjuster : public GrTextureProducer {
public:
    /** Makes the subset of the texture safe to use with the given texture parameters.
        outOffset will be the top-left corner of the subset if a copy is not made. Otherwise,
        the copy will be tight to the contents and outOffset will be (0, 0). If the copy's size
        does not match subset's dimensions then the contents are scaled to fit the copy.*/
    GrTexture* refTextureSafeForParams(const GrTextureParams&, SkIPoint* outOffset);

protected:
    /** No subset, use the whole texture */
    explicit GrTextureAdjuster(GrTexture* original): fOriginal(original) {}

    GrTextureAdjuster(GrTexture* original, const SkIRect& subset);

    GrTexture* originalTexture() { return fOriginal; }

    /** Returns the subset or null for the whole original texture */
    const SkIRect* subset() { return fSubset.getMaybeNull(); }

private:
    GrTexture* internalRefTextureSafeForParams(GrTexture*, const SkIRect* subset,
                                               const GrTextureParams&, SkIPoint* outOffset);
    SkTLazy<SkIRect>    fSubset;
    GrTexture*          fOriginal;

    typedef GrTextureProducer INHERITED;
};

/** 
 * Base class for sources that start out as something other than a texture (encoded image,
 * picture, ...).
 */
class GrTextureMaker : public GrTextureProducer {
public:

    GrTextureMaker(int width, int height) : fWidth(width), fHeight(height) {}

    int width() const { return fWidth; }
    int height() const { return fHeight; }

    /** Returns a texture that is safe for use with the params. If the size of the returned texture
        does not match width()/height() then the contents of the original must be scaled to fit
        the texture. */
    GrTexture* refTextureForParams(GrContext*, const GrTextureParams&);

protected:
    /**
     *  Return the maker's "original" texture. It is the responsibility of the maker
     *  to make this efficient ... if the texture is being generated, the maker must handle
     *  caching it (if desired).
     */
    virtual GrTexture* refOriginalTexture(GrContext*) = 0;

    /**
     *  If we need to copy the producer's original texture, the producer is asked to return a key
     *  that identifies its original + the CopyParms parameter. If the maker does not want to cache
     *  the stretched version (e.g. the producer is volatile), this should simply return without
     *  initializing the copyKey.
     */
    virtual void makeCopyKey(const CopyParams&, GrUniqueKey* copyKey) = 0;

    /**
     *  Return a new (uncached) texture that is the stretch of the maker's original.
     *
     *  The base-class handles general logic for this, and only needs access to the following
     *  method:
     *  - refOriginalTexture()
     *
     *  Subclass may override this if they can handle creating the texture more directly than
     *  by copying.
     */
    virtual GrTexture* generateTextureForParams(GrContext*, const CopyParams&);

private:
    const int fWidth;
    const int fHeight;

    typedef GrTextureProducer INHERITED;
};

#endif
