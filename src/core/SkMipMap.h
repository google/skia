/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMipMap_DEFINED
#define SkMipMap_DEFINED

#include "include/core/SkPixmap.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkCachedData.h"
#include "src/shaders/SkShaderBase.h"

class SkBitmap;
class SkDiscardableMemory;

typedef SkDiscardableMemory* (*SkDiscardableFactoryProc)(size_t bytes);

/*
 * SkMipMap will generate mipmap levels when given a base mipmap level image.
 *
 * Any function which deals with mipmap levels indices will start with index 0
 * being the first mipmap level which was generated. Said another way, it does
 * not include the base level in its range.
 */
class SkMipMap : public SkCachedData {
public:
    static SkMipMap* Build(const SkPixmap& src, SkDiscardableFactoryProc);
    static SkMipMap* Build(const SkBitmap& src, SkDiscardableFactoryProc);

    // Determines how many levels a SkMipMap will have without creating that mipmap.
    // This does not include the base mipmap level that the user provided when
    // creating the SkMipMap.
    static int ComputeLevelCount(int baseWidth, int baseHeight);

    // Determines the size of a given mipmap level.
    // |level| is an index into the generated mipmap levels. It does not include
    // the base level. So index 0 represents mipmap level 1.
    static SkISize ComputeLevelSize(int baseWidth, int baseHeight, int level);

    // We use a block of (possibly discardable) memory to hold an array of Level structs, followed
    // by the pixel data for each level. On 32-bit platforms, Level would naturally be 4 byte
    // aligned, so the pixel data could end up with 4 byte alignment. If the pixel data is F16,
    // it must be 8 byte aligned. To ensure this, keep the Level struct 8 byte aligned as well.
    struct alignas(8) Level {
        SkPixmap    fPixmap;
        SkSize      fScale; // < 1.0
    };

    bool extractLevel(const SkSize& scale, Level*) const;

    // countLevels returns the number of mipmap levels generated (which does not
    // include the base mipmap level).
    int countLevels() const;

    // |index| is an index into the generated mipmap levels. It does not include
    // the base level. So index 0 represents mipmap level 1.
    bool getLevel(int index, Level*) const;

protected:
    void onDataChange(void* oldData, void* newData) override {
        fLevels = (Level*)newData; // could be nullptr
    }

private:
    sk_sp<SkColorSpace> fCS;
    Level*              fLevels;    // managed by the baseclass, may be null due to onDataChanged.
    int                 fCount;

    SkMipMap(void* malloc, size_t size) : INHERITED(malloc, size) {}
    SkMipMap(size_t size, SkDiscardableMemory* dm) : INHERITED(size, dm) {}

    static size_t AllocLevelsSize(int levelCount, size_t pixelSize);

    typedef SkCachedData INHERITED;
};

#endif
