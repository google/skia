/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClip_DEFINED
#define GrClip_DEFINED

#include "SkClipStack.h"
#include "GrSurface.h"

struct SkIRect;

/**
 * GrClip encapsulates the information required to construct the clip
 * masks. 'A GrClip is either wide open, just an IRect, just a Rect(TODO), or a full clipstack.
 * If the clip is a clipstack than the origin is used to translate the stack with
 * respect to device coordinates. This allows us to use a clip stack that is
 * specified for a root device with a layer device that is restricted to a subset
 * of the original canvas. For other clip types the origin will always be (0,0).
 *
 * NOTE: GrClip *must* point to a const clipstack
 */
class GrClip : SkNoncopyable {
public:
    GrClip() : fClipType(kWideOpen_ClipType) {}
    GrClip(const SkIRect& rect) : fClipType(kIRect_ClipType) {
        fClip.fIRect = rect;
    }
    ~GrClip() { this->reset(); }

    const GrClip& operator=(const GrClip& other) {
        this->reset();
        fClipType = other.fClipType;
        switch (other.fClipType) {
            default:
                SkFAIL("Incomplete Switch\n");
            case kWideOpen_ClipType:
                break;
            case kClipStack_ClipType:
                fClip.fClipStack.fStack = SkRef(other.clipStack());
                fClip.fClipStack.fOrigin = other.origin();
                break;
            case kIRect_ClipType:
                fClip.fIRect = other.irect();
                break;
        }
        return *this;
    }

    bool operator==(const GrClip& other) const {
        if (this->clipType() != other.clipType()) {
            return false;
        }

        switch (fClipType) {
            default:
                SkFAIL("Incomplete Switch\n");
                return false;
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
    }

    bool operator!=(const GrClip& other) const {
        return !(*this == other);
    }

    const SkClipStack* clipStack() const {
        SkASSERT(kClipStack_ClipType == fClipType);
        return fClip.fClipStack.fStack;
    }

    void setClipStack(const SkClipStack* clipStack, const SkIPoint* origin = NULL) {
        if (clipStack->isWideOpen()) {
            fClipType = kWideOpen_ClipType;
        } else {
            fClipType = kClipStack_ClipType;
            fClip.fClipStack.fStack = SkRef(clipStack);
            if (origin) {
                fClip.fClipStack.fOrigin = *origin;
            } else {
                fClip.fClipStack.fOrigin.setZero();
            }
        }
    }

    const SkIRect& irect() const {
        SkASSERT(kIRect_ClipType == fClipType);
        return fClip.fIRect;
    }

    void reset() {
        if (kClipStack_ClipType == fClipType) {
            fClip.fClipStack.fStack->unref();
            fClip.fClipStack.fStack = NULL;
        }
        fClipType = kWideOpen_ClipType;
    }

    const SkIPoint& origin() const {
        SkASSERT(kClipStack_ClipType == fClipType);
        return fClip.fClipStack.fOrigin;
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

    void getConservativeBounds(const GrSurface* surface,
                               SkIRect* devResult,
                               bool* isIntersectionOfRects = NULL) const {
        this->getConservativeBounds(surface->width(), surface->height(),
                                    devResult, isIntersectionOfRects);
    }

    void getConservativeBounds(int width, int height,
                               SkIRect* devResult,
                               bool* isIntersectionOfRects = NULL) const;

    static const GrClip& WideOpen() {
        static SkAlignedSStorage<sizeof(GrClip)> g_WideOpenClip_Storage;
        static GrClip* g_WideOpenClip SkNEW_PLACEMENT(g_WideOpenClip_Storage.get(), GrClip);
        static SkAutoTDestroy<GrClip> g_WideOpenClip_ad(g_WideOpenClip);
        return *g_WideOpenClip_ad;
    }

    enum ClipType {
        kClipStack_ClipType,
        kWideOpen_ClipType,
        kIRect_ClipType,
    };

    ClipType clipType() const { return fClipType; }

private:
    union Clip {
        struct ClipStack {
            const SkClipStack* fStack;
            SkIPoint fOrigin;
        } fClipStack;
        SkIRect fIRect;
    } fClip;

    ClipType fClipType;
};

#endif
