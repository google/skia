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
    virtual void applyTo8888(SkPMColor* dst, const RGBA32* src, int len) const = 0;
    virtual void applyToF16(RGBAF16* dst, const RGBA32* src, int len) const = 0;

    virtual ~SkColorSpaceXform() {}
};

template <SkColorSpace::GammaNamed Dst>
class SkFastXform : public SkColorSpaceXform {
public:

    void applyTo8888(SkPMColor* dst, const RGBA32* src, int len) const override;
    void applyToF16(RGBAF16* dst, const RGBA32* src, int len) const override;

private:
    SkFastXform(const sk_sp<SkColorSpace>& srcSpace, const SkMatrix44& srcToDst,
                const sk_sp<SkColorSpace>& dstSpace);

    static constexpr int kDstGammaTableSize = 1024;

    // May contain pointers into storage or pointers into precomputed tables.
    const float*         fSrcGammaTables[3];
    float                fSrcGammaTableStorage[3 * 256];

    float                fSrcToDst[12];

    // May contain pointers into storage or pointers into precomputed tables.
    const uint8_t*       fDstGammaTables[3];
    uint8_t              fDstGammaTableStorage[3 * kDstGammaTableSize];

    friend class SkColorSpaceXform;
};

/**
 *  Works for any valid src and dst profiles.
 */
// TODO (msarett):
// Merge with SkFastXform and delete this.  SkFastXform can almost do everything that
// this does.
class SkDefaultXform : public SkColorSpaceXform {
public:

    void applyTo8888(SkPMColor* dst, const RGBA32* src, int len) const override;
    void applyToF16(RGBAF16* dst, const RGBA32* src, int len) const override;

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
