/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkMipmap.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkMipmapBuilder.h"

#include <new>

//
// ColorTypeFilter is the "Type" we pass to some downsample template functions.
// It controls how we expand a pixel into a large type, with space between each component,
// so we can then perform our simple filter (either box or triangle) and store the intermediates
// in the expanded type.
//

///////////////////////////////////////////////////////////////////////////////////////////////////

SkMipmap::SkMipmap(void* malloc, size_t size) : SkCachedData(malloc, size) {}
SkMipmap::SkMipmap(size_t size, SkDiscardableMemory* dm) : SkCachedData(size, dm) {}

SkMipmap::~SkMipmap() = default;

size_t SkMipmap::AllocLevelsSize(int levelCount, size_t pixelSize) {
    if (levelCount < 0) {
        return 0;
    }
    int64_t size = sk_64_mul(levelCount + 1, sizeof(Level)) + pixelSize;
    if (!SkTFitsIn<int32_t>(size)) {
        return 0;
    }
    return SkTo<int32_t>(size);
}

SkMipmap* SkMipmap::Build(const SkPixmap& src, SkDiscardableFactoryProc fact,
                          bool computeContents) {
    if (src.width() <= 1 && src.height() <= 1) {
        return nullptr;
    }

    const SkColorType ct = src.colorType();
    const SkAlphaType at = src.alphaType();

    // whip through our loop to compute the exact size needed
    size_t size = 0;
    int countLevels = ComputeLevelCount(src.width(), src.height());
    for (int currentMipLevel = countLevels; currentMipLevel >= 0; currentMipLevel--) {
        SkISize mipSize = ComputeLevelSize(src.width(), src.height(), currentMipLevel);
        size += SkColorTypeMinRowBytes(ct, mipSize.fWidth) * mipSize.fHeight;
    }

    size_t storageSize = SkMipmap::AllocLevelsSize(countLevels, size);
    if (0 == storageSize) {
        return nullptr;
    }

    SkMipmap* mipmap;
    if (fact) {
        SkDiscardableMemory* dm = fact(storageSize);
        if (nullptr == dm) {
            return nullptr;
        }
        mipmap = new SkMipmap(storageSize, dm);
    } else {
        mipmap = new SkMipmap(sk_malloc_throw(storageSize), storageSize);
    }

    // init
    mipmap->fCS = sk_ref_sp(src.info().colorSpace());
    mipmap->fCount = countLevels;
    mipmap->fLevels = (Level*)mipmap->writable_data();
    SkASSERT(mipmap->fLevels);

    Level* levels = mipmap->fLevels;
    uint8_t*    baseAddr = (uint8_t*)&levels[countLevels];
    uint8_t*    addr = baseAddr;
    int         width = src.width();
    int         height = src.height();
    uint32_t    rowBytes;
    SkPixmap    srcPM(src);

    // Depending on architecture and other factors, the pixel data alignment may need to be as
    // large as 8 (for F16 pixels). See the comment on SkMipmap::Level.
    SkASSERT(SkIsAlign8((uintptr_t)addr));

    std::unique_ptr<SkMipmapDownSampler> downsampler;
    if (computeContents) {
        downsampler = SkMakeHQDownSampler(src);
//        downsampler = SkMakeDrawDownSampler(src);
        if (!downsampler) {
            return nullptr;
        }
    }

    for (int i = 0; i < countLevels; ++i) {
        width = std::max(1, width >> 1);
        height = std::max(1, height >> 1);
        rowBytes = SkToU32(SkColorTypeMinRowBytes(ct, width));

        // We make the Info w/o any colorspace, since that storage is not under our control, and
        // will not be deleted in a controlled fashion. When the caller is given the pixmap for
        // a given level, we augment this pixmap with fCS (which we do manage).
        new (&levels[i].fPixmap) SkPixmap(SkImageInfo::Make(width, height, ct, at), addr, rowBytes);
        levels[i].fScale  = SkSize::Make(SkIntToScalar(width)  / src.width(),
                                         SkIntToScalar(height) / src.height());

        const SkPixmap& dstPM = levels[i].fPixmap;
        if (downsampler) {
            downsampler->buildLevel(dstPM, srcPM);
        }
        srcPM = dstPM;
        addr += height * rowBytes;
    }
    SkASSERT(addr == baseAddr + size);

    SkASSERT(mipmap->fLevels);
    return mipmap;
}

