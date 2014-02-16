
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkWindow.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkOSMenu.h"
#include "SkSystemEventTypes.h"
#include "SkTime.h"

#define SK_EventDelayInval "\xd" "n" "\xa" "l"

#define TEST_BOUNDERx

#include "SkBounder.h"
class TestSkBounder : public SkBounder {
public:
    explicit TestSkBounder(const SkBitmap& bm) : fCanvas(bm) {}

protected:
    virtual bool onIRect(const SkIRect& r) SK_OVERRIDE {
        SkRect    rr;

        rr.set(SkIntToScalar(r.fLeft), SkIntToScalar(r.fTop),
                SkIntToScalar(r.fRight), SkIntToScalar(r.fBottom));

        SkPaint    p;

        p.setStyle(SkPaint::kStroke_Style);
        p.setColor(SK_ColorYELLOW);

#if 0
        rr.inset(SK_ScalarHalf, SK_ScalarHalf);
#else
        rr.inset(-SK_ScalarHalf, -SK_ScalarHalf);
#endif

        fCanvas.drawRect(rr, p);
        return true;
    }
private:
    SkCanvas    fCanvas;
};

SkWindow::SkWindow() : fFocusView(NULL) {
    fClicks.reset();
    fWaitingOnInval = false;

#ifdef SK_BUILD_FOR_WINCE
    fColorType = kRGB_565_SkColorType;
#else
    fColorType = kPMColor_SkColorType;
#endif

    fMatrix.reset();
}

SkWindow::~SkWindow() {
    fClicks.deleteAll();
    fMenus.deleteAll();
}

SkCanvas* SkWindow::createCanvas() {
    return new SkCanvas(this->getBitmap());
}

void SkWindow::setMatrix(const SkMatrix& matrix) {
    if (fMatrix != matrix) {
        fMatrix = matrix;
        this->inval(NULL);
    }
}

void SkWindow::preConcat(const SkMatrix& matrix) {
    SkMatrix m;
    m.setConcat(fMatrix, matrix);
    this->setMatrix(m);
}

void SkWindow::postConcat(const SkMatrix& matrix) {
    SkMatrix m;
    m.setConcat(matrix, fMatrix);
    this->setMatrix(m);
}

void SkWindow::setColorType(SkColorType ct) {
    this->resize(fBitmap.width(), fBitmap.height(), ct);
}

void SkWindow::resize(int width, int height, SkColorType ct) {
    if (ct == kUnknown_SkColorType)
        ct = fColorType;

    if (width != fBitmap.width() || height != fBitmap.height() || ct != fColorType) {
        fColorType = ct;
        fBitmap.allocPixels(SkImageInfo::Make(width, height,
                                              ct, kPremul_SkAlphaType));

        this->setSize(SkIntToScalar(width), SkIntToScalar(height));
        this->inval(NULL);
    }
}

bool SkWindow::handleInval(const SkRect* localR) {
    SkIRect ir;

    if (localR) {
        SkRect devR;
        SkMatrix inverse;
        if (!fMatrix.invert(&inverse)) {
            return false;
        }
        fMatrix.mapRect(&devR, *localR);
        devR.round(&ir);
    } else {
        ir.set(0, 0,
               SkScalarRoundToInt(this->width()),
               SkScalarRoundToInt(this->height()));
    }
    fDirtyRgn.op(ir, SkRegion::kUnion_Op);

    this->onHandleInval(ir);
    return true;
}

void SkWindow::forceInvalAll() {
    fDirtyRgn.setRect(0, 0,
                      SkScalarCeilToInt(this->width()),
                      SkScalarCeilToInt(this->height()));
}

#if defined(SK_BUILD_FOR_WINCE) && defined(USE_GX_SCREEN)
    #include <windows.h>
    #include <gx.h>
    extern GXDisplayProperties gDisplayProps;
#endif

#ifdef SK_SIMULATE_FAILED_MALLOC
extern bool gEnableControlledThrow;
#endif

bool SkWindow::update(SkIRect* updateArea) {
    if (!fDirtyRgn.isEmpty()) {
        SkBitmap bm = this->getBitmap();

#if defined(SK_BUILD_FOR_WINCE) && defined(USE_GX_SCREEN)
        char* buffer = (char*)GXBeginDraw();
        SkASSERT(buffer);

        RECT    rect;
        GetWindowRect((HWND)((SkOSWindow*)this)->getHWND(), &rect);
        buffer += rect.top * gDisplayProps.cbyPitch + rect.left * gDisplayProps.cbxPitch;

        bm.setPixels(buffer);
#endif

        SkAutoTUnref<SkCanvas> canvas(this->createCanvas());

        canvas->clipRegion(fDirtyRgn);
        if (updateArea)
            *updateArea = fDirtyRgn.getBounds();

        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(fMatrix);

        // empty this now, so we can correctly record any inval calls that
        // might be made during the draw call.
        fDirtyRgn.setEmpty();

#ifdef TEST_BOUNDER
        TestSkBounder bounder(bm);
        canvas->setBounder(&bounder);
#endif
#ifdef SK_SIMULATE_FAILED_MALLOC
        gEnableControlledThrow = true;
#endif
#ifdef SK_BUILD_FOR_WIN32
        //try {
            this->draw(canvas);
        //}
        //catch (...) {
        //}
#else
        this->draw(canvas);
#endif
#ifdef SK_SIMULATE_FAILED_MALLOC
        gEnableControlledThrow = false;
#endif
#ifdef TEST_BOUNDER
        canvas->setBounder(NULL);
#endif

#if defined(SK_BUILD_FOR_WINCE) && defined(USE_GX_SCREEN)
        GXEndDraw();
#endif

        return true;
    }
    return false;
}

