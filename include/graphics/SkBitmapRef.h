#ifndef SkBitmapRef_DEFINED
#define SkBitmapRef_DEFINED 

#include "SkBitmap.h"

class SkStream;

/**	Helper class to manage a cache of decoded images from the file system
*/
class SkBitmapRef : public SkRefCnt {
public:
    /** Create a non-cached bitmap, trasfering ownership of pixels if needed
     */
    SkBitmapRef(const SkBitmap& src, bool transferOwnsPixels);
    virtual ~SkBitmapRef();

	const SkBitmap&	bitmap();

    static SkBitmapRef* create(const SkBitmap& src, bool transferOwnsPixels);
    static SkBitmapRef* DecodeFile(const char file[], bool forceDecode);
    static SkBitmapRef* DecodeMemory(const void* bytes, size_t len);
    static SkBitmapRef* DecodeStream(SkStream* stream); 
    
    /**	Frees all cached images, asserting that all references have been removed
	*/
    static void PurgeCacheAll();

	/** frees one cached image, returning true, or returns false if none could be freed
	*/
	static bool PurgeCacheOne();

private:
	struct Rec;
	Rec*	fRec;

	SkBitmapRef(Rec*);

	friend class SkBitmapRef_Globals;
};

class SkAutoBitmapRef {
public:
	SkAutoBitmapRef(const char file[], bool forceDecode)
	{
		fRef = SkBitmapRef::DecodeFile(file, forceDecode);
	}
	~SkAutoBitmapRef() { delete fRef; }

	const SkBitmap*	bitmap() const
	{
		return fRef ? &fRef->bitmap() : nil;
	}
private:
	SkBitmapRef*	fRef;
};


#endif
