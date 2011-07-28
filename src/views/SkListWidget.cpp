
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkWidgetViews.h"

#include "SkAnimator.h"
#include "SkScrollBarView.h"

extern void init_skin_anim(const char name[], SkAnimator*);

struct SkListView::BindingRec {
	SkString	fSlotName;
	int			fFieldIndex;
};

SkListView::SkListView()
{
	fSource = NULL;				// our list-source
	fScrollBar = NULL;
	fAnims = NULL;				// array of animators[fVisibleRowCount]
	fBindings = NULL;			// our fields->slot array
	fBindingCount = 0;			// number of entries in fSlots array
	fScrollIndex = 0;			// number of cells to skip before first visible cell
	fCurrIndex = -1;			// index of "selected" cell
	fVisibleRowCount = 0;		// number of cells that can fit in our bounds
	fAnimContentDirty = true;	// true if fAnims[] have their correct content
	fAnimFocusDirty = true;

	fHeights[kNormal_Height] = SkIntToScalar(16);
	fHeights[kSelected_Height] = SkIntToScalar(16);
	
	this->setFlags(this->getFlags() | kFocusable_Mask);
}

SkListView::~SkListView()
{
	SkSafeUnref(fScrollBar);
	SkSafeUnref(fSource);
	delete[] fAnims;
	delete[] fBindings;
}

void SkListView::setHasScrollBar(bool hasSB)
{
	if (hasSB != this->hasScrollBar())
	{
		if (hasSB)
		{
			SkASSERT(fScrollBar == NULL);
			fScrollBar = (SkScrollBarView*)SkWidgetFactory(kScroll_WidgetEnum);
			fScrollBar->setVisibleP(true);
			this->attachChildToFront(fScrollBar);
			fScrollBar->setHeight(this->height());	// assume it auto-sets its width
		//	fScrollBar->setLoc(this->getContentWidth(), 0);
			fScrollBar->setLoc(this->width()-SkIntToScalar(10), 0);
		}
		else
		{
			SkASSERT(fScrollBar);
			fScrollBar->detachFromParent();
			fScrollBar->unref();
			fScrollBar = NULL;
		}
		this->dirtyCache(kAnimContent_DirtyFlag);
	}
}

void SkListView::setSelection(int index)
{
	if (fCurrIndex != index)
	{
		fAnimFocusDirty = true;
		this->inval(NULL);

		this->invalSelection();
		fCurrIndex = index;
		this->invalSelection();
		this->ensureSelectionIsVisible();
	}
}

bool SkListView::moveSelectionUp()
{
	if (fSource)
	{
		int	index = fCurrIndex;
		if (index < 0)	// no selection
			index = fSource->countRecords() - 1;
		else
			index = SkMax32(index - 1, 0);
		
		if (fCurrIndex != index)
		{
			this->setSelection(index);
			return true;
		}
	}
	return false;
}

bool SkListView::moveSelectionDown()
{
	if (fSource)
	{
		int	index = fCurrIndex;
		if (index < 0)	// no selection
			index = 0;
		else
			index = SkMin32(index + 1, fSource->countRecords() - 1);
		
		if (fCurrIndex != index)
		{
			this->setSelection(index);
			return true;
		}
	}
	return false;
}

void SkListView::invalSelection()
{
	SkRect	r;
	if (this->getRowRect(fCurrIndex, &r))
		this->inval(&r);
}

void SkListView::ensureSelectionIsVisible()
{
	if (fSource && (unsigned)fCurrIndex < (unsigned)fSource->countRecords())
	{
		int index = this->logicalToVisualIndex(fCurrIndex);

		if ((unsigned)index >= (unsigned)fVisibleRowCount)	// need to scroll
		{
			int newIndex;
			
			if (index < 0)	// too high
				newIndex = fCurrIndex;
			else
				newIndex = fCurrIndex - fVisibleRowCount + 1;
			SkASSERT((unsigned)newIndex < (unsigned)fSource->countRecords());
			this->inval(NULL);
			
			if (fScrollIndex != newIndex)
			{
				fScrollIndex = newIndex;
				if (fScrollBar)
					fScrollBar->setStart(newIndex);
				this->dirtyCache(kAnimContent_DirtyFlag);
			}
		}
	}
}

