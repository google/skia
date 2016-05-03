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
#include "SkMatrix44.h"

struct SkFloat3 {
    float fVec[3];

    void dump() const;
};

class SkColorSpace : public SkRefCnt {
private:
    struct SkGammaCurve {
        bool isValue() const {
            bool result = (0.0f != fValue);
            SkASSERT(!result || (0 == fTableSize));
            return result;
        }

        bool isTable() const {
            bool result = (0 != fTableSize);
            SkASSERT(!result || (0.0f == fValue));
            SkASSERT(!result || fTable);
            return result;
        }

        bool isParametric() const { return false; }

        // We have three different ways to represent gamma.
        // (1) A single value:
        float                    fValue;

        // (2) A lookup table:
        uint32_t                 fTableSize;
        std::unique_ptr<float[]> fTable;

        // (3) Parameters for a curve:
        // FIXME (msarett): Handle parametric curves.

        SkGammaCurve() {
            memset(this, 0, sizeof(struct SkGammaCurve));
        }

        SkGammaCurve(float value)
            : fValue(value)
            , fTableSize(0)
            , fTable(nullptr)
        {}
    };

    struct SkColorLookUpTable {
        static const uint8_t kMaxChannels = 16;

        uint8_t                  fInputChannels;
        uint8_t                  fOutputChannels;
        uint8_t                  fGridPoints[kMaxChannels];
        std::unique_ptr<float[]> fTable;

        SkColorLookUpTable() {
            memset(this, 0, sizeof(struct SkColorLookUpTable));
        }
    };

public:
    enum Named {
        kUnknown_Named,
        kSRGB_Named,
    };

    struct SkGammas {
    public:
        SkGammas(float red, float green, float blue)
            : fRed(red)
            , fGreen(green)
            , fBlue(blue)
        {}

        SkGammas() {}

        SkDEBUGCODE(float red() const { return fRed.fValue; })
        SkDEBUGCODE(float green() const { return fGreen.fValue; })
        SkDEBUGCODE(float blue() const { return fBlue.fValue; })

    private:
        SkGammaCurve fRed;
        SkGammaCurve fGreen;
        SkGammaCurve fBlue;

        friend class SkColorSpace;
    };

    /**
     *  Return a colorspace instance, given a transform from linear_RGB to D50_XYZ
     *  and the src-gamma, return a ColorSpace
     */
    static sk_sp<SkColorSpace> NewRGB(SkGammas gammas, const SkMatrix44& toXYZD50);

    static sk_sp<SkColorSpace> NewNamed(Named);
    static sk_sp<SkColorSpace> NewICC(const void*, size_t);

    const SkGammas& gammas() const { return fGammas; }
    SkMatrix44 xyz() const { return fToXYZD50; }
    Named named() const { return fNamed; }
    uint32_t uniqueID() const { return fUniqueID; }

private:

    static bool LoadGammas(SkGammaCurve* gammas, uint32_t num, const uint8_t* src, size_t len);


    static bool LoadColorLUT(SkColorLookUpTable* colorLUT, uint32_t inputChannels,
                             uint32_t outputChannels, const uint8_t* src, size_t len);


    static bool LoadA2B0(SkColorLookUpTable* colorLUT, SkGammas* gammas, SkMatrix44* toXYZ,
                         const uint8_t* src, size_t len);

    SkColorSpace(SkGammas gammas, const SkMatrix44& toXYZ, Named);

    SkColorSpace(SkColorLookUpTable colorLUT, SkGammas gammas,
                 const SkMatrix44& toXYZ);

    const SkColorLookUpTable fColorLUT;
    const SkGammas           fGammas;
    const SkMatrix44         fToXYZD50;

    const uint32_t           fUniqueID;
    const Named              fNamed;
};

#endif
