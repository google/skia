/* include/graphics/SkWindow.h
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#ifndef SkWindow_DEFINED
#define SkWindow_DEFINED

#include "SkView.h"
#include "SkBitmap.h"
#include "SkRegion.h"
#include "SkEvent.h"
#include "SkKey.h"
#include "SkTDArray.h"

#ifdef SK_BUILD_FOR_WINCEx
    #define SHOW_FPS
#endif
//#define USE_GX_SCREEN

class SkOSMenu;

class SkWindow : public SkView {
public:
            SkWindow();
    virtual ~SkWindow();

    const SkBitmap& getBitmap() const { return fBitmap; }

    void    setConfig(SkBitmap::Config);
    void    resize(int width, int height, SkBitmap::Config config = SkBitmap::kNo_Config);
    void    eraseARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b);
    void    eraseRGB(U8CPU r, U8CPU g, U8CPU b);

    bool    isDirty() const { return !fDirtyRgn.isEmpty(); }
    bool    update(SkRect16* updateArea);
    bool    handleClick(int x, int y, Click::State);
    bool    handleChar(SkUnichar);
    bool    handleKey(SkKey);
    bool    handleKeyUp(SkKey);
    bool    handleMenu(U32 os_cmd);

    void    addMenu(SkOSMenu*);

protected:
    virtual bool onEvent(const SkEvent&);

    // called if part of our bitmap is invalidated
    virtual void onHandleInval(const SkRect16&);
    virtual bool onHandleChar(SkUnichar);
    virtual bool onHandleKey(SkKey);
    virtual bool onHandleKeyUp(SkKey);
    virtual void onAddMenu(const SkOSMenu*) {}

    // overrides from SkView
    virtual bool handleInval(const SkRect&);
    virtual bool onGetFocusView(SkView** focus) const;
    virtual bool onSetFocusView(SkView* focus);

private:
    SkBitmap::Config    fConfig;
    SkBitmap    fBitmap;
    SkRegion    fDirtyRgn;
    Click*      fClick; // to track clicks

    SkTDArray<SkOSMenu*>    fMenus;

    SkView* fFocusView;
    bool    fWaitingOnInval;

    typedef SkView INHERITED;
};

///////////////////////////////////////////////////////////

#ifndef SK_USE_WXWIDGETS
#ifdef SK_BUILD_FOR_MAC
    #include "SkOSWindow_Mac.h"
#elif defined(SK_BUILD_FOR_WIN)
    #include "SkOSWindow_Win.h"
#elif defined(SK_BUILD_FOR_UNIXx)
  #include "SkOSWindow_Unix.h"
#endif
#else
  #include "SkOSWindow_wxwidgets.h"
#endif

#endif

