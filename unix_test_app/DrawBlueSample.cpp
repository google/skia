
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkEvent.h"
#include "SkView.h"

class DrawBlue : public SkView {

public:
     DrawBlue() {}
protected:
    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorBLUE);
    }
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "DrawBlue");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
private:
    typedef SkView INHERITED;
};

static SkView* MyFactory() { return new DrawBlue; }
static SkViewRegister reg(MyFactory);
