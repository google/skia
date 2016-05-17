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
