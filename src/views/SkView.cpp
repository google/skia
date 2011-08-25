
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkView.h"
#include "SkCanvas.h"

////////////////////////////////////////////////////////////////////////

SkView::SkView(uint32_t flags) : fFlags(SkToU8(flags))
{
	fWidth = fHeight = 0;
	fLoc.set(0, 0);
	fParent = fFirstChild = fNextSibling = fPrevSibling = NULL;
	
	fContainsFocus = 0;
}

SkView::~SkView()
{
	this->detachAllChildren();
}

void SkView::setFlags(uint32_t flags)
{
	SkASSERT((flags & ~kAllFlagMasks) == 0);

	uint32_t diff = fFlags ^ flags;

	if (diff & kVisible_Mask)
		this->inval(NULL);

	fFlags = SkToU8(flags);

	if (diff & kVisible_Mask)
	{
		this->inval(NULL);
	}
}

void SkView::setVisibleP(bool pred)
{
	this->setFlags(SkSetClearShift(fFlags, pred, kVisible_Shift));
}

void SkView::setEnabledP(bool pred)
{
	this->setFlags(SkSetClearShift(fFlags, pred, kEnabled_Shift));
}

void SkView::setFocusableP(bool pred)
{
	this->setFlags(SkSetClearShift(fFlags, pred, kFocusable_Shift));
}

void SkView::setClipToBounds(bool pred) {
    this->setFlags(SkSetClearShift(fFlags, !pred, kNoClip_Shift));
}

void SkView::setSize(SkScalar width, SkScalar height)
{
	width = SkMaxScalar(0, width);
	height = SkMaxScalar(0, height);

	if (fWidth != width || fHeight != height)
	{
		this->inval(NULL);
		fWidth = width;
		fHeight = height;
		this->inval(NULL);
		this->onSizeChange();
		this->invokeLayout();
	}
}

void SkView::setLoc(SkScalar x, SkScalar y)
{
	if (fLoc.fX != x || fLoc.fY != y)
	{
		this->inval(NULL);
		fLoc.set(x, y);
		this->inval(NULL);
	}
}

void SkView::offset(SkScalar dx, SkScalar dy)
{
	if (dx || dy)
		this->setLoc(fLoc.fX + dx, fLoc.fY + dy);
}

void SkView::draw(SkCanvas* canvas)
{
	if (fWidth && fHeight && this->isVisible())
	{
		SkRect	r;
		r.set(fLoc.fX, fLoc.fY, fLoc.fX + fWidth, fLoc.fY + fHeight);
		if (this->isClipToBounds() &&
            canvas->quickReject(r, SkCanvas::kBW_EdgeType)) {
                return;
        }

		SkAutoCanvasRestore	as(canvas, true);

        if (this->isClipToBounds()) {
            canvas->clipRect(r);
        }
		canvas->translate(fLoc.fX, fLoc.fY);

        if (fParent) {
            fParent->beforeChild(this, canvas);
        }

        int sc = canvas->save();
		this->onDraw(canvas);
        canvas->restoreToCount(sc);

        if (fParent) {
            fParent->afterChild(this, canvas);
        }
        
		B2FIter	iter(this);
		SkView*	child;

        SkCanvas* childCanvas = this->beforeChildren(canvas);

		while ((child = iter.next()) != NULL)
			child->draw(childCanvas);
        
        this->afterChildren(canvas);
	}
}

void SkView::inval(SkRect* rect) {
	SkView*	view = this;
    SkRect storage;

	for (;;) {
        if (!view->isVisible()) {
            return;
        }
        if (view->isClipToBounds()) {
            SkRect bounds;
            view->getLocalBounds(&bounds);
            if (rect && !bounds.intersect(*rect)) {
                return;
            }
            storage = bounds;
            rect = &storage;
        }
        if (view->handleInval(rect)) {
            return;
        }

		SkView* parent = view->fParent;
        if (parent == NULL) {
            return;
        }

        if (rect) {
            rect->offset(view->fLoc.fX, view->fLoc.fY);
        }
        view = parent;
	}
}

////////////////////////////////////////////////////////////////////////////

bool SkView::setFocusView(SkView* fv)
{
	SkView* view = this;
	
	do {
		if (view->onSetFocusView(fv))
			return true;
	} while ((view = view->fParent) != NULL);
	return false;
}

SkView* SkView::getFocusView() const
{
	SkView*			focus = NULL;
	const SkView*	view = this;
	do {
		if (view->onGetFocusView(&focus))
			break;
	} while ((view = view->fParent) != NULL);
	return focus;
}

