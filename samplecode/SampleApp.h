/*
 * Copyright (C) 2011 Skia
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SampleWindow_DEFINED
#define SampleWindow_DEFINED

#include "SkWindow.h"

#include "SampleCode.h"
#include "SkPath.h"
#include "SkScalar.h"
#include "SkTDArray.h"
#include "SkTouchGesture.h"
#include "SkWindow.h"

class GrContext;

class SkEvent;
class SkCanvas;
class SkGpuCanvas;
class SkPicture;
class SkTypeface;
class SkData;

enum SkTriState {
    kFalse_SkTriState,
    kTrue_SkTriState,
    kUnknown_SkTriState,
};

class SampleWindow : public SkOSWindow {
    SkTDArray<SkViewFactory> fSamples;
public:
    SampleWindow(void* hwnd, int argc, char** argv);
    virtual ~SampleWindow();

    virtual void draw(SkCanvas* canvas);

    void toggleRendering();
    void toggleSlideshow();
    void toggleFPS();
    bool drawsToHardware() { return fCanvasType == kGPU_CanvasType; }
    bool setGrContext(GrContext*);
    GrContext* getGrContext();
    void setZoomCenter(float x, float y);
    void changeZoomLevel(float delta);
    bool nextSample();
    bool previousSample();
    bool goToSample(int i);
    SkString getSampleTitle(int i);
    int  sampleCount();
    bool handleTouch(int ownerId, float x, float y,
            SkView::Click::State state);
    void saveToPdf();
    SkData* getPDFData() { return fPDFData; }
    void postInvalDelay();

protected:
    virtual void onDraw(SkCanvas* canvas);
    virtual bool onHandleKey(SkKey key);
    virtual bool onHandleChar(SkUnichar);
    virtual void onSizeChange();

    virtual SkCanvas* beforeChildren(SkCanvas*);
    virtual void afterChildren(SkCanvas*);
    virtual void beforeChild(SkView* child, SkCanvas* canvas);
    virtual void afterChild(SkView* child, SkCanvas* canvas);

    virtual bool onEvent(const SkEvent& evt);
    virtual bool onQuery(SkEvent* evt);

    virtual bool onDispatchClick(int x, int y, Click::State, void* owner);
    virtual bool onClick(Click* click);
    virtual Click* onFindClickHandler(SkScalar x, SkScalar y);

private:
    int fCurrIndex;

    SkPicture* fPicture;
    SkGpuCanvas* fGpuCanvas;
    GrContext* fGrContext;
    SkPath fClipPath;

    SkTouchGesture fGesture;
    SkScalar fZoomLevel;
    SkScalar fZoomScale;

    enum CanvasType {
        kRaster_CanvasType,
        kPicture_CanvasType,
        kGPU_CanvasType
    };
    CanvasType fCanvasType;

    bool fSaveToPdf;
    SkCanvas* fPdfCanvas;
    SkData* fPDFData;

    bool fUseClip;
    bool fNClip;
    bool fRepeatDrawing;
    bool fAnimating;
    bool fRotate;
    bool fScale;
    bool fRequestGrabImage;
    bool fUsePipe;
    bool fMeasureFPS;
    SkMSec fMeasureFPS_Time;

    // The following are for the 'fatbits' drawing
    // Latest position of the mouse.
    int fMouseX, fMouseY;
    int fFatBitsScale;
    // Used by the text showing position and color values.
    SkTypeface* fTypeface;
    bool fShowZoomer;

    SkTriState fLCDState;
    SkTriState fAAState;
    SkTriState fFilterState;
    SkTriState fHintingState;
    unsigned   fFlipAxis;

    int fScrollTestX, fScrollTestY;
    SkScalar fZoomCenterX, fZoomCenterY;

    bool make3DReady();

    void loadView(SkView*);
    void updateTitle();

    void toggleZoomer();
    bool zoomIn();
    bool zoomOut();
    void updatePointer(int x, int y);
    void showZoomer(SkCanvas* canvas);

    void postAnimatingEvent();

    static CanvasType cycle_canvastype(CanvasType);

    typedef SkOSWindow INHERITED;
};

#endif
