
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#include <stdint.h>

#include "GrContext.h"
#include "SkGpuCanvas.h"
#include "SkPaint.h"
#include "SkString.h"
#include "SkTime.h"

#include "gl/GrGLConfig.h"

static GrContext* make_context() {
    SkDebugf("---- before create\n");
    GrContext* ctx = GrContext::Create(GrGpu::kOpenGL_Shaders_Engine, 0);
    SkDebugf("---- after create %p\n", ctx);
    return ctx;
}

///////////////////////////////////////////////////////////////////////////////

void gr_run_unittests() {}

#include "FlingState.h"
#include "SkTouchGesture.h"
#include "SkView.h"

typedef SkView* (*SkViewFactory)();

// these values must match those in Ganesh.java
enum TouchState {
    kUnknown_TouchState,
    kDown_TouchState,
    kMoved_TouchState,
    kUp_TouchState
};

struct State {
    State();
    ~State();

    int countSlides() const { return fFactory.count(); }
    const char* getSlideTitle(int index) const;
    void chooseSlide(int index) {
        SkDebugf("----- index %d\n", index);
        if (index < fFactory.count()) {
            this->setView(fFactory[index]());
        }
    }

    void setViewport(int w, int h);
    int getWidth() const { return fViewport.fX; }
    int getHeight() const { return fViewport.fY; }

    void handleTouch(void*, TouchState, float x, float y);
    void applyMatrix(SkCanvas*);

    SkView* getView() const { return fView; }

private:
    SkView*     fView;
    SkIPoint    fViewport;

    SkTouchGesture fGesture;

    SkTDArray<SkViewFactory> fFactory;

    void setView(SkView* view) {
        SkSafeUnref(fView);
        fView = view;

        view->setVisibleP(true);
        view->setClipToBounds(false);
        view->setSize(SkIntToScalar(fViewport.fX),
                      SkIntToScalar(fViewport.fY));
    }
};

///////////////////////////////////////////////////////////////////////////////

#include "SampleCode.h"

SkViewRegister* SkViewRegister::gHead;
SkViewRegister::SkViewRegister(SkViewFactory fact) : fFact(fact) {
    static bool gOnce;
    if (!gOnce) {
        gHead = NULL;
        gOnce = true;
    }

    fChain = gHead;
    gHead = this;
}

static const char gCharEvtName[] = "SampleCode_Char_Event";
static const char gKeyEvtName[] = "SampleCode_Key_Event";
static const char gTitleEvtName[] = "SampleCode_Title_Event";
static const char gPrefSizeEvtName[] = "SampleCode_PrefSize_Event";
static const char gFastTextEvtName[] = "SampleCode_FastText_Event";

bool SampleCode::CharQ(const SkEvent& evt, SkUnichar* outUni) {
    if (evt.isType(gCharEvtName, sizeof(gCharEvtName) - 1)) {
        if (outUni) {
            *outUni = evt.getFast32();
        }
        return true;
    }
    return false;
}

bool SampleCode::KeyQ(const SkEvent& evt, SkKey* outKey) {
    if (evt.isType(gKeyEvtName, sizeof(gKeyEvtName) - 1)) {
        if (outKey) {
            *outKey = (SkKey)evt.getFast32();
        }
        return true;
    }
    return false;
}

bool SampleCode::TitleQ(const SkEvent& evt) {
    return evt.isType(gTitleEvtName, sizeof(gTitleEvtName) - 1);
}

void SampleCode::TitleR(SkEvent* evt, const char title[]) {
    GrAssert(evt && TitleQ(*evt));
    evt->setString(gTitleEvtName, title);
}

bool SampleCode::PrefSizeQ(const SkEvent& evt) {
    return evt.isType(gPrefSizeEvtName, sizeof(gPrefSizeEvtName) - 1);
}

void SampleCode::PrefSizeR(SkEvent* evt, SkScalar width, SkScalar height) {
    GrAssert(evt && PrefSizeQ(*evt));
    SkScalar size[2];
    size[0] = width;
    size[1] = height;
    evt->setScalars(gPrefSizeEvtName, 2, size);
}

