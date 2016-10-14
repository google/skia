/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXform_Base_DEFINED
#define SkColorSpaceXform_Base_DEFINED

#include "SkColorSpace.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpaceXform.h"
#include "SkResourceCache.h"

class SkColorSpaceXform_Base : public SkColorSpaceXform {
protected:
    virtual bool onApply(ColorFormat dstFormat, void* dst, ColorFormat srcFormat, const void* src,
                         int count, SkAlphaType alphaType) const = 0;

    virtual void onSetCachedFields(const SkResourceCache::Rec& rec) = 0;

private:
    friend class SkColorSpaceXform;
    friend struct SkColorSpaceXformRec;
};

enum SrcGamma {
    kLinear_SrcGamma,
    kTable_SrcGamma,
};

enum DstGamma {
    kLinear_DstGamma,
    kSRGB_DstGamma,
    k2Dot2_DstGamma,
    kTable_DstGamma,
};

enum ColorSpaceMatch {
    kNone_ColorSpaceMatch,
    kGamut_ColorSpaceMatch,
    kFull_ColorSpaceMatch,
};

template <SrcGamma kSrc, DstGamma kDst, ColorSpaceMatch kCSM>
class SkColorSpaceXform_XYZ : public SkColorSpaceXform_Base {
public:
    static constexpr int kDstGammaTableSize = 1024;

protected:
    bool onApply(ColorFormat dstFormat, void* dst, ColorFormat srcFormat, const void* src,
                 int count, SkAlphaType alphaType) const override;

    void onSetCachedFields(const SkResourceCache::Rec& rec) override;

private:
    SkColorSpaceXform_XYZ(SkColorSpace* srcSpace, const SkMatrix44& srcToDst,
                          SkColorSpace* dstSpace);

    sk_sp<SkColorLookUpTable> fColorLUT;

    // Contain pointers into storage or pointers into precomputed tables.
    const float*              fSrcGammaTables[3];
    const uint8_t*            fDstGammaTables[3];
    sk_sp<SkData>             fSrcStorage;
    sk_sp<SkData>             fDstStorage;

    float                     fSrcToDst[16];

    friend class SkColorSpaceXform;
    friend std::unique_ptr<SkColorSpaceXform> SlowIdentityXform(SkColorSpace* space);
};

static void* gColorSpaceXformAddress;
struct SkColorSpaceXformKey : public SkResourceCache::Key {
public:
    const uint32_t fDstSpaceID;

    SkColorSpaceXformKey(uint32_t dstSpaceID)
        : fDstSpaceID(dstSpaceID)
    {
        this->init(&gColorSpaceXformAddress, 0, sizeof(uint32_t));
    }
};

struct SkColorSpaceXformRec : public SkResourceCache::Rec {
    SkColorSpaceXformKey fKey;

    // No need to ref these pointers.  They point into storage, so are valid while
    // storage is valid.
    const uint8_t*       fDstGammaTables[3];
    sk_sp<SkData>        fStorage;

    SkColorSpaceXformRec(const SkColorSpaceXformKey& key, const uint8_t* dstTables[3],
                         sk_sp<SkData> storage)
        : fKey(key)
        , fStorage(std::move(storage))
    {
        fDstGammaTables[0] = dstTables[0];
        fDstGammaTables[1] = dstTables[1];
        fDstGammaTables[2] = dstTables[2];
    }

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override { return sizeof(SkColorSpaceXformRec) + fStorage->size(); }
    const char* getCategory() const override { return "SkColorSpaceXform"; }

    static bool Visitor(const SkResourceCache::Rec& rec, void* context) {
        ((SkColorSpaceXform_Base*) context)->onSetCachedFields(rec);
        return true;
    }
};

// For testing.  Bypasses opts for when src and dst color spaces are equal.
std::unique_ptr<SkColorSpaceXform> SlowIdentityXform(SkColorSpace* space);

#endif
