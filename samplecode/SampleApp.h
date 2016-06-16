/*
 * Copyright 2011 Skia
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SampleApp_DEFINED
#define SampleApp_DEFINED

#include "SkOSMenu.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkScalar.h"
#include "SkTDArray.h"
#include "SkTouchGesture.h"
#include "SkWindow.h"
#include "timer/Timer.h"

class GrContext;
class GrRenderTarget;

class SkCanvas;
class SkData;
class SkDocument;
class SkEvent;
class SkTypeface;
class SkViewFactory;

class SampleWindow : public SkOSWindow {
    SkTDArray<const SkViewFactory*> fSamples;
public:
    enum DeviceType {
        kRaster_DeviceType,
#if SK_SUPPORT_GPU
        kGPU_DeviceType,
#if SK_ANGLE
        kANGLE_DeviceType,
#endif // SK_ANGLE
#endif // SK_SUPPORT_GPU
        kDeviceTypeCnt
    };

    static bool IsGpuDeviceType(DeviceType devType) {
    #if SK_SUPPORT_GPU
        switch (devType) {
            case kGPU_DeviceType:
    #if SK_ANGLE
            case kANGLE_DeviceType:
    #endif // SK_ANGLE
                return true;
            default:
                return false;
        }
    #endif // SK_SUPPORT_GPU
        return false;
    }

    /**
     * SampleApp ports can subclass this manager class if they want to:
     *      * filter the types of devices supported
     *      * customize plugging of SkBaseDevice objects into an SkCanvas
     *      * customize publishing the results of draw to the OS window
     *      * manage GrContext / GrRenderTarget lifetimes
     */
    class DeviceManager : public SkRefCnt {
    public:


        virtual void setUpBackend(SampleWindow* win, int msaaSampleCount, bool deepColor) = 0;

        virtual void tearDownBackend(SampleWindow* win) = 0;

        // called before drawing. should install correct device
        // type on the canvas. Will skip drawing if returns false.
        virtual SkSurface* createSurface(DeviceType dType, SampleWindow* win) = 0;

        // called after drawing, should get the results onto the
        // screen.
        virtual void publishCanvas(DeviceType dType,
                                   SkCanvas* canvas,
                                   SampleWindow* win) = 0;

        // called when window changes size, guaranteed to be called
        // at least once before first draw (after init)
        virtual void windowSizeChanged(SampleWindow* win) = 0;

        // return the GrContext backing gpu devices (nullptr if not built with GPU support)
        virtual GrContext* getGrContext() = 0;

        // return the GrRenderTarget backing gpu devices (nullptr if not built with GPU support)
        virtual GrRenderTarget* getGrRenderTarget() = 0;

        // return the color depth of the output device
        virtual int getColorBits() = 0;

    private:
        typedef SkRefCnt INHERITED;
    };

    SampleWindow(void* hwnd, int argc, char** argv, DeviceManager*);
    virtual ~SampleWindow();

    SkSurface* createSurface() override {
        SkSurface* surface = nullptr;
        if (fDevManager) {
            surface = fDevManager->createSurface(fDeviceType, this);
        }
        if (nullptr == surface) {
            surface = this->INHERITED::createSurface();
        }
        return surface;
    }

    void draw(SkCanvas*) override;

    void setDeviceType(DeviceType type);
    void setDeviceColorType(SkColorType, sk_sp<SkColorSpace>);
    void toggleRendering();
    void toggleSlideshow();
    void toggleFPS();
    void showOverview();
    void toggleDistanceFieldFonts();
    void setPixelGeometry(int pixelGeometryIndex);

    GrContext* getGrContext() const { return fDevManager->getGrContext(); }

    void setZoomCenter(float x, float y);
    void changeZoomLevel(float delta);
    void changeOffset(SkVector delta);
    bool nextSample();
    bool previousSample();
    bool goToSample(int i);
    SkString getSampleTitle(int i);
    int  sampleCount();
    bool handleTouch(int ownerId, float x, float y,
            SkView::Click::State state);
    void saveToPdf();
    void postInvalDelay();

    DeviceType getDeviceType() const { return fDeviceType; }

protected:
    void onDraw(SkCanvas* canvas) override;
    bool onHandleKey(SkKey key) override;
    bool onHandleChar(SkUnichar) override;
    void onSizeChange() override;

    SkCanvas* beforeChildren(SkCanvas*) override;
    void afterChildren(SkCanvas*) override;
    void beforeChild(SkView* child, SkCanvas* canvas) override;

    bool onEvent(const SkEvent& evt) override;
    bool onQuery(SkEvent* evt) override;

    virtual bool onDispatchClick(int x, int y, Click::State, void* owner,
                                 unsigned modi) override;
    bool onClick(Click* click) override;
    virtual Click* onFindClickHandler(SkScalar x, SkScalar y,
                                      unsigned modi) override;

private:
    class DefaultDeviceManager;

    int fCurrIndex;

    SkPictureRecorder fRecorder;
    SkAutoTDelete<SkCanvas> fFlagsFilterCanvas;
    SkPath fClipPath;

    SkTouchGesture fGesture;
    SkScalar fZoomLevel;
    SkScalar fZoomScale;
    SkVector fOffset;

    DeviceType fDeviceType;
    DeviceManager* fDevManager;

    bool fSaveToPdf;
    bool fSaveToSKP;
    sk_sp<SkDocument> fPDFDocument;

    bool fUseClip;
    bool fUsePicture;
    bool fAnimating;
    bool fRotate;
    bool fPerspAnim;
    bool fRequestGrabImage;
    bool fMeasureFPS;
    WallTimer fTimer;
    double fMeasureFPS_Time;
    bool fMagnify;
    int fTilingMode;

    // The following are for the 'fatbits' drawing
    // Latest position of the mouse.
    int fMouseX, fMouseY;
    int fFatBitsScale;
    // Used by the text showing position and color values.
    sk_sp<SkTypeface> fTypeface;
    bool fShowZoomer;

    SkOSMenu::TriState fLCDState;
    SkOSMenu::TriState fAAState;
    SkOSMenu::TriState fSubpixelState;
    int fHintingState;
    int fPixelGeometryIndex;
    int fFilterQualityIndex;
    unsigned   fFlipAxis;

    int fMSAASampleCount;
    bool fDeepColor;

    SkScalar fZoomCenterX, fZoomCenterY;

    //Stores global settings
    SkOSMenu* fAppMenu; // We pass ownership to SkWindow, when we call addMenu
    //Stores slide specific settings
    SkOSMenu* fSlideMenu; // We pass ownership to SkWindow, when we call addMenu

    void loadView(SkView*);
    void updateTitle();
    bool getRawTitle(SkString*);

    bool zoomIn();
    bool zoomOut();
    void updatePointer(int x, int y);
    void magnify(SkCanvas* canvas);
    void showZoomer(SkCanvas* canvas);
    void updateMatrix();
    void postAnimatingEvent();
    int findByTitle(const char*);
    void listTitles();
    SkSize tileSize() const;
    bool sendAnimatePulse();

    typedef SkOSWindow INHERITED;
};

#endif
