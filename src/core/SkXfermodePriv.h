/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXfermodePriv_DEFINED
#define SkXfermodePriv_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkRefCnt.h"

class SkXfermode : public SkRefCnt {
public:
    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const = 0;

    /** Return an SkXfermode object for the specified mode.
     */
    static sk_sp<SkXfermode> Make(SkBlendMode);

    /**
     *  Skia maintains global xfermode objects corresponding to each BlendMode. This returns a
     *  ptr to that global xfermode (or null if the mode is srcover). Thus the caller may use
     *  the returned ptr, but it should leave its refcnt untouched.
     */
    static SkXfermode* Peek(SkBlendMode mode) {
        sk_sp<SkXfermode> xfer = Make(mode);
        if (!xfer) {
            SkASSERT(SkBlendMode::kSrcOver == mode);
            return nullptr;
        }
        SkASSERT(!xfer->unique());
        return xfer.get();
    }

protected:
    SkXfermode() {}
};

#endif
