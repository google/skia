/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkTime.h"
#include "SkInterpolator.h"

extern bool is_overview(SkView* view);

static const char gIsTransitionQuery[] = "is-transition";
static const char gReplaceTransitionEvt[] = "replace-transition-view";

static bool is_transition(SkView* view) {
    SkEvent isTransition(gIsTransitionQuery);
    return view->doQuery(&isTransition);
}

class TransitionView : public SampleView {
public:
    TransitionView(SkView* prev, SkView* next, int direction) : fInterp(4, 2){
        fAnimationDirection = (Direction)(1 << (direction % 8));

        fPrev = prev;
        fPrev->setClipToBounds(false);
        fPrev->setVisibleP(true);
        (void)SampleView::SetUsePipe(fPrev, SkOSMenu::kOffState);
        //Not calling unref because fPrev is assumed to have been created, so
        //this will result in a transfer of ownership
        this->attachChildToBack(fPrev);

        fNext = next;
        fNext->setClipToBounds(true);
        fNext->setVisibleP(true);
        (void)SampleView::SetUsePipe(fNext, SkOSMenu::kOffState);
        //Calling unref because next is a newly created view and TransitionView
        //is now the sole owner of fNext
        this->attachChildToFront(fNext)->unref();

        fDone = false;
        //SkDebugf("--created transition\n");
    }

    ~TransitionView(){
        //SkDebugf("--deleted transition\n");
    }

    virtual void requestMenu(SkOSMenu* menu) {
        if (SampleView::IsSampleView(fNext))
            ((SampleView*)fNext)->requestMenu(menu);
    }

protected:
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SkString title;
            if (SampleCode::RequestTitle(fNext, &title)) {
                SampleCode::TitleR(evt, title.c_str());
                return true;
            }
            return false;
        }
        if (evt->isType(gIsTransitionQuery)) {
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    virtual bool onEvent(const SkEvent& evt) {
        if (evt.isType(gReplaceTransitionEvt)) {
            fPrev->detachFromParent();
            fPrev = (SkView*)SkEventSink::FindSink(evt.getFast32());
            (void)SampleView::SetUsePipe(fPrev, SkOSMenu::kOffState);
            //attach the new fPrev and call unref to balance the ref in onDraw
            this->attachChildToBack(fPrev)->unref();
            this->inval(NULL);
            return true;
        }
        if (evt.isType("transition-done")) {
            fNext->setLoc(0, 0);
            fNext->setClipToBounds(false);
            SkEvent* evt = new SkEvent(gReplaceTransitionEvt,
                                       this->getParent()->getSinkID());
            evt->setFast32(fNext->getSinkID());
            //increate ref count of fNext so it survives detachAllChildren
            fNext->ref();
            this->detachAllChildren();
            evt->post();
            return true;
        }
        return this->INHERITED::onEvent(evt);
    }
    virtual void onDrawBackground(SkCanvas* canvas) {}
    virtual void onDrawContent(SkCanvas* canvas) {
        if (fDone)
            return;

        if (is_overview(fNext) || is_overview(fPrev)) {
            fPipeState = SkOSMenu::kOffState;
        }

        SkScalar values[4];
        SkInterpolator::Result result = fInterp.timeToValues(SkTime::GetMSecs(), values);
        //SkDebugf("transition %x %d pipe:%d\n", this, result, fUsePipe);
        //SkDebugf("%f %f %f %f %d\n", values[0], values[1], values[2], values[3], result);
        if (SkInterpolator::kNormal_Result == result) {
            fPrev->setLocX(values[kPrevX]);
            fPrev->setLocY(values[kPrevY]);
            fNext->setLocX(values[kNextX]);
            fNext->setLocY(values[kNextY]);
            this->inval(NULL);
        }
        else {
            (new SkEvent("transition-done", this->getSinkID()))->post();
            fDone = true;
        }
    }

    virtual void onSizeChange() {
        this->INHERITED::onSizeChange();

        fNext->setSize(this->width(), this->height());
        fPrev->setSize(this->width(), this->height());

        SkScalar lr = 0, ud = 0;
        if (fAnimationDirection & (kLeftDirection|kULDirection|kDLDirection))
            lr = this->width();
        if (fAnimationDirection & (kRightDirection|kURDirection|kDRDirection))
            lr = -this->width();
        if (fAnimationDirection & (kUpDirection|kULDirection|kURDirection))
            ud = this->height();
        if (fAnimationDirection & (kDownDirection|kDLDirection|kDRDirection))
            ud = -this->height();

        fBegin[kPrevX] = fBegin[kPrevY] = 0;
        fBegin[kNextX] = lr;
        fBegin[kNextY] = ud;
        fNext->setLocX(lr);
        fNext->setLocY(ud);

        if (is_transition(fPrev))
            lr = ud = 0;
        fEnd[kPrevX] = -lr;
        fEnd[kPrevY] = -ud;
        fEnd[kNextX] = fEnd[kNextY] = 0;
        SkScalar blend[] = { SkFloatToScalar(0.8f), SkFloatToScalar(0.0f),
                             SkFloatToScalar(0.0f), SK_Scalar1 };
        fInterp.setKeyFrame(0, SkTime::GetMSecs(), fBegin, blend);
        fInterp.setKeyFrame(1, SkTime::GetMSecs()+500, fEnd, blend);
    }

private:
    enum {
        kPrevX = 0,
        kPrevY = 1,
        kNextX = 2,
        kNextY = 3
    };
    SkView* fPrev;
    SkView* fNext;
    bool    fDone;
    SkInterpolator fInterp;

    enum Direction{
        kUpDirection    = 1,
        kURDirection    = 1 << 1,
        kRightDirection = 1 << 2,
        kDRDirection    = 1 << 3,
        kDownDirection  = 1 << 4,
        kDLDirection    = 1 << 5,
        kLeftDirection  = 1 << 6,
        kULDirection    = 1 << 7
    };

    Direction fAnimationDirection;
    SkScalar fBegin[4];
    SkScalar fEnd[4];

    typedef SampleView INHERITED;
};

// FIXME: this should go in a header
SkView* create_transition(SkView* prev, SkView* next, int direction);

SkView* create_transition(SkView* prev, SkView* next, int direction) {
    return SkNEW_ARGS(TransitionView, (prev, next, direction));
};
