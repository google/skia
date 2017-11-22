
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
        kEnabled_Shift,
        kFocusable_Shift,
        kFlexH_Shift,
        kFlexV_Shift,
        kNoClip_Shift,

        kFlagShiftCount
    };
    enum Flag_Mask {
        kVisible_Mask   = 1 << kVisible_Shift,      //!< set if the view is visible
        kEnabled_Mask   = 1 << kEnabled_Shift,      //!< set if the view is enabled
        kFocusable_Mask = 1 << kFocusable_Shift,    //!< set if the view can receive focus
        kFlexH_Mask     = 1 << kFlexH_Shift,        //!< set if the view's width is stretchable
        kFlexV_Mask     = 1 << kFlexV_Shift,        //!< set if the view's height is stretchable
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
    int         isEnabled() const { return fFlags & kEnabled_Mask; }
    int         isFocusable() const { return fFlags & kFocusable_Mask; }
    int         isClipToBounds() const { return !(fFlags & kNoClip_Mask); }
    /** Helper to set/clear the view's kVisible_Mask flag */
    void        setVisibleP(bool);
    void        setEnabledP(bool);
    void        setFocusableP(bool);
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
    /** Return a rectangle set to [0, 0, width, height] */
    void        getLocalBounds(SkRect* bounds) const;

    /** Loc - the view's offset with respect to its parent in its view hiearchy.
        NOTE: For more complex transforms, use Local Matrix. The tranformations
        are applied in the following order:
             canvas->translate(fLoc.fX, fLoc.fY);
             canvas->concat(fMatrix);
    */
    /** Return the view's left edge */
    SkScalar    locX() const { return fLoc.fX; }
    /** Return the view's top edge */
    SkScalar    locY() const { return fLoc.fY; }
    /** Set the view's left and top edge. This does not affect the view's size */
    void        setLoc(SkScalar x, SkScalar y);
    void        setLoc(const SkPoint& loc) { this->setLoc(loc.fX, loc.fY); }
    void        setLocX(SkScalar x) { this->setLoc(x, fLoc.fY); }
    void        setLocY(SkScalar y) { this->setLoc(fLoc.fX, y); }

    /** Local Matrix - matrix used to tranform the view with respect to its
        parent in its view hiearchy. Use setLocalMatrix to apply matrix
        transformations to the current view and in turn affect its children.
        NOTE: For simple offsets, use Loc. The transformations are applied in
        the following order:
             canvas->translate(fLoc.fX, fLoc.fY);
             canvas->concat(fMatrix);
    */
    const SkMatrix& getLocalMatrix() const { return fMatrix; }
    void            setLocalMatrix(const SkMatrix& matrix);

    /** Offset (move) the view by the specified dx and dy. This does not affect the view's size */
    void        offset(SkScalar dx, SkScalar dy);

    /** Call this to have the view draw into the specified canvas. */
    virtual void draw(SkCanvas* canvas);

    /** Call this to invalidate part of all of a view, requesting that the view's
        draw method be called. The rectangle parameter specifies the part of the view
        that should be redrawn. If it is null, it specifies the entire view bounds.
    */
    void        inval(SkRect* rectOrNull);

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

    /** Convert the specified point from global coordinates into view-local coordinates
     *  Return true on success; false on failure
     */
    bool        globalToLocal(SkPoint* pt) const {
        if (pt) {
            return this->globalToLocal(pt->fX, pt->fY, pt);
        }
        return true;  // nothing to do so return true
    }
    /** Convert the specified x,y from global coordinates into view-local coordinates, returning
        the answer in the local parameter.
    */
    bool        globalToLocal(SkScalar globalX, SkScalar globalY, SkPoint* local) const;

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
    SkMatrix    fMatrix;
    SkPoint     fLoc;
    uint8_t     fFlags;

    /** Compute the matrix to transform view-local coordinates into global ones */
    void    localToGlobal(SkMatrix* matrix) const;
};

#endif
