/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureMaker_DEFINED
#define GrTextureMaker_DEFINED

#include "SkGrPriv.h"

class GrContext;
class GrTexture;
class GrTextureParams;
class GrUniqueKey;
class SkBitmap;

class GrTextureMaker {
public:
    GrTextureMaker(int width, int height) : fWidth(width), fHeight(height) {}
    virtual ~GrTextureMaker() {}

    int width() const { return fWidth; }
    int height() const { return fHeight; }

    GrTexture* refCachedTexture(GrContext*, const GrTextureParams&);

protected:
    /**
     *  Return the maker's "original" unstretched texture. It is the responsibility of the maker
     *  to make this efficient ... if the texture is being generated, the maker must handle
     *  caching it.
     */
    virtual GrTexture* onRefUnstretchedTexture(GrContext*) = 0;

    /**
     *  If we need to stretch the maker's original texture, the maker is asked to return a key
     *  that identifies its origianl + the stretch parameter. If the maker does not want to cache
     *  the stretched version (e.g. the maker is volatile), this should ignore the key parameter
     *  and return false.
     */
    virtual bool onMakeStretchedKey(const SkGrStretch&, GrUniqueKey* stretchedKey) = 0;

    /**
     *  Return a new (uncached) texture that is the stretch of the maker's original.
     *
     *  The base-class handles general logic for this, and only needs access to the following
     *  methods:
     *  - onRefUnstretchedTexture()
     *  - onGetROBitmap()
     *
     *  Subclass may override this if they can handle stretching more efficiently.
     */
    virtual GrTexture* onGenerateStretchedTexture(GrContext*, const SkGrStretch&);

    /**
     *  If a stretched version of the texture is generated, it may be cached (assuming that
     *  onMakeStretchedKey() returns true). In that case, the maker is notified in case it
     *  wants to note that for when the maker is destroyed.
     */
    virtual void onNotifyStretchCached(const GrUniqueKey& stretchedKey) = 0;

    /**
     *  Some GPUs are unreliable w/ very small texture sizes. If we run into that case, this
     *  method will be called (in service of onGenerateStretchedTexture) to return a raster version
     *  of the original texture.
     */
    virtual bool onGetROBitmap(SkBitmap*) = 0;

private:
    const int fWidth;
    const int fHeight;
};

#endif
