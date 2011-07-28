
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkWidget.h"
#include "SkCanvas.h"
#include "SkEvent.h"
#include "SkKey.h"
#include "SkParsePaint.h"
#include "SkSystemEventTypes.h"

#if 0

SkEvent* SkListSource::getEvent(int index)
{
	return NULL;
}

#include "SkOSFile.h"

class SkDirListSource : public SkListSource {
public:
	SkDirListSource(const char path[], const char suffix[], const char target[])
		: fPath(path), fSuffix(suffix), fTarget(target)
	{
		fCount = -1;
	}
	virtual int	countRows()
	{
		if (fCount < 0)
		{
			fCount = 0;
			fIter.reset(fPath.c_str(), fSuffix.c_str());
			while (fIter.next(NULL))
				fCount += 1;
			fIter.reset(fPath.c_str(), fSuffix.c_str());
			fIndex = 0;
		}
		return fCount;
	}
	virtual void getRow(int index, SkString* left, SkString* right)
	{
		(void)this->countRows();
		SkASSERT((unsigned)index < (unsigned)fCount);

		if (fIndex > index)
		{
			fIter.reset(fPath.c_str(), fSuffix.c_str());
			fIndex = 0;
		}

		while (fIndex < index)
		{
			fIter.next(NULL);
			fIndex += 1;
		}

		if (fIter.next(left))
		{
			if (left)
				left->remove(left->size() - fSuffix.size(), fSuffix.size());
		}
		else
		{
			if (left)
				left->reset();
		}
		if (right)	// only set to ">" if we know we're on a sub-directory
			right->reset();

		fIndex += 1;
	}
	virtual SkEvent* getEvent(int index)
	{
		SkASSERT((unsigned)index < (unsigned)fCount);

		SkEvent*	evt = new SkEvent();
		SkString	label;

		this->getRow(index, &label, NULL);
		evt->setString("name", label.c_str());

		int c = fPath.c_str()[fPath.size() - 1];
		if (c != '/' && c != '\\')
			label.prepend("/");
		label.prepend(fPath);
		label.append(fSuffix);
		evt->setString("path", label.c_str());
		evt->setS32("index", index);
		evt->setS32("duration", 22);
		evt->setType(fTarget);
		return evt;
	}

private:
	SkString		fPath, fSuffix;
	SkString		fTarget;
	SkOSFile::Iter	fIter;
	int				fCount;
	int				fIndex;
};

SkListSource* SkListSource::CreateFromDir(const char path[], const char suffix[], const char target[])
{
	return new SkDirListSource(path, suffix, target);
}

//////////////////////////////////////////////////////////////////

class SkDOMListSource : public SkListSource {
public:
	enum Type {
		kUnknown_Type,
		kDir_Type,
		kToggle_Type
	};
	struct ItemRec {
		SkString	fLabel;
		SkString	fTail, fAltTail;
		SkString	fTarget;
		Type		fType;
	};

	SkDOMListSource(const SkDOM& dom, const SkDOM::Node* node) : fDirTail(">")
	{
		const SkDOM::Node* child = dom.getFirstChild(node, "item");
		int	count = 0;

		while (child)
		{
			count += 1;
			child = dom.getNextSibling(child, "item");
		}

		fCount = count;
		fList = NULL;
		if (count)
		{
			ItemRec* rec = fList = new ItemRec[count];

			child = dom.getFirstChild(node, "item");
			while (child)
			{
				rec->fLabel.set(dom.findAttr(child, "label"));
				rec->fTail.set(dom.findAttr(child, "tail"));
				rec->fAltTail.set(dom.findAttr(child, "alt-tail"));
				rec->fTarget.set(dom.findAttr(child, "target"));
				rec->fType = kUnknown_Type;

				int	index = dom.findList(child, "type", "dir,toggle");
				if (index >= 0)
					rec->fType = (Type)(index + 1);

				child = dom.getNextSibling(child, "item");
				rec += 1;
			}
		}
	}
	virtual ~SkDOMListSource()
	{
		delete[] fList;
	}
	virtual int	countRows()
	{
		return fCount;
	}
	virtual void getRow(int index, SkString* left, SkString* right)
	{
		SkASSERT((unsigned)index < (unsigned)fCount);

		if (left)
			*left = fList[index].fLabel;
		if (right)
			*right = fList[index].fType == kDir_Type ? fDirTail : fList[index].fTail;
	}
	virtual SkEvent* getEvent(int index)
	{
		SkASSERT((unsigned)index < (unsigned)fCount);

		if (fList[index].fType == kDir_Type)
		{
			SkEvent* evt = new SkEvent();
			evt->setType(fList[index].fTarget);
			evt->setFast32(index);
			return evt;
		}
		if (fList[index].fType == kToggle_Type)
			fList[index].fTail.swap(fList[index].fAltTail);

		return NULL;
	}

private:
	int			fCount;
	ItemRec*	fList;
	SkString	fDirTail;
};

