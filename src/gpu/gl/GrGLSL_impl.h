/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSL_impl_DEFINED
#define GrGLSL_impl_DEFINED

#include "SkString.h"

namespace {
template<int N>
GrSLConstantVec return_const_vecf(GrSLConstantVec constVec, SkString* outAppend, bool omitAppend) {
    GrAssert(kNone_GrSLConstantVec != constVec);
    if (!omitAppend) {
        if (kZeros_GrSLConstantVec == constVec) {
            outAppend->append(GrGLSLZerosVecf(N));
        } else {
            outAppend->append(GrGLSLOnesVecf(N));
        }
    }
    return constVec;
}
}

template <int N>
GrSLConstantVec GrGLSLModulatef(SkString* outAppend,
                                const char* in0,
                                const char* in1,
                                GrSLConstantVec default0,
                                GrSLConstantVec default1,
                                bool omitIfConstVec) {
    GrAssert(N > 0 && N <= 4);
    GrAssert(NULL != outAppend);

    bool has0 = NULL != in0 && '\0' != *in0;
    bool has1 = NULL != in1 && '\0' != *in1;

    GrAssert(has0 || kNone_GrSLConstantVec != default0);
    GrAssert(has1 || kNone_GrSLConstantVec != default1);

    if (!has0 && !has1) {
        GrAssert(kZeros_GrSLConstantVec == default0 || kOnes_GrSLConstantVec == default0);
        GrAssert(kZeros_GrSLConstantVec == default1 || kOnes_GrSLConstantVec == default1);
        if (kZeros_GrSLConstantVec == default0 || kZeros_GrSLConstantVec == default1) {
            return return_const_vecf<N>(kZeros_GrSLConstantVec, outAppend, omitIfConstVec);
        } else {
            // both inputs are ones vectors
            return return_const_vecf<N>(kOnes_GrSLConstantVec, outAppend, omitIfConstVec);
        }
    } else if (!has0) {
        GrAssert(kZeros_GrSLConstantVec == default0 || kOnes_GrSLConstantVec == default0);
        if (kZeros_GrSLConstantVec == default0) {
            return return_const_vecf<N>(kZeros_GrSLConstantVec, outAppend, omitIfConstVec);
        } else {
            outAppend->appendf("%s(%s)", GrGLSLFloatVectorTypeString(N), in1);
            return kNone_GrSLConstantVec;
        }
    } else if (!has1) {
        GrAssert(kZeros_GrSLConstantVec == default1 || kOnes_GrSLConstantVec == default1);
        if (kZeros_GrSLConstantVec == default1) {
            return return_const_vecf<N>(kZeros_GrSLConstantVec, outAppend, omitIfConstVec);
        } else {
            outAppend->appendf("%s(%s)", GrGLSLFloatVectorTypeString(N), in0);
            return kNone_GrSLConstantVec;
        }
    } else {
        outAppend->appendf("%s((%s) * (%s))", GrGLSLFloatVectorTypeString(N), in0, in1);
        return kNone_GrSLConstantVec;
    }
}

