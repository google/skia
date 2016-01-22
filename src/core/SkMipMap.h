/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMipMap_DEFINED
#define SkMipMap_DEFINED

#include "SkCachedData.h"
#include "SkPixmap.h"
#include "SkScalar.h"
#include "SkSize.h"

class SkBitmap;
class SkDiscardableMemory;

typedef SkDiscardableMemory* (*SkDiscardableFactoryProc)(size_t bytes);

class SkMipMap : public SkCachedData {
public:
    static SkMipMap* Build(const SkPixmap& src, SkDiscardableFactoryProc);
    static SkMipMap* Build(const SkBitmap& src, SkDiscardableFactoryProc);

    struct Level {
        SkPixmap    fPixmap;
        SkSize      fScale; // < 1.0
    };

    bool extractLevel(SkScalar scale, Level*) const;

protected:
    void onDataChange(void* oldData, void* newData) override {
        fLevels = (Level*)newData; // could be nullptr
    }

private:
    Level*  fLevels;
    int     fCount;

    // we take ownership of levels, and will free it with sk_free()
    SkMipMap(void* malloc, size_t size) : INHERITED(malloc, size) {}
    SkMipMap(size_t size, SkDiscardableMemory* dm) : INHERITED(size, dm) {}

    static size_t AllocLevelsSize(int levelCount, size_t pixelSize);

    typedef SkCachedData INHERITED;
};

#endif
