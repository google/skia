/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkTypes.h"
#include "include/private/SkColorData.h"
#include "include/private/SkHalf.h"
#include "include/private/SkImageInfoPriv.h"
#include "include/private/SkNx.h"
#include "include/private/SkTo.h"
#include "include/private/SkVx.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkMipmapBuilder.h"
#include <new>

//
// ColorTypeFilter is the "Type" we pass to some downsample template functions.
// It controls how we expand a pixel into a large type, with space between each component,
// so we can then perform our simple filter (either box or triangle) and store the intermediates
// in the expanded type.
//

struct ColorTypeFilter_8888 {
    typedef uint32_t Type;
    static Sk4h Expand(uint32_t x) {
        return SkNx_cast<uint16_t>(Sk4b::Load(&x));
    }
    static uint32_t Compact(const Sk4h& x) {
        uint32_t r;
        SkNx_cast<uint8_t>(x).store(&r);
        return r;
    }
};

struct ColorTypeFilter_565 {
    typedef uint16_t Type;
    static uint32_t Expand(uint16_t x) {
        return (x & ~SK_G16_MASK_IN_PLACE) | ((x & SK_G16_MASK_IN_PLACE) << 16);
    }
    static uint16_t Compact(uint32_t x) {
        return ((x & ~SK_G16_MASK_IN_PLACE) & 0xFFFF) | ((x >> 16) & SK_G16_MASK_IN_PLACE);
    }
};

struct ColorTypeFilter_4444 {
    typedef uint16_t Type;
    static uint32_t Expand(uint16_t x) {
        return (x & 0xF0F) | ((x & ~0xF0F) << 12);
    }
    static uint16_t Compact(uint32_t x) {
        return (x & 0xF0F) | ((x >> 12) & ~0xF0F);
    }
};

struct ColorTypeFilter_8 {
    typedef uint8_t Type;
    static unsigned Expand(unsigned x) {
        return x;
    }
    static uint8_t Compact(unsigned x) {
        return (uint8_t)x;
    }
};

struct ColorTypeFilter_Alpha_F16 {
    typedef uint16_t Type;
    static Sk4f Expand(uint16_t x) {
        return SkHalfToFloat_finite_ftz((uint64_t) x); // expand out to four lanes

    }
    static uint16_t Compact(const Sk4f& x) {
        uint64_t r;
        SkFloatToHalf_finite_ftz(x).store(&r);
        return r & 0xFFFF;  // but ignore the extra 3 here
    }
};

struct ColorTypeFilter_RGBA_F16 {
    typedef uint64_t Type; // SkHalf x4
    static Sk4f Expand(uint64_t x) {
        return SkHalfToFloat_finite_ftz(x);
    }
    static uint64_t Compact(const Sk4f& x) {
        uint64_t r;
        SkFloatToHalf_finite_ftz(x).store(&r);
        return r;
    }
};

struct ColorTypeFilter_88 {
    typedef uint16_t Type;
    static uint32_t Expand(uint16_t x) {
        return (x & 0xFF) | ((x & ~0xFF) << 8);
    }
    static uint16_t Compact(uint32_t x) {
        return (x & 0xFF) | ((x >> 8) & ~0xFF);
    }
};

struct ColorTypeFilter_1616 {
    typedef uint32_t Type;
    static uint64_t Expand(uint32_t x) {
        return (x & 0xFFFF) | ((x & ~0xFFFF) << 16);
    }
    static uint16_t Compact(uint64_t x) {
        return (x & 0xFFFF) | ((x >> 16) & ~0xFFFF);
    }
};

struct ColorTypeFilter_F16F16 {
    typedef uint32_t Type;
    static Sk4f Expand(uint32_t x) {
        return SkHalfToFloat_finite_ftz((uint64_t) x); // expand out to four lanes
    }
    static uint32_t Compact(const Sk4f& x) {
        uint64_t r;
        SkFloatToHalf_finite_ftz(x).store(&r);
        return (uint32_t) (r & 0xFFFFFFFF);  // but ignore the extra 2 here
    }
};

struct ColorTypeFilter_16161616 {
    typedef uint64_t Type;
    static skvx::Vec<4, uint32_t> Expand(uint64_t x) {
        return skvx::cast<uint32_t>(skvx::Vec<4, uint16_t>::Load(&x));
    }
    static uint64_t Compact(const skvx::Vec<4, uint32_t>& x) {
        uint64_t r;
        skvx::cast<uint16_t>(x).store(&r);
        return r;
    }
};

