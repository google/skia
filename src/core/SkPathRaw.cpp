/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkPathRaw.h"

const uint8_t gVerbToSegmentMask[] = {
    0,  // move
    kLine_SkPathSegmentMask,
    kQuad_SkPathSegmentMask,
    kConic_SkPathSegmentMask,
    kCubic_SkPathSegmentMask,
    0,  // close
};

uint8_t SkPathPriv::ComputeSegmentMask(SkSpan<const SkPathVerb> verbs) {
    unsigned mask = 0;
    for (auto v : verbs) {
        unsigned i = static_cast<unsigned>(v);
        SkASSERT(i < std::size(gVerbToSegmentMask));
        mask |= gVerbToSegmentMask[i];
    }
    return SkTo<uint8_t>(mask);
}

////////

std::optional<SkPathRaw::IterRec> SkPathRaw::Iter::next() {
    if (vIndex >= fVerbs.size()) {
        return {};
    }

    size_t n = 0;

    float w = -1;
    SkPathVerb v;
    switch (v = (SkPathVerb)fVerbs[vIndex++]) {
        case SkPathVerb::kMove:
            fPointStorage[1] = fPoints[pIndex++]; // remember for close
            return IterRec{{&fPointStorage[1], 1}, w, v};
        case SkPathVerb::kLine:  n = 1; break;
        case SkPathVerb::kQuad:  n = 2; break;
        case SkPathVerb::kConic: n = 2; w = fConics[cIndex++]; break;
        case SkPathVerb::kCubic: n = 3; break;
        case SkPathVerb::kClose:
            SkASSERT(pIndex > 0);
            fPointStorage[0] = fPoints[pIndex-1];   // the last point we saw
            return IterRec{fPointStorage, w, v};
    }

    SkASSERT(pIndex > 0);
    auto start = pIndex - 1;
    SkASSERT(n >= 1 && n <= 3);
    pIndex += n;
    return IterRec{{&fPoints[start], n+1}, w, v};

}

///////////////////////////////////////////////

SkPathRaw::ContourIter::ContourIter(const SkPathRaw& src) {
    fPoints = src.fPoints;
    fVerbs = src.fVerbs;
}

std::optional<SkPathRaw::ContourRec> SkPathRaw::ContourIter::next() {
    if (fVerbs.empty()) {
        return {};
    }

    SkASSERT(fVerbs[0] == SkPathVerb::kMove);
    size_t npts = 1, nvbs = 1, nws = 0;

    for (size_t i = 1; i < fVerbs.size(); ++i) {
        switch ((SkPathVerb)fVerbs[i]) {
            case SkPathVerb::kMove: goto DONE;
            case SkPathVerb::kLine:  npts += 1; break;
            case SkPathVerb::kQuad:  npts += 2; break;
            case SkPathVerb::kConic: npts += 2; nws += 1; break;
            case SkPathVerb::kCubic: npts += 3; break;
            case SkPathVerb::kClose: nvbs += 1; goto DONE;
        }
        nvbs += 1;
    }
DONE:
    ContourRec rec = {
        fPoints.subspan(0, npts),
        fVerbs.subspan(0, nvbs),
        fConics.subspan(0, nws),
    };
    fPoints = fPoints.last(fPoints.size() - npts);
    fVerbs  = fVerbs.last( fVerbs.size()  - nvbs);
    fConics = fConics.last(fConics.size() - nws);
    return rec;
}
