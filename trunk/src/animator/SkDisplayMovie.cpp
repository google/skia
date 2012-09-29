
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDisplayMovie.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkPaint.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDisplayMovie::fInfo[] = {
    SK_MEMBER(src, String)
};

#endif

DEFINE_GET_MEMBER(SkDisplayMovie);

SkDisplayMovie::SkDisplayMovie() : fDecodedSuccessfully(false), fLoaded(false), fMovieBuilt(false) {
    fMovie.fMaker->fInMovie = true;
}

SkDisplayMovie::~SkDisplayMovie() {
}

void SkDisplayMovie::buildMovie() {
    if (fMovieBuilt)
        return;
    SkAnimateMaker* movieMaker = fMovie.fMaker;
    SkAnimateMaker* parentMaker = movieMaker->fParentMaker;
    if (src.size() == 0 || parentMaker == NULL)
        return;
    movieMaker->fPrefix.set(parentMaker->fPrefix);
    fDecodedSuccessfully = fMovie.fMaker->decodeURI(src.c_str());
    if (fDecodedSuccessfully == false) {

        if (movieMaker->getErrorCode() != SkXMLParserError::kNoError || movieMaker->getNativeCode() != -1) {
            movieMaker->setInnerError(parentMaker, src);
            parentMaker->setErrorCode(SkDisplayXMLParserError::kInMovie);
        } else {
            parentMaker->setErrorNoun(src);
            parentMaker->setErrorCode(SkDisplayXMLParserError::kMovieNameUnknownOrMissing);
        }
    }
    fMovieBuilt = true;
}

SkDisplayable* SkDisplayMovie::deepCopy(SkAnimateMaker* maker) {
    SkDisplayMovie* copy = (SkDisplayMovie*) INHERITED::deepCopy(maker);
    copy->fMovie.fMaker->fParentMaker = fMovie.fMaker->fParentMaker;
    copy->fMovie.fMaker->fHostEventSinkID = fMovie.fMaker->fHostEventSinkID;
    copy->fMovieBuilt = false;
    *fMovie.fMaker->fParentMaker->fMovies.append() = copy;
    return copy;
}

void SkDisplayMovie::dirty() {
    buildMovie();
}

bool SkDisplayMovie::doEvent(SkDisplayEvent::Kind kind, SkEventState* state) {
    if (fLoaded == false)
        return false;
    fMovie.fMaker->fEnableTime = fMovie.fMaker->fParentMaker->fEnableTime;
    return fMovie.fMaker->fEvents.doEvent(*fMovie.fMaker, kind, state);
}

bool SkDisplayMovie::draw(SkAnimateMaker& maker) {
    if (fDecodedSuccessfully == false)
        return false;
    if (fLoaded == false)
        enable(maker);
    maker.fCanvas->save();
    SkPaint local = SkPaint(*maker.fPaint);
    bool result = fMovie.draw(maker.fCanvas, &local,
        maker.fDisplayList.getTime()) != SkAnimator::kNotDifferent;
    maker.fDisplayList.fInvalBounds.join(fMovie.fMaker->fDisplayList.fInvalBounds);
    maker.fCanvas->restore();
    return result;
}

#ifdef SK_DUMP_ENABLED
void SkDisplayMovie::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    SkDebugf("src=\"%s\"/>\n",  src.c_str());
    SkAnimateMaker* movieMaker = fMovie.fMaker;
    SkDisplayList::fIndent += 4;
    movieMaker->fDisplayList.dumpInner(movieMaker);
    SkDisplayList::fIndent -= 4;
    dumpEnd(maker);
}

void SkDisplayMovie::dumpEvents() {
    fMovie.fMaker->fEvents.dump(*fMovie.fMaker);
}
#endif

bool SkDisplayMovie::enable(SkAnimateMaker& maker) {
    if (fDecodedSuccessfully == false)
        return false;
    SkAnimateMaker* movieMaker = fMovie.fMaker;
    movieMaker->fEvents.doEvent(*movieMaker, SkDisplayEvent::kOnload, NULL);
    movieMaker->fEvents.removeEvent(SkDisplayEvent::kOnload, NULL);
    fLoaded = true;
    movieMaker->fLoaded = true;
    return false;
}

bool SkDisplayMovie::hasEnable() const {
    return true;
}

void SkDisplayMovie::onEndElement(SkAnimateMaker& maker) {
#if defined SK_DEBUG && defined SK_DEBUG_ANIMATION_TIMING
    fMovie.fMaker->fDebugTimeBase = maker.fDebugTimeBase;
#endif
    fMovie.fMaker->fPrefix.set(maker.fPrefix);
    fMovie.fMaker->fHostEventSinkID = maker.fHostEventSinkID;
    fMovie.fMaker->fParentMaker = &maker;
    buildMovie();
    *maker.fMovies.append() = this;
}