bool SkView::hasFocus() const
{
	return this == this->getFocusView();
}

bool SkView::acceptFocus()
{
	return this->isFocusable() && this->setFocusView(this);
}

/*
	Try to give focus to this view, or its children
*/
SkView* SkView::acceptFocus(FocusDirection dir)
{
	if (dir == kNext_FocusDirection)
	{
		if (this->acceptFocus())
			return this;

		B2FIter	iter(this);
		SkView*	child, *focus;
		while ((child = iter.next()) != NULL)
			if ((focus = child->acceptFocus(dir)) != NULL)
				return focus;
	}
	else // prev
	{
		F2BIter	iter(this);
		SkView*	child, *focus;
		while ((child = iter.next()) != NULL)
			if ((focus = child->acceptFocus(dir)) != NULL)
				return focus;

		if (this->acceptFocus())
			return this;
	}

	return NULL;
}

SkView* SkView::moveFocus(FocusDirection dir)
{
	SkView* focus = this->getFocusView();

	if (focus == NULL)
	{	// start with the root
		focus = this;
		while (focus->fParent)
			focus = focus->fParent;
	}

	SkView*	child, *parent;

	if (dir == kNext_FocusDirection)
	{
		parent = focus;
		child = focus->fFirstChild;
		if (child)
			goto FIRST_CHILD;
		else
			goto NEXT_SIB;

		do {
			while (child != parent->fFirstChild)
			{
	FIRST_CHILD:
				if ((focus = child->acceptFocus(dir)) != NULL)
					return focus;
				child = child->fNextSibling;
			}
	NEXT_SIB:
			child = parent->fNextSibling;
			parent = parent->fParent;
		} while (parent != NULL);
	}
	else	// prevfocus
	{
		parent = focus->fParent;
		if (parent == NULL)	// we're the root
			return focus->acceptFocus(dir);
		else
		{
			child = focus;
			while (parent)
			{
				while (child != parent->fFirstChild)
				{
					child = child->fPrevSibling;
					if ((focus = child->acceptFocus(dir)) != NULL)
						return focus;
				}
				if (parent->acceptFocus())
					return parent;

				child = parent;
				parent = parent->fParent;
			}
		}
	}
	return NULL;
}

void SkView::onFocusChange(bool gainFocusP)
{
	this->inval(NULL);
}

////////////////////////////////////////////////////////////////////////////

SkView::Click::Click(SkView* target)
{
    SkASSERT(target);
    fTargetID = target->getSinkID();
    fType = NULL;
    fWeOwnTheType = false;
    fOwner = NULL;
}

SkView::Click::~Click()
{
	this->resetType();
}

void SkView::Click::resetType()
{
	if (fWeOwnTheType)
	{
		sk_free(fType);
		fWeOwnTheType = false;
	}
	fType = NULL;
}

bool SkView::Click::isType(const char type[]) const
{
	const char* t = fType;

	if (type == t)
		return true;

	if (type == NULL)
		type = "";
	if (t == NULL)
		t = "";
	return !strcmp(t, type);
}

void SkView::Click::setType(const char type[])
{
	this->resetType();
	fType = (char*)type;
}

void SkView::Click::copyType(const char type[])
{
	if (fType != type)
	{
		this->resetType();
		if (type)
		{
			size_t	len = strlen(type) + 1;
			fType = (char*)sk_malloc_throw(len);
			memcpy(fType, type, len);
			fWeOwnTheType = true;
		}
	}
}

SkView::Click* SkView::findClickHandler(SkScalar x, SkScalar y)
{
	if (x < 0 || y < 0 || x >= fWidth || y >= fHeight) {
		return NULL;
    }

    if (this->onSendClickToChildren(x, y)) {
        F2BIter	iter(this);
        SkView*	child;

        while ((child = iter.next()) != NULL)
        {
            Click* click = child->findClickHandler(x - child->fLoc.fX,
                                                   y - child->fLoc.fY);
            if (click) {
                return click;
            }
        }
    }
	return this->onFindClickHandler(x, y);
}

void SkView::DoClickDown(Click* click, int x, int y)
{
	SkASSERT(click);

	SkView* target = (SkView*)SkEventSink::FindSink(click->fTargetID);
	if (target == NULL)
		return;

	click->fIOrig.set(x, y);
	click->fICurr = click->fIPrev = click->fIOrig;

	click->fOrig.iset(x, y);
	target->globalToLocal(&click->fOrig);
	click->fPrev = click->fCurr = click->fOrig;

	click->fState = Click::kDown_State;
	target->onClick(click);
}

