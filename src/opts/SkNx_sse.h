/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNx_sse_DEFINED
#define SkNx_sse_DEFINED

// This file may assume <= SSE2, but must check SK_CPU_SSE_LEVEL for anything more recent.
#include <immintrin.h>

template <>
class SkNb<2, 4> {
public:
    SkNb(const __m128i& vec) : fVec(vec) {}

    SkNb() {}
    bool allTrue() const { return 0xff == (_mm_movemask_epi8(fVec) & 0xff); }
    bool anyTrue() const { return 0x00 != (_mm_movemask_epi8(fVec) & 0xff); }

private:
    __m128i fVec;
};

template <>
class SkNb<4, 4> {
public:
    SkNb(const __m128i& vec) : fVec(vec) {}

    SkNb() {}
    bool allTrue() const { return 0xffff == _mm_movemask_epi8(fVec); }
    bool anyTrue() const { return 0x0000 != _mm_movemask_epi8(fVec); }

private:
    __m128i fVec;
};

template <>
class SkNb<2, 8> {
public:
    SkNb(const __m128i& vec) : fVec(vec) {}

    SkNb() {}
    bool allTrue() const { return 0xffff == _mm_movemask_epi8(fVec); }
    bool anyTrue() const { return 0x0000 != _mm_movemask_epi8(fVec); }

private:
    __m128i fVec;
};


template <>
class SkNf<2, float> {
    typedef SkNb<2, 4> Nb;
public:
    SkNf(const __m128& vec) : fVec(vec) {}

    SkNf() {}
    explicit SkNf(float val) : fVec(_mm_set1_ps(val)) {}
    static SkNf Load(const float vals[2]) {
        return _mm_castsi128_ps(_mm_loadl_epi64((const __m128i*)vals));
    }
    SkNf(float a, float b) : fVec(_mm_setr_ps(a,b,0,0)) {}

    void store(float vals[2]) const { _mm_storel_pi((__m64*)vals, fVec); }

    SkNf operator + (const SkNf& o) const { return _mm_add_ps(fVec, o.fVec); }
    SkNf operator - (const SkNf& o) const { return _mm_sub_ps(fVec, o.fVec); }
    SkNf operator * (const SkNf& o) const { return _mm_mul_ps(fVec, o.fVec); }
    SkNf operator / (const SkNf& o) const { return _mm_div_ps(fVec, o.fVec); }

    Nb operator == (const SkNf& o) const { return _mm_castps_si128(_mm_cmpeq_ps (fVec, o.fVec)); }
    Nb operator != (const SkNf& o) const { return _mm_castps_si128(_mm_cmpneq_ps(fVec, o.fVec)); }
    Nb operator  < (const SkNf& o) const { return _mm_castps_si128(_mm_cmplt_ps (fVec, o.fVec)); }
    Nb operator  > (const SkNf& o) const { return _mm_castps_si128(_mm_cmpgt_ps (fVec, o.fVec)); }
    Nb operator <= (const SkNf& o) const { return _mm_castps_si128(_mm_cmple_ps (fVec, o.fVec)); }
    Nb operator >= (const SkNf& o) const { return _mm_castps_si128(_mm_cmpge_ps (fVec, o.fVec)); }

    static SkNf Min(const SkNf& l, const SkNf& r) { return _mm_min_ps(l.fVec, r.fVec); }
    static SkNf Max(const SkNf& l, const SkNf& r) { return _mm_max_ps(l.fVec, r.fVec); }

    SkNf  sqrt() const { return _mm_sqrt_ps (fVec);  }
    SkNf rsqrt() const { return _mm_rsqrt_ps(fVec); }

    SkNf       invert() const { return SkNf(1) / *this; }
    SkNf approxInvert() const { return _mm_rcp_ps(fVec); }

    template <int k> float kth() const {
        SkASSERT(0 <= k && k < 2);
        union { __m128 v; float fs[4]; } pun = {fVec};
        return pun.fs[k&1];
    }

private:
    __m128 fVec;
};

template <>
class SkNf<2, double> {
    typedef SkNb<2, 8> Nb;
public:
    SkNf(const __m128d& vec) : fVec(vec) {}

    SkNf() {}
    explicit SkNf(double val)           : fVec( _mm_set1_pd(val) ) {}
    static SkNf Load(const double vals[2]) { return _mm_loadu_pd(vals); }
    SkNf(double a, double b) : fVec(_mm_setr_pd(a,b)) {}

    void store(double vals[2]) const { _mm_storeu_pd(vals, fVec); }

