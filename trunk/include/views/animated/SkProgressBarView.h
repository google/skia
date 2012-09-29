
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkProgressBarView_DEFINED
#define SkProgressBarView_DEFINED

#include "SkView.h"
#include "SkWidgetViews.h"
#include "SkAnimator.h"

class SkProgressBarView : public SkWidgetView {
    public:
        SkProgressBarView();
        //SkProgressBarView(int max);

        //inflate: "sk-progress"

        void reset();   //reset progress to zero
        void setProgress(int progress);
        void changeProgress(int diff);
        void setMax(int max);

        int getProgress() const { return fProgress; }
        int getMax() const { return fMax; }

    protected:
        //overrides
        virtual void onInflate(const SkDOM& dom, const SkDOM::Node* node);
        virtual void onSizeChange();
        virtual void onDraw(SkCanvas* canvas);
        virtual bool onEvent(const SkEvent& evt);

    private:
        SkAnimator  fAnim;
        int         fProgress;
        int         fMax;

        typedef SkWidgetView INHERITED;
};




#endif
