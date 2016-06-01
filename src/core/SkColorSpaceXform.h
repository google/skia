/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXform_DEFINED
#define SkColorSpaceXform_DEFINED

#include "SkColorSpace.h"

class SkColorSpaceXform {
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
     *  The src is stored in RGBA_8888 and the dst is stored in 8888 platform format.
     *  The output is not premultiplied.
     */
    virtual void xform_RGBA_8888(uint32_t* dst, const uint32_t* src, uint32_t len) const = 0;

    virtual ~SkColorSpaceXform() {}
};

class SkGammaByValueXform : public SkColorSpaceXform {
public:

    void xform_RGBA_8888(uint32_t* dst, const uint32_t* src, uint32_t len) const override;

private:
    SkGammaByValueXform(float srcGammas[3], const SkMatrix44& srcToDst, float dstGammas[3]);

    float            fSrcGammas[3];
    const SkMatrix44 fSrcToDst;
    float            fDstGammas[3];

    friend class SkColorSpaceXform;
};

#endif