void SkView::DoClickMoved(Click* click, int x, int y)
{
	SkASSERT(click);

	SkView* target = (SkView*)SkEventSink::FindSink(click->fTargetID);
	if (target == NULL)
		return;

	click->fIPrev = click->fICurr;
	click->fICurr.set(x, y);

	click->fPrev = click->fCurr;
	click->fCurr.iset(x, y);
	target->globalToLocal(&click->fCurr);

	click->fState = Click::kMoved_State;
	target->onClick(click);
}

void SkView::DoClickUp(Click* click, int x, int y)
{
	SkASSERT(click);

	SkView* target = (SkView*)SkEventSink::FindSink(click->fTargetID);
	if (target == NULL)
		return;

	click->fIPrev = click->fICurr;
	click->fICurr.set(x, y);

	click->fPrev = click->fCurr;
	click->fCurr.iset(x, y);
	target->globalToLocal(&click->fCurr);

	click->fState = Click::kUp_State;
	target->onClick(click);
}

//////////////////////////////////////////////////////////////////////

void SkView::invokeLayout() {
	SkView::Layout* layout = this->getLayout();

	if (layout) {
		layout->layoutChildren(this);
    }
}

void SkView::onDraw(SkCanvas* canvas) {
	Artist* artist = this->getArtist();

	if (artist) {
		artist->draw(this, canvas);
    }
}

void SkView::onSizeChange() {}

bool SkView::onSendClickToChildren(SkScalar x, SkScalar y) {
    return true;
}

SkView::Click* SkView::onFindClickHandler(SkScalar x, SkScalar y) {
	return NULL;
}

bool SkView::onClick(Click*) {
	return false;
}

bool SkView::handleInval(const SkRect*) {
	return false;
}

//////////////////////////////////////////////////////////////////////

void SkView::getLocalBounds(SkRect* bounds) const
{
	if (bounds)
		bounds->set(0, 0, fWidth, fHeight);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void SkView::detachFromParent_NoLayout()
{
	if (fParent == NULL)
		return;

	if (fContainsFocus)
		(void)this->setFocusView(NULL);

	this->inval(NULL);

	SkView*	next = NULL;

	if (fNextSibling != this)	// do we have any siblings
	{
		fNextSibling->fPrevSibling = fPrevSibling;
		fPrevSibling->fNextSibling = fNextSibling;
		next = fNextSibling;
	}

	if (fParent->fFirstChild == this)
		fParent->fFirstChild = next;

	fParent = fNextSibling = fPrevSibling = NULL;

	this->unref();
}

void SkView::detachFromParent()
{
	SkView* parent = fParent;

	if (parent)
	{
		this->detachFromParent_NoLayout();
		parent->invokeLayout();
	}
}

SkView* SkView::attachChildToBack(SkView* child)
{
	SkASSERT(child != this);

	if (child == NULL || fFirstChild == child)
		goto DONE;

	child->ref();
	child->detachFromParent_NoLayout();

	if (fFirstChild == NULL)
	{
		child->fNextSibling = child;
		child->fPrevSibling = child;
	}
	else
	{
		child->fNextSibling = fFirstChild;
		child->fPrevSibling = fFirstChild->fPrevSibling;
		fFirstChild->fPrevSibling->fNextSibling = child;
		fFirstChild->fPrevSibling = child;
	}

	fFirstChild = child;
	child->fParent = this;
	child->inval(NULL);

	this->invokeLayout();
DONE:
	return child;
}

SkView* SkView::attachChildToFront(SkView* child)
{
	SkASSERT(child != this);

	if (child == NULL || (fFirstChild && fFirstChild->fPrevSibling == child))
		goto DONE;

	child->ref();
	child->detachFromParent_NoLayout();

	if (fFirstChild == NULL)
	{
		fFirstChild = child;
		child->fNextSibling = child;
		child->fPrevSibling = child;
	}
	else
	{
		child->fNextSibling = fFirstChild;
		child->fPrevSibling = fFirstChild->fPrevSibling;
		fFirstChild->fPrevSibling->fNextSibling = child;
		fFirstChild->fPrevSibling = child;
	}

	child->fParent = this;
	child->inval(NULL);

	this->invokeLayout();
DONE:
	return child;
}

void SkView::detachAllChildren()
{
	while (fFirstChild)
		fFirstChild->detachFromParent_NoLayout();
}

void SkView::globalToLocal(SkScalar x, SkScalar y, SkPoint* local) const
{
	SkASSERT(this);

	if (local)
	{
		const SkView* view = this;
		while (view)
		{
			x -= view->fLoc.fX;
			y -= view->fLoc.fY;
			view = view->fParent;
		}
		local->set(x, y);
	}
}

//////////////////////////////////////////////////////////////////

/*	Even if the subclass overrides onInflate, they should always be
	sure to call the inherited method, so that we get called.
*/
void SkView::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
	SkScalar x, y;

	x = this->locX();
	y = this->locY();
	(void)dom.findScalar(node, "x", &x);
	(void)dom.findScalar(node, "y", &y);
	this->setLoc(x, y);

	x = this->width();
	y = this->height();
	(void)dom.findScalar(node, "width", &x);
	(void)dom.findScalar(node, "height", &y);
	this->setSize(x, y);

	// inflate the flags

	static const char* gFlagNames[] = {
		"visible", "enabled", "focusable", "flexH", "flexV"
	};
	SkASSERT(SK_ARRAY_COUNT(gFlagNames) == kFlagShiftCount);

	bool     b;
	uint32_t flags = this->getFlags();
	for (unsigned i = 0; i < SK_ARRAY_COUNT(gFlagNames); i++)
		if (dom.findBool(node, gFlagNames[i], &b))
			flags = SkSetClearShift(flags, b, i);
	this->setFlags(flags);
}

