/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkXfer4f.h"
#include "SkPM4fPriv.h"
#include "SkUtils.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

void CLEAR_pm41p(uint32_t dst[], const SkPM4f& src, int count) {
    sk_bzero(dst, count * sizeof(uint32_t));
}

void CLEAR_pm4np(uint32_t dst[], const SkPM4f src[], int count) {
    sk_bzero(dst, count * sizeof(uint32_t));
}

//////////

template <bool isSRGB> void SRC_pm41p(uint32_t dst[], const SkPM4f& src, int count) {
    uint32_t res;
    if (isSRGB) {
        res = Sk4f_toS32(Sk4f::Load(src.fVec));
    } else {
        res = Sk4f_toL32(Sk4f::Load(src.fVec));
    }
    sk_memset32(dst, res, count);
}

template <bool isSRGB> void SRC_pm4np(uint32_t dst[], const SkPM4f src[], int count) {
    if (isSRGB) {
        SkPM4f_s32_src_mode(dst, src, count);
    } else {
        SkPM4f_l32_src_mode(dst, src, count);
    }
}

//////////

void DST_pm41p(uint32_t dst[], const SkPM4f& src, int count) {}
void DST_pm4np(uint32_t dst[], const SkPM4f src[], int count) {}

//////////

template <bool isSRGB> void SRCOVER_pm41p(uint32_t dst[], const SkPM4f& src, int count) {
    SkASSERT(src.isUnit());
    Sk4f s4 = Sk4f::Load(src.fVec);
    Sk4f scale(1 - s4.kth<SkPM4f::A>());

    if (!isSRGB) {
        s4 = s4 * Sk4f(255);
    }

    for (int i = 0; i < count; ++i) {
        if (isSRGB) {
            Sk4f d4 = Sk4f_fromS32(dst[i]);
            dst[i] = Sk4f_toS32(s4 + d4 * scale);
        } else {
            Sk4f d4 = to_4f(dst[i]);
            dst[i] = to_4b(s4 + d4 * scale + Sk4f(0.5f));
        }
    }
}

template <bool isSRGB> void SRCOVER_pm4np(uint32_t dst[], const SkPM4f src[], int count) {
    if (isSRGB) {
        SkPM4f_s32_srcover_mode(dst, src, count);
    } else {
        SkPM4f_l32_srcover_mode(dst, src, count);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

struct Pair {
    SkPM4fXfer1Proc fProc1;
    SkPM4fXferNProc fProcN;
};

const Pair gClearPairs[] = {
    { CLEAR_pm41p, CLEAR_pm4np },
    { CLEAR_pm41p, CLEAR_pm4np },
    { CLEAR_pm41p, CLEAR_pm4np },
    { CLEAR_pm41p, CLEAR_pm4np },
};

const Pair gSrcPairs[] = {
    { SRC_pm41p<false>, SRC_pm4np<false> }, // linear [alpha  ignored]
    { SRC_pm41p<false>, SRC_pm4np<false> }, // linear [opaque ignored]
    { SRC_pm41p<true>,  SRC_pm4np<true>  }, // srgb   [alpha  ignored]
    { SRC_pm41p<true>,  SRC_pm4np<true>  }, // srgb   [opaque ignored]
};

const Pair gDstPairs[] = {
    { DST_pm41p, DST_pm4np },
    { DST_pm41p, DST_pm4np },
    { DST_pm41p, DST_pm4np },
    { DST_pm41p, DST_pm4np },
};

const Pair gSrcOverPairs[] = {
    { SRCOVER_pm41p<false>,   SRCOVER_pm4np<false>  },  // linear   alpha
    { SRC_pm41p<false>,       SRC_pm4np<false>      },  // linear   opaque
    { SRCOVER_pm41p<true>,    SRCOVER_pm4np<true>   },  // srgb     alpha
    { SRC_pm41p<true>,        SRC_pm4np<true>       },  // srgb     opaque
};

static const Pair* find_pair(SkXfermode::Mode mode, uint32_t flags) {
    SkASSERT(0 == (flags & ~3));
    const Pair* pairs = nullptr;

    switch (mode) {
        case SkXfermode::kClear_Mode:   pairs = gClearPairs;
        case SkXfermode::kSrc_Mode:     pairs = gSrcPairs; break;
        case SkXfermode::kDst_Mode:     pairs = gDstPairs;
        case SkXfermode::kSrcOver_Mode: pairs = gSrcOverPairs; break;
        default: return nullptr;
    }
    return &pairs[flags & 3];
}

SkPM4fXfer1Proc SkPM4fXfer1ProcFactory(SkXfermode::Mode mode, uint32_t flags) {
    const Pair* pair = find_pair(mode, flags);
    return pair ? pair->fProc1 : nullptr;
}

SkPM4fXferNProc SkPM4fXferNProcFactory(SkXfermode::Mode mode, uint32_t flags) {
    const Pair* pair = find_pair(mode, flags);
    return pair ? pair->fProcN : nullptr;
}
