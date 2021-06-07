/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDstProxyView_DEFINED
#define GrDstProxyView_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrSurfaceProxyView.h"

/**
 * GrDstProxyView holds a texture containing the destination pixel values, and an integer-coordinate
 * offset from device-space to the space of the texture. When framebuffer fetch is not available, a
 * GrDstProxyView may be used to support blending in the fragment shader/xfer processor.
 */
class GrDstProxyView {
public:
    GrDstProxyView() {}

    GrDstProxyView(const GrDstProxyView& other) {
        *this = other;
    }

    GrDstProxyView& operator=(const GrDstProxyView& other) {
        fProxyView = other.fProxyView;
        fOffset = other.fOffset;
        fDstSampleFlags = other.fDstSampleFlags;
        return *this;
    }

    bool operator==(const GrDstProxyView& that) const {
        return fProxyView == that.fProxyView &&
               fOffset == that.fOffset &&
               fDstSampleFlags == that.fDstSampleFlags;
    }
    bool operator!=(const GrDstProxyView& that) const { return !(*this == that); }

    const SkIPoint& offset() const { return fOffset; }

    void setOffset(const SkIPoint& offset) { fOffset = offset; }
    void setOffset(int ox, int oy) { fOffset.set(ox, oy); }

    GrSurfaceProxy* proxy() const { return fProxyView.proxy(); }
    const GrSurfaceProxyView& proxyView() const { return fProxyView; }

    void setProxyView(GrSurfaceProxyView view) {
        fProxyView = std::move(view);
        if (!fProxyView.proxy()) {
            fOffset = {0, 0};
        }
    }

    GrDstSampleFlags dstSampleFlags() const { return fDstSampleFlags; }

    void setDstSampleFlags(GrDstSampleFlags dstSampleFlags) { fDstSampleFlags = dstSampleFlags; }

private:
    GrSurfaceProxyView       fProxyView;
    SkIPoint                 fOffset = {0, 0};
    GrDstSampleFlags         fDstSampleFlags = GrDstSampleFlags::kNone;
};

#endif
