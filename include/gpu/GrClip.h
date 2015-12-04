/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClip_DEFINED
#define GrClip_DEFINED

#include "SkClipStack.h"

struct SkIRect;

/**
 * GrClip encapsulates the information required to construct the clip
 * masks. 'A GrClip is either wide open, just an IRect, just a Rect, or a full clipstack.
 * If the clip is a clipstack than the origin is used to translate the stack with
 * respect to device coordinates. This allows us to use a clip stack that is
 * specified for a root device with a layer device that is restricted to a subset
 * of the original canvas. For other clip types the origin will always be (0,0).
 *
 * NOTE: GrClip *must* point to a const clipstack
 */
class GrClip : SkNoncopyable {
public:
    GrClip() : fClipType(kWideOpen_ClipType) {
        fOrigin.setZero();
    }

    GrClip(const SkIRect& rect) : fClipType(kIRect_ClipType) {
        fOrigin.setZero();
        fClip.fIRect = rect;
    }

    GrClip(const SkRect& rect) : fClipType(kIRect_ClipType) {
        fOrigin.setZero();
        fClip.fIRect.fLeft   = SkScalarRoundToInt(rect.fLeft);
        fClip.fIRect.fTop    = SkScalarRoundToInt(rect.fTop);
        fClip.fIRect.fRight  = SkScalarRoundToInt(rect.fRight);
        fClip.fIRect.fBottom = SkScalarRoundToInt(rect.fBottom);
    }

    ~GrClip() { this->reset(); }

    const GrClip& operator=(const GrClip& other) {
        this->reset();
        fClipType = other.fClipType;
        switch (other.fClipType) {
            case kWideOpen_ClipType:
                fOrigin.setZero();
                break;
            case kClipStack_ClipType:
                fClip.fStack = SkRef(other.clipStack());
                fOrigin = other.origin();
                break;
            case kIRect_ClipType:
                fClip.fIRect = other.irect();
                fOrigin.setZero();
                break;
        }
        return *this;
    }

    bool operator==(const GrClip& other) const {
        if (this->clipType() != other.clipType()) {
            return false;
        }

        switch (fClipType) {
            case kWideOpen_ClipType:
                return true;
            case kClipStack_ClipType:
                if (this->origin() != other.origin()) {
                    return false;
                }

                if (this->clipStack() && other.clipStack()) {
                    return *this->clipStack() == *other.clipStack();
                } else {
                    return this->clipStack() == other.clipStack();
                }
                break;
            case kIRect_ClipType:
                return this->irect() == other.irect();
                break;
        }
        SkFAIL("This should not occur\n");
        return false;
    }

    bool operator!=(const GrClip& other) const {
        return !(*this == other);
    }

    const SkClipStack* clipStack() const {
        SkASSERT(kClipStack_ClipType == fClipType);
        return fClip.fStack;
    }

    void setClipStack(const SkClipStack* clipStack, const SkIPoint* origin = NULL) {
        this->reset();
        if (clipStack->isWideOpen()) {
            fClipType = kWideOpen_ClipType;
            fOrigin.setZero();
        } else {
            fClipType = kClipStack_ClipType;
            fClip.fStack = SkRef(clipStack);
            if (origin) {
                fOrigin = *origin;
            } else {
                fOrigin.setZero();
            }
        }
    }

    const SkIRect& irect() const {
        SkASSERT(kIRect_ClipType == fClipType);
        return fClip.fIRect;
    }

    void reset() {
        if (kClipStack_ClipType == fClipType) {
            fClip.fStack->unref();
            fClip.fStack = NULL;
        }
        fClipType = kWideOpen_ClipType;
        fOrigin.setZero();
    }

    // We support this for all cliptypes to simplify the logic a bit in clip mask manager.
    // non clipstack clip types MUST have a (0,0) origin
    const SkIPoint& origin() const {
        SkASSERT(fClipType == kClipStack_ClipType || (fOrigin.fX == 0 && fOrigin.fY == 0));
        return fOrigin;
    }

    bool isWideOpen(const SkRect& rect) const {
        return (kWideOpen_ClipType == fClipType) ||
               (kClipStack_ClipType == fClipType && this->clipStack()->isWideOpen()) ||
               (kIRect_ClipType == fClipType && this->irect().contains(rect));
    }

    bool isWideOpen(const SkIRect& rect) const {
        return (kWideOpen_ClipType == fClipType) ||
               (kClipStack_ClipType == fClipType && this->clipStack()->isWideOpen()) ||
               (kIRect_ClipType == fClipType && this->irect().contains(rect));
    }

    bool isWideOpen() const {
        return (kWideOpen_ClipType == fClipType) ||
               (kClipStack_ClipType == fClipType && this->clipStack()->isWideOpen());
    }

    bool quickContains(const SkRect& rect) const {
        return (kWideOpen_ClipType == fClipType) ||
               (kClipStack_ClipType == fClipType && this->clipStack()->quickContains(rect)) ||
               (kIRect_ClipType == fClipType && this->irect().contains(rect));
    }

    void getConservativeBounds(int width, int height,
                               SkIRect* devResult,
                               bool* isIntersectionOfRects = NULL) const;

    static const GrClip& WideOpen();

    enum ClipType {
        kClipStack_ClipType,
        kWideOpen_ClipType,
        kIRect_ClipType,
    };

    ClipType clipType() const { return fClipType; }

private:
    union Clip {
        const SkClipStack* fStack;
        SkIRect fIRect;
    } fClip;

    SkIPoint fOrigin;
    ClipType fClipType;
};

#endif