struct ColorTypeFilter_16 {
    typedef uint16_t Type;
    static uint32_t Expand(uint16_t x) {
        return x;
    }
    static uint16_t Compact(uint32_t x) {
        return (uint16_t) x;
    }
};

struct ColorTypeFilter_1010102 {
    typedef uint32_t Type;
    static uint64_t Expand(uint64_t x) {
        return (((x      ) & 0x3ff)      ) |
               (((x >> 10) & 0x3ff) << 20) |
               (((x >> 20) & 0x3ff) << 40) |
               (((x >> 30) & 0x3  ) << 60);
    }
    static uint32_t Compact(uint64_t x) {
        return (((x      ) & 0x3ff)      ) |
               (((x >> 20) & 0x3ff) << 10) |
               (((x >> 40) & 0x3ff) << 20) |
               (((x >> 60) & 0x3  ) << 30);
    }
};

template <typename T> T add_121(const T& a, const T& b, const T& c) {
    return a + b + b + c;
}

template <typename T> T shift_right(const T& x, int bits) {
    return x >> bits;
}

Sk4f shift_right(const Sk4f& x, int bits) {
    return x * (1.0f / (1 << bits));
}

template <typename T> T shift_left(const T& x, int bits) {
    return x << bits;
}

Sk4f shift_left(const Sk4f& x, int bits) {
    return x * (1 << bits);
}

//
//  To produce each mip level, we need to filter down by 1/2 (e.g. 100x100 -> 50,50)
//  If the starting dimension is odd, we floor the size of the lower level (e.g. 101 -> 50)
//  In those (odd) cases, we use a triangle filter, with 1-pixel overlap between samplings,
//  else for even cases, we just use a 2x box filter.
//
//  This produces 4 possible isotropic filters: 2x2 2x3 3x2 3x3 where WxH indicates the number of
//  src pixels we need to sample in each dimension to produce 1 dst pixel.
//
//  OpenGL expects a full mipmap stack to contain anisotropic space as well.
//  This means a 100x1 image would continue down to a 50x1 image, 25x1 image...
//  Because of this, we need 4 more anisotropic filters: 1x2, 1x3, 2x1, 3x1.

template <typename F> void downsample_1_2(void* dst, const void* src, size_t srcRB, int count) {
    SkASSERT(count > 0);
    auto p0 = static_cast<const typename F::Type*>(src);
    auto p1 = (const typename F::Type*)((const char*)p0 + srcRB);
    auto d = static_cast<typename F::Type*>(dst);

    for (int i = 0; i < count; ++i) {
        auto c00 = F::Expand(p0[0]);
        auto c10 = F::Expand(p1[0]);

        auto c = c00 + c10;
        d[i] = F::Compact(shift_right(c, 1));
        p0 += 2;
        p1 += 2;
    }
}

template <typename F> void downsample_1_3(void* dst, const void* src, size_t srcRB, int count) {
    SkASSERT(count > 0);
    auto p0 = static_cast<const typename F::Type*>(src);
    auto p1 = (const typename F::Type*)((const char*)p0 + srcRB);
    auto p2 = (const typename F::Type*)((const char*)p1 + srcRB);
    auto d = static_cast<typename F::Type*>(dst);

    for (int i = 0; i < count; ++i) {
        auto c00 = F::Expand(p0[0]);
        auto c10 = F::Expand(p1[0]);
        auto c20 = F::Expand(p2[0]);

        auto c = add_121(c00, c10, c20);
        d[i] = F::Compact(shift_right(c, 2));
        p0 += 2;
        p1 += 2;
        p2 += 2;
    }
}

template <typename F> void downsample_2_1(void* dst, const void* src, size_t srcRB, int count) {
    SkASSERT(count > 0);
    auto p0 = static_cast<const typename F::Type*>(src);
    auto d = static_cast<typename F::Type*>(dst);

    for (int i = 0; i < count; ++i) {
        auto c00 = F::Expand(p0[0]);
        auto c01 = F::Expand(p0[1]);

        auto c = c00 + c01;
        d[i] = F::Compact(shift_right(c, 1));
        p0 += 2;
    }
}