template <int N>
GrSLConstantVec GrGLSLAddf(SkString* outAppend,
                           const char* in0,
                           const char* in1,
                           GrSLConstantVec default0,
                           GrSLConstantVec default1,
                           bool omitIfConstVec) {
    GrAssert(N > 0 && N <= 4);
    GrAssert(NULL != outAppend);

    bool has0 = NULL != in0 && '\0' != *in0;
    bool has1 = NULL != in1 && '\0' != *in1;

    if (!has0 && !has1) {
        GrAssert(kNone_GrSLConstantVec != default0);
        GrAssert(kNone_GrSLConstantVec != default1);
        int sum = (kOnes_GrSLConstantVec == default0) + (kOnes_GrSLConstantVec == default1);
        if (0 == sum) {
            return return_const_vecf<N>(kZeros_GrSLConstantVec, outAppend, omitIfConstVec);
        } else if (1 == sum) {
            outAppend->append(GrGLSLOnesVecf(N));
            return return_const_vecf<N>(kOnes_GrSLConstantVec, outAppend, omitIfConstVec);
        } else {
            GrAssert(2 == sum);
            outAppend->appendf("%s(2)", GrGLSLFloatVectorTypeString(N));
            return kNone_GrSLConstantVec;
        }
    } else if (!has0) {
        GrAssert(kNone_GrSLConstantVec != default0);
        if (kZeros_GrSLConstantVec == default0) {
            outAppend->appendf("%s(%s)", GrGLSLFloatVectorTypeString(N), in1);
        } else {
            outAppend->appendf("%s(%s) + %s",
                               GrGLSLFloatVectorTypeString(N),
                               in1,
                               GrGLSLOnesVecf(N));
        }
        return kNone_GrSLConstantVec;
    } else if (!has1) {
        GrAssert(kNone_GrSLConstantVec != default1);
        if (kZeros_GrSLConstantVec == default1) {
            outAppend->appendf("%s(%s)", GrGLSLFloatVectorTypeString(N), in0);
        } else {
            outAppend->appendf("%s(%s) + %s",
                               GrGLSLFloatVectorTypeString(N),
                               in0,
                               GrGLSLOnesVecf(N));
        }
        return kNone_GrSLConstantVec;
    } else {
        outAppend->appendf("(%s(%s) + %s(%s))",
                           GrGLSLFloatVectorTypeString(N),
                           in0,
                           GrGLSLFloatVectorTypeString(N),
                           in1);
        return kNone_GrSLConstantVec;
    }
}

template <int N>
GrSLConstantVec GrGLSLSubtractf(SkString* outAppend,
                                 const char* in0,
                                 const char* in1,
                                 GrSLConstantVec default0,
                                 GrSLConstantVec default1,
                                 bool omitIfConstVec) {
    GrAssert(N > 0 && N <= 4);
    GrAssert(NULL != outAppend);

    bool has0 = NULL != in0 && '\0' != *in0;
    bool has1 = NULL != in1 && '\0' != *in1;

    if (!has0 && !has1) {
        GrAssert(kNone_GrSLConstantVec != default0);
        GrAssert(kNone_GrSLConstantVec != default1);
        int diff = (kOnes_GrSLConstantVec == default0) - (kOnes_GrSLConstantVec == default1);
        if (-1 == diff) {
            outAppend->appendf("%s(-1)", GrGLSLFloatVectorTypeString(N));
            return kNone_GrSLConstantVec;
        } else if (0 == diff) {
            return return_const_vecf<N>(kZeros_GrSLConstantVec, outAppend, omitIfConstVec);
        } else {
            GrAssert(1 == diff);
            return return_const_vecf<N>(kOnes_GrSLConstantVec, outAppend, omitIfConstVec);
        }
    } else if (!has0) {
        GrAssert(kNone_GrSLConstantVec != default0);
        if (kZeros_GrSLConstantVec == default0) {
            outAppend->appendf("-%s(%s)", GrGLSLFloatVectorTypeString(N), in1);
        } else {
            outAppend->appendf("%s - %s(%s)",
                               GrGLSLOnesVecf(N),
                               GrGLSLFloatVectorTypeString(N),
                               in1);
        }
        return kNone_GrSLConstantVec;
    } else if (!has1) {
        GrAssert(kNone_GrSLConstantVec != default1);
        if (kZeros_GrSLConstantVec == default1) {
            outAppend->appendf("%s(%s)", GrGLSLFloatVectorTypeString(N), in0);
        } else {
            outAppend->appendf("%s(%s) - %s",
                               GrGLSLFloatVectorTypeString(N),
                               in0,
                               GrGLSLOnesVecf(N));
        }
        return kNone_GrSLConstantVec;
    } else {
        outAppend->appendf("(%s(%s) - %s(%s))",
                           GrGLSLFloatVectorTypeString(N),
                           in0,
                           GrGLSLFloatVectorTypeString(N),
                           in1);
        return kNone_GrSLConstantVec;
    }
}

#endif
