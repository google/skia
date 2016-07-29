/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXform_DEFINED
#define SkColorSpaceXform_DEFINED

#include "SkColorSpace.h"
#include "SkColorSpace_Base.h"

class SkColorSpaceXform : SkNoncopyable {
public:

    typedef uint32_t RGBA32;
    typedef uint32_t BGRA32;
    typedef uint64_t RGBAF16;

    /**
     *  Create an object to handle color space conversions.
     *
     *  @param srcSpace The encoded color space.
     *  @param dstSpace The destination color space.
     *
     */
    static std::unique_ptr<SkColorSpaceXform> New(const sk_sp<SkColorSpace>& srcSpace,
                                                  const sk_sp<SkColorSpace>& dstSpace);

    /**
     *  Apply the color conversion to a src buffer, storing the output in the dst buffer.
     *  The src is stored as RGBA (8888) and is treated as opaque.
     *  TODO (msarett): Support non-opaque srcs.
     */
    virtual void applyToRGBA(RGBA32* dst, const RGBA32* src, int len) const = 0;
    virtual void applyToBGRA(BGRA32* dst, const RGBA32* src, int len) const = 0;
    virtual void applyToF16(RGBAF16* dst, const RGBA32* src, int len) const = 0;

    virtual ~SkColorSpaceXform() {}
};

template <SkColorSpace::GammaNamed Dst>
class SkColorSpaceXform_Base : public SkColorSpaceXform {
public:

    void applyToRGBA(RGBA32* dst, const RGBA32* src, int len) const override;
    void applyToBGRA(BGRA32* dst, const RGBA32* src, int len) const override;
    void applyToF16(RGBAF16* dst, const RGBA32* src, int len) const override;

    static constexpr int      kDstGammaTableSize = 1024;

private:
    SkColorSpaceXform_Base(const sk_sp<SkColorSpace>& srcSpace, const SkMatrix44& srcToDst,
                           const sk_sp<SkColorSpace>& dstSpace);

    sk_sp<SkColorLookUpTable> fColorLUT;

    // May contain pointers into storage or pointers into precomputed tables.
    const float*              fSrcGammaTables[3];
    float                     fSrcGammaTableStorage[3 * 256];

    float                     fSrcToDst[16];

    // May contain pointers into storage or pointers into precomputed tables.
    const uint8_t*            fDstGammaTables[3];
    uint8_t                   fDstGammaTableStorage[3 * kDstGammaTableSize];

    friend class SkColorSpaceXform;
};

#endif