void SkView::inflate(const SkDOM& dom, const SkDOM::Node* node)
{
	this->onInflate(dom, node);
}

void SkView::onPostInflate(const SkTDict<SkView*>&)
{
	// override in subclass as needed
}

void SkView::postInflate(const SkTDict<SkView*>& dict)
{
	this->onPostInflate(dict);

	B2FIter	iter(this);
	SkView*	child;
	while ((child = iter.next()) != NULL)
		child->postInflate(dict);
}

//////////////////////////////////////////////////////////////////

SkView* SkView::sendEventToParents(const SkEvent& evt)
{
	SkView* parent = fParent;
    
	while (parent)
	{
		if (parent->doEvent(evt))
			return parent;
		parent = parent->fParent;
	}
	return NULL;
}

SkView* SkView::sendQueryToParents(SkEvent* evt) {
	SkView* parent = fParent;
    
	while (parent) {
		if (parent->doQuery(evt)) {
			return parent;
        }
		parent = parent->fParent;
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

SkView::F2BIter::F2BIter(const SkView* parent)
{
	fFirstChild = parent ? parent->fFirstChild : NULL;
	fChild = fFirstChild ? fFirstChild->fPrevSibling : NULL;
}

SkView*	SkView::F2BIter::next()
{
	SkView* curr = fChild;

	if (fChild)
	{
		if (fChild == fFirstChild)
			fChild = NULL;
		else
			fChild = fChild->fPrevSibling;
	}
	return curr;
}

SkView::B2FIter::B2FIter(const SkView* parent)
{
	fFirstChild = parent ? parent->fFirstChild : NULL;
	fChild = fFirstChild;
}

SkView*	SkView::B2FIter::next()
{
	SkView* curr = fChild;

	if (fChild)
	{
		SkView* next = fChild->fNextSibling;
		if (next == fFirstChild)
			next = NULL;
		fChild = next;
	}
	return curr;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

static inline void show_if_nonzero(const char name[], SkScalar value)
{
	if (value)
		SkDebugf("%s=\"%g\"", name, value/65536.);
}

static void tab(int level)
{
	for (int i = 0; i < level; i++)
		SkDebugf("    ");
}

static void dumpview(const SkView* view, int level, bool recurse)
{
	tab(level);

	SkDebugf("<view");
	show_if_nonzero(" x", view->locX());
	show_if_nonzero(" y", view->locY());
	show_if_nonzero(" width", view->width());
	show_if_nonzero(" height", view->height());

	if (recurse)
	{
		SkView::B2FIter	iter(view);
		SkView*			child;
		bool			noChildren = true;

		while ((child = iter.next()) != NULL)
		{
			if (noChildren)
				SkDebugf(">\n");
			noChildren = false;
			dumpview(child, level + 1, true);
		}

		if (!noChildren)
		{
			tab(level);
			SkDebugf("</view>\n");
		}
		else
			goto ONELINER;
	}
	else
	{
	ONELINER:
		SkDebugf(" />\n");
	}
}

void SkView::dump(bool recurse) const
{
	dumpview(this, 0, recurse);
}

#endif
