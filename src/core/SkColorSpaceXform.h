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
     *  The src is opaque and stored in RGBA_8888, and the dst is also opaque and stored
     *  in 8888 platform format.
     */
    virtual void xform_RGB1_8888(uint32_t* dst, const uint32_t* src, uint32_t len) const = 0;

    virtual ~SkColorSpaceXform() {}
};

template <SkColorSpace::GammaNamed Src, SkColorSpace::GammaNamed Dst>
class SkFastXform : public SkColorSpaceXform {
public:

    void xform_RGB1_8888(uint32_t* dst, const uint32_t* src, uint32_t len) const override;

private:
    SkFastXform(const SkMatrix44& srcToDst);

    float fSrcToDst[12];

    friend class SkColorSpaceXform;
};

/**
 *  Works for any valid src and dst profiles.
 */
class SkDefaultXform : public SkColorSpaceXform {
public:

    void xform_RGB1_8888(uint32_t* dst, const uint32_t* src, uint32_t len) const override;

private:
    SkDefaultXform(const sk_sp<SkColorSpace>& srcSpace, const SkMatrix44& srcToDst,
                   const sk_sp<SkColorSpace>& dstSpace);

    static constexpr int      kDstGammaTableSize = 1024;

    sk_sp<SkColorLookUpTable> fColorLUT;

    // May contain pointers into storage or pointers into precomputed tables.
    const float*              fSrcGammaTables[3];
    float                     fSrcGammaTableStorage[3 * 256];

    const SkMatrix44          fSrcToDst;

    // May contain pointers into storage or pointers into precomputed tables.
    const uint8_t*            fDstGammaTables[3];
    uint8_t                   fDstGammaTableStorage[3 * kDstGammaTableSize];

    friend class SkColorSpaceXform;
};

#endif
