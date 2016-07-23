/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkViewPriv.h"

//////////////////////////////////////////////////////////////////////

void SkView::Artist::draw(SkView* view, SkCanvas* canvas)
{
    SkASSERT(view && canvas);
    this->onDraw(view, canvas);
}

void SkView::Artist::inflate(const SkDOM& dom, const SkDOM::Node* node)
{
    SkASSERT(node);
    this->onInflate(dom, node);
}

void SkView::Artist::onInflate(const SkDOM&, const SkDOM::Node*)
{
    // subclass should override this as needed
}

SkView::Artist* SkView::getArtist() const
{
    Artist_SkTagList* rec = (Artist_SkTagList*)this->findTagList(kViewArtist_SkTagList);
    SkASSERT(rec == nullptr || rec->fArtist != nullptr);

    return rec ? rec->fArtist : nullptr;
}

SkView::Artist* SkView::setArtist(Artist* obj)
{
    if (obj == nullptr)
    {
        this->removeTagList(kViewArtist_SkTagList);
    }
    else    // add/replace
    {
        Artist_SkTagList* rec = (Artist_SkTagList*)this->findTagList(kViewArtist_SkTagList);

        if (rec)
            SkRefCnt_SafeAssign(rec->fArtist, obj);
        else
            this->addTagList(new Artist_SkTagList(obj));
    }
    return obj;
}

////////////////////////////////////////////////////////////////////////////////

void SkView::Layout::layoutChildren(SkView* parent)
{
    SkASSERT(parent);
    if (parent->width() > 0 && parent->height() > 0)
        this->onLayoutChildren(parent);
}

void SkView::Layout::inflate(const SkDOM& dom, const SkDOM::Node* node)
{
    SkASSERT(node);
    this->onInflate(dom, node);
}

void SkView::Layout::onInflate(const SkDOM&, const SkDOM::Node*)
{
    // subclass should override this as needed
}

SkView::Layout* SkView::getLayout() const
{
    Layout_SkTagList* rec = (Layout_SkTagList*)this->findTagList(kViewLayout_SkTagList);
    SkASSERT(rec == nullptr || rec->fLayout != nullptr);

    return rec ? rec->fLayout : nullptr;
}

SkView::Layout* SkView::setLayout(Layout* obj, bool invokeLayoutNow)
{
    if (obj == nullptr)
    {
        this->removeTagList(kViewLayout_SkTagList);
    }
    else    // add/replace
    {
        Layout_SkTagList* rec = (Layout_SkTagList*)this->findTagList(kViewLayout_SkTagList);

        if (rec)
            SkRefCnt_SafeAssign(rec->fLayout, obj);
        else
            this->addTagList(new Layout_SkTagList(obj));
    }

    if (invokeLayoutNow)
        this->invokeLayout();

    return obj;
}
