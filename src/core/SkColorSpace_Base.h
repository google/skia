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
#include "SkTemplates.h"

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

        SkColorSpace::GammaNamed fNamed;
        float                    fValue;
        Table                    fTable;
        size_t                   fParamOffset;

        const Params& params(const SkGammas* base) const {
            return *SkTAddOffset<const Params>(base, sizeof(SkGammas) + fParamOffset);
        }
    };

    bool isNamed(int i) const {
        SkASSERT(0 <= i && i < 3);
        return (&fRedType)[i] == Type::kNamed_Type;
    }

    bool isValue(int i) const {
        SkASSERT(0 <= i && i < 3);
        return (&fRedType)[i] == Type::kValue_Type;
    }

    bool isTable(int i) const {
        SkASSERT(0 <= i && i < 3);
        return (&fRedType)[i] == Type::kTable_Type;
    }

    bool isParametric(int i) const {
        SkASSERT(0 <= i && i < 3);
        return (&fRedType)[i] == Type::kParam_Type;
    }

    const Data& data(int i) const {
        SkASSERT(0 <= i && i < 3);
        return (&fRedData)[i];
    }

    const float* table(int i) const {
        SkASSERT(isTable(i));
        return (&fRedData)[i].fTable.table(this);
    }

    const Params& params(int i) const {
        SkASSERT(isParametric(i));
        return (&fRedData)[i].params(this);
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
    uint8_t                  fInputChannels;
    uint8_t                  fOutputChannels;
    uint8_t                  fGridPoints[3];
    std::unique_ptr<float[]> fTable;

    SkColorLookUpTable()
        : fInputChannels(0)
        , fOutputChannels(0)
        , fTable(nullptr)
    {
        fGridPoints[0] = fGridPoints[1] = fGridPoints[2] = 0;
    }
};

class SkColorSpace_Base : public SkColorSpace {
public:

    static sk_sp<SkColorSpace> NewRGB(float gammas[3], const SkMatrix44& toXYZD50);

    const SkGammas* gammas() const { return fGammas.get(); }

    const SkColorLookUpTable* colorLUT() const { return fColorLUT.get(); }

    /**
     *  Writes this object as an ICC profile.
     */
    sk_sp<SkData> writeToICC() const;

private:

    static sk_sp<SkColorSpace> NewRGB(GammaNamed gammaNamed, const SkMatrix44& toXYZD50);

    SkColorSpace_Base(GammaNamed gammaNamed, const SkMatrix44& toXYZ, Named named);

    SkColorSpace_Base(sk_sp<SkColorLookUpTable> colorLUT, GammaNamed gammaNamed,
                      sk_sp<SkGammas> gammas, const SkMatrix44& toXYZ, sk_sp<SkData> profileData);

    sk_sp<SkColorLookUpTable> fColorLUT;
    sk_sp<SkGammas>           fGammas;
    sk_sp<SkData>             fProfileData;

    friend class SkColorSpace;
    friend class ColorSpaceXformTest;
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