    SkNf operator + (const SkNf& o) const { return _mm_add_pd(fVec, o.fVec); }
    SkNf operator - (const SkNf& o) const { return _mm_sub_pd(fVec, o.fVec); }
    SkNf operator * (const SkNf& o) const { return _mm_mul_pd(fVec, o.fVec); }
    SkNf operator / (const SkNf& o) const { return _mm_div_pd(fVec, o.fVec); }

    Nb operator == (const SkNf& o) const { return _mm_castpd_si128(_mm_cmpeq_pd (fVec, o.fVec)); }
    Nb operator != (const SkNf& o) const { return _mm_castpd_si128(_mm_cmpneq_pd(fVec, o.fVec)); }
    Nb operator  < (const SkNf& o) const { return _mm_castpd_si128(_mm_cmplt_pd (fVec, o.fVec)); }
    Nb operator  > (const SkNf& o) const { return _mm_castpd_si128(_mm_cmpgt_pd (fVec, o.fVec)); }
    Nb operator <= (const SkNf& o) const { return _mm_castpd_si128(_mm_cmple_pd (fVec, o.fVec)); }
    Nb operator >= (const SkNf& o) const { return _mm_castpd_si128(_mm_cmpge_pd (fVec, o.fVec)); }

    static SkNf Min(const SkNf& l, const SkNf& r) { return _mm_min_pd(l.fVec, r.fVec); }
    static SkNf Max(const SkNf& l, const SkNf& r) { return _mm_max_pd(l.fVec, r.fVec); }

    SkNf  sqrt() const { return _mm_sqrt_pd(fVec);  }
    SkNf rsqrt() const { return _mm_cvtps_pd(_mm_rsqrt_ps(_mm_cvtpd_ps(fVec))); }

    SkNf       invert() const { return SkNf(1) / *this; }
    SkNf approxInvert() const { return _mm_cvtps_pd(_mm_rcp_ps(_mm_cvtpd_ps(fVec))); }

    template <int k> double kth() const {
        SkASSERT(0 <= k && k < 2);
        union { __m128d v; double ds[2]; } pun = {fVec};
        return pun.ds[k&1];
    }

private:
    __m128d fVec;
};

template <>
class SkNf<4, float> {
    typedef SkNb<4, 4> Nb;
public:
    SkNf(const __m128& vec) : fVec(vec) {}

    SkNf() {}
    explicit SkNf(float val)           : fVec( _mm_set1_ps(val) ) {}
    static SkNf Load(const float vals[4]) { return _mm_loadu_ps(vals); }
    SkNf(float a, float b, float c, float d) : fVec(_mm_setr_ps(a,b,c,d)) {}

    void store(float vals[4]) const { _mm_storeu_ps(vals, fVec); }

    SkNf operator + (const SkNf& o) const { return _mm_add_ps(fVec, o.fVec); }
    SkNf operator - (const SkNf& o) const { return _mm_sub_ps(fVec, o.fVec); }
    SkNf operator * (const SkNf& o) const { return _mm_mul_ps(fVec, o.fVec); }
    SkNf operator / (const SkNf& o) const { return _mm_div_ps(fVec, o.fVec); }

    Nb operator == (const SkNf& o) const { return _mm_castps_si128(_mm_cmpeq_ps (fVec, o.fVec)); }
    Nb operator != (const SkNf& o) const { return _mm_castps_si128(_mm_cmpneq_ps(fVec, o.fVec)); }
    Nb operator  < (const SkNf& o) const { return _mm_castps_si128(_mm_cmplt_ps (fVec, o.fVec)); }
    Nb operator  > (const SkNf& o) const { return _mm_castps_si128(_mm_cmpgt_ps (fVec, o.fVec)); }
    Nb operator <= (const SkNf& o) const { return _mm_castps_si128(_mm_cmple_ps (fVec, o.fVec)); }
    Nb operator >= (const SkNf& o) const { return _mm_castps_si128(_mm_cmpge_ps (fVec, o.fVec)); }

    static SkNf Min(const SkNf& l, const SkNf& r) { return _mm_min_ps(l.fVec, r.fVec); }
    static SkNf Max(const SkNf& l, const SkNf& r) { return _mm_max_ps(l.fVec, r.fVec); }

    SkNf  sqrt() const { return _mm_sqrt_ps (fVec);  }
    SkNf rsqrt() const { return _mm_rsqrt_ps(fVec); }

    SkNf       invert() const { return SkNf(1) / *this; }
    SkNf approxInvert() const { return _mm_rcp_ps(fVec); }

    template <int k> float kth() const {
        SkASSERT(0 <= k && k < 4);
        union { __m128 v; float fs[4]; } pun = {fVec};
        return pun.fs[k&3];
    }

protected:
    __m128 fVec;
};


#endif//SkNx_sse_DEFINED
