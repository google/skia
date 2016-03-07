/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpace_DEFINED
#define SkColorSpace_DEFINED

// Some terms
//
//  PCS : Profile Connection Space : where color number values have an absolute meaning.
//        Part of the work float is to convert colors to and from this space...
//        src_linear_unit_floats --> PCS --> PCS' --> dst_linear_unit_floats
//
// Some nice documents
//
// http://www.cambridgeincolour.com/tutorials/color-space-conversion.htm
// https://www.w3.org/Graphics/Color/srgb
// http://www.poynton.com/notes/colour_and_gamma/ColorFAQ.html
//

#include "SkRefCnt.h"

struct SkFloat3 {
    float fVec[3];

    void dump() const;
};

struct SkFloat3x3 {
    float fMat[9];

    void dump() const;
};

struct SkPM4f;
void SkApply3x3ToPM4f(const SkFloat3x3&, const SkPM4f src[], SkPM4f dst[], int count);

class SkColorSpace : public SkRefCnt {
public:
    enum Named {
        kUnknown_Named,
        kDevice_Named,
        kSRGB_Named,
    };

    /**
     *  Return a colorspace instance, given a 3x3 transform from linear_RGB to D50_XYZ
     *  and the src-gamma, return a ColorSpace
     */
    static sk_sp<SkColorSpace> NewRGB(const SkFloat3x3& toXYZD50, const SkFloat3& gamma);

    static sk_sp<SkColorSpace> NewNamed(Named);
    static sk_sp<SkColorSpace> NewICC(const void*, size_t);

    SkFloat3 gamma() const { return fGamma; }
    SkFloat3x3 xyz() const { return fToXYZD50; }
    Named named() const { return fNamed; }
    uint32_t uniqueID() const { return fUniqueID; }

    enum Result {
        kFailure_Result,
        kIdentity_Result,
        kNormal_Result,
    };

    /**
     *  Given a src and dst colorspace, return the 3x3 matrix that will convert src_linear_RGB
     *  values into dst_linear_RGB values.
     */
    static Result Concat(const SkColorSpace* src, const SkColorSpace* dst, SkFloat3x3* result);

    static void Test();
    void dump() const;

protected:
    SkColorSpace(const SkFloat3x3& toXYZ, const SkFloat3& gamma, Named);

private:
    const SkFloat3x3 fToXYZD50;
    const SkFloat3   fGamma;
    const uint32_t   fUniqueID;
    const Named      fNamed;
};

#endif
