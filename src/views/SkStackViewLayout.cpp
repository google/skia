
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkStackViewLayout.h"

SkStackViewLayout::SkStackViewLayout()
{
    fMargin.set(0, 0, 0, 0);
    fSpacer    = 0;
    fOrient    = kHorizontal_Orient;
    fPack    = kStart_Pack;
    fAlign    = kStart_Align;
    fRound    = false;
}

void SkStackViewLayout::setOrient(Orient ori)
{
    SkASSERT((unsigned)ori < kOrientCount);
    fOrient = SkToU8(ori);
}

void SkStackViewLayout::getMargin(SkRect* margin) const
{
    if (margin)
        *margin = fMargin;
}

void SkStackViewLayout::setMargin(const SkRect& margin)
{
    fMargin = margin;
}

void SkStackViewLayout::setSpacer(SkScalar spacer)
{
    fSpacer = spacer;
}

void SkStackViewLayout::setPack(Pack pack)
{
    SkASSERT((unsigned)pack < kPackCount);
    fPack = SkToU8(pack);
}

void SkStackViewLayout::setAlign(Align align)
{
    SkASSERT((unsigned)align < kAlignCount);
    fAlign = SkToU8(align);
}

void SkStackViewLayout::setRound(bool r)
{
    fRound = SkToU8(r);
}

////////////////////////////////////////////////////////////////////////////////

typedef SkScalar (*AlignProc)(SkScalar childLimit, SkScalar parentLimit);
typedef SkScalar (SkView::*GetSizeProc)() const;
typedef void (SkView::*SetLocProc)(SkScalar coord);
typedef void (SkView::*SetSizeProc)(SkScalar coord);

static SkScalar left_align_proc(SkScalar childLimit, SkScalar parentLimit) { return 0; }
static SkScalar center_align_proc(SkScalar childLimit, SkScalar parentLimit) { return SkScalarHalf(parentLimit - childLimit); }
static SkScalar right_align_proc(SkScalar childLimit, SkScalar parentLimit) { return parentLimit - childLimit; }
static SkScalar fill_align_proc(SkScalar childLimit, SkScalar parentLimit) { return 0; }

/*    Measure the main-dimension for all the children. If a child is marked flex in that direction
    ignore its current value but increment the counter for flexChildren
*/
static SkScalar compute_children_limit(SkView* parent, GetSizeProc sizeProc, int* count,
                                       uint32_t flexMask, int* flexCount)
{
    SkView::B2FIter    iter(parent);
    SkView*            child;
    SkScalar        limit = 0;
    int                n = 0, flex = 0;

    while ((child = iter.next()) != NULL)
    {
        n += 1;
        if (child->getFlags() & flexMask)
            flex += 1;
        else
            limit += (child->*sizeProc)();
    }
    if (count)
        *count = n;
    if (flexCount)
        *flexCount = flex;
    return limit;
}