bool SkWindow::handleChar(SkUnichar uni) {
    if (this->onHandleChar(uni))
        return true;

    SkView* focus = this->getFocusView();
    if (focus == NULL)
        focus = this;

    SkEvent evt(SK_EventType_Unichar);
    evt.setFast32(uni);
    return focus->doEvent(evt);
}

bool SkWindow::handleKey(SkKey key) {
    if (key == kNONE_SkKey)
        return false;

    if (this->onHandleKey(key))
        return true;

    // send an event to the focus-view
    {
        SkView* focus = this->getFocusView();
        if (focus == NULL)
            focus = this;

        SkEvent evt(SK_EventType_Key);
        evt.setFast32(key);
        if (focus->doEvent(evt))
            return true;
    }

    if (key == kUp_SkKey || key == kDown_SkKey) {
        if (this->moveFocus(key == kUp_SkKey ? kPrev_FocusDirection : kNext_FocusDirection) == NULL)
            this->onSetFocusView(NULL);
        return true;
    }
    return false;
}

bool SkWindow::handleKeyUp(SkKey key) {
    if (key == kNONE_SkKey)
        return false;

    if (this->onHandleKeyUp(key))
        return true;

    //send an event to the focus-view
    {
        SkView* focus = this->getFocusView();
        if (focus == NULL)
            focus = this;

        //should this one be the same?
        SkEvent evt(SK_EventType_KeyUp);
        evt.setFast32(key);
        if (focus->doEvent(evt))
            return true;
    }
    return false;
}

void SkWindow::addMenu(SkOSMenu* menu) {
    *fMenus.append() = menu;
    this->onAddMenu(menu);
}

void SkWindow::setTitle(const char title[]) {
    if (NULL == title) {
        title = "";
    }
    fTitle.set(title);
    this->onSetTitle(title);
}

bool SkWindow::onEvent(const SkEvent& evt) {
    if (evt.isType(SK_EventDelayInval)) {
        for (SkRegion::Iterator iter(fDirtyRgn); !iter.done(); iter.next())
            this->onHandleInval(iter.rect());
        fWaitingOnInval = false;
        return true;
    }
    return this->INHERITED::onEvent(evt);
}

bool SkWindow::onGetFocusView(SkView** focus) const {
    if (focus)
        *focus = fFocusView;
    return true;
}

bool SkWindow::onSetFocusView(SkView* focus) {
    if (fFocusView != focus) {
        if (fFocusView)
            fFocusView->onFocusChange(false);
        fFocusView = focus;
        if (focus)
            focus->onFocusChange(true);
    }
    return true;
}

void SkWindow::onHandleInval(const SkIRect&) {
}

bool SkWindow::onHandleChar(SkUnichar) {
    return false;
}

bool SkWindow::onHandleKey(SkKey) {
    return false;
}

bool SkWindow::onHandleKeyUp(SkKey) {
    return false;
}

bool SkWindow::handleClick(int x, int y, Click::State state, void *owner,
                           unsigned modifierKeys) {
    return this->onDispatchClick(x, y, state, owner, modifierKeys);
}

bool SkWindow::onDispatchClick(int x, int y, Click::State state,
                               void* owner, unsigned modifierKeys) {
    bool handled = false;

    // First, attempt to find an existing click with this owner.
    int index = -1;
    for (int i = 0; i < fClicks.count(); i++) {
        if (owner == fClicks[i]->fOwner) {
            index = i;
            break;
        }
    }

    switch (state) {
        case Click::kDown_State: {
            if (index != -1) {
                delete fClicks[index];
                fClicks.remove(index);
            }
            Click* click = this->findClickHandler(SkIntToScalar(x),
                                                  SkIntToScalar(y), modifierKeys);

            if (click) {
                click->fOwner = owner;
                *fClicks.append() = click;
                SkView::DoClickDown(click, x, y, modifierKeys);
                handled = true;
            }
            break;
        }
        case Click::kMoved_State:
            if (index != -1) {
                SkView::DoClickMoved(fClicks[index], x, y, modifierKeys);
                handled = true;
            }
            break;
        case Click::kUp_State:
            if (index != -1) {
                SkView::DoClickUp(fClicks[index], x, y, modifierKeys);
                delete fClicks[index];
                fClicks.remove(index);
                handled = true;
            }
            break;
        default:
            // Do nothing
            break;
    }
    return handled;
}