int SkMipmap::ComputeLevelCount(int baseWidth, int baseHeight) {
    if (baseWidth < 1 || baseHeight < 1) {
        return 0;
    }

    // OpenGL's spec requires that each mipmap level have height/width equal to
    // max(1, floor(original_height / 2^i)
    // (or original_width) where i is the mipmap level.
    // Continue scaling down until both axes are size 1.

    const int largestAxis = std::max(baseWidth, baseHeight);
    if (largestAxis < 2) {
        // SkMipmap::Build requires a minimum size of 2.
        return 0;
    }
    const int leadingZeros = SkCLZ(static_cast<uint32_t>(largestAxis));
    // If the value 00011010 has 3 leading 0s then it has 5 significant bits
    // (the bits which are not leading zeros)
    const int significantBits = (sizeof(uint32_t) * 8) - leadingZeros;
    // This is making the assumption that the size of a byte is 8 bits
    // and that sizeof(uint32_t)'s implementation-defined behavior is 4.
    int mipLevelCount = significantBits;

    // SkMipmap does not include the base mip level.
    // For example, it contains levels 1-x instead of 0-x.
    // This is because the image used to create SkMipmap is the base level.
    // So subtract 1 from the mip level count.
    if (mipLevelCount > 0) {
        --mipLevelCount;
    }

    return mipLevelCount;
}

SkISize SkMipmap::ComputeLevelSize(int baseWidth, int baseHeight, int level) {
    if (baseWidth < 1 || baseHeight < 1) {
        return SkISize::Make(0, 0);
    }

    int maxLevelCount = ComputeLevelCount(baseWidth, baseHeight);
    if (level >= maxLevelCount || level < 0) {
        return SkISize::Make(0, 0);
    }
    // OpenGL's spec requires that each mipmap level have height/width equal to
    // max(1, floor(original_height / 2^i)
    // (or original_width) where i is the mipmap level.

    // SkMipmap does not include the base mip level.
    // For example, it contains levels 1-x instead of 0-x.
    // This is because the image used to create SkMipmap is the base level.
    // So subtract 1 from the mip level to get the index stored by SkMipmap.
    int width = std::max(1, baseWidth >> (level + 1));
    int height = std::max(1, baseHeight >> (level + 1));

    return SkISize::Make(width, height);
}

///////////////////////////////////////////////////////////////////////////////

// Returns fractional level value. floor(level) is the index of the larger level.
// < 0 means failure.
float SkMipmap::ComputeLevel(SkSize scaleSize) {
    SkASSERT(scaleSize.width() >= 0 && scaleSize.height() >= 0);

#ifndef SK_SUPPORT_LEGACY_ANISOTROPIC_MIPMAP_SCALE
    // Use the smallest scale to match the GPU impl.
    const float scale = std::min(scaleSize.width(), scaleSize.height());
#else
    // Ideally we'd pick the smaller scale, to match Ganesh.  But ignoring one of the
    // scales can produce some atrocious results, so for now we use the geometric mean.
    // (https://bugs.chromium.org/p/skia/issues/detail?id=4863)
    const float scale = sk_float_sqrt(scaleSize.width() * scaleSize.height());
#endif

    if (scale >= SK_Scalar1 || scale <= 0 || !SkScalarIsFinite(scale)) {
        return -1;
    }

    // The -0.5 bias here is to emulate GPU's sharpen mipmap option.
    float L = std::max(-SkScalarLog2(scale) - 0.5f, 0.f);
    if (!SkScalarIsFinite(L)) {
        return -1;
    }
    return L;
}

bool SkMipmap::extractLevel(SkSize scaleSize, Level* levelPtr) const {
    if (nullptr == fLevels) {
        return false;
    }

    float L = ComputeLevel(scaleSize);
    int level = sk_float_round2int(L);
    if (level <= 0) {
        return false;
    }

    if (level > fCount) {
        level = fCount;
    }
    if (levelPtr) {
        *levelPtr = fLevels[level - 1];
        // need to augment with our colorspace
        levelPtr->fPixmap.setColorSpace(fCS);
    }
    return true;
}

bool SkMipmap::validForRootLevel(const SkImageInfo& root) const {
    if (nullptr == fLevels) {
        return false;
    }

    const SkISize dimension = root.dimensions();
    if (dimension.width() <= 1 && dimension.height() <= 1) {
        return false;
    }

    if (fLevels[0].fPixmap. width() != std::max(1, dimension. width() >> 1) ||
        fLevels[0].fPixmap.height() != std::max(1, dimension.height() >> 1)) {
        return false;
    }

    for (int i = 0; i < this->countLevels(); ++i) {
        if (fLevels[i].fPixmap.colorType() != root.colorType() ||
            fLevels[i].fPixmap.alphaType() != root.alphaType()) {
            return false;
        }
    }
    return true;
}

// Helper which extracts a pixmap from the src bitmap
//
SkMipmap* SkMipmap::Build(const SkBitmap& src, SkDiscardableFactoryProc fact) {
    SkPixmap srcPixmap;
    if (!src.peekPixels(&srcPixmap)) {
        return nullptr;
    }
    return Build(srcPixmap, fact);
}

int SkMipmap::countLevels() const {
    return fCount;
}

bool SkMipmap::getLevel(int index, Level* levelPtr) const {
    if (nullptr == fLevels) {
        return false;
    }
    if (index < 0) {
        return false;
    }
    if (index > fCount - 1) {
        return false;
    }
    if (levelPtr) {
        *levelPtr = fLevels[index];
        // need to augment with our colorspace
        levelPtr->fPixmap.setColorSpace(fCS);
    }
    return true;
}
