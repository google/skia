/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBuffer.h"
#include "SkPath.h"
#include "SkPathRef.h"

SK_DEFINE_INST_COUNT(SkPathRef);

SkPoint* SkPathRef::Editor::growForVerb(int /*SkPath::Verb*/ verb) {
    fPathRef->validate();
    return fPathRef->growForVerb(verb);
}

SkPoint* SkPathRef::Editor::growForConic(SkScalar w) {
    fPathRef->validate();
    SkPoint* pts = fPathRef->growForVerb(SkPath::kConic_Verb);
    *fPathRef->fConicWeights.append() = w;
    return pts;
}

SkPoint* SkPathRef::growForVerb(int /* SkPath::Verb*/ verb) {
    this->validate();
    int pCnt;
    switch (verb) {
        case SkPath::kMove_Verb:
            pCnt = 1;
            break;
        case SkPath::kLine_Verb:
            pCnt = 1;
            break;
        case SkPath::kQuad_Verb:
            // fall through
        case SkPath::kConic_Verb:
            pCnt = 2;
            break;
        case SkPath::kCubic_Verb:
            pCnt = 3;
            break;
        case SkPath::kClose_Verb:
            pCnt = 0;
            break;
        case SkPath::kDone_Verb:
            SkDEBUGFAIL("growForVerb called for kDone");
            // fall through
        default:
            SkDEBUGFAIL("default is not reached");
            pCnt = 0;
    }
    size_t space = sizeof(uint8_t) + pCnt * sizeof (SkPoint);
    this->makeSpace(space);
    this->fVerbs[~fVerbCnt] = verb;
    SkPoint* ret = fPoints + fPointCnt;
    fVerbCnt += 1;
    fPointCnt += pCnt;
    fFreeSpace -= space;
    fBoundsIsDirty = true;  // this also invalidates fIsFinite
    this->validate();
    return ret;
}

SkPathRef* SkPathRef::CreateFromBuffer(SkRBuffer* buffer
#ifndef DELETE_THIS_CODE_WHEN_SKPS_ARE_REBUILT_AT_V14_AND_ALL_OTHER_INSTANCES_TOO
                                   , bool newFormat, int32_t oldPacked
#endif
    ) {
    SkPathRef* ref = SkNEW(SkPathRef);
#ifndef DELETE_THIS_CODE_WHEN_SKPS_ARE_REBUILT_AT_V14_AND_ALL_OTHER_INSTANCES_TOO
    if (newFormat) {
#endif
        int32_t packed = buffer->readU32();

        ref->fIsFinite = (packed >> kIsFinite_SerializationShift) & 1;
#ifndef DELETE_THIS_CODE_WHEN_SKPS_ARE_REBUILT_AT_V14_AND_ALL_OTHER_INSTANCES_TOO
    } else {
        ref->fIsFinite = (oldPacked >> SkPath::kOldIsFinite_SerializationShift) & 1;
    }
#endif

    ref->fGenerationID = buffer->readU32();
    int32_t verbCount = buffer->readS32();
    int32_t pointCount = buffer->readS32();
    int32_t conicCount = buffer->readS32();
    ref->resetToSize(verbCount, pointCount, conicCount);

    SkASSERT(verbCount == ref->countVerbs());
    SkASSERT(pointCount == ref->countPoints());
    SkASSERT(conicCount == ref->fConicWeights.count());
    buffer->read(ref->verbsMemWritable(), verbCount * sizeof(uint8_t));
    buffer->read(ref->fPoints, pointCount * sizeof(SkPoint));
    buffer->read(ref->fConicWeights.begin(), conicCount * sizeof(SkScalar));
    buffer->read(&ref->fBounds, sizeof(SkRect));
    ref->fBoundsIsDirty = false;
    return ref;
}

/**
 * Writes the path points and verbs to a buffer.
 */
void SkPathRef::writeToBuffer(SkWBuffer* buffer) {
    this->validate();
    SkDEBUGCODE(size_t beforePos = buffer->pos();)

    // Call getBounds() to ensure (as a side-effect) that fBounds
    // and fIsFinite are computed.
    const SkRect& bounds = this->getBounds();

    int32_t packed = ((fIsFinite & 1) << kIsFinite_SerializationShift);
    buffer->write32(packed);

    // TODO: write gen ID here. Problem: We don't know if we're cross process or not from
    // SkWBuffer. Until this is fixed we write 0.
    buffer->write32(0);
    buffer->write32(fVerbCnt);
    buffer->write32(fPointCnt);
    buffer->write32(fConicWeights.count());
    buffer->write(verbsMemBegin(), fVerbCnt * sizeof(uint8_t));
    buffer->write(fPoints, fPointCnt * sizeof(SkPoint));
    buffer->write(fConicWeights.begin(), fConicWeights.bytes());
    buffer->write(&bounds, sizeof(bounds));

    SkASSERT(buffer->pos() - beforePos == (size_t) this->writeSize());
}
