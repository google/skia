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
    bool onQuery(SkEvent*) override;
    bool onEvent(const SkEvent&) override;
    void onDrawContent(SkCanvas*) override;
    void onDrawBackground(SkCanvas*) override;
    bool onAnimate(const SkAnimTimer&) override;

private:
    GM* fGM;
    typedef SampleView INHERITED;
};

#endif
