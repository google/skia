/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapFilter_DEFINED
#define SkBitmapFilter_DEFINED

#include "SkFixed.h"
#include "SkMath.h"
#include "SkScalar.h"

#include "SkNx.h"

// size of the precomputed bitmap filter tables for high quality filtering.
// Used to precompute the shape of the filter kernel.
// Table size chosen from experiments to see where I could start to see a difference.

#define SKBITMAP_FILTER_TABLE_SIZE 128

class SkBitmapFilter {
public:
    SkBitmapFilter(float width) : fWidth(width), fInvWidth(1.f/width) {
        fPrecomputed = false;
        fLookupMultiplier = this->invWidth() * (SKBITMAP_FILTER_TABLE_SIZE-1);
    }
    virtual ~SkBitmapFilter() {}

    SkScalar lookupScalar(float x) const {
        if (!fPrecomputed) {
            precomputeTable();
        }
        int filter_idx = int(sk_float_abs(x * fLookupMultiplier));
        SkASSERT(filter_idx < SKBITMAP_FILTER_TABLE_SIZE);
        return fFilterTableScalar[filter_idx];
    }

    float width() const { return fWidth; }
    float invWidth() const { return fInvWidth; }
    virtual float evaluate(float x) const = 0;

    virtual float evaluate_n(float val, float diff, int count, float* output) const {
        float sum = 0;
        for (int index = 0; index < count; index++) {
            float filterValue = evaluate(val);
            *output++ = filterValue;
            sum += filterValue;
            val += diff;
        }
        return sum;
    }

protected:
    float fWidth;
    float fInvWidth;
    float fLookupMultiplier;

    mutable bool fPrecomputed;
    mutable SkScalar fFilterTableScalar[SKBITMAP_FILTER_TABLE_SIZE];

private:
    void precomputeTable() const {
        fPrecomputed = true;
        SkScalar *ftpScalar = fFilterTableScalar;
        for (int x = 0; x < SKBITMAP_FILTER_TABLE_SIZE; ++x) {
            float fx = ((float)x + .5f) * this->width() / SKBITMAP_FILTER_TABLE_SIZE;
            float filter_value = evaluate(fx);
            *ftpScalar++ = filter_value;
        }
    }
};

class SkMitchellFilter final : public SkBitmapFilter {
public:
    SkMitchellFilter()
        : INHERITED(2)
        , fB(1.f / 3.f)
        , fC(1.f / 3.f)
        , fA1(-fB - 6*fC)
        , fB1(6*fB + 30*fC)
        , fC1(-12*fB - 48*fC)
        , fD1(8*fB + 24*fC)
        , fA2(12 - 9*fB - 6*fC)
        , fB2(-18 + 12*fB + 6*fC)
        , fD2(6 - 2*fB)
    {}

    float evaluate(float x) const override {
        x = fabsf(x);
        if (x > 2.f) {
            return 0;
        } else if (x > 1.f) {
            return (((fA1 * x + fB1) * x + fC1) * x + fD1) * (1.f/6.f);
        } else {
            return ((fA2 * x + fB2) * x*x + fD2) * (1.f/6.f);
        }
    }

    Sk4f evalcore_n(const Sk4f& val) const {
        Sk4f x = val.abs();
        Sk4f over2 = x > Sk4f(2);
        Sk4f over1 = x > Sk4f(1);
        Sk4f poly1 = (((Sk4f(fA1) * x + Sk4f(fB1)) * x + Sk4f(fC1)) * x + Sk4f(fD1))
                     * Sk4f(1.f/6.f);
        Sk4f poly0 = ((Sk4f(fA2) * x + Sk4f(fB2)) * x*x + Sk4f(fD2)) * Sk4f(1.f/6.f);
        return over2.thenElse(Sk4f(0), over1.thenElse(poly1, poly0));
    }

    float evaluate_n(float val, float diff, int count, float* output) const override {
        Sk4f sum(0);
        while (count >= 4) {
            float v0 = val;
            float v1 = val += diff;
            float v2 = val += diff;
            float v3 = val += diff;
            val += diff;
            Sk4f filterValue = evalcore_n(Sk4f(v0, v1, v2, v3));
            filterValue.store(output);
            output += 4;
            sum = sum + filterValue;
            count -= 4;
        }
        float sums[4];
        sum.store(sums);
        float result = sums[0] + sums[1] + sums[2] + sums[3];
        result += INHERITED::evaluate_n(val, diff, count, output);
        return result;
    }

  protected:
      float fB, fC;
      float fA1, fB1, fC1, fD1;
      float fA2, fB2, fD2;
private:
    typedef SkBitmapFilter INHERITED;
};

class SkGaussianFilter final : public SkBitmapFilter {
    float fAlpha, fExpWidth;

public:
    SkGaussianFilter(float a, float width = 2)
        : SkBitmapFilter(width)
        , fAlpha(a)
        , fExpWidth(expf(-a * width * width))
    {}

    float evaluate(float x) const override {
        return SkTMax(0.f, float(expf(-fAlpha*x*x) - fExpWidth));
    }
};

class SkTriangleFilter final : public SkBitmapFilter {
public:
    SkTriangleFilter(float width = 1) : SkBitmapFilter(width) {}

    float evaluate(float x) const override {
        return SkTMax(0.f, fWidth - fabsf(x));
    }
};

class SkBoxFilter final : public SkBitmapFilter {
public:
    SkBoxFilter(float width = 0.5f) : SkBitmapFilter(width) {}

    float evaluate(float x) const override {
        return (x >= -fWidth && x < fWidth) ? 1.0f : 0.0f;
    }
};

class SkHammingFilter final : public SkBitmapFilter {
public:
    SkHammingFilter(float width = 1) : SkBitmapFilter(width) {}

    float evaluate(float x) const override {
        if (x <= -fWidth || x >= fWidth) {
            return 0.0f;  // Outside of the window.
        }
        if (x > -FLT_EPSILON && x < FLT_EPSILON) {
            return 1.0f;  // Special case the sinc discontinuity at the origin.
        }
        const float xpi = x * static_cast<float>(SK_ScalarPI);

        return ((sk_float_sin(xpi) / xpi) *  // sinc(x)
                (0.54f + 0.46f * sk_float_cos(xpi / fWidth)));  // hamming(x)
    }
};

class SkLanczosFilter final : public SkBitmapFilter {
public:
    SkLanczosFilter(float width = 3.f) : SkBitmapFilter(width) {}

    float evaluate(float x) const override {
        if (x <= -fWidth || x >= fWidth) {
            return 0.0f;  // Outside of the window.
        }
        if (x > -FLT_EPSILON && x < FLT_EPSILON) {
            return 1.0f;  // Special case the discontinuity at the origin.
        }
        float xpi = x * static_cast<float>(SK_ScalarPI);
        return (sk_float_sin(xpi) / xpi) *  // sinc(x)
               sk_float_sin(xpi / fWidth) / (xpi / fWidth);  // sinc(x/fWidth)
    }
};


#endif
