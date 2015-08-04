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

// size of the precomputed bitmap filter tables for high quality filtering.
// Used to precompute the shape of the filter kernel.
// Table size chosen from experiments to see where I could start to see a difference.

#define SKBITMAP_FILTER_TABLE_SIZE 128

class SkBitmapFilter {
  public:
      SkBitmapFilter(float width)
      : fWidth(width), fInvWidth(1.f/width) {
          fPrecomputed = false;
          fLookupMultiplier = this->invWidth() * (SKBITMAP_FILTER_TABLE_SIZE-1);
      }

      SkFixed lookup(float x) const {
          if (!fPrecomputed) {
              precomputeTable();
          }
          int filter_idx = int(sk_float_abs(x * fLookupMultiplier));
          SkASSERT(filter_idx < SKBITMAP_FILTER_TABLE_SIZE);
          return fFilterTable[filter_idx];
      }

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
      virtual ~SkBitmapFilter() {}

      static SkBitmapFilter* Allocate();
  protected:
      float fWidth;
      float fInvWidth;

      float fLookupMultiplier;

      mutable bool fPrecomputed;
      mutable SkFixed fFilterTable[SKBITMAP_FILTER_TABLE_SIZE];
      mutable SkScalar fFilterTableScalar[SKBITMAP_FILTER_TABLE_SIZE];
  private:
      void precomputeTable() const {
          fPrecomputed = true;
          SkFixed *ftp = fFilterTable;
          SkScalar *ftpScalar = fFilterTableScalar;
          for (int x = 0; x < SKBITMAP_FILTER_TABLE_SIZE; ++x) {
              float fx = ((float)x + .5f) * this->width() / SKBITMAP_FILTER_TABLE_SIZE;
              float filter_value = evaluate(fx);
              *ftpScalar++ = filter_value;
              *ftp++ = SkFloatToFixed(filter_value);
          }
      }
};

class SkMitchellFilter: public SkBitmapFilter {
  public:
      SkMitchellFilter(float b, float c, float width=2.0f)
      : SkBitmapFilter(width), B(b), C(c) {
      }

      float evaluate(float x) const override {
          x = fabsf(x);
          if (x > 2.f) {
              return 0;
          } else if (x > 1.f) {
              return ((-B - 6*C) * x*x*x + (6*B + 30*C) * x*x +
                      (-12*B - 48*C) * x + (8*B + 24*C)) * (1.f/6.f);
          } else {
              return ((12 - 9*B - 6*C) * x*x*x +
                      (-18 + 12*B + 6*C) * x*x +
                      (6 - 2*B)) * (1.f/6.f);
          }
      }
  protected:
      float B, C;
};

class SkGaussianFilter: public SkBitmapFilter {
  public:
      SkGaussianFilter(float a, float width=2.0f)
      : SkBitmapFilter(width), alpha(a), expWidth(expf(-alpha * width * width)) {
      }

      float evaluate(float x) const override {
          return SkTMax(0.f, float(expf(-alpha*x*x) - expWidth));
      }
  protected:
      float alpha, expWidth;
};

class SkTriangleFilter: public SkBitmapFilter {
  public:
      SkTriangleFilter(float width=1)
      : SkBitmapFilter(width) {
      }

      float evaluate(float x) const override {
          return SkTMax(0.f, fWidth - fabsf(x));
      }
  protected:
};

class SkBoxFilter: public SkBitmapFilter {
  public:
      SkBoxFilter(float width=0.5f)
      : SkBitmapFilter(width) {
      }

      float evaluate(float x) const override {
          return (x >= -fWidth && x < fWidth) ? 1.0f : 0.0f;
      }
  protected:
};

class SkHammingFilter: public SkBitmapFilter {
public:
    SkHammingFilter(float width=1.f)
    : SkBitmapFilter(width) {
    }
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

class SkLanczosFilter: public SkBitmapFilter {
  public:
      SkLanczosFilter(float width=3.f)
      : SkBitmapFilter(width) {
      }

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
