/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GMSampleView_DEFINED
#define GMSampleView_DEFINED

#include "SampleCode.h"
#include "gm.h"

class GMSampleView : public SampleView {
private:
    bool fShowSize;
    typedef skiagm::GM GM;

public:
    GMSampleView(GM*);
    virtual ~GMSampleView();

    static SkEvent* NewShowSizeEvt(bool doShowSize);

protected:
    bool onQuery(SkEvent*) SK_OVERRIDE;
    bool onEvent(const SkEvent&) SK_OVERRIDE;
    void onDrawContent(SkCanvas*) SK_OVERRIDE;
    void onDrawBackground(SkCanvas*) SK_OVERRIDE;
    bool onAnimate(const SkAnimTimer&) SK_OVERRIDE;

private:
    GM* fGM;
    typedef SampleView INHERITED;
};

#endif
