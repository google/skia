
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkView_DEFINED
#define SkView_DEFINED

#include "SkEventSink.h"
#include "SkRect.h"
#include "SkMatrix.h"
#include "SkMetaData.h"

class SkCanvas;

/** \class SkView

    SkView is the base class for screen management. All widgets and controls inherit
    from SkView.
*/
class SkView : public SkEventSink {
public:
    enum Flag_Shift {
        kVisible_Shift,
        kNoClip_Shift,

        kFlagShiftCount
    };
    enum Flag_Mask {
        kVisible_Mask   = 1 << kVisible_Shift,      //!< set if the view is visible
        kNoClip_Mask    = 1 << kNoClip_Shift,        //!< set if the view is not clipped to its bounds

        kAllFlagMasks   = (uint32_t)(0 - 1) >> (32 - kFlagShiftCount)
    };

                SkView(uint32_t flags = 0);
    virtual     ~SkView();

    /** Return the flags associated with the view
    */
    uint32_t    getFlags() const { return fFlags; }
    /** Set the flags associated with the view
    */
    void        setFlags(uint32_t flags);

    /** Helper that returns non-zero if the kVisible_Mask bit is set in the view's flags
    */
    int         isVisible() const { return fFlags & kVisible_Mask; }
    int         isClipToBounds() const { return !(fFlags & kNoClip_Mask); }
    /** Helper to set/clear the view's kVisible_Mask flag */
    void        setVisibleP(bool);
    void        setClipToBounds(bool);

    /** Return the view's width */
    SkScalar    width() const { return fWidth; }
    /** Return the view's height */
    SkScalar    height() const { return fHeight; }
    /** Set the view's width and height. These must both be >= 0. This does not affect the view's loc */
    void        setSize(SkScalar width, SkScalar height);
    void        setSize(const SkPoint& size) { this->setSize(size.fX, size.fY); }
    void        setWidth(SkScalar width) { this->setSize(width, fHeight); }
    void        setHeight(SkScalar height) { this->setSize(fWidth, height); }

    /** Call this to have the view draw into the specified canvas. */
    virtual void draw(SkCanvas* canvas);

    //  Click handling

    class Click {
    public:
        Click(SkView* target);
        virtual ~Click();

        enum State {
            kDown_State,
            kMoved_State,
            kUp_State
        };
        SkPoint     fOrig, fPrev, fCurr;
        SkIPoint    fIOrig, fIPrev, fICurr;
        State       fState;
        unsigned    fModifierKeys;

        SkMetaData  fMeta;
    private:
        SkEventSinkID   fTargetID;

        friend class SkView;
    };
    Click*  findClickHandler(SkScalar x, SkScalar y, unsigned modifierKeys);

    static void DoClickDown(Click*, int x, int y, unsigned modi);
    static void DoClickMoved(Click*, int x, int y, unsigned modi);
    static void DoClickUp(Click*, int x, int y, unsigned modi);

protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    virtual void    onDraw(SkCanvas*);
    /** Override this to be notified when the view's size changes. Be sure to call the inherited version too */
    virtual void    onSizeChange();

    /** Override this if you might handle the click
    */
    virtual Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi);
    /** Override this to track clicks, returning true as long as you want to track
        the pen/mouse.
    */
    virtual bool    onClick(Click*);

private:
    SkScalar    fWidth, fHeight;
    uint8_t     fFlags;
};

#endif
