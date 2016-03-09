/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapProcShader_DEFINED
#define SkBitmapProcShader_DEFINED

#include "SkShader.h"
#include "SkSmallAllocator.h"

struct SkBitmapProcState;
class SkBitmapProvider;

class SkBitmapProcShader : public SkShader {
public:
    SkBitmapProcShader(const SkBitmap& src, TileMode tx, TileMode ty,
                       const SkMatrix* localMatrix = nullptr);

    bool isOpaque() const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBitmapProcShader)

#if SK_SUPPORT_GPU
    const GrFragmentProcessor* asFragmentProcessor(GrContext*, const SkMatrix& viewM,
                                                   const SkMatrix*, SkFilterQuality) const override;
#endif

protected:
    void flatten(SkWriteBuffer&) const override;
    size_t onContextSize(const ContextRec& rec) const override {
        return ContextSize(rec, fRawBitmap.info());
    }
    Context* onCreateContext(const ContextRec&, void* storage) const override;
    bool onIsABitmap(SkBitmap*, SkMatrix*, TileMode*) const override;

    SkBitmap    fRawBitmap;
    uint8_t     fTileModeX, fTileModeY;

private:
    friend class SkImageShader;

    static size_t ContextSize(const ContextRec&, const SkImageInfo& srcInfo);
    static Context* MakeContext(const SkShader&, TileMode tmx, TileMode tmy,
                                const SkBitmapProvider&, const ContextRec&, void* storage);

    typedef SkShader INHERITED;
};

// Commonly used allocator. It currently is only used to allocate up to 3 objects. The total
// bytes requested is calculated using one of our large shaders, its context size plus the size of
// an Sk3DBlitter in SkDraw.cpp
// Note that some contexts may contain other contexts (e.g. for compose shaders), but we've not
// yet found a situation where the size below isn't big enough.
typedef SkSmallAllocator<3, 2100> SkTBlitterAllocator;

// If alloc is non-nullptr, it will be used to allocate the returned SkShader, and MUST outlive
// the SkShader.
sk_sp<SkShader> SkMakeBitmapShader(const SkBitmap& src, SkShader::TileMode, SkShader::TileMode,
                                   const SkMatrix* localMatrix, SkTBlitterAllocator* alloc);

#endif
