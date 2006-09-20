#ifndef SkMask_DEFINED
#define SkMask_DEFINED

#include "SkRect.h"

/**	\class SkMask
	SkMask is used to describe alpha bitmaps, either 1bit, 8bit, or
	the 3-channel 3D format. These are passed to SkMaskFilter objects.
*/
struct SkMask {
	enum Format {
		kBW_Format,	//!< 1bit per pixel mask (e.g. monochrome)
		kA8_Format,	//!< 8bits per pixel mask (e.g. antialiasing)
		k3D_Format	//!< 3 8bit per pixl planes: alpha, mul, add
	};

	uint8_t*	fImage;
	SkRect16	fBounds;
	uint16_t	fRowBytes;
	uint8_t		fFormat;	// Format

	/**	Return the byte size of the mask, assuming only 1 plane.
		Does not account for k3D_Format. For that, use computeFormatImageSize()
	*/
	size_t computeImageSize() const;
	/**	Return the byte size of the mask, taking into account
		any extra planes (e.g. k3D_Format).
	*/
	size_t computeTotalImageSize() const;

	/**	Returns the address of the byte that holds the specified bit.
		Asserts that the mask is kBW_Format, and that x,y are in range.
		x,y are in the same coordiate space as fBounds.
	*/
	uint8_t* getAddr1(int x, int y) const
	{
		SkASSERT(fFormat == kBW_Format);
		SkASSERT(fBounds.contains(x, y));
		SkASSERT(fImage != nil);
		return fImage + ((x - fBounds.fLeft) >> 3) + (y - fBounds.fTop) * fRowBytes;
	}
	/**	Returns the address of the specified byte.
		Asserts that the mask is kA8_Format, and that x,y are in range.
		x,y are in the same coordiate space as fBounds.
	*/
	uint8_t* getAddr(int x, int y) const
	{
		SkASSERT(fFormat != kBW_Format);
		SkASSERT(fBounds.contains(x, y));
		SkASSERT(fImage != nil);
		return fImage + x - fBounds.fLeft + (y - fBounds.fTop) * fRowBytes;
	}

	static uint8_t*	AllocImage(size_t bytes);
	static void	FreeImage(uint8_t* image);
    
    enum CreateMode {
        kJustComputeBounds_CreateMode,      //!< compute bounds and return
        kJustRenderImage_CreateMode,        //!< render into preallocate mask
        kComputeBoundsAndRenderImage_CreateMode  //!< compute bounds, alloc image and render into it
    };
};

#endif

