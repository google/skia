/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkReadPixelsRec_DEFINED
#define SkReadPixelsRec_DEFINED

#include "include/core/SkImageInfo.h"

/**
 *  Helper class to package and trim the parameters passed to readPixels()
 */
struct SkReadPixelsRec {
    SkReadPixelsRec(const SkImageInfo& info, void* pixels, size_t rowBytes, int x, int y)
        : fPixels(pixels)
        , fRowBytes(rowBytes)
        , fInfo(info)
        , fX(x)
        , fY(y)
    {}

    SkReadPixelsRec(const SkPixmap& pm, int x, int y)
        : fPixels(pm.writable_addr())
        , fRowBytes(pm.rowBytes())
        , fInfo(pm.info())
        , fX(x)
        , fY(y)
    {}

    void*       fPixels;
    size_t      fRowBytes;
    SkImageInfo fInfo;
    int         fX;
    int         fY;

    /*
     *  On true, may have modified its fields (except fRowBytes) to make it a legal subset
     *  of the specified src width/height.
     *
     *  On false, leaves self unchanged, but indicates that it does not overlap src, or
     *  is not valid (e.g. bad fInfo) for readPixels().
     */
    bool trim(int srcWidth, int srcHeight);
};

#endif