SkListSource* SkListSource::CreateFromDOM(const SkDOM& dom, const SkDOM::Node* node)
{
	return new SkDOMListSource(dom, node);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

SkListView::SkListView(U32 flags) : SkWidgetView(flags)
{
	fSource = NULL;
	fScrollIndex = 0;
	fCurrIndex = -1;
	fRowHeight = SkIntToScalar(16);
	fVisibleRowCount = 0;
	fStrCache = NULL;

	fPaint[kBG_Attr].setColor(0);
	fPaint[kNormalText_Attr].setTextSize(SkIntToScalar(14));
	fPaint[kHiliteText_Attr].setTextSize(SkIntToScalar(14));
	fPaint[kHiliteText_Attr].setColor(SK_ColorWHITE);
	fPaint[kHiliteCell_Attr].setColor(SK_ColorBLUE);
}

SkListView::~SkListView()
{
	delete[] fStrCache;
	fSource->safeUnref();
}

void SkListView::setRowHeight(SkScalar height)
{
	SkASSERT(height >= 0);

	if (fRowHeight != height)
	{
		fRowHeight = height;
		this->inval(NULL);
		this->onSizeChange();
	}
}

void SkListView::setSelection(int index)
{
	if (fCurrIndex != index)
	{
		this->invalSelection();
		fCurrIndex = index;
		this->invalSelection();
		this->ensureSelectionIsVisible();

		{
			SkEvent	evt;
			evt.setType("listview-selection");
			evt.setFast32(index);
			this->sendEventToParents(evt);
		}
	}
}

void SkListView::moveSelectionUp()
{
	if (fSource)
	{
		int	index = fCurrIndex;
		if (index < 0)	// no selection
			index = fSource->countRows() - 1;
		else
			index = SkMax32(index - 1, 0);
		this->setSelection(index);
	}
}

void SkListView::moveSelectionDown()
{
	if (fSource)
	{
		int	index = fCurrIndex;
		if (index < 0)	// no selection
			index = 0;
		else
			index = SkMin32(index + 1, fSource->countRows() - 1);
		this->setSelection(index);
	}
}

void SkListView::invalSelection()
{
	SkRect	r;
	if (this->getRowRect(fCurrIndex, &r))
		this->inval(&r);
}

void SkListView::ensureSelectionIsVisible()
{
	if (fSource == NULL)
		return;

	if ((unsigned)fCurrIndex < (unsigned)fSource->countRows())
	{
		int index = this->logicalToVisualIndex(fCurrIndex);

		if ((unsigned)index >= (unsigned)fVisibleRowCount)	// need to scroll
		{
			if (index < 0)	// too high
				fScrollIndex = fCurrIndex;
			else
				fScrollIndex = fCurrIndex - fVisibleRowCount + 1;
			SkASSERT((unsigned)fScrollIndex < (unsigned)fSource->countRows());

			this->dirtyStrCache();
			this->inval(NULL);
		}
	}
}

bool SkListView::getRowRect(int index, SkRect* r) const
{
	SkASSERT(r);
	index = this->logicalToVisualIndex(index);
	if (index >= 0)
	{
		SkScalar top = index * fRowHeight;

		if (top < this->height())
		{
			if (r)
				r->set(0, top, this->width(), top + fRowHeight);
			return true;
		}
	}
	return false;
}

SkPaint& SkListView::paint(Attr attr)
{
	SkASSERT((unsigned)attr < kAttrCount);
	return fPaint[attr];
}

SkListSource* SkListView::setListSource(SkListSource* src)
{
	if (fSource != src)
	{
		SkRefCnt_SafeAssign(fSource, src);
		this->dirtyStrCache();
		this->ensureSelectionIsVisible();
		this->inval(NULL);
	}
	return src;
}

void SkListView::onDraw(SkCanvas* canvas)
{
	this->INHERITED::onDraw(canvas);

	canvas->drawPaint(fPaint[kBG_Attr]);

	int	visibleCount = SkMin32(fVisibleRowCount, fSource->countRows() - fScrollIndex);
	if (visibleCount == 0)
		return;

	this->ensureStrCache(visibleCount);
	int currIndex = this->logicalToVisualIndex(fCurrIndex);

	if ((unsigned)currIndex < (unsigned)visibleCount)
	{
		SkAutoCanvasRestore	restore(canvas, true);
		SkRect	r;

		canvas->translate(0, currIndex * fRowHeight);
		(void)this->getRowRect(fScrollIndex, &r);
		canvas->drawRect(r, fPaint[kHiliteCell_Attr]);
	}

	SkPaint*	p;
	SkScalar	y, x = SkIntToScalar(6);
	SkScalar	rite = this->width() - x;

	{
		SkScalar ascent, descent;
		fPaint[kNormalText_Attr].measureText(0, NULL, &ascent, &descent);
		y = SkScalarHalf(fRowHeight - descent + ascent) - ascent;
	}

	for (int i = 0; i < visibleCount; i++)
	{
		if (i == currIndex)
			p = &fPaint[kHiliteText_Attr];
		else
			p = &fPaint[kNormalText_Attr];

		p->setTextAlign(SkPaint::kLeft_Align);
		canvas->drawText(fStrCache[i].c_str(), fStrCache[i].size(), x, y, *p);
		p->setTextAlign(SkPaint::kRight_Align);
		canvas->drawText(fStrCache[i + visibleCount].c_str(), fStrCache[i + visibleCount].size(), rite, y, *p);
		canvas->translate(0, fRowHeight);
	}
}

void SkListView::onSizeChange()
{
	SkScalar count = SkScalarDiv(this->height(), fRowHeight);
	int		 n = SkScalarFloor(count);

	// only want to show rows that are mostly visible
	if (n == 0 || count - SkIntToScalar(n) > SK_Scalar1*75/100)
		n += 1;

	if (fVisibleRowCount != n)
	{
		fVisibleRowCount = n;
		this->ensureSelectionIsVisible();
		this->dirtyStrCache();
	}
}

void SkListView::dirtyStrCache()
{
	if (fStrCache)
	{
		delete[] fStrCache;
		fStrCache = NULL;
	}
}

void SkListView::ensureStrCache(int count)
{
	if (fStrCache == NULL)
	{
		fStrCache = new SkString[count << 1];

		if (fSource)
			for (int i = 0; i < count; i++)
				fSource->getRow(i + fScrollIndex, &fStrCache[i], &fStrCache[i + count]);
	}
}

bool SkListView::onEvent(const SkEvent& evt)
{
	if (evt.isType(SK_EventType_Key))
	{
		switch (evt.getFast32()) {
		case kUp_SkKey:
			this->moveSelectionUp();
			return true;
		case kDown_SkKey:
			this->moveSelectionDown();
			return true;
		case kRight_SkKey:
		case kOK_SkKey:
			if (fSource && fCurrIndex >= 0)
			{
				SkEvent* evt = fSource->getEvent(fCurrIndex);
				if (evt)
				{
					SkView* view = this->sendEventToParents(*evt);
					delete evt;
					return view != NULL;
				}
				else	// hack to make toggle work
				{
					this->dirtyStrCache();
					this->inval(NULL);
				}
			}
			break;
		}
	}
	return this->INHERITED::onEvent(evt);
}

void SkListView::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
	this->INHERITED::onInflate(dom, node);

	SkScalar			x;
	const SkDOM::Node*	child;

	if (dom.findScalar(node, "row-height", &x))
		this->setRowHeight(x);

	if ((child = dom.getFirstChild(node, "hilite-paint")) != NULL)
		SkPaint_Inflate(&this->paint(kHiliteCell_Attr), dom, child);

	// look for a listsource
	{
		SkListSource* src = NULL;

		if ((child = dom.getFirstChild(node, "file-listsource")) != NULL)
		{
			const char* path = dom.findAttr(child, "path");
			if (path)
				src = SkListSource::CreateFromDir(	path,
													dom.findAttr(child, "filter"),
													dom.findAttr(child, "target"));
		}
		else if ((child = dom.getFirstChild(node, "xml-listsource")) != NULL)
		{
			src = SkListSource::CreateFromDOM(dom, child);
		}

		if (src)
		{
			this->setListSource(src)->unref();
			this->setSelection(0);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

#include "SkImageDecoder.h"
#include "SkShader.h"

class SkScrollBarView : public SkView {
public:
	SkScrollBarView(const char bg[], const char fg[])
	{
		fBGRef = SkBitmapRef::Decode(bg, true);
		fFGRef = SkBitmapRef::Decode(fg, true);

		if (fBGRef)
			this->setWidth(SkIntToScalar(fBGRef->bitmap().width()));
	}
	~SkScrollBarView()
	{
		delete fBGRef;
		delete fFGRef;
	}
protected:
	virtual void onDraw(SkCanvas* canvas)
	{
		if (fBGRef == NULL) return;

		SkPaint	paint;

		SkShader* shader = SkShader::CreateBitmapShader(fBGRef->bitmap(), false, SkPaint::kNo_FilterType, SkShader::kClamp_TileMode);
		paint.setShader(shader)->unref();

		canvas->drawPaint(paint);
	}
private:
	SkBitmapRef*	fBGRef, *fFGRef;
};

SkGridView::SkGridView(U32 flags) : SkWidgetView(flags)
{
	fSource = NULL;
	fCurrIndex = -1;
	fVisibleCount.set(0, 0);

	fPaint[kBG_Attr].setColor(SK_ColorWHITE);
	fPaint[kHiliteCell_Attr].setColor(SK_ColorYELLOW);
	fPaint[kHiliteCell_Attr].setStyle(SkPaint::kStroke_Style);
	fPaint[kHiliteCell_Attr].setAntiAliasOn(true);
	fPaint[kHiliteCell_Attr].setStrokeWidth(SK_Scalar1*3);

	fScrollBar = new SkScrollBarView("icons/scrollbarGrey.jpg", "icons/scrollbarBlue.jpg");
	this->attachChildToFront(fScrollBar)->unref();
	fScrollBar->setVisibleP(true);
}

SkGridView::~SkGridView()
{
	fSource->safeUnref();
}

void SkGridView::getCellSize(SkPoint* size) const
{
	if (size)
		*size = fCellSize;
}

void SkGridView::setCellSize(SkScalar x, SkScalar y)
{
	SkASSERT(x >= 0 && y >= 0);

	if (!fCellSize.equals(x, y))
	{
		fCellSize.set(x, y);
		this->inval(NULL);
	}
}

void SkGridView::setSelection(int index)
{
	if (fCurrIndex != index)
	{
		this->invalSelection();
		fCurrIndex = index;
		this->invalSelection();
		this->ensureSelectionIsVisible();

		// this generates the click
		{
			SkEvent	evt;
			evt.setType("listview-selection");
			evt.setFast32(index);
			this->sendEventToParents(evt);
		}
	}
}

void SkGridView::moveSelectionUp()
{
	if (fSource)
	{
		int	index = fCurrIndex;
		if (index < 0)	// no selection
			index = fSource->countRows() - 1;
		else
			index = SkMax32(index - 1, 0);
		this->setSelection(index);
	}
}

void SkGridView::moveSelectionDown()
{
	if (fSource)
	{
		int	index = fCurrIndex;
		if (index < 0)	// no selection
			index = 0;
		else
			index = SkMin32(index + 1, fSource->countRows() - 1);
		this->setSelection(index);
	}
}

void SkGridView::invalSelection()
{
	SkRect	r;
	if (this->getCellRect(fCurrIndex, &r))
	{
		SkScalar inset = 0;
		if (fPaint[kHiliteCell_Attr].getStyle() != SkPaint::kFill_Style)
			inset += fPaint[kHiliteCell_Attr].getStrokeWidth() / 2;
		if (fPaint[kHiliteCell_Attr].isAntiAliasOn())
			inset += SK_Scalar1;
		r.inset(-inset, -inset);
		this->inval(&r);
	}
}

void SkGridView::ensureSelectionIsVisible()
{
	if (fSource == NULL)
		return;
#if 0
	if ((unsigned)fCurrIndex < (unsigned)fSource->countRows())
	{
		int index = this->logicalToVisualIndex(fCurrIndex);

		if ((unsigned)index >= (unsigned)fVisibleRowCount)	// need to scroll
		{
			if (index < 0)	// too high
				fScrollIndex = fCurrIndex;
			else
				fScrollIndex = fCurrIndex - fVisibleRowCount + 1;
			SkASSERT((unsigned)fScrollIndex < (unsigned)fSource->countRows());

			this->dirtyStrCache();
			this->inval(NULL);
		}
	}
#endif
}

bool SkGridView::getCellRect(int index, SkRect* r) const
{
	if (fVisibleCount.fY == 0)
		return false;

	index = this->logicalToVisualIndex(index);
	if (index >= 0)
	{
		SkRect	bounds;
		int row = index / fVisibleCount.fY;
		int col = index % fVisibleCount.fY;

		bounds.set(0, 0, fCellSize.fX, fCellSize.fY);
		bounds.offset(col * (fCellSize.fX + SkIntToScalar(col > 0)),
					  row * (fCellSize.fY + SkIntToScalar(row > 0)));

		if (bounds.fTop < this->height())
		{
			if (r)
				*r = bounds;
			return true;
		}
	}
	return false;
}

SkPaint& SkGridView::paint(Attr attr)
{
	SkASSERT((unsigned)attr < kAttrCount);
	return fPaint[attr];
}

SkListSource* SkGridView::setListSource(SkListSource* src)
{
	if (fSource != src)
	{
		SkRefCnt_SafeAssign(fSource, src);
	//	this->dirtyStrCache();
		this->ensureSelectionIsVisible();
		this->inval(NULL);
	}
	return src;
}

#include "SkShader.h"

static void copybits(SkCanvas* canvas, const SkBitmap& bm, const SkRect& dst, const SkPaint& paint)
{
	SkRect		src;
	SkMatrix	matrix;

	src.set(0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height()));
	if (matrix.setRectToRect(src, dst))
	{
		SkPaint	  p(paint);
		SkShader* shader = SkShader::CreateBitmapShader(bm, false, SkPaint::kNo_FilterType, SkShader::kClamp_TileMode);
		p.setShader(shader)->unref();

		shader->setLocalMatrix(matrix);
		canvas->drawRect(dst, p);
	}
}

#include "SkImageDecoder.h"

void SkGridView::onDraw(SkCanvas* canvas)
{
	this->INHERITED::onDraw(canvas);

	canvas->drawPaint(fPaint[kBG_Attr]);

	if (fSource == NULL)
		return;

#if 0
	int	visibleCount = SkMin32(fVisibleRowCount, fSource->countRows() - fScrollIndex);
	if (visibleCount == 0)
		return;

	this->ensureStrCache(visibleCount);
	int currIndex = this->logicalToVisualIndex(fCurrIndex);
#endif

	SkPaint	p;
	for (int i = 0; i < fSource->countRows(); i++)
	{
		bool	 forced = false;
		SkEvent* evt = fSource->getEvent(i);
		SkASSERT(evt);
		SkString path(evt->findString("path"));
		delete evt;

		SkBitmapRef* bmr = SkBitmapRef::Decode(path.c_str(), false);
		if (bmr == NULL)
		{
			bmr = SkBitmapRef::Decode(path.c_str(), true);
			if (bmr)
				forced = true;
		}

		if (bmr)
		{
			SkAutoTDelete<SkBitmapRef>	autoRef(bmr);
			SkRect	r;
			if (!this->getCellRect(i, &r))
				break;
			copybits(canvas, bmr->bitmap(), r, p);
		}
		// only draw one forced bitmap at a time
		if (forced)
		{
			this->inval(NULL);	// could inval only the remaining visible cells...
			break;
		}
	}

	// draw the hilite
	{
		SkRect	r;
		if (fCurrIndex >= 0 && this->getCellRect(fCurrIndex, &r))
			canvas->drawRect(r, fPaint[kHiliteCell_Attr]);
	}
}

static int check_count(int n, SkScalar s)
{
	// only want to show cells that are mostly visible
	if (n == 0 || s - SkIntToScalar(n) > SK_Scalar1*75/100)
		n += 1;
	return n;
}

void SkGridView::onSizeChange()
{
	fScrollBar->setHeight(this->height());
	fScrollBar->setLoc(this->locX() + this->width() - fScrollBar->width(), 0);

	if (fCellSize.equals(0, 0))
	{
		fVisibleCount.set(0, 0);
		return;
	}

	SkScalar rows = SkScalarDiv(this->height(), fCellSize.fY);
	SkScalar cols = SkScalarDiv(this->width(), fCellSize.fX);
	int		 y = SkScalarFloor(rows);
	int		 x = SkScalarFloor(cols);

	y = check_count(y, rows);
	x = check_count(x, cols);

	if (!fVisibleCount.equals(x, y))
	{
		fVisibleCount.set(x, y);
		this->ensureSelectionIsVisible();
	//	this->dirtyStrCache();
	}
}

bool SkGridView::onEvent(const SkEvent& evt)
{
	if (evt.isType(SK_EventType_Key))
	{
		switch (evt.getFast32()) {
		case kUp_SkKey:
			this->moveSelectionUp();
			return true;
		case kDown_SkKey:
			this->moveSelectionDown();
			return true;
		case kRight_SkKey:
		case kOK_SkKey:
			if (fSource && fCurrIndex >= 0)
			{
				SkEvent* evt = fSource->getEvent(fCurrIndex);
				if (evt)
				{
					// augment the event with our local rect
					(void)this->getCellRect(fCurrIndex, (SkRect*)evt->setScalars("local-rect", 4, NULL));

					SkView* view = this->sendEventToParents(*evt);
					delete evt;
					return view != NULL;
				}
			}
			break;
		}
	}
	return this->INHERITED::onEvent(evt);
}

void SkGridView::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
	this->INHERITED::onInflate(dom, node);

	SkScalar			x[2];
	const SkDOM::Node*	child;

	if (dom.findScalars(node, "cell-size", x, 2))
		this->setCellSize(x[0], x[1]);

	if ((child = dom.getFirstChild(node, "hilite-paint")) != NULL)
		SkPaint_Inflate(&this->paint(kHiliteCell_Attr), dom, child);

	// look for a listsource
	{
		SkListSource* src = NULL;

		if ((child = dom.getFirstChild(node, "file-listsource")) != NULL)
		{
			const char* path = dom.findAttr(child, "path");
			if (path)
				src = SkListSource::CreateFromDir(	path,
													dom.findAttr(child, "filter"),
													dom.findAttr(child, "target"));
		}
		else if ((child = dom.getFirstChild(node, "xml-listsource")) != NULL)
		{
			src = SkListSource::CreateFromDOM(dom, child);
		}

		if (src)
		{
			this->setListSource(src)->unref();
			this->setSelection(0);
		}
	}
	this->onSizeChange();
}

#endif
