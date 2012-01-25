/*
 *  EdgeApp.cpp
 *  edge
 *
 *  Created by Cary Clark on 7/6/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkGraphics.h"
#include "SkImageEncoder.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkTime.h"
#include "SkWindow.h"

#include "SkTouchGesture.h"
#include "SkTypeface.h"

#include "Intersection_Tests.h"

extern void CreateSweep(SkBitmap* , float width);
extern void CreateHorz(SkBitmap* );
extern void CreateVert(SkBitmap* );
extern void CreateAngle(SkBitmap* sweep, float angle);
extern void SkAntiEdge_Test();


static const char gCharEvtName[] = "Char_Event";
static const char gKeyEvtName[] = "Key_Event";

class SkEdgeView : public SkView {
public:
    SkEdgeView() {
        CreateSweep(&fSweep_1_0, 1);
        CreateSweep(&fSweep_1_2, 1.2f);
        CreateSweep(&fSweep_1_4, 1.4f);
        CreateSweep(&fSweep_1_6, 1.6f);
        CreateHorz(&fBitmapH);
        CreateVert(&fBitmapV);
        CreateAngle(&fAngle_12, 12);
        CreateAngle(&fAngle_45, 45);
    }
    virtual ~SkEdgeView() {}

protected:
    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
        canvas->drawBitmap(fSweep_1_0, 0, 10);
        canvas->drawBitmap(fBitmapH, 110, 10);
        canvas->drawBitmap(fBitmapV, 220, 10);
        canvas->drawBitmap(fSweep_1_2, 0, 110);
        canvas->drawBitmap(fSweep_1_4, 100, 110);
        canvas->drawBitmap(fSweep_1_6, 200, 110);
        canvas->drawBitmap(fAngle_12, 0, 200);
        canvas->drawBitmap(fAngle_45, 124, 220);
    }

private:
    SkBitmap fSweep_1_0;
    SkBitmap fSweep_1_2;
    SkBitmap fSweep_1_4;
    SkBitmap fSweep_1_6;
    SkBitmap fBitmapH;
    SkBitmap fBitmapV;
    SkBitmap fAngle_12;
    SkBitmap fAngle_45;
    typedef SkView INHERITED;
};

class EdgeWindow : public SkOSWindow {
public:
    EdgeWindow(void* hwnd) : INHERITED(hwnd) {
        this->setConfig(SkBitmap::kARGB_8888_Config);
        this->setVisibleP(true);
        fView.setVisibleP(true);
        fView.setClipToBounds(false);
        this->attachChildToFront(&fView)->unref();
    }
    virtual ~EdgeWindow() {}

    virtual void draw(SkCanvas* canvas){
        this->INHERITED::draw(canvas);
    }


protected:
    SkEdgeView fView;
    
    virtual void onDraw(SkCanvas* canvas) {
    }

    virtual bool onHandleKey(SkKey key) {
        SkEvent evt(gKeyEvtName);
        evt.setFast32(key);
        if (fView.doQuery(&evt)) {
            return true;
        }
        return this->INHERITED::onHandleKey(key);
    }

    virtual bool onHandleChar(SkUnichar uni) {
        SkEvent evt(gCharEvtName);
        evt.setFast32(uni);
        if (fView.doQuery(&evt)) {
                return true;
        }
        return this->INHERITED::onHandleChar(uni);
    }

    virtual void onSizeChange() {
        fView.setSize(this->width(), this->height());
        this->INHERITED::onSizeChange();
    }

    virtual SkCanvas* beforeChildren(SkCanvas* canvas) {
        return this->INHERITED::beforeChildren(canvas);
    }
    
    virtual void afterChildren(SkCanvas*) {}
    virtual void beforeChild(SkView* child, SkCanvas* canvas) {}
    virtual void afterChild(SkView* child, SkCanvas* canvas) {}

    virtual bool onEvent(const SkEvent& evt) {
        return this->INHERITED::onEvent(evt);
    }
    
    virtual bool onQuery(SkEvent* evt) {
        return this->INHERITED::onQuery(evt);
    }

    virtual bool onDispatchClick(int x, int y, Click::State state, void* owner) {
        int w = SkScalarRound(this->width());
        int h = SkScalarRound(this->height());
        // check for the resize-box
        if (w - x < 16 && h - y < 16) {
            return false;   // let the OS handle the click
        } else {
            return this->INHERITED::onDispatchClick(x, y, state, owner);
        }
    }

    virtual bool onClick(Click* click) {
        return false;
    }

    virtual Click* onFindClickHandler(SkScalar x, SkScalar y) {
        return 0;
    }

    typedef SkOSWindow INHERITED;
};


#include "SkApplication.h"

SkOSWindow* create_sk_window(void* hwnd, int argc, char** argv) {
    return new EdgeWindow(hwnd);
}

void application_init() {
    SkGraphics::Init();
    SkEvent::Init();

    Intersection_Tests();
    SkAntiEdge_Test();
}

void application_term() {
    SkEvent::Term();
    SkGraphics::Term();
}

