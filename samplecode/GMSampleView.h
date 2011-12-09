
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
    typedef skiagm::GM GM;

public:
    GMSampleView(GM* gm)
    : fGM(gm) {}
    
    virtual ~GMSampleView() {
        delete fGM;
    }
    
protected:
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SkString name("GM:");
            name.append(fGM->shortName());
            SampleCode::TitleR(evt, name.c_str());
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    virtual void onDrawContent(SkCanvas* canvas) {
        fGM->drawContent(canvas);
    }

    virtual void onDrawBackground(SkCanvas* canvas) {
        fGM->drawBackground(canvas);
    }

private:
    GM* fGM;
    typedef SampleView INHERITED;
};

#endif
