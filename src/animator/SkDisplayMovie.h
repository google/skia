
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDisplayMovie_DEFINED
#define SkDisplayMovie_DEFINED

#include "SkAnimator.h"
#include "SkDrawable.h"
#include "SkMemberInfo.h"

struct SkEventState;

class SkDisplayMovie : public SkDrawable {
    DECLARE_DISPLAY_MEMBER_INFO(Movie);
    SkDisplayMovie();
    virtual ~SkDisplayMovie();
    void buildMovie();
    virtual SkDisplayable* deepCopy(SkAnimateMaker* );
    virtual void dirty();
    bool doEvent(const SkEvent& evt) {
        return fLoaded && fMovie.doEvent(evt);
    }
    virtual bool doEvent(SkDisplayEvent::Kind , SkEventState* state );
    virtual bool draw(SkAnimateMaker& );
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker* );
    virtual void dumpEvents();
#endif
    virtual bool enable(SkAnimateMaker& );
    const SkAnimator* getAnimator() const { return &fMovie; }
    virtual bool hasEnable() const;
    virtual void onEndElement(SkAnimateMaker& );
protected:
    SkString src;
    SkAnimator fMovie;
    SkBool8 fDecodedSuccessfully;
    SkBool8 fLoaded;
    SkBool8 fMovieBuilt;
    friend class SkAnimateMaker;
    friend class SkPost;
private:
    typedef SkDrawable INHERITED;
};

#endif // SkDisplayMovie_DEFINED
