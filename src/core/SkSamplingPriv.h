/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSamplingPriv_DEFINED
#define SkSamplingPriv_DEFINED

#include "include/core/SkSamplingOptions.h"

class SkReadBuffer;
class SkWriteBuffer;

// Private copy of SkFilterQuality, just for legacy deserialization
// Matches values in SkFilterQuality
enum SkLegacyFQ {
    kNone_SkLegacyFQ   = 0,    //!< nearest-neighbor; fastest but lowest quality
    kLow_SkLegacyFQ    = 1,    //!< bilerp
    kMedium_SkLegacyFQ = 2,    //!< bilerp + mipmaps; good for down-scaling
    kHigh_SkLegacyFQ   = 3,    //!< bicubic resampling; slowest but good quality

    kLast_SkLegacyFQ = kHigh_SkLegacyFQ,
};

// Matches values in SkSamplingOptions::MediumBehavior
enum SkMediumAs {
    kNearest_SkMediumAs,
    kLinear_SkMediumAs,
};

class SkSamplingPriv {
public:
    enum {
        kFlatSize = 3 * sizeof(uint32_t)  // bool32 + [2 floats | 2 ints]
    };

    // Returns true if the sampling can be ignored when the CTM is identity.
    static bool NoChangeWithIdentityMatrix(const SkSamplingOptions& sampling) {
        // If B == 0, the cubic resampler should have no effect for identity matrices
        // https://entropymine.com/imageworsener/bicubic/
        return !sampling.useCubic || sampling.cubic.B == 0;
    }

    static SkSamplingOptions Read(SkReadBuffer&);
    static void Write(SkWriteBuffer&, const SkSamplingOptions&);

    static SkSamplingOptions FromFQ(SkLegacyFQ, SkMediumAs = kNearest_SkMediumAs);
};

#endif