SkScalar SkListView::getContentWidth() const
{
	SkScalar width = this->width();
	
	if (fScrollBar)
	{
		width -= fScrollBar->width();
		if (width < 0)
			width = 0;
	}
	return width;
}

bool SkListView::getRowRect(int index, SkRect* r) const
{
	SkASSERT(r);

	index = this->logicalToVisualIndex(index);
	if (index >= 0)
	{
		int	selection = this->logicalToVisualIndex(fCurrIndex);
		
		SkScalar height = fHeights[index == selection ? kSelected_Height : kNormal_Height];
		SkScalar top = index * fHeights[kNormal_Height];

		if (index > selection && selection >= 0)
			top += fHeights[kSelected_Height] - fHeights[kNormal_Height];	

		if (top < this->height())
		{
			if (r)
				r->set(0, top, this->getContentWidth(), top + height);
			return true;
		}
	}
	return false;
}

SkListSource* SkListView::setListSource(SkListSource* src)
{
	if (fSource != src)
	{
		SkRefCnt_SafeAssign(fSource, src);
		this->ensureSelectionIsVisible();
		this->inval(NULL);
		
		if (fScrollBar)
			fScrollBar->setTotal(fSource->countRecords());
	}
	return src;
}

void SkListView::dirtyCache(unsigned dirtyFlags)
{
	if (dirtyFlags & kAnimCount_DirtyFlag)
	{
		delete fAnims;
		fAnims = NULL;
		fAnimContentDirty = true;
		fAnimFocusDirty = true;
	}
	if (dirtyFlags & kAnimContent_DirtyFlag)
	{
		if (!fAnimContentDirty)
		{
			this->inval(NULL);
			fAnimContentDirty = true;
		}
		fAnimFocusDirty = true;
	}
}

bool SkListView::ensureCache()
{
	if (fSkinName.size() == 0)
		return false;

	if (fAnims == NULL)
	{
		int n = SkMax32(1, fVisibleRowCount);

		SkASSERT(fAnimContentDirty);
		fAnims = new SkAnimator[n];
		for (int i = 0; i < n; i++)
		{
			fAnims[i].setHostEventSink(this);
			init_skin_anim(fSkinName.c_str(), &fAnims[i]);
		}
		
		fHeights[kNormal_Height] = fAnims[0].getScalar("idleHeight", "value");
		fHeights[kSelected_Height] = fAnims[0].getScalar("focusedHeight", "value");

		fAnimFocusDirty = true;
	}

	if (fAnimContentDirty && fSource)
	{
		fAnimContentDirty = false;

		SkString	str;
		SkEvent		evt("user");
		evt.setString("id", "setFields");
		evt.setS32("rowCount", fVisibleRowCount);
		
		SkEvent	dimEvt("user");
		dimEvt.setString("id", "setDim");
		dimEvt.setScalar("dimX", this->getContentWidth());
		dimEvt.setScalar("dimY", this->height());

		for (int i = fScrollIndex; i < fScrollIndex + fVisibleRowCount; i++)
		{
			evt.setS32("relativeIndex", i - fScrollIndex);
			for (int j = 0; j < fBindingCount; j++)
			{
				fSource->getRecord(i, fBindings[j].fFieldIndex, &str);
//SkDEBUGF(("getRecord(%d,%d,%s) slot(%s)\n", i, fBindings[j].fFieldIndex, str.c_str(), fBindings[j].fSlotName.c_str()));
				evt.setString(fBindings[j].fSlotName.c_str(), str.c_str());
			}
			(void)fAnims[i % fVisibleRowCount].doUserEvent(evt);
			(void)fAnims[i % fVisibleRowCount].doUserEvent(dimEvt);
		}
		fAnimFocusDirty = true;
	}

	if (fAnimFocusDirty)
	{
//SkDEBUGF(("service fAnimFocusDirty\n"));
		fAnimFocusDirty = false;

		SkEvent		focusEvt("user");
		focusEvt.setString("id", "setFocus");

		for (int i = fScrollIndex; i < fScrollIndex + fVisibleRowCount; i++)
		{
			focusEvt.setS32("FOCUS", i == fCurrIndex);
			(void)fAnims[i % fVisibleRowCount].doUserEvent(focusEvt);
		}
	}

	return true;
}

