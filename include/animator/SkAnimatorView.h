
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkAnimatorView_DEFINED
#define SkAnimatorView_DEFINED

#include "SkView.h"
#include "SkAnimator.h"

class SkAnimatorView : public SkView {
public:
            SkAnimatorView();
    virtual ~SkAnimatorView();

    SkAnimator* getAnimator() const { return fAnimator; }

    bool    decodeFile(const char path[]);
    bool    decodeMemory(const void* buffer, size_t size);
    bool    decodeStream(SkStream* stream);

protected:
    // overrides
    virtual bool onEvent(const SkEvent&);
    virtual void onDraw(SkCanvas*);
    virtual void onInflate(const SkDOM&, const SkDOM::Node*);

private:
    SkAnimator* fAnimator;

    typedef SkView INHERITED;
};

#endif

