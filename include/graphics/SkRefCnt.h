#ifndef SkRefCnt_DEFINED
#define SkRefCnt_DEFINED

#include "SkTypes.h"

/**	\class SkRefCnt

	SkRefCnt is the base class for objects that may be shared by multiple objects.
	When a new owner wants a reference, it calls ref(). When an owner wants to release
	its reference, it calls unref(). When the shared object's reference count goes to
	zero as the result of an unref() call, its (virtual) destructor is called. It is
	an error for the destructor to be called explicitly (or via the object going out
	of scope on the stack or calling delete) if getRefCnt() > 1.
*/
class SkRefCnt {
public:
	/**	Default construct, initializing the reference count to 1.
	*/
			SkRefCnt() : fRefCnt(1) {}
	/**	 Destruct, asserting that the reference count is 1.
	*/
	virtual	~SkRefCnt() { SkASSERT(fRefCnt == 1); }

	/**	Return the reference count.
	*/
	int		getRefCnt() const { return fRefCnt; }
	/**	Increment the reference count. Must be balanced by a call to unref().
	*/
	void	ref() const { SkASSERT(fRefCnt > 0); ++fRefCnt; }
	/**	Decrement the reference count. If the reference count is 1 before the
		decrement, then call delete on the object. Note that if this is the case,
		then the object needs to have been allocated via new, and not on the stack.
	*/
	void	unref() const
	{
		SkASSERT(fRefCnt > 0);
		if (fRefCnt == 1)
			delete this;
		else
			--fRefCnt;
	}

	/**	Helper version of ref(), that first checks to see if this is not nil.
		If this is nil, then do nothing.
	*/
	void safeRef() const { if (this) this->ref(); }
	/**	Helper version of unref(), that first checks to see if this is not nil.
		If this is nil, then do nothing.
	*/
	void safeUnref() const { if (this) this->unref(); }

private:
	mutable int	fRefCnt;
};

/**	\class SkAutoUnref

	SkAutoUnref is a stack-helper class that will automatically call unref() on
	the object it points to when the SkAutoUnref object goes out of scope.
*/
class SkAutoUnref {
public:
	SkAutoUnref(SkRefCnt* obj) : fObj(obj) {}
	~SkAutoUnref();

	SkRefCnt*	get() const { return fObj; }
	bool		ref();
	bool		unref();
	SkRefCnt*	detach();

private:
	SkRefCnt*	fObj;
};

/**	Helper macro to safely assign one SkRefCnt* to another, checking for
	nil in on each side of the assignment, and ensuring that ref() is called
	before unref(), in case the two pointers point to the same object.
*/
#define SkRefCnt_SafeAssign(dst, src)	\
	do {								\
		if (src) src->ref();			\
		if (dst) dst->unref();			\
		dst = src;						\
	} while (0)

#endif