bool SampleCode::FastTextQ(const SkEvent& evt) {
    return evt.isType(gFastTextEvtName, sizeof(gFastTextEvtName) - 1);
}

static SkMSec gAnimTime;
static SkMSec gAnimTimePrev;

SkMSec SampleCode::GetAnimTime() { return gAnimTime; }
SkMSec SampleCode::GetAnimTimeDelta() { return gAnimTime - gAnimTimePrev; }
SkScalar SampleCode::GetAnimSecondsDelta() {
    return SkDoubleToScalar(GetAnimTimeDelta() / 1000.0);
}

SkScalar SampleCode::GetAnimScalar(SkScalar speed, SkScalar period) {
    // since gAnimTime can be up to 32 bits, we can't convert it to a float
    // or we'll lose the low bits. Hence we use doubles for the intermediate
    // calculations
    double seconds = (double)gAnimTime / 1000.0;
    double value = SkScalarToDouble(speed) * seconds;
    if (period) {
        value = ::fmod(value, SkScalarToDouble(period));
    }
    return SkDoubleToScalar(value);
}

static void drawIntoCanvas(State* state, SkCanvas* canvas) {
    gAnimTime = SkTime::GetMSecs();
    SkView* view = state->getView();
    view->draw(canvas);
}

///////////////////////////////////////////////////////////////////////////////

static void resetGpuState();

State::State() {
    fViewport.set(0, 0);

    const SkViewRegister* reg = SkViewRegister::Head();
    while (reg) {
        *fFactory.append() = reg->factory();
        reg = reg->next();
    }

    SkDebugf("----- %d slides\n", fFactory.count());
    fView = NULL;
    this->chooseSlide(0);
}

State::~State() {
    SkSafeUnref(fView);
}

void State::setViewport(int w, int h) {
    fViewport.set(w, h);
    if (fView) {
        fView->setSize(SkIntToScalar(w), SkIntToScalar(h));
    }
    resetGpuState();
}

const char* State::getSlideTitle(int index) const {
    SkEvent evt(gTitleEvtName);
    evt.setFast32(index);
    {
        SkView* view = fFactory[index]();
        view->doQuery(&evt);
        view->unref();
    }
    return evt.findString(gTitleEvtName);
}

void State::handleTouch(void* owner, TouchState state, float x, float y) {
    switch (state) {
        case kDown_TouchState:
            fGesture.touchBegin(owner, x, y);
            break;
        case kMoved_TouchState:
            fGesture.touchMoved(owner, x, y);
            break;
        case kUp_TouchState:
            fGesture.touchEnd(owner);
            break;
    }
}

void State::applyMatrix(SkCanvas* canvas) {
    const SkMatrix& localM = fGesture.localM();
    if (localM.getType() & SkMatrix::kScale_Mask) {
        canvas->setExternalMatrix(&localM);
    }
    canvas->concat(localM);
    canvas->concat(fGesture.globalM());
}

static State* get_state() {
    static State* gState;
    if (NULL == gState) {
        gState = SkNEW(State);
    }
    return gState;
}

///////////////////////////////////////////////////////////////////////////////

static GrContext* gContext;
static int gWidth;
static int gHeight;
static float gX, gY;

static void resetGpuState() {
    if (NULL == gContext) {
        SkDebugf("creating context for first time\n");
        gContext = make_context();
    } else {
        SkDebugf("------ gContext refcnt=%d\n", gContext->refcnt());
        gContext->abandonAllTextures();
        gContext->unref();
        gContext = make_context();
    }
}

