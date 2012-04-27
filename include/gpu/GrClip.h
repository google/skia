
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrClip_DEFINED
#define GrClip_DEFINED

#include "GrClipIterator.h"
#include "GrRect.h"
#include "GrTemplates.h"

#include "SkPath.h"
#include "SkTArray.h"

class GrClip {
public:
    GrClip();
    GrClip(const GrClip& src);
    /**
     *  If specified, the conservativeBounds parameter already takes (tx,ty)
     *  into account.
     */
    GrClip(GrClipIterator* iter, GrScalar tx, GrScalar ty,
           const GrRect* conservativeBounds = NULL);
    GrClip(const GrIRect& rect);
    GrClip(const GrRect& rect);

    ~GrClip();

    GrClip& operator=(const GrClip& src);

    bool hasConservativeBounds() const { return fConservativeBoundsValid; }

    const GrRect& getConservativeBounds() const { return fConservativeBounds; }

    bool requiresAA() const { return fRequiresAA; }

    int getElementCount() const { return fList.count(); }

    GrClipType getElementType(int i) const { return fList[i].fType; }

    const SkPath& getPath(int i) const {
        GrAssert(kPath_ClipType == fList[i].fType);
        return fList[i].fPath;
    }

    GrPathFill getPathFill(int i) const {
        GrAssert(kPath_ClipType == fList[i].fType);
        return fList[i].fPathFill;
    }

    const GrRect& getRect(int i) const {
        GrAssert(kRect_ClipType == fList[i].fType);
        return fList[i].fRect;
    }

    SkRegion::Op getOp(int i) const { return fList[i].fOp; }

    bool getDoAA(int i) const   { return fList[i].fDoAA; }

    bool isRect() const {
        if (1 == fList.count() && kRect_ClipType == fList[0].fType && 
            (SkRegion::kIntersect_Op == fList[0].fOp ||
             SkRegion::kReplace_Op == fList[0].fOp)) {
            // if we determined that the clip is a single rect
            // we ought to have also used that rect as the bounds.
            GrAssert(fConservativeBoundsValid);
            GrAssert(fConservativeBounds == fList[0].fRect);
            return true;
        } else {
            return false;
        }
    }

    bool isEmpty() const { return 0 == fList.count(); }

    /**
     *  Resets this clip to be empty
     */
    void setEmpty();

    /**
     *  If specified, the bounds parameter already takes (tx,ty) into account.
     */
    void setFromIterator(GrClipIterator* iter, GrScalar tx, GrScalar ty,
                         const GrRect* conservativeBounds = NULL);
    void setFromRect(const GrRect& rect);
    void setFromIRect(const GrIRect& rect);

    friend bool operator==(const GrClip& a, const GrClip& b) {
        if (a.fList.count() != b.fList.count()) {
            return false;
        }
        int count = a.fList.count();
        for (int i = 0; i < count; ++i) {
            if (a.fList[i] != b.fList[i]) {
                return false;
            }
        }
        return true;
    }
    friend bool operator!=(const GrClip& a, const GrClip& b) {
        return !(a == b);
    }

private:
    struct Element {
        GrClipType   fType;
        GrRect       fRect;
        SkPath       fPath;
        GrPathFill   fPathFill;
        SkRegion::Op fOp;
        bool         fDoAA;
        bool operator ==(const Element& e) const {
            if (e.fType != fType || e.fOp != fOp || e.fDoAA != fDoAA) {
                return false;
            }
            switch (fType) {
                case kRect_ClipType:
                    return fRect == e.fRect;
                case kPath_ClipType:
                    return fPath == e.fPath;
                default:
                    GrCrash("Unknown clip element type.");
                    return false; // suppress warning
            }
        }
        bool operator !=(const Element& e) const { return !(*this == e); }
    };

    GrRect              fConservativeBounds;
    bool                fConservativeBoundsValid;

    bool                fRequiresAA;

    enum {
        kPreAllocElements = 4,
    };
    SkSTArray<kPreAllocElements, Element>   fList;
};
#endif

