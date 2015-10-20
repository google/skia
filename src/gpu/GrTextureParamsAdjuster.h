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

class GrContext;
class GrTexture;
class GrTextureParams;
class GrUniqueKey;
class SkBitmap;

/**
 * Different GPUs and API extensions have different requirements with respect to what texture
 * sampling parameters may be used with textures of various types. This class facilitates making
 * texture compatible with a given GrTextureParams. It abstracts the source of the original data
 * which may be an already existing texture, CPU pixels, a codec, ... so that various sources can
 * be used with common code that scales or copies the data to make it compatible with a
 * GrTextureParams.
 */
class GrTextureParamsAdjuster {
public:
    struct CopyParams {
        GrTextureParams::FilterMode fFilter;
        int                         fWidth;
        int                         fHeight;
    };

    GrTextureParamsAdjuster(int width, int height) : fWidth(width), fHeight(height) {}
    virtual ~GrTextureParamsAdjuster() {}

    int width() const { return fWidth; }
    int height() const { return fHeight; }

    /** Returns a texture that is safe for use with the params */
    GrTexture* refTextureForParams(GrContext*, const GrTextureParams&);

protected:
    /** If the original is a inherently texture that can be returned for "free" then return it
        without ref'ing it. Otherwise, return null. */
    virtual GrTexture* peekOriginalTexture() = 0;

    /**
     *  Return the maker's "original" texture. It is the responsibility of the maker
     *  to make this efficient ... if the texture is being generated, the maker must handle
     *  caching it (if desired).
     */
    virtual GrTexture* refOriginalTexture(GrContext*) = 0;

    /**
     *  If we need to copy the maker's original texture, the maker is asked to return a key
     *  that identifies its original + the CopyParms parameter. If the maker does not want to cache
     *  the stretched version (e.g. the maker is volatile), this should simply return without
     *  initializing the copyKey.
     */
    virtual void makeCopyKey(const CopyParams&, GrUniqueKey* copyKey) = 0;

    /**
     *  Return a new (uncached) texture that is the stretch of the maker's original.
     *
     *  The base-class handles general logic for this, and only needs access to the following
     *  methods:
     *  - onRefOriginalTexture()
     *  - onGetROBitmap()
     *
     *  Subclass may override this if they can handle creating the texture more directly than
     *  by copying.
     */
    virtual GrTexture* generateTextureForParams(GrContext*, const CopyParams&);

    /**
     *  If a stretched version of the texture is generated, it may be cached (assuming that
     *  onMakeParamsKey() returns true). In that case, the maker is notified in case it
     *  wants to note that for when the maker is destroyed.
     */
    virtual void didCacheCopy(const GrUniqueKey& copyKey) = 0;

    /**
     *  Some GPUs are unreliable w/ very small texture sizes. If we run into that case, this
     *  method will be called (in service of onGenerateParamsTexture) to return a raster version
     *  of the original texture.
     */
    virtual bool getROBitmap(SkBitmap*) = 0;

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

private:
    const int fWidth;
    const int fHeight;
};

#endif
