/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixmapPriv_DEFINED
#define SkPixmapPriv_DEFINED

#include "SkPixmap.h"
#include "SkEncodedOrigin.h"
#include "SkAutoPixmapStorage.h"

class SkPixmapPriv {
public:
    // These flag are applied in this order (swap is applied last)
    enum OrientFlags {
        kMirrorX = 1 << 0,
        kMirrorY = 1 << 1,
        kSwapXY  = 1 << 2,
    };

    static OrientFlags OriginToOrient(SkEncodedOrigin);

    /**
     *  Copy the pixels in this pixmap into dst, applying the orientation transformations specified
     *  by the flags. If the inputs are invalid, this returns false and no copy is made.
     */
    static bool Orient(const SkPixmap& dst, const SkPixmap& src, OrientFlags);

    static bool ShouldSwapWidthHeight(SkEncodedOrigin o);
    static SkImageInfo SwapWidthHeight(const SkImageInfo& info);

    /**
     *  Helper for reorienting based on SkEncodedOrigin.
     */
    class Orienter {
    public:
        /**
         *  Create a new Orienter. May need to allocate storage. If that fails, preOrientDst
         *  will return null.
         *
         *  @param requestInfo output SkImageInfo after orienting according to origin.
         *  @param dst Memory to write to, matching requestInfo. Does not change ownership.
         *  @param rowBytes of dst
         *  @param origin of the encoded image.
         */
        Orienter(const SkImageInfo& requestInfo, void* dst, size_t rowBytes,
                 SkEncodedOrigin origin)
            : fRequest(requestInfo , dst, rowBytes)
            , fPreOrientDst(nullptr)
            , fOrigin(origin)
        {
            if (kTopLeft_SkEncodedOrigin == origin) {
                fPreOrientDst = &fRequest;
            } else {
                SkImageInfo info = requestInfo;
                if (SkPixmapPriv::ShouldSwapWidthHeight(origin)) {
                    info = SkPixmapPriv::SwapWidthHeight(info);
                }
                if (fStorage.tryAlloc(info)) {
                    fPreOrientDst = &fStorage;
                }
            }
        }

        /**
         *  Return the destination to write to, prior to orienting.
         *
         *  Returns nullptr on failure to allocate.
         */
        const SkPixmap* preOrientDst() { return fPreOrientDst; }

        /**
         *  Orient to the dst passed to the constructor if necessary.
         */
        bool orientIfNecessary() {
            if (fPreOrientDst == &fRequest) {
                SkASSERT(kTopLeft_SkEncodedOrigin == fOrigin);
                return true;
            }

            if (!fPreOrientDst) {
                return false;
            }

            return Orient(fRequest, *fPreOrientDst, OriginToOrient(fOrigin));
        }

    private:
        SkPixmap            fRequest;
        SkPixmap*           fPreOrientDst;
        SkEncodedOrigin     fOrigin;
        SkAutoPixmapStorage fStorage;    // used if we have to post-orient the output from the codec

    };
    #define Orienter(...) SK_REQUIRE_LOCAL_VAR(Orienter);
};

#endif