static void doDraw() {
    if (NULL == gContext) {
        gContext = make_context();
    }

    State* state = get_state();
    SkBitmap viewport;
    viewport.setConfig(SkBitmap::kARGB_8888_Config,
                       state->getWidth(), state->getHeight());

    SkGpuCanvas canvas(gContext);

    canvas.setBitmapDevice(viewport);
    state->applyMatrix(&canvas);

    drawIntoCanvas(state, &canvas);

    GrGLCheckErr();
    GrGLClearErr();
//    gContext->checkError();
//    gContext->clearError();

    if (true) {
        static const int FRAME_COUNT = 32;
        static SkMSec gDuration;

        static SkMSec gNow;
        static int gFrameCounter;
        if (++gFrameCounter == FRAME_COUNT) {
            gFrameCounter = 0;
            SkMSec now = SkTime::GetMSecs();

            gDuration = now - gNow;
            gNow = now;
        }

        int fps = (FRAME_COUNT * 1000) / gDuration;
        SkString str;
        str.printf("FPS=%3d MS=%3d", fps, gDuration / FRAME_COUNT);

        SkGpuCanvas c(gContext);
        c.setBitmapDevice(viewport);

        SkPaint p;
        p.setAntiAlias(true);
        SkRect r = { 0, 0, 110, 16 };
        p.setColor(SK_ColorWHITE);
        c.drawRect(r, p);
        p.setColor(SK_ColorBLACK);
        c.drawText(str.c_str(), str.size(), 4, 12, p);
    }
}

///////////////////////////////////////////////////////////////////////////////

extern "C" {
    JNIEXPORT void JNICALL Java_com_tetrark_ganesh_MyRenderer_nativeSurfaceCreated(
                                                           JNIEnv*, jobject);
    JNIEXPORT void JNICALL Java_com_tetrark_ganesh_MyRenderer_nativeViewport(JNIEnv*, jobject,
                                                                             jint w, jint h);
    JNIEXPORT void JNICALL Java_com_tetrark_ganesh_MyRenderer_nativeDrawFrame(JNIEnv*, jobject);
    JNIEXPORT void JNICALL Java_com_tetrark_ganesh_MyRenderer_nativeTouch(JNIEnv*, jobject,
                                        jint id, jint type, jfloat x, jfloat y);

    JNIEXPORT int JNICALL Java_com_tetrark_ganesh_MyRenderer_nativeCountSlides(JNIEnv*, jobject);
    JNIEXPORT jobject JNICALL Java_com_tetrark_ganesh_MyRenderer_nativeGetSlideTitle(JNIEnv*, jobject, jint index);
    JNIEXPORT void JNICALL Java_com_tetrark_ganesh_MyRenderer_nativeChooseSlide(JNIEnv*, jobject, jint index);
}

JNIEXPORT void JNICALL Java_com_tetrark_ganesh_MyRenderer_nativeSurfaceCreated(
                                                            JNIEnv*, jobject) {
    SkDebugf("------ nativeSurfaceCreated\n");
    resetGpuState();
    SkDebugf("------ end nativeSurfaceCreated\n");
}

JNIEXPORT void JNICALL Java_com_tetrark_ganesh_MyRenderer_nativeViewport(JNIEnv*, jobject,
                                                       jint w, jint h) {
    State* state = get_state();
    SkDebugf("---- state.setviewport %p %d %d\n", state, w, h);
    state->setViewport(w, h);
    SkDebugf("---- end setviewport\n");
}

JNIEXPORT void JNICALL Java_com_tetrark_ganesh_MyRenderer_nativeDrawFrame(JNIEnv*, jobject) {
    doDraw();
}

union IntPtr {
    jint    fInt;
    void*   fPtr;
};
static void* int2ptr(jint n) {
    IntPtr data;
    data.fInt = n;
    return data.fPtr;
}

JNIEXPORT void JNICALL Java_com_tetrark_ganesh_MyRenderer_nativeTouch(JNIEnv*, jobject,
                                      jint id, jint type, jfloat x, jfloat y) {
    get_state()->handleTouch(int2ptr(id), (TouchState)type, x, y);
}

////////////

JNIEXPORT int JNICALL Java_com_tetrark_ganesh_MyRenderer_nativeCountSlides(JNIEnv*, jobject) {
    return get_state()->countSlides();
}

JNIEXPORT jobject JNICALL Java_com_tetrark_ganesh_MyRenderer_nativeGetSlideTitle(JNIEnv* env, jobject, jint index) {
    return env->NewStringUTF(get_state()->getSlideTitle(index));
}

JNIEXPORT void JNICALL Java_com_tetrark_ganesh_MyRenderer_nativeChooseSlide(JNIEnv*, jobject, jint index) {
    get_state()->chooseSlide(index);
}





