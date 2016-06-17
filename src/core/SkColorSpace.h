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

#include "SkMatrix44.h"
#include "SkRefCnt.h"
#include "../private/SkTemplates.h"

struct SkColorLookUpTable;
struct SkGammaCurve;
struct SkGammas;

class SkColorSpace : public SkRefCnt {
public:

    enum Named {
        kUnknown_Named,
        kSRGB_Named,
        kAdobeRGB_Named,
    };

    /**
     *  Given the src gamma and a transform from src gamut to D50_XYZ, return a SkColorSpace.
     */
    static sk_sp<SkColorSpace> NewRGB(float gammas[3], const SkMatrix44& toXYZD50);

    static sk_sp<SkColorSpace> NewNamed(Named);
    static sk_sp<SkColorSpace> NewICC(const void*, size_t);

    /**
     *  Used only by test code.
     */
    SkGammas* gammas() const { return fGammas.get(); }

    SkMatrix44 xyz() const { return fToXYZD50; }
    Named named() const { return fNamed; }
    uint32_t uniqueID() const { return fUniqueID; }

private:

    static bool LoadGammas(SkGammaCurve* gammas, uint32_t num, const uint8_t* src, size_t len);

    static bool LoadColorLUT(SkColorLookUpTable* colorLUT, uint32_t inputChannels,
                             uint32_t outputChannels, const uint8_t* src, size_t len);

    static bool LoadA2B0(SkColorLookUpTable* colorLUT, SkGammaCurve*, SkMatrix44* toXYZ,
                         const uint8_t* src, size_t len);

    SkColorSpace(sk_sp<SkGammas> gammas, const SkMatrix44& toXYZ, Named);

    SkColorSpace(SkColorLookUpTable* colorLUT, sk_sp<SkGammas> gammas, const SkMatrix44& toXYZ);

    SkAutoTDelete<SkColorLookUpTable> fColorLUT;
    sk_sp<SkGammas>                   fGammas;
    const SkMatrix44                  fToXYZD50;

    const uint32_t                    fUniqueID;
    const Named                       fNamed;
};

#endif
