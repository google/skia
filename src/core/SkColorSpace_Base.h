/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpace_Base_DEFINED
#define SkColorSpace_Base_DEFINED

#include "SkColorSpace.h"
#include "SkData.h"
#include "SkOnce.h"
#include "SkTemplates.h"

enum SkGammaNamed : uint8_t {
    kLinear_SkGammaNamed,
    kSRGB_SkGammaNamed,
    k2Dot2Curve_SkGammaNamed,
    kNonStandard_SkGammaNamed,
};

struct SkGammas : SkRefCnt {

    // There are four possible representations for gamma curves.  kNone_Type is used
    // as a placeholder until the struct is initialized.  It is not a valid value.
    enum class Type : uint8_t {
        kNone_Type,
        kNamed_Type,
        kValue_Type,
        kTable_Type,
        kParam_Type,
    };

    // Contains information for a gamma table.
    struct Table {
        size_t fOffset;
        int    fSize;

        const float* table(const SkGammas* base) const {
            return SkTAddOffset<const float>(base, sizeof(SkGammas) + fOffset);
        }
    };

    // Contains the parameters for a parametric curve.
    struct Params {
        //     Y = (aX + b)^g + c  for X >= d
        //     Y = eX + f          otherwise
        float                    fG;
        float                    fA;
        float                    fB;
        float                    fC;
        float                    fD;
        float                    fE;
        float                    fF;
    };

    // Contains the actual gamma curve information.  Should be interpreted
    // based on the type of the gamma curve.
    union Data {
        Data()
            : fTable{ 0, 0 }
        {}

        inline bool operator==(const Data& that) const {
            return this->fTable.fOffset == that.fTable.fOffset &&
                   this->fTable.fSize == that.fTable.fSize;
        }

        SkGammaNamed             fNamed;
        float                    fValue;
        Table                    fTable;
        size_t                   fParamOffset;

        const Params& params(const SkGammas* base) const {
            return *SkTAddOffset<const Params>(base, sizeof(SkGammas) + fParamOffset);
        }
    };

    bool isNamed(int i) const {
        return Type::kNamed_Type == this->type(i);
    }

    bool isValue(int i) const {
        return Type::kValue_Type == this->type(i);
    }

    bool isTable(int i) const {
        return Type::kTable_Type == this->type(i);
    }

    bool isParametric(int i) const {
        return Type::kParam_Type == this->type(i);
    }

    const Data& data(int i) const {
        switch (i) {
            case 0:
                return fRedData;
            case 1:
                return fGreenData;
            case 2:
                return fBlueData;
            default:
                SkASSERT(false);
                return fRedData;
        }
    }

    const float* table(int i) const {
        SkASSERT(isTable(i));
        return this->data(i).fTable.table(this);
    }

    const Params& params(int i) const {
        SkASSERT(isParametric(i));
        return this->data(i).params(this);
    }

    Type type(int i) const {
        switch (i) {
            case 0:
                return fRedType;
            case 1:
                return fGreenType;
            case 2:
                return fBlueType;
            default:
                SkASSERT(false);
                return fRedType;
        }
    }

    SkGammas()
        : fRedType(Type::kNone_Type)
        , fGreenType(Type::kNone_Type)
        , fBlueType(Type::kNone_Type)
    {}

    // These fields should only be modified when initializing the struct.
    Data fRedData;
    Data fGreenData;
    Data fBlueData;
    Type fRedType;
    Type fGreenType;
    Type fBlueType;

    // Objects of this type are sometimes created in a custom fashion using
    // sk_malloc_throw and therefore must be sk_freed.  We overload new to
    // also call sk_malloc_throw so that memory can be unconditionally released
    // using sk_free in an overloaded delete. Overloading regular new means we
    // must also overload placement new.
    void* operator new(size_t size) { return sk_malloc_throw(size); }
    void* operator new(size_t, void* p) { return p; }
    void operator delete(void* p) { sk_free(p); }
};

struct SkColorLookUpTable : public SkRefCnt {
    static constexpr uint8_t kOutputChannels = 3;

    uint8_t                  fInputChannels;
    uint8_t                  fGridPoints[3];

    const float* table() const {
        return SkTAddOffset<const float>(this, sizeof(SkColorLookUpTable));
    }

    SkColorLookUpTable(uint8_t inputChannels, uint8_t gridPoints[3])
        : fInputChannels(inputChannels)
    {
        SkASSERT(3 == inputChannels);
        memcpy(fGridPoints, gridPoints, 3 * sizeof(uint8_t));
    }

    // Objects of this type are created in a custom fashion using sk_malloc_throw
    // and therefore must be sk_freed.
    void* operator new(size_t size) = delete;
    void* operator new(size_t, void* p) { return p; }
    void operator delete(void* p) { sk_free(p); }
};

class SkColorSpace_Base : public SkColorSpace {
public:

    static sk_sp<SkColorSpace> NewRGB(const float gammas[3], const SkMatrix44& toXYZD50);

    SkGammaNamed gammaNamed() const { return fGammaNamed; }
    const SkGammas* gammas() const { return fGammas.get(); }

    const SkColorLookUpTable* colorLUT() const { return fColorLUT.get(); }

    const SkMatrix44& toXYZD50() const { return fToXYZD50; }
    const SkMatrix44& fromXYZD50() const;

private:

    /**
     *  FIXME (msarett):
     *  Hiding this function until we can determine if we need it.  Known issues include:
     *  Only writes 3x3 matrices
     *  Only writes float gammas
     *  Rejected by some parsers because the "profile description" is empty
     */
    sk_sp<SkData> writeToICC() const;

    static sk_sp<SkColorSpace> NewRGB(SkGammaNamed gammaNamed, const SkMatrix44& toXYZD50);

    SkColorSpace_Base(SkGammaNamed gammaNamed, const SkMatrix44& toXYZ);

    SkColorSpace_Base(sk_sp<SkColorLookUpTable> colorLUT, SkGammaNamed gammaNamed,
                      sk_sp<SkGammas> gammas, const SkMatrix44& toXYZ, sk_sp<SkData> profileData);

    sk_sp<SkColorLookUpTable> fColorLUT;
    const SkGammaNamed        fGammaNamed;
    sk_sp<SkGammas>           fGammas;
    sk_sp<SkData>             fProfileData;

    const SkMatrix44          fToXYZD50;
    mutable SkMatrix44        fFromXYZD50;
    mutable SkOnce            fFromXYZOnce;

    friend class SkColorSpace;
    friend class ColorSpaceXformTest;
    friend class ColorSpaceTest;
    typedef SkColorSpace INHERITED;
};

static inline SkColorSpace_Base* as_CSB(SkColorSpace* colorSpace) {
    return static_cast<SkColorSpace_Base*>(colorSpace);
}

static inline const SkColorSpace_Base* as_CSB(const SkColorSpace* colorSpace) {
    return static_cast<const SkColorSpace_Base*>(colorSpace);
}

static inline SkColorSpace_Base* as_CSB(const sk_sp<SkColorSpace>& colorSpace) {
    return static_cast<SkColorSpace_Base*>(colorSpace.get());
}

#endif
