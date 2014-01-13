/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureUtils_DEFINED
#define SkPictureUtils_DEFINED

#include "SkPicture.h"
#include "SkTDArray.h"

class SkData;
struct SkRect;

class SK_API SkPictureUtils {
public:
    /**
     *  Given a rectangular visible "window" into the picture, return an array
     *  of SkPixelRefs that might intersect that area. To keep the call fast,
     *  the returned list is not guaranteed to be exact, so it may miss some,
     *  and it may return false positives.
     *
     *  The pixelrefs returned in the SkData are already owned by the picture,
     *  so the returned pointers are only valid while the picture is in scope
     *  and remains unchanged.
     */
    static SkData* GatherPixelRefs(SkPicture* pict, const SkRect& area);

    /**
     * SkPixelRefContainer provides a base class for more elaborate pixel ref
     * query structures (e.g., rtrees, quad-trees, etc.)
     */
    class SkPixelRefContainer : public SkRefCnt {
    public:
        virtual void add(SkPixelRef* pr, const SkRect& rect) = 0;

        // The returned array may contain duplicates
        virtual void query(const SkRect& queryRect, SkTDArray<SkPixelRef*> *result) = 0;

    private:
        typedef SkRefCnt INHERITED;
    };

    // Simple query structure that just stores a linked list of pixel refs
    // and rects.
    class SkPixelRefsAndRectsList : public SkPixelRefContainer {
    public:
        virtual void add(SkPixelRef* pr, const SkRect& rect) SK_OVERRIDE {
            PixelRefAndRect *dst = fArray.append();

            dst->fPixelRef = pr;
            dst->fRect = rect;
        }

        virtual void query(const SkRect& queryRect, SkTDArray<SkPixelRef*> *result) SK_OVERRIDE {
            for (int i = 0; i < fArray.count(); ++i) {
                if (SkRect::Intersects(fArray[i].fRect, queryRect)) {
                    *result->append() = fArray[i].fPixelRef;
                }
            }
        }

    private:
        struct PixelRefAndRect {
            SkPixelRef* fPixelRef;
            SkRect      fRect;
        };

        SkTDArray<PixelRefAndRect> fArray;

        typedef SkPixelRefContainer INHERITED;
    };

    /**
     *  Fill the provided pixel ref container with the picture's pixel ref
     *  and rect information.
     */
    static void GatherPixelRefsAndRects(SkPicture* pict, SkPixelRefContainer* prCont);
};

#endif