template <typename F> void downsample_2_2(void* dst, const void* src, size_t srcRB, int count) {
    SkASSERT(count > 0);
    auto p0 = static_cast<const typename F::Type*>(src);
    auto p1 = (const typename F::Type*)((const char*)p0 + srcRB);
    auto d = static_cast<typename F::Type*>(dst);

    for (int i = 0; i < count; ++i) {
        auto c00 = F::Expand(p0[0]);
        auto c01 = F::Expand(p0[1]);
        auto c10 = F::Expand(p1[0]);
        auto c11 = F::Expand(p1[1]);

        auto c = c00 + c10 + c01 + c11;
        d[i] = F::Compact(shift_right(c, 2));
        p0 += 2;
        p1 += 2;
    }
}

template <typename F> void downsample_2_3(void* dst, const void* src, size_t srcRB, int count) {
    SkASSERT(count > 0);
    auto p0 = static_cast<const typename F::Type*>(src);
    auto p1 = (const typename F::Type*)((const char*)p0 + srcRB);
    auto p2 = (const typename F::Type*)((const char*)p1 + srcRB);
    auto d = static_cast<typename F::Type*>(dst);

    for (int i = 0; i < count; ++i) {
        auto c00 = F::Expand(p0[0]);
        auto c01 = F::Expand(p0[1]);
        auto c10 = F::Expand(p1[0]);
        auto c11 = F::Expand(p1[1]);
        auto c20 = F::Expand(p2[0]);
        auto c21 = F::Expand(p2[1]);

        auto c = add_121(c00, c10, c20) + add_121(c01, c11, c21);
        d[i] = F::Compact(shift_right(c, 3));
        p0 += 2;
        p1 += 2;
        p2 += 2;
    }
}

template <typename F> void downsample_3_1(void* dst, const void* src, size_t srcRB, int count) {
    SkASSERT(count > 0);
    auto p0 = static_cast<const typename F::Type*>(src);
    auto d = static_cast<typename F::Type*>(dst);

    auto c02 = F::Expand(p0[0]);
    for (int i = 0; i < count; ++i) {
        auto c00 = c02;
        auto c01 = F::Expand(p0[1]);
             c02 = F::Expand(p0[2]);

        auto c = add_121(c00, c01, c02);
        d[i] = F::Compact(shift_right(c, 2));
        p0 += 2;
    }
}

template <typename F> void downsample_3_2(void* dst, const void* src, size_t srcRB, int count) {
    SkASSERT(count > 0);
    auto p0 = static_cast<const typename F::Type*>(src);
    auto p1 = (const typename F::Type*)((const char*)p0 + srcRB);
    auto d = static_cast<typename F::Type*>(dst);

    // Given pixels:
    // a0 b0 c0 d0 e0 ...
    // a1 b1 c1 d1 e1 ...
    // We want:
    // (a0 + 2*b0 + c0 + a1 + 2*b1 + c1) / 8
    // (c0 + 2*d0 + e0 + c1 + 2*d1 + e1) / 8
    // ...

    auto c0 = F::Expand(p0[0]);
    auto c1 = F::Expand(p1[0]);
    auto c = c0 + c1;
    for (int i = 0; i < count; ++i) {
        auto a = c;

        auto b0 = F::Expand(p0[1]);
        auto b1 = F::Expand(p1[1]);
        auto b = b0 + b0 + b1 + b1;

        c0 = F::Expand(p0[2]);
        c1 = F::Expand(p1[2]);
        c = c0 + c1;

        auto sum = a + b + c;
        d[i] = F::Compact(shift_right(sum, 3));
        p0 += 2;
        p1 += 2;
    }
}

