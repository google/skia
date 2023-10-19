/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMipmapBuilder_DEFINED
#define SkMipmapBuilder_DEFINED

#include "include/core/SkRefCnt.h"

class SkImage;
class SkMipmap;
class SkPixmap;
struct SkImageInfo;

class SkMipmapBuilder {
public:
    SkMipmapBuilder(const SkImageInfo&);
    ~SkMipmapBuilder();

    int countLevels() const;
    SkPixmap level(int index) const;

    /**
     *  If these levels are compatible with src, return a new Image that combines src's base level
     *  with these levels as mip levels. If not compatible, this returns nullptr.
     */
    sk_sp<SkImage> attachTo(const sk_sp<const SkImage>& src);

private:
    sk_sp<SkMipmap> fMM;
};

#endif
