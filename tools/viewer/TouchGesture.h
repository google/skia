
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef TouchGesture_DEFINED
#define TouchGesture_DEFINED

#include "include/core/SkMatrix.h"
#include "include/private/SkTDArray.h"

class TouchGesture {
public:
    TouchGesture();
    ~TouchGesture();

    void touchBegin(void* owner, float x, float y);
    void touchMoved(void* owner, float x, float y);
    void touchEnd(void* owner);
    void reset();
    void resetTouchState();

    bool isActive() { return fFlinger.isActive(); }
    void stop() { fFlinger.stop(); }
    bool isBeingTouched() { return kEmpty_State != fState; }
    bool isFling(SkPoint* dir);

    void startZoom();
    void updateZoom(float scale, float startX, float startY, float lastX, float lastY);
    void endZoom();

    const SkMatrix& localM();
    const SkMatrix& globalM() const { return fGlobalM; }

    void setTransLimit(const SkRect& contentRect, const SkRect& windowRect,
                       const SkMatrix& preTouchM);

private:
    enum State {
        kEmpty_State,
        kTranslate_State,
        kZoom_State,
    };

    struct Rec {
        void*   fOwner;
        float   fStartX, fStartY;
        float   fPrevX, fPrevY;
        float   fLastX, fLastY;
        float   fPrevT, fLastT;
    };
    SkTDArray<Rec> fTouches;

    State           fState;
    SkMatrix        fLocalM, fGlobalM, fPreTouchM;

    struct FlingState {
        FlingState() : fActive(false) {}

        bool isActive() const { return fActive; }
        void stop() { fActive = false; }

        void reset(float sx, float sy);
        bool evaluateMatrix(SkMatrix* matrix);

        void get(SkPoint* dir, SkScalar* speed) {
            *dir = fDirection;
            *speed = fSpeed0;
        }

    private:
        SkPoint     fDirection;
        SkScalar    fSpeed0;
        double      fTime0;
        bool        fActive;
    };
    FlingState      fFlinger;
    double          fLastUpMillis;
    SkPoint         fLastUpP;

    // The following rects are used to limit the translation so the content never leaves the window
    SkRect          fContentRect, fWindowRect;
    bool            fIsTransLimited = false;

    void limitTrans(); // here we only limit the translation with respect to globalM
    void flushLocalM();
    int findRec(void* owner) const;
    void appendNewRec(void* owner, float x, float y);
    float computePinch(const Rec&, const Rec&);
    bool handleDblTap(float, float);
};

#endif
