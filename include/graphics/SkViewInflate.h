#ifndef SkViewInflate_DEFINED
#define SkViewInflate_DEFINED

#include "SkDOM.h"
#include "SkTDict.h"
#include "SkEvent.h"

class SkView;

class SkViewInflate {
public: 
			SkViewInflate();
	virtual	~SkViewInflate();

	/**	Return the tree of inflated views. If root is nil, create the root element
		as a view, otherwise assume root is that view, and just "inflate" it.

		Returns nil if the tree cannot be built.
	*/
	SkView*	inflate(const SkDOM& dom, const SkDOM::Node* node, SkView* root = nil);
	SkView*	inflate(const char xml[], size_t len, SkView* root = nil);

	/**	Given an id attribute value, return the corresponding view, or nil
		if no match is found.
	*/
	SkView*	findViewByID(const char id[]) const;
	
	SkDEBUGCODE(void dump() const;)

protected:
	/*	Override this in your subclass to handle instantiating views
		Call the inherited version for nodes you don't recognize.

		Do not call "inflate" on the view, just return it. This will
		get called automatically after createView returns.
	*/
	virtual SkView* createView(const SkDOM& dom, const SkDOM::Node* node);
	/**	Base implementation calls view->inflate(dom, node). Subclasses may override this
		to perform additional initializations to view, either before or after calling
		the inherited version.
	*/
	virtual void inflateView(SkView* view, const SkDOM& dom, const SkDOM::Node* node);

private:
	enum {
		kMinIDStrAlloc = 64
	};
	SkTDict<SkView*> fIDs;

	struct IDStr {
		SkView*	fView;
		char*	fStr;
	};
	SkTDArray<IDStr>	fListenTo, fBroadcastTo;
	SkChunkAlloc		fStrings;

	void addIDStr(SkTDArray<IDStr>* list, SkView*, const char* str);

	void	rInflate(const SkDOM& dom, const SkDOM::Node* node, SkView* parent);
};

#endif