void SkStackViewLayout::onLayoutChildren(SkView* parent)
{
    static AlignProc gAlignProcs[] = {
        left_align_proc,
        center_align_proc,
        right_align_proc,
        fill_align_proc
    };

    SkScalar            startM, endM, crossStartM, crossLimit;
    GetSizeProc            mainGetSizeP, crossGetSizeP;
    SetLocProc            mainLocP, crossLocP;
    SetSizeProc            mainSetSizeP, crossSetSizeP;
    SkView::Flag_Mask    flexMask;

    if (fOrient == kHorizontal_Orient)
    {
        startM        = fMargin.fLeft;
        endM        = fMargin.fRight;
        crossStartM    = fMargin.fTop;
        crossLimit    = -fMargin.fTop - fMargin.fBottom;

        mainGetSizeP    = &SkView::width;
        crossGetSizeP    = &SkView::height;
        mainLocP    = &SkView::setLocX;
        crossLocP    = &SkView::setLocY;

        mainSetSizeP  = &SkView::setWidth;
        crossSetSizeP = &SkView::setHeight;

        flexMask    = SkView::kFlexH_Mask;
    }
    else
    {
        startM        = fMargin.fTop;
        endM        = fMargin.fBottom;
        crossStartM    = fMargin.fLeft;
        crossLimit    = -fMargin.fLeft - fMargin.fRight;

        mainGetSizeP    = &SkView::height;
        crossGetSizeP    = &SkView::width;
        mainLocP    = &SkView::setLocY;
        crossLocP    = &SkView::setLocX;

        mainSetSizeP  = &SkView::setHeight;
        crossSetSizeP = &SkView::setWidth;

        flexMask    = SkView::kFlexV_Mask;
    }
    crossLimit += (parent->*crossGetSizeP)();
    if (fAlign != kStretch_Align)
        crossSetSizeP = NULL;

    int            childCount, flexCount;
    SkScalar    childLimit = compute_children_limit(parent, mainGetSizeP, &childCount, flexMask, &flexCount);

    if (childCount == 0)
        return;

    childLimit += (childCount - 1) * fSpacer;

    SkScalar        parentLimit = (parent->*mainGetSizeP)() - startM - endM;
    SkScalar        pos = startM + gAlignProcs[fPack](childLimit, parentLimit);
    SkScalar        flexAmount = 0;
    SkView::B2FIter    iter(parent);
    SkView*            child;

    if (flexCount > 0 && parentLimit > childLimit)
        flexAmount = (parentLimit - childLimit) / flexCount;

    while ((child = iter.next()) != NULL)
    {
        if (fRound)
            pos = SkIntToScalar(SkScalarRound(pos));
        (child->*mainLocP)(pos);
        SkScalar crossLoc = crossStartM + gAlignProcs[fAlign]((child->*crossGetSizeP)(), crossLimit);
        if (fRound)
            crossLoc = SkIntToScalar(SkScalarRound(crossLoc));
        (child->*crossLocP)(crossLoc);

        if (crossSetSizeP)
            (child->*crossSetSizeP)(crossLimit);
        if (child->getFlags() & flexMask)
            (child->*mainSetSizeP)(flexAmount);
        pos += (child->*mainGetSizeP)() + fSpacer;
    }
}

//////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
    static void assert_no_attr(const SkDOM& dom, const SkDOM::Node* node, const char attr[])
    {
        const char* value = dom.findAttr(node, attr);
        if (value)
            SkDebugf("unknown attribute %s=\"%s\"\n", attr, value);
    }
#else
    #define assert_no_attr(dom, node, attr)
#endif

void SkStackViewLayout::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
    int            index;
    SkScalar    value[4];

    if ((index = dom.findList(node, "orient", "horizontal,vertical")) >= 0)
        this->setOrient((Orient)index);
    else {
        assert_no_attr(dom, node, "orient");
        }

    if (dom.findScalars(node, "margin", value, 4))
    {
        SkRect    margin;
        margin.set(value[0], value[1], value[2], value[3]);
        this->setMargin(margin);
    }
    else {
        assert_no_attr(dom, node, "margin");
        }

    if (dom.findScalar(node, "spacer", value))
        this->setSpacer(value[0]);
    else {
        assert_no_attr(dom, node, "spacer");
        }

    if ((index = dom.findList(node, "pack", "start,center,end")) >= 0)
        this->setPack((Pack)index);
    else {
        assert_no_attr(dom, node, "pack");
        }

    if ((index = dom.findList(node, "align", "start,center,end,stretch")) >= 0)
        this->setAlign((Align)index);
    else {
        assert_no_attr(dom, node, "align");
        }
}

///////////////////////////////////////////////////////////////////////////////////////////

SkFillViewLayout::SkFillViewLayout()
{
    fMargin.setEmpty();
}

void SkFillViewLayout::getMargin(SkRect* r) const
{
    if (r)
        *r = fMargin;
}

void SkFillViewLayout::setMargin(const SkRect& margin)
{
    fMargin = margin;
}

void SkFillViewLayout::onLayoutChildren(SkView* parent)
{
    SkView::B2FIter    iter(parent);
    SkView*            child;

    while ((child = iter.next()) != NULL)
    {
        child->setLoc(fMargin.fLeft, fMargin.fTop);
        child->setSize(    parent->width() - fMargin.fRight - fMargin.fLeft,
                        parent->height() - fMargin.fBottom - fMargin.fTop);
    }
}

void SkFillViewLayout::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
    this->INHERITED::onInflate(dom, node);
    (void)dom.findScalars(node, "margin", (SkScalar*)&fMargin, 4);
}
