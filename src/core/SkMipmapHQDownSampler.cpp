/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#ifndef SK_USE_DRAWING_MIPMAP_DOWNSAMPLER

#include "include/private/SkColorData.h"
#include "src/base/SkHalf.h"
#include "src/base/SkVx.h"
#include "src/core/SkMipmap.h"

namespace {

struct ColorTypeFilter_8888 {
    typedef uint32_t Type;
    static skvx::Vec<4, uint16_t> Expand(uint32_t x) {
        return skvx::cast<uint16_t>(skvx::byte4::Load(&x));
    }
    static uint32_t Compact(const skvx::Vec<4, uint16_t>& x) {
        uint32_t r;
        skvx::cast<uint8_t>(x).store(&r);
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
    static skvx::float4 Expand(uint16_t x) {
        return SkHalfToFloat_finite_ftz((uint64_t) x); // expand out to four lanes

    }
    static uint16_t Compact(const skvx::float4& x) {
        uint64_t r;
        SkFloatToHalf_finite_ftz(x).store(&r);
        return r & 0xFFFF;  // but ignore the extra 3 here
    }
};

struct ColorTypeFilter_RGBA_F16 {
    typedef uint64_t Type; // SkHalf x4
    static skvx::float4 Expand(uint64_t x) {
        return SkHalfToFloat_finite_ftz(x);
    }
    static uint64_t Compact(const skvx::float4& x) {
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
    static skvx::float4 Expand(uint32_t x) {
        return SkHalfToFloat_finite_ftz((uint64_t) x); // expand out to four lanes
    }
    static uint32_t Compact(const skvx::float4& x) {
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

skvx::float4 shift_right(const skvx::float4& x, int bits) {
    return x * (1.0f / (1 << bits));
}

template <typename T> T shift_left(const T& x, int bits) {
    return x << bits;
}

skvx::float4 shift_left(const skvx::float4& x, int bits) {
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


typedef void FilterProc(void*, const void* srcPtr, size_t srcRB, int count);

struct HQDownSampler : SkMipmapDownSampler {
    FilterProc* proc_1_2 = nullptr;
    FilterProc* proc_1_3 = nullptr;
    FilterProc* proc_2_1 = nullptr;
    FilterProc* proc_2_2 = nullptr;
    FilterProc* proc_2_3 = nullptr;
    FilterProc* proc_3_1 = nullptr;
    FilterProc* proc_3_2 = nullptr;
    FilterProc* proc_3_3 = nullptr;

    void buildLevel(const SkPixmap& dst, const SkPixmap& src) override;
};

void HQDownSampler::buildLevel(const SkPixmap& dst, const SkPixmap& src) {
    const int width = src.width();
    const int height = src.height();

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

    const void* srcBasePtr = src.addr();
    const size_t srcRB = src.rowBytes();
    void* dstBasePtr = dst.writable_addr();

    for (int y = 0; y < dst.height(); y++) {
        proc(dstBasePtr, srcBasePtr, srcRB, dst.width());
        srcBasePtr = (const char*)srcBasePtr + srcRB * 2; // jump two rows
        dstBasePtr = (      char*)dstBasePtr + dst.rowBytes();
    }
}

} // namespace

std::unique_ptr<SkMipmapDownSampler> SkMipmap::MakeDownSampler(const SkPixmap& root) {
    FilterProc* proc_1_2 = nullptr;
    FilterProc* proc_1_3 = nullptr;
    FilterProc* proc_2_1 = nullptr;
    FilterProc* proc_2_2 = nullptr;
    FilterProc* proc_2_3 = nullptr;
    FilterProc* proc_3_1 = nullptr;
    FilterProc* proc_3_2 = nullptr;
    FilterProc* proc_3_3 = nullptr;

    switch (root.colorType()) {
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
        case kR8_unorm_SkColorType:
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
        case kBGR_101010x_XR_SkColorType:  // TODO: use 1010102?
        case kRGBA_10x6_SkColorType:
        case kRGBA_F32_SkColorType:
            return nullptr;

        case kSRGBA_8888_SkColorType:  // TODO: needs careful handling
            return nullptr;
    }

    auto sampler = std::make_unique<HQDownSampler>();
    sampler->proc_1_2 = proc_1_2;
    sampler->proc_1_3 = proc_1_3;
    sampler->proc_2_1 = proc_2_1;
    sampler->proc_2_2 = proc_2_2;
    sampler->proc_2_3 = proc_2_3;
    sampler->proc_3_1 = proc_3_1;
    sampler->proc_3_2 = proc_3_2;
    sampler->proc_3_3 = proc_3_3;
    return sampler;
}

#endif
