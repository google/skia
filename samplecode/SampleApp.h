
/*
 * Copyright 2011 Skia
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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
#include "SkOSMenu.h"

class GrContext;
class GrRenderTarget;

class SkEvent;
class SkCanvas;
class SkPicture;
class SkTypeface;
class SkData;

class SampleWindow : public SkOSWindow {
    SkTDArray<const SkViewFactory*> fSamples;
public:
    enum DeviceType {
        kRaster_DeviceType,
        kPicture_DeviceType,
        kGPU_DeviceType,
#if SK_ANGLE
        kANGLE_DeviceType,
#endif
        kNullGPU_DeviceType
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
        virtual void setUpBackend(SampleWindow* win, int msaaSampleCount) = 0;

        virtual void tearDownBackend(SampleWindow* win) = 0;

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

        // return the GrRenderTarget backing gpu devices
        virtual GrRenderTarget* getGrRenderTarget() = 0;
    };

    SampleWindow(void* hwnd, int argc, char** argv, DeviceManager*);
    virtual ~SampleWindow();

    virtual void draw(SkCanvas* canvas);

    void setDeviceType(DeviceType type);
    void toggleRendering();
    void toggleSlideshow();
    void toggleFPS();
    void showOverview();

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

    DeviceType getDeviceType() const { return fDeviceType; }

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

    void registerPictFileSamples(char** argv, int argc);
    void registerPictFileSample(char** argv, int argc);

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
    bool fAnimating;
    bool fRotate;
    bool fPerspAnim;
    SkScalar fPerspAnimTime;
    bool fScale;
    bool fRequestGrabImage;
    bool fMeasureFPS;
    SkMSec fMeasureFPS_Time;
    bool fMagnify;


    SkOSMenu::TriState fPipeState;  // Mixed uses a tiled pipe
                                    // On uses a normal pipe
                                    // Off uses no pipe
    int  fUsePipeMenuItemID;
    bool fDebugger;

    // The following are for the 'fatbits' drawing
    // Latest position of the mouse.
    int fMouseX, fMouseY;
    int fFatBitsScale;
    // Used by the text showing position and color values.
    SkTypeface* fTypeface;
    bool fShowZoomer;

    SkOSMenu::TriState fLCDState;
    SkOSMenu::TriState fAAState;
    SkOSMenu::TriState fFilterState;
    SkOSMenu::TriState fHintingState;
    unsigned   fFlipAxis;

    int fMSAASampleCount;

    int fScrollTestX, fScrollTestY;
    SkScalar fZoomCenterX, fZoomCenterY;

    //Stores global settings
    SkOSMenu* fAppMenu; // We pass ownership to SkWindow, when we call addMenu
    //Stores slide specific settings
    SkOSMenu* fSlideMenu; // We pass ownership to SkWindow, when we call addMenu

    int fTransitionNext;
    int fTransitionPrev;

    void loadView(SkView*);
    void updateTitle();

    bool zoomIn();
    bool zoomOut();
    void updatePointer(int x, int y);
    void magnify(SkCanvas* canvas);
    void showZoomer(SkCanvas* canvas);
    void updateMatrix();
    void postAnimatingEvent();
    void installDrawFilter(SkCanvas*);
    int findByTitle(const char*);
    void listTitles();

    typedef SkOSWindow INHERITED;
};

#endif
