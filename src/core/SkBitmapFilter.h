
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBitmapFilter_DEFINED
#define SkBitmapFilter_DEFINED

#include "SkMath.h"

// size of the precomputed bitmap filter tables for high quality filtering.
// Used to precompute the shape of the filter kernel.
// Table size chosen from experiments to see where I could start to see a difference.

#define SKBITMAP_FILTER_TABLE_SIZE 32

class SkBitmapFilter {
  public:
      SkBitmapFilter(float width)
      : fWidth(width), fInvWidth(1.f/width) {
          precomputed = false;
      }
      
      SkFixed lookup( float x ) const {
          if (!precomputed) {
              precomputeTable();
          }
          int filter_idx = int(fabsf(x * invWidth() * SKBITMAP_FILTER_TABLE_SIZE));
          return fFilterTable[ SkTMin(filter_idx, SKBITMAP_FILTER_TABLE_SIZE-1) ];
      }      
      
      float lookupFloat( float x ) const {
          if (!precomputed) {
              precomputeTable();
          }
          int filter_idx = int(fabsf(x * invWidth() * SKBITMAP_FILTER_TABLE_SIZE));
          return fFilterTableFloat[ SkTMin(filter_idx, SKBITMAP_FILTER_TABLE_SIZE-1) ];
      }      
      
      float width() const { return fWidth; }
      float invWidth() const { return fInvWidth; }
      virtual float evaluate(float x) const = 0;
      virtual ~SkBitmapFilter() {}
  protected:
      float fWidth;
      float fInvWidth;
      
      mutable bool precomputed;
      mutable SkFixed fFilterTable[SKBITMAP_FILTER_TABLE_SIZE];
      mutable float fFilterTableFloat[SKBITMAP_FILTER_TABLE_SIZE];
  private:
      void precomputeTable() const {
          precomputed = true;
          SkFixed *ftp = fFilterTable;
          float *ftp_float = fFilterTableFloat;
          for (int x = 0; x < SKBITMAP_FILTER_TABLE_SIZE; ++x) {
              float fx = ((float)x + .5f) * this->width() / SKBITMAP_FILTER_TABLE_SIZE;
              float filter_value = evaluate(fx);
              *ftp_float++ = filter_value;
              *ftp++ = SkFloatToFixed(filter_value);
          }
      }
};

class SkMitchellFilter: public SkBitmapFilter {
  public:
      SkMitchellFilter(float b, float c, float width=2.0f)
      : SkBitmapFilter(width), B(b), C(c) { 
      }
      
      virtual float evaluate(float x) const SK_OVERRIDE {
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
      
      virtual float evaluate(float x) const SK_OVERRIDE {
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
      
      virtual float evaluate(float x) const SK_OVERRIDE {
          return SkTMax(0.f, fWidth - fabsf(x));
      }
  protected:
};

class SkBoxFilter: public SkBitmapFilter {
  public:
      SkBoxFilter(float width=0.5f)
      : SkBitmapFilter(width) { 
      }
      
      virtual float evaluate(float x) const SK_OVERRIDE {
          return 1;
      }
  protected:
};


class SkSincFilter: public SkBitmapFilter {
  public:
      SkSincFilter(float t, float width=3.f)
      : SkBitmapFilter(width), tau(t) { 
      }
      
      virtual float evaluate(float x) const SK_OVERRIDE {
          x = sk_float_abs(x * fInvWidth);
          if (x < 1e-5f) return 1.f;
          if (x > 1.f)   return 0.f;
          x *= (float) M_PI;
          float sinc = sk_float_sin(x) / x;
          float lanczos = sk_float_sin(x * tau) / (x * tau);
          return sinc * lanczos;
      }
  protected:
      float tau;
};


#endif
