/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrGradientBitmapCache_DEFINED
#define GrGradientBitmapCache_DEFINED

#include "include/core/SkScalar.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkNoncopyable.h"
#include "src/core/SkColorData.h"

#include <cstddef>

class SkBitmap;
class SkColorSpace;
enum SkAlphaType : int;
enum SkColorType : int;

class GrGradientBitmapCache : SkNoncopyable {
public:
    GrGradientBitmapCache(int maxEntries, int resolution);
    ~GrGradientBitmapCache();

    // Assumes colors are compatible with the specified alphaType (e.g. if it's premul then colors
    // are already premultiplied). Thread safe.
    void getGradient(const SkPMColor4f* colors,
                     const SkScalar* positions,
                     int count,
                     bool colorsAreOpaque,
                     const SkGradientShader::Interpolation& interpolation,
                     const SkColorSpace* intermediateColorSpace,
                     const SkColorSpace* dstColorSpace,
                     SkColorType colorType,
                     SkAlphaType alphaType,
                     SkBitmap* bitmap);

private:
    SkMutex fMutex;

    int fEntryCount;
    const int fMaxEntries;
    const int fResolution;

    struct Entry;
    mutable Entry*  fHead;
    mutable Entry*  fTail;

    inline Entry* release(Entry*) const;
    inline void attachToHead(Entry*) const;

    bool find(const void* buffer, size_t len, SkBitmap*) const;
    void add(const void* buffer, size_t len, const SkBitmap&);

    void fillGradient(const SkPMColor4f* colors,
                      const SkScalar* positions,
                      int count,
                      bool colorsAreOpaque,
                      const SkGradientShader::Interpolation& interpolation,
                      const SkColorSpace* intermediateColorSpace,
                      const SkColorSpace* dstColorSpace,
                      SkBitmap* bitmap);

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

    class AutoValidate : SkNoncopyable {
    public:
        AutoValidate(const GrGradientBitmapCache* bc) : fBC(bc) { bc->validate(); }
        ~AutoValidate() { fBC->validate(); }
    private:
        const GrGradientBitmapCache* fBC;
    };
};

#endif
