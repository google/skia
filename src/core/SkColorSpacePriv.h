/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpacePriv_DEFINED
#define SkColorSpacePriv_DEFINED

struct SkGammaCurve {
    bool isValue() const {
        bool result = (0.0f != fValue);
        SkASSERT(!result || (0 == fTableSize));
        SkASSERT(!result || (0.0f == fG));
        return result;
    }

    bool isTable() const {
        bool result = (0 != fTableSize);
        SkASSERT(!result || (0.0f == fValue));
        SkASSERT(!result || (0.0f == fG));
        SkASSERT(!result || fTable);
        return result;
    }

    bool isParametric() const {
        bool result = (0.0f != fG);
        SkASSERT(!result || (0.0f == fValue));
        SkASSERT(!result || (0 == fTableSize));
        return result;
    }

    // We have three different ways to represent gamma.
    // (1) A single value:
    float                    fValue;

    // (2) A lookup table:
    uint32_t                 fTableSize;
    std::unique_ptr<float[]> fTable;

    // (3) Parameters for a curve:
    //     Y = (aX + b)^g + c  for X >= d
    //     Y = eX + f          otherwise
    float                    fG;
    float                    fA;
    float                    fB;
    float                    fC;
    float                    fD;
    float                    fE;
    float                    fF;

    SkGammaCurve() {
        memset(this, 0, sizeof(struct SkGammaCurve));
    }

    SkGammaCurve(float value)
        : fValue(value)
        , fTableSize(0)
        , fTable(nullptr)
        , fG(0.0f)
        , fA(0.0f)
        , fB(0.0f)
        , fC(0.0f)
        , fD(0.0f)
        , fE(0.0f)
        , fF(0.0f)
    {}
};

struct SkGammas : public SkRefCnt {
public:
    bool isValues() const {
        return fRed.isValue() && fGreen.isValue() && fBlue.isValue();
    }

    const SkGammaCurve fRed;
    const SkGammaCurve fGreen;
    const SkGammaCurve fBlue;

    SkGammas(float red, float green, float blue)
        : fRed(red)
        , fGreen(green)
        , fBlue(blue)
    {}

    SkGammas(SkGammaCurve red, SkGammaCurve green, SkGammaCurve blue)
        : fRed(std::move(red))
        , fGreen(std::move(green))
        , fBlue(std::move(blue))
    {}

    SkGammas() {}

    friend class SkColorSpace;
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

#endif
