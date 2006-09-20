#ifndef SkMaskFilter_DEFINED
#define SkMaskFilter_DEFINED

#include "SkFlattenable.h"
#include "SkMask.h"

class SkBlitter;
class SkBounder;
class SkMatrix;
class SkPath;
class SkRegion;

/**	\class SkMaskFilter

	SkMaskFilter is the base class for object that perform transformations on
	an alpha-channel mask before drawing it. A subclass of SkMaskFilter may be
	installed into a SkPaint. Once there, each time a primitive is drawn, it
	is first scan converted into a SkMask::kA8_Format mask, and handed to the
	filter, calling its filterMask() method. If this returns true, then the
	new mask is used to render into the device.

	Blur and emboss are implemented as subclasses of SkMaskFilter.
*/
class SkMaskFilter : public SkFlattenable {
public:
    SkMaskFilter() {}

	/**	Returns the format of the resulting mask that this subclass will return
		when its filterMask() method is called.
	*/
	virtual SkMask::Format	getFormat() = 0;

	/**	Create a new mask by filter the src mask.
		If src.fImage == nil, then do not allocate or create the dst image
		but do fill out the other fields in dstMask.
		If you do allocate a dst image, use SkMask::AllocImage()
		If this returns false, dst mask is ignored.
		@param	dst	the result of the filter. If src.fImage == nil, dst should not allocate its image
		@param src the original image to be filtered.
		@param matrix the CTM
		@param margin	if not nil, return the buffer dx/dy need when calculating the effect. Used when
						drawing a clipped object to know how much larger to allocate the src before
						applying the filter.
		@return true if the dst mask was correctly created.
	*/
	virtual bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&, SkPoint16* margin);

	/**	Helper method that, given a path in device space, will rasterize it into a kA8_Format mask
		and then call filterMask(). If this returns true, the specified blitter will be called
		to render that mask. Returns false if filterMask() returned false.
        This method is not exported to java.
	*/
	bool filterPath(const SkPath& devPath, const SkMatrix& devMatrix,
					const SkRegion& devClip, SkBounder*, SkBlitter* blitter);

protected:
    // empty for now, but lets get our subclass to remember to init us for the future
    SkMaskFilter(SkRBuffer&) {}
};

/**	\class SkAutoMaskImage

	Stack class used to manage the fImage buffer in a SkMask.
	When this object loses scope, the buffer is freed with SkMask::FreeImage().
*/
class SkAutoMaskImage {
public:
	SkAutoMaskImage(SkMask* mask, bool alloc)
	{
		if (alloc)
			mask->fImage = SkMask::AllocImage(mask->computeImageSize());
		fImage = mask->fImage;
	}
	~SkAutoMaskImage()
	{
		SkMask::FreeImage(fImage);
	}
private:
	uint8_t*    fImage;
};

#endif