void SkListView::ensureVisibleRowCount()
{
	SkScalar	height = this->height();
	int			n = 0;
	
	if (height > 0)
	{
		n = 1;
		height -= fHeights[kSelected_Height];
		if (height > 0)
		{
			SkScalar count = SkScalarDiv(height, fHeights[kNormal_Height]);
			n += SkScalarFloor(count);
			if (count - SkIntToScalar(n) > SK_Scalar1*3/4)
				n += 1;
				
		//	SkDebugf("count %g, n %d\n", count/65536., n);
		}
	}

	if (fVisibleRowCount != n)
	{
		if (fScrollBar)
			fScrollBar->setShown(n);

		fVisibleRowCount = n;
		this->ensureSelectionIsVisible();
		this->dirtyCache(kAnimCount_DirtyFlag | kAnimContent_DirtyFlag);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////

#include "SkSystemEventTypes.h"
#include "SkTime.h"

void SkListView::onSizeChange()
{
	this->INHERITED::onSizeChange();

	if (fScrollBar)
		fScrollBar->setLoc(this->width()-SkIntToScalar(10), 0);

	this->ensureVisibleRowCount();
}

void SkListView::onDraw(SkCanvas* canvas)
{
	this->INHERITED::onDraw(canvas);

	this->ensureVisibleRowCount();

	int	visibleCount = SkMin32(fVisibleRowCount, fSource->countRecords() - fScrollIndex);
	if (visibleCount == 0 || !this->ensureCache())
		return;

//SkDebugf("visibleCount %d scrollIndex %d currIndex %d\n", visibleCount, fScrollIndex, fCurrIndex);

	SkAutoCanvasRestore	ar(canvas, true);
	SkMSec				now = SkTime::GetMSecs();
	SkRect				bounds;

	bounds.fLeft	= 0;
	bounds.fRight	= this->getContentWidth();
	bounds.fBottom	= 0;
	// assign bounds.fTop inside the loop

	// hack to reveal our bounds for debugging
	if (this->hasFocus())
		canvas->drawARGB(0x11, 0, 0, 0xFF);
	else
		canvas->drawARGB(0x11, 0x88, 0x88, 0x88);

	for (int i = fScrollIndex; i < fScrollIndex + visibleCount; i++)
	{
		SkPaint	 paint;
		SkScalar height = fHeights[i == fCurrIndex ? kSelected_Height : kNormal_Height];

		bounds.fTop = bounds.fBottom;
		bounds.fBottom += height;
		
		canvas->save();
		if (fAnims[i % fVisibleRowCount].draw(canvas, &paint, now) != SkAnimator::kNotDifferent)
			this->inval(&bounds);
		canvas->restore();

		canvas->translate(0, height);
	}
}

bool SkListView::onEvent(const SkEvent& evt)
{
	if (evt.isType(SK_EventType_Key))
	{
		switch (evt.getFast32()) {
		case kUp_SkKey:
			return this->moveSelectionUp();
		case kDown_SkKey:
			return this->moveSelectionDown();
		case kRight_SkKey:
		case kOK_SkKey:
			this->postWidgetEvent();
			return true;
		default:
			break;
		}
	}
	return this->INHERITED::onEvent(evt);
}

///////////////////////////////////////////////////////////////////////////////////////////////

static const char gListViewEventSlot[] = "sk-listview-slot-name";

/*virtual*/ bool SkListView::onPrepareWidgetEvent(SkEvent* evt)
{
	if (fSource && fCurrIndex >= 0 && this->INHERITED::onPrepareWidgetEvent(evt) &&
		fSource->prepareWidgetEvent(evt, fCurrIndex))
	{
		evt->setS32(gListViewEventSlot, fCurrIndex);
		return true;
	}
	return false;
}

int SkListView::GetWidgetEventListIndex(const SkEvent& evt)
{
	int32_t	index;

	return evt.findS32(gListViewEventSlot, &index) ? index : -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////

void SkListView::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
	this->INHERITED::onInflate(dom, node);
	
	{
		bool hasScrollBar;
		if (dom.findBool(node, "scrollBar", &hasScrollBar))
			this->setHasScrollBar(hasScrollBar);
	}

	const SkDOM::Node*	child;

	if ((child = dom.getFirstChild(node, "bindings")) != NULL)
	{
		delete[] fBindings;
		fBindings = NULL;
		fBindingCount = 0;

		SkListSource* listSrc = SkListSource::Factory(dom.findAttr(child, "data-fields"));
		SkASSERT(listSrc);
		fSkinName.set(dom.findAttr(child, "skin-slots"));
		SkASSERT(fSkinName.size());

		this->setListSource(listSrc)->unref();
			
		int count = dom.countChildren(child, "bind");
		if (count > 0)
		{
			fBindings = new BindingRec[count];
			count = 0;	// reuse this to count up to the number of valid bindings

			child = dom.getFirstChild(child, "bind");
			SkASSERT(child);
			do {
				const char* fieldName = dom.findAttr(child, "field");
				const char* slotName = dom.findAttr(child, "slot");
				if (fieldName && slotName)
				{
					fBindings[count].fFieldIndex = listSrc->findFieldIndex(fieldName);
					if (fBindings[count].fFieldIndex >= 0)
						fBindings[count++].fSlotName.set(slotName);
				}
			} while ((child = dom.getNextSibling(child, "bind")) != NULL);

			fBindingCount = SkToU16(count);
			if (count == 0)
			{
				SkDEBUGF(("SkListView::onInflate: no valid <bind> elements in <listsource>\n"));
				delete[] fBindings;
			}
		}
		this->dirtyCache(kAnimCount_DirtyFlag);
		this->setSelection(0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

class SkXMLListSource : public SkListSource {
public:
	SkXMLListSource(const char doc[], size_t len);
	virtual ~SkXMLListSource()
	{
		delete[] fFields;
		delete[] fRecords;
	}

	virtual int countFields() { return fFieldCount; }
	virtual void getFieldName(int index, SkString* field)
	{
		SkASSERT((unsigned)index < (unsigned)fFieldCount);
		if (field)
			*field = fFields[index];
	}
	virtual int findFieldIndex(const char field[])
	{
		for (int i = 0; i < fFieldCount; i++)
			if (fFields[i].equals(field))
				return i;
		return -1;
	}

	virtual int	countRecords() { return fRecordCount; }
	virtual void getRecord(int rowIndex, int fieldIndex, SkString* data)
	{
		SkASSERT((unsigned)rowIndex < (unsigned)fRecordCount);
		SkASSERT((unsigned)fieldIndex < (unsigned)fFieldCount);
		if (data)
			*data = fRecords[rowIndex * fFieldCount + fieldIndex];
	}

	virtual bool prepareWidgetEvent(SkEvent* evt, int rowIndex)
	{
		// hack, for testing right now. Need the xml to tell us what to jam in and where
		SkString	data;
		
		this->getRecord(rowIndex, 0, &data);
		evt->setString("xml-listsource", data.c_str());
		return true;
	}
	
private:
	SkString*	fFields;	// [fFieldCount]
	SkString*	fRecords;	// [fRecordCount][fFieldCount]
	int			fFieldCount, fRecordCount;
};

#include "SkDOM.h"

SkXMLListSource::SkXMLListSource(const char doc[], size_t len)
{
	fFieldCount = fRecordCount = 0;
	fFields = fRecords = NULL;

	SkDOM	dom;

	const SkDOM::Node* node = dom.build(doc, len);
	SkASSERT(node);
	const SkDOM::Node*	child;	

	child = dom.getFirstChild(node, "fields");
	if (child)
	{
		fFieldCount = dom.countChildren(child, "field");
		fFields = new SkString[fFieldCount];

		int n = 0;
		child = dom.getFirstChild(child, "field");
		while (child)
		{
			fFields[n].set(dom.findAttr(child, "name"));
			child = dom.getNextSibling(child, "field");
			n += 1;
		}
		SkASSERT(n == fFieldCount);
	}
	
	child = dom.getFirstChild(node, "records");
	if (child)
	{
		fRecordCount = dom.countChildren(child, "record");
		fRecords = new SkString[fRecordCount * fFieldCount];

		int n = 0;
		child = dom.getFirstChild(child, "record");
		while (child)
		{
			for (int i = 0; i < fFieldCount; i++)
				fRecords[n * fFieldCount + i].set(dom.findAttr(child, fFields[i].c_str()));
			child = dom.getNextSibling(child, "record");
			n += 1;
		}
		SkASSERT(n == fRecordCount);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

SkListSource* SkListSource::Factory(const char name[])
{
	static const char gDoc[] =
		"<db name='contacts.db'>"
			"<fields>"
				"<field name='name'/>"
				"<field name='work-num'/>"
				"<field name='home-num'/>"
				"<field name='type'/>"
			"</fields>"
			"<records>"
				"<record name='Andy McFadden' work-num='919 357-1234' home-num='919 123-4567' type='0'/>"
				"<record name='Brian Swetland' work-num='919 123-1234' home-num='929 123-4567' type='1' />"
				"<record name='Chris Desalvo' work-num='919 345-1234' home-num='949 123-4567' type='1' />"
				"<record name='Chris White' work-num='919 234-1234' home-num='939 123-4567' type='2' />"
				"<record name='Dan Bornstein' work-num='919 357-1234' home-num='919 123-4567' type='0' />"
				"<record name='Don Cung' work-num='919 123-1234' home-num='929 123-4567' type='2' />"
				"<record name='Eric Fischer' work-num='919 345-1234' home-num='949 123-4567' type='2' />"
				"<record name='Ficus Kirkpatric' work-num='919 234-1234' home-num='939 123-4567' type='1' />"
				"<record name='Jack Veenstra' work-num='919 234-1234' home-num='939 123-4567' type='2' />"
				"<record name='Jeff Yaksick' work-num='919 234-1234' home-num='939 123-4567' type='0' />"
				"<record name='Joe Onorato' work-num='919 234-1234' home-num='939 123-4567' type='0' />"
				"<record name='Mathias Agopian' work-num='919 234-1234' home-num='939 123-4567' type='1' />"
				"<record name='Mike Fleming' work-num='919 234-1234' home-num='939 123-4567' type='2' />"
				"<record name='Nick Sears' work-num='919 234-1234' home-num='939 123-4567' type='1' />"
				"<record name='Rich Miner' work-num='919 234-1234' home-num='939 123-4567' type='1' />"
				"<record name='Tracey Cole' work-num='919 234-1234' home-num='939 123-4567' type='0' />"
				"<record name='Wei Huang' work-num='919 234-1234' home-num='939 123-4567' type='0' />"
			"</records>"
		"</db>";
		
//SkDebugf("doc size %d\n", sizeof(gDoc)-1);
	return new SkXMLListSource(gDoc, sizeof(gDoc) - 1);
}