template <typename F> void downsample_3_3(void* dst, const void* src, size_t srcRB, int count) {
    SkASSERT(count > 0);
    auto p0 = static_cast<const typename F::Type*>(src);
    auto p1 = (const typename F::Type*)((const char*)p0 + srcRB);
    auto p2 = (const typename F::Type*)((const char*)p1 + srcRB);
    auto d = static_cast<typename F::Type*>(dst);

    // Given pixels:
    // a0 b0 c0 d0 e0 ...
    // a1 b1 c1 d1 e1 ...
    // a2 b2 c2 d2 e2 ...
    // We want:
    // (a0 + 2*b0 + c0 + 2*a1 + 4*b1 + 2*c1 + a2 + 2*b2 + c2) / 16
    // (c0 + 2*d0 + e0 + 2*c1 + 4*d1 + 2*e1 + c2 + 2*d2 + e2) / 16
    // ...

    auto c0 = F::Expand(p0[0]);
    auto c1 = F::Expand(p1[0]);
    auto c2 = F::Expand(p2[0]);
    auto c = add_121(c0, c1, c2);
    for (int i = 0; i < count; ++i) {
        auto a = c;

        auto b0 = F::Expand(p0[1]);
        auto b1 = F::Expand(p1[1]);
        auto b2 = F::Expand(p2[1]);
        auto b = shift_left(add_121(b0, b1, b2), 1);

        c0 = F::Expand(p0[2]);
        c1 = F::Expand(p1[2]);
        c2 = F::Expand(p2[2]);
        c = add_121(c0, c1, c2);

        auto sum = a + b + c;
        d[i] = F::Compact(shift_right(sum, 4));
        p0 += 2;
        p1 += 2;
        p2 += 2;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

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
    typedef void FilterProc(void*, const void* srcPtr, size_t srcRB, int count);

    FilterProc* proc_1_2 = nullptr;
    FilterProc* proc_1_3 = nullptr;
    FilterProc* proc_2_1 = nullptr;
    FilterProc* proc_2_2 = nullptr;
    FilterProc* proc_2_3 = nullptr;
    FilterProc* proc_3_1 = nullptr;
    FilterProc* proc_3_2 = nullptr;
    FilterProc* proc_3_3 = nullptr;

    const SkColorType ct = src.colorType();
    const SkAlphaType at = src.alphaType();

    switch (ct) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            proc_1_2 = downsample_1_2<ColorTypeFilter_8888>;
            proc_1_3 = downsample_1_3<ColorTypeFilter_8888>;
            proc_2_1 = downsample_2_1<ColorTypeFilter_8888>;
            proc_2_2 = downsample_2_2<ColorTypeFilter_8888>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_8888>;
            proc_3_1 = downsample_3_1<ColorTypeFilter_8888>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_8888>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_8888>;
            break;
        case kRGB_565_SkColorType:
            proc_1_2 = downsample_1_2<ColorTypeFilter_565>;
            proc_1_3 = downsample_1_3<ColorTypeFilter_565>;
            proc_2_1 = downsample_2_1<ColorTypeFilter_565>;
            proc_2_2 = downsample_2_2<ColorTypeFilter_565>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_565>;
            proc_3_1 = downsample_3_1<ColorTypeFilter_565>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_565>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_565>;
            break;
        case kARGB_4444_SkColorType:
            proc_1_2 = downsample_1_2<ColorTypeFilter_4444>;
            proc_1_3 = downsample_1_3<ColorTypeFilter_4444>;
            proc_2_1 = downsample_2_1<ColorTypeFilter_4444>;
            proc_2_2 = downsample_2_2<ColorTypeFilter_4444>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_4444>;
            proc_3_1 = downsample_3_1<ColorTypeFilter_4444>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_4444>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_4444>;
            break;
        case kAlpha_8_SkColorType:
        case kGray_8_SkColorType:
            proc_1_2 = downsample_1_2<ColorTypeFilter_8>;
            proc_1_3 = downsample_1_3<ColorTypeFilter_8>;
            proc_2_1 = downsample_2_1<ColorTypeFilter_8>;
            proc_2_2 = downsample_2_2<ColorTypeFilter_8>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_8>;
            proc_3_1 = downsample_3_1<ColorTypeFilter_8>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_8>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_8>;
            break;
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType:
            proc_1_2 = downsample_1_2<ColorTypeFilter_RGBA_F16>;
            proc_1_3 = downsample_1_3<ColorTypeFilter_RGBA_F16>;
            proc_2_1 = downsample_2_1<ColorTypeFilter_RGBA_F16>;
            proc_2_2 = downsample_2_2<ColorTypeFilter_RGBA_F16>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_RGBA_F16>;
            proc_3_1 = downsample_3_1<ColorTypeFilter_RGBA_F16>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_RGBA_F16>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_RGBA_F16>;
            break;
        case kR8G8_unorm_SkColorType:
            proc_1_2 = downsample_1_2<ColorTypeFilter_88>;
            proc_1_3 = downsample_1_3<ColorTypeFilter_88>;
            proc_2_1 = downsample_2_1<ColorTypeFilter_88>;
            proc_2_2 = downsample_2_2<ColorTypeFilter_88>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_88>;
            proc_3_1 = downsample_3_1<ColorTypeFilter_88>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_88>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_88>;
            break;
        case kR16G16_unorm_SkColorType:
            proc_1_2 = downsample_1_2<ColorTypeFilter_1616>;
            proc_1_3 = downsample_1_3<ColorTypeFilter_1616>;
            proc_2_1 = downsample_2_1<ColorTypeFilter_1616>;
            proc_2_2 = downsample_2_2<ColorTypeFilter_1616>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_1616>;
            proc_3_1 = downsample_3_1<ColorTypeFilter_1616>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_1616>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_1616>;
            break;
        case kA16_unorm_SkColorType:
            proc_1_2 = downsample_1_2<ColorTypeFilter_16>;
            proc_1_3 = downsample_1_3<ColorTypeFilter_16>;
            proc_2_1 = downsample_2_1<ColorTypeFilter_16>;
            proc_2_2 = downsample_2_2<ColorTypeFilter_16>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_16>;
            proc_3_1 = downsample_3_1<ColorTypeFilter_16>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_16>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_16>;
            break;
        case kRGBA_1010102_SkColorType:
        case kBGRA_1010102_SkColorType:
            proc_1_2 = downsample_1_2<ColorTypeFilter_1010102>;
            proc_1_3 = downsample_1_3<ColorTypeFilter_1010102>;
            proc_2_1 = downsample_2_1<ColorTypeFilter_1010102>;
            proc_2_2 = downsample_2_2<ColorTypeFilter_1010102>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_1010102>;
            proc_3_1 = downsample_3_1<ColorTypeFilter_1010102>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_1010102>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_1010102>;
            break;
        case kA16_float_SkColorType:
            proc_1_2 = downsample_1_2<ColorTypeFilter_Alpha_F16>;
            proc_1_3 = downsample_1_3<ColorTypeFilter_Alpha_F16>;
            proc_2_1 = downsample_2_1<ColorTypeFilter_Alpha_F16>;
            proc_2_2 = downsample_2_2<ColorTypeFilter_Alpha_F16>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_Alpha_F16>;
            proc_3_1 = downsample_3_1<ColorTypeFilter_Alpha_F16>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_Alpha_F16>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_Alpha_F16>;
            break;
        case kR16G16_float_SkColorType:
            proc_1_2 = downsample_1_2<ColorTypeFilter_F16F16>;
            proc_1_3 = downsample_1_3<ColorTypeFilter_F16F16>;
            proc_2_1 = downsample_2_1<ColorTypeFilter_F16F16>;
            proc_2_2 = downsample_2_2<ColorTypeFilter_F16F16>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_F16F16>;
            proc_3_1 = downsample_3_1<ColorTypeFilter_F16F16>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_F16F16>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_F16F16>;
            break;
        case kR16G16B16A16_unorm_SkColorType:
            proc_1_2 = downsample_1_2<ColorTypeFilter_16161616>;
            proc_1_3 = downsample_1_3<ColorTypeFilter_16161616>;
            proc_2_1 = downsample_2_1<ColorTypeFilter_16161616>;
            proc_2_2 = downsample_2_2<ColorTypeFilter_16161616>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_16161616>;
            proc_3_1 = downsample_3_1<ColorTypeFilter_16161616>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_16161616>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_16161616>;
            break;

        case kUnknown_SkColorType:
        case kRGB_888x_SkColorType:     // TODO: use 8888?
        case kRGB_101010x_SkColorType:  // TODO: use 1010102?
        case kBGR_101010x_SkColorType:  // TODO: use 1010102?
        case kRGBA_F32_SkColorType:
            return nullptr;
    }

    if (src.width() <= 1 && src.height() <= 1) {
        return nullptr;
    }
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

    for (int i = 0; i < countLevels; ++i) {
        FilterProc* proc;
        if (height & 1) {
            if (height == 1) {        // src-height is 1
                if (width & 1) {      // src-width is 3
                    proc = proc_3_1;
                } else {              // src-width is 2
                    proc = proc_2_1;
                }
            } else {                  // src-height is 3
                if (width & 1) {
                    if (width == 1) { // src-width is 1
                        proc = proc_1_3;
                    } else {          // src-width is 3
                        proc = proc_3_3;
                    }
                } else {              // src-width is 2
                    proc = proc_2_3;
                }
            }
        } else {                      // src-height is 2
            if (width & 1) {
                if (width == 1) {     // src-width is 1
                    proc = proc_1_2;
                } else {              // src-width is 3
                    proc = proc_3_2;
                }
            } else {                  // src-width is 2
                proc = proc_2_2;
            }
        }
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
        if (computeContents) {
            const void* srcBasePtr = srcPM.addr();
            void* dstBasePtr = dstPM.writable_addr();

            const size_t srcRB = srcPM.rowBytes();
            for (int y = 0; y < height; y++) {
                proc(dstBasePtr, srcBasePtr, srcRB, width);
                srcBasePtr = (char*)srcBasePtr + srcRB * 2; // jump two rows
                dstBasePtr = (char*)dstBasePtr + dstPM.rowBytes();
            }
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
    const SkScalar scale = std::min(scaleSize.width(), scaleSize.height());
#else
    // Ideally we'd pick the smaller scale, to match Ganesh.  But ignoring one of the
    // scales can produce some atrocious results, so for now we use the geometric mean.
    // (https://bugs.chromium.org/p/skia/issues/detail?id=4863)
    const SkScalar scale = SkScalarSqrt(scaleSize.width() * scaleSize.height());
#endif

    if (scale >= SK_Scalar1 || scale <= 0 || !SkScalarIsFinite(scale)) {
        return -1;
    }

    SkScalar L = -SkScalarLog2(scale);
    if (!SkScalarIsFinite(L)) {
        return -1;
    }
    SkASSERT(L >= 0);
    return L;
}

bool SkMipmap::extractLevel(SkSize scaleSize, Level* levelPtr) const {
    if (nullptr == fLevels) {
        return false;
    }

    float L = ComputeLevel(scaleSize);
    int level = SkScalarFloorToInt(L);
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

    const SkPixmap& pm = fLevels[0].fPixmap;
    if (pm. width() != std::max(1, dimension. width() >> 1) ||
        pm.height() != std::max(1, dimension.height() >> 1)) {
        return false;
    }

    for (int i = 0; i < this->countLevels(); ++i) {
        const SkPixmap& pm = fLevels[0].fPixmap;
        if (pm.colorType() != root.colorType() ||
            pm.alphaType() != root.alphaType())
            return false;
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

//////////////////////////////////////////////////////////////////////////////////////////////////

#include "include/core/SkImageGenerator.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

static sk_sp<SkData> encode_to_data(const SkPixmap& pm) {
    SkDynamicMemoryWStream stream;
    if (SkPngEncoder::Encode(&stream, pm, SkPngEncoder::Options())) {
        return stream.detachAsData();
    }
    return nullptr;
}

/*  Format
        count_levels:32
        for each level, starting with the biggest (index 0 in our iterator)
            encoded_size:32
            encoded_data (padded)
 */
sk_sp<SkData> SkMipmap::serialize() const {
    const int count = this->countLevels();

    SkBinaryWriteBuffer buffer;
    buffer.write32(count);
    for (int i = 0; i < count; ++i) {
        Level level;
        if (this->getLevel(i, &level)) {
            buffer.writeDataAsByteArray(encode_to_data(level.fPixmap).get());
        } else {
            return nullptr;
        }
    }
    return buffer.snapshotAsData();
}

bool SkMipmap::Deserialize(SkMipmapBuilder* builder, const void* data, size_t size) {
    SkReadBuffer buffer(data, size);

    int count = buffer.read32();
    if (builder->countLevels() != count) {
        return false;
    }
    for (int i = 0; i < count; ++i) {
        size_t size = buffer.read32();
        const void* ptr = buffer.skip(size);
        if (!ptr) {
            return false;
        }
        auto gen = SkImageGenerator::MakeFromEncoded(
                             SkData::MakeWithProc(ptr, size, nullptr, nullptr));
        if (!gen) {
            return false;
        }

        SkPixmap pm = builder->level(i);
        if (gen->getInfo().dimensions() != pm.dimensions()) {
            return false;
        }
        if (!gen->getPixels(pm)) {
            return false;
        }
    }
    return buffer.isValid();
}
