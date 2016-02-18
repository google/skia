/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkWindow_DEFINED
#define SkWindow_DEFINED

#include "../private/SkTDArray.h"
#include "SkView.h"
#include "SkBitmap.h"
#include "SkMatrix.h"
#include "SkRegion.h"
#include "SkEvent.h"
#include "SkKey.h"
#include "SkSurfaceProps.h"

class SkSurface;
class SkOSMenu;

#if SK_SUPPORT_GPU
struct GrGLInterface;
class GrContext;
class GrRenderTarget;
#endif

class SkWindow : public SkView {
public:
            SkWindow();
    virtual ~SkWindow();

    struct AttachmentInfo {
        int fSampleCount;
        int fStencilBits;
    };

    SkSurfaceProps getSurfaceProps() const { return fSurfaceProps; }
    void setSurfaceProps(const SkSurfaceProps& props) {
        fSurfaceProps = props;
    }

    SkImageInfo info() const { return fBitmap.info(); }
    const SkBitmap& getBitmap() const { return fBitmap; }

    void    resize(int width, int height);
    void    resize(const SkImageInfo&);
    void    setColorType(SkColorType, SkColorProfileType);

    bool    isDirty() const { return !fDirtyRgn.isEmpty(); }
    bool    update(SkIRect* updateArea);
    // does not call through to onHandleInval(), but does force the fDirtyRgn
    // to be wide open. Call before update() to ensure we redraw everything.
    void    forceInvalAll();
    // return the bounds of the dirty/inval rgn, or [0,0,0,0] if none
    const SkIRect& getDirtyBounds() const { return fDirtyRgn.getBounds(); }

    bool    handleClick(int x, int y, Click::State, void* owner, unsigned modi = 0);
    bool    handleChar(SkUnichar);
    bool    handleKey(SkKey);
    bool    handleKeyUp(SkKey);

    void    addMenu(SkOSMenu*);
    const SkTDArray<SkOSMenu*>* getMenus() { return &fMenus; }

    const char* getTitle() const { return fTitle.c_str(); }
    void    setTitle(const char title[]);

    const SkMatrix& getMatrix() const { return fMatrix; }
    void    setMatrix(const SkMatrix&);
    void    preConcat(const SkMatrix&);
    void    postConcat(const SkMatrix&);

    virtual SkSurface* createSurface();

protected:
    virtual bool onEvent(const SkEvent&);
    virtual bool onDispatchClick(int x, int y, Click::State, void* owner, unsigned modi);
    // called if part of our bitmap is invalidated
    virtual void onHandleInval(const SkIRect&);
    virtual bool onHandleChar(SkUnichar);
    virtual bool onHandleKey(SkKey);
    virtual bool onHandleKeyUp(SkKey);
    virtual void onAddMenu(const SkOSMenu*) {};
    virtual void onUpdateMenu(const SkOSMenu*) {};
    virtual void onSetTitle(const char title[]) {}

    // overrides from SkView
    virtual bool handleInval(const SkRect*);
    virtual bool onGetFocusView(SkView** focus) const;
    virtual bool onSetFocusView(SkView* focus);

#if SK_SUPPORT_GPU
    GrRenderTarget* renderTarget(const AttachmentInfo& attachmentInfo,
                                 const GrGLInterface* , GrContext* grContext);
#endif

private:
    SkSurfaceProps  fSurfaceProps;
    SkBitmap    fBitmap;
    SkRegion    fDirtyRgn;

    SkTDArray<Click*>       fClicks; // to track clicks

    SkTDArray<SkOSMenu*>    fMenus;

    SkView* fFocusView;
    bool    fWaitingOnInval;

    SkString    fTitle;
    SkMatrix    fMatrix;

    typedef SkView INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

#if defined(SK_USE_SDL)
    #include "SkOSWindow_SDL.h"
#elif defined(SK_BUILD_FOR_MAC)
    #include "SkOSWindow_Mac.h"
#elif defined(SK_BUILD_FOR_WIN)
    #include "SkOSWindow_Win.h"
#elif defined(SK_BUILD_FOR_ANDROID)
    #include "SkOSWindow_Android.h"
#elif defined(SK_BUILD_FOR_UNIX)
    #include "SkOSWindow_Unix.h"
#elif defined(SK_BUILD_FOR_IOS)
    #include "SkOSWindow_iOS.h"
#endif

#endif
