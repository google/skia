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
class GrRenderTarget;

class SkEvent;
class SkCanvas;
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
    enum DeviceType {
        kRaster_DeviceType,
        kPicture_DeviceType,
        kGPU_DeviceType
    };
    /**
     * SampleApp ports can subclass this manager class if they want to:
     *      * filter the types of devices supported
     *      * customize plugging of SkDevice objects into an SkCanvas
     *      * customize publishing the results of draw to the OS window
     *      * manage GrContext / GrRenderTarget lifetimes
     */
    class DeviceManager : public SkRefCnt {
    public:
        // called at end of SampleWindow cons
        virtual void init(SampleWindow* win) = 0;

        // called when selecting a new device type
        // can disallow a device type by returning false.
        virtual bool supportsDeviceType(DeviceType dType) = 0;

        // called before drawing. should install correct device
        // type on the canvas. Will skip drawing if returns false.
        virtual bool prepareCanvas(DeviceType dType,
                                   SkCanvas* canvas,
                                   SampleWindow* win) = 0;

        // called after drawing, should get the results onto the
        // screen.
        virtual void publishCanvas(DeviceType dType,
                                   SkCanvas* canvas,
                                   SampleWindow* win) = 0;

        // called when window changes size, guaranteed to be called
        // at least once before first draw (after init)
        virtual void windowSizeChanged(SampleWindow* win) = 0;

        // return the GrContext backing gpu devices
        virtual GrContext* getGrContext() = 0;
    };

    SampleWindow(void* hwnd, int argc, char** argv, DeviceManager*);
    virtual ~SampleWindow();

    virtual void draw(SkCanvas* canvas);

    void toggleRendering();
    void toggleSlideshow();
    void toggleFPS();

    GrContext* getGrContext() const { return fDevManager->getGrContext(); }

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
    class DefaultDeviceManager;

    int fCurrIndex;

    SkPicture* fPicture;
    SkPath fClipPath;

    SkTouchGesture fGesture;
    SkScalar fZoomLevel;
    SkScalar fZoomScale;

    DeviceType fDeviceType;
    DeviceManager* fDevManager;

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

    void loadView(SkView*);
    void updateTitle();

    void toggleZoomer();
    bool zoomIn();
    bool zoomOut();
    void updatePointer(int x, int y);
    void showZoomer(SkCanvas* canvas);

    void postAnimatingEvent();

    typedef SkOSWindow INHERITED;
};

#endif
