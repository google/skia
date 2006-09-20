#ifndef SkShader_DEFINED
#define SkShader_DEFINED

#include "SkRefCnt.h"
#include "SkBitmap.h"
#include "SkMask.h"
#include "SkMatrix.h"
#include "SkPaint.h"

class SkPath;

/**	\class SkShader

	SkShader is the based class for objects that return horizontal spans of colors during drawing.
	A subclass of SkShader is installed in a SkPaint calling paint.setShader(shader). After that
	any object (other than a bitmap) that is drawn with that paint will get its color(s) from the
	shader.
*/
class SkShader : public SkRefCnt {
public:
			SkShader();
	virtual	~SkShader();

	/**	Return the shader's optional local matrix, or nil.
	*/
	const SkMatrix*	getLocalMatrix() const { return fLocalMatrix; }
	/**	Set the shader's optional local matrix. If the specified matrix is identity, then
		getLocalMatrix() will return nil.
	*/
	void setLocalMatrix(const SkMatrix&);

	enum TileMode {
		kClamp_TileMode,	//!< replicate the edge color if the shader draws outside of its original bounds
		kRepeat_TileMode,	//!< repeat the shader's image horizontally and vertically
		kMirror_TileMode,	//!< repeat the shader's image horizontally and vertically, alternating mirror images so that adjacent images always seam

		kTileModeCount
	};

	// override these in your subclass

	enum Flags {
		kOpaqueAlpha_Flag	= 0x01,	//!< set if all of the colors will be opaque (if so, kConstAlpha_Flag will not be set)
		kConstAlpha_Flag	= 0x02,	//!< set if all of the colors have the same (non-opaque) alpha
		kHasSpan16_Flag		= 0x04,	//!< set if this shader's shadeSpanOpaque16() method can be called

		kFlagsMask			= kOpaqueAlpha_Flag | kConstAlpha_Flag | kHasSpan16_Flag
	};

	/**	Called sometimes before drawing with this shader.
		Return the type of alpha your shader will return.
		The default implementation returns 0. Your subclass should override if it can
		(even sometimes) report a non-zero value, since that will enable various blitters
		to perform faster.
	*/
	virtual U32		getFlags();

	/**	Called once before drawing, with the current paint and
		device matrix. Return true if your shader supports these
		parameters, or false if not. If false is returned, nothing
		will be drawn.
	*/
	virtual bool	setContext(	const SkBitmap& device,
								const SkPaint& paint,
								const SkMatrix& matrix);

	/**	Called for each span of the object being drawn. Your subclass
		should set the appropriate colors (with premultiplied alpha) that
		correspond to the specified device coordinates.
	*/
	virtual void	shadeSpan(int x, int y, SkPMColor[], int count) = 0;
	/**	Called only for 16bit devices when getFlags() returns kOpaqueAlphaFlag | kHasSpan16_Flag
	*/
	virtual void	shadeSpanOpaque16(int x, int y, U16[], int count);
	/**	Similar to shadeSpan, but only returns the alpha-channel for a span.
		The default implementation calls shadeSpan() and then extracts the alpha
		values from the returned colors.
	*/
	virtual void	shadeSpanAlpha(int x, int y, U8 alpha[], int count);

	/**	Helper function that returns true if this shader's shadeSpanOpaque16() method can
		be called.
	*/
	bool canCallShadeSpanOpaque16()
	{
		return SkShader::CanCallShadeSpanOpaque16(this->getFlags());
	}

	/**	Helper to check the flags to know if it is legal to call shadeSpanOpaque16()
	*/
	static bool CanCallShadeSpanOpaque16(U32 flags)
	{
		return (flags & (kOpaqueAlpha_Flag | kHasSpan16_Flag)) == (kOpaqueAlpha_Flag | kHasSpan16_Flag);
	}

	//////////////////////////////////////////////////////////////////////////
	//	Factory methods for stock shaders

	/**	Call this to create a new shader that will draw with the specified bitmap.
		@param src	The bitmap to use inside the shader
		@param transferOwnershipOfPixels	If true, the shader will call setOwnsPixels(true) on its private bitmap
											and setOwnsPixels(false) on the src bitmap, resulting in the bitmap's pixels
											being disposed when the shader is deleted.
		@param ft	The filter type to be used when scaling or rotating the bitmap when it is drawn.
		@param tmx	The tiling mode to use when sampling the bitmap in the x-direction.
		@param tmy	The tiling mode to use when sampling the bitmap in the y-direction.
		@return		Returns a new shader object. Note: this function never returns nil.
	*/
	static SkShader* CreateBitmapShader(const SkBitmap& src,
										bool transferOwnershipOfPixels,
										SkPaint::FilterType ft,
										TileMode tmx, TileMode tmy);

protected:
	enum MatrixClass {
		kLinear_MatrixClass,			// no perspective
		kFixedStepInX_MatrixClass,		// fast perspective, need to call fixedStepInX() each scanline
		kPerspective_MatrixClass		// slow perspective, need to mappoints each pixel
	};
	static MatrixClass ComputeMatrixClass(const SkMatrix&);

	// These can be called by your subclass after setContext() has been called
	U8				 getPaintAlpha() const { return fPaintAlpha; }
	SkBitmap::Config getDeviceConfig() const { return (SkBitmap::Config)fDeviceConfig; }
	const SkMatrix&	 getTotalInverse() const { return fTotalInverse; }
	MatrixClass		 getInverseClass() const { return (MatrixClass)fTotalInverseClass; }
	SkMatrix::MapPtProc	getInverseMapPtProc() const { return fInverseMapPtProc; }

private:
	SkMatrix*	fLocalMatrix;
	SkMatrix	fTotalInverse;
	SkMatrix::MapPtProc	fInverseMapPtProc;
	U8			fPaintAlpha;
	U8			fDeviceConfig;
	U8			fTotalInverseClass;

	static SkShader* CreateBitmapShader(const SkBitmap& src,
										bool transferOwnershipOfPixels,
										SkPaint::FilterType,
										TileMode, TileMode,
										void* storage, size_t storageSize);
	friend class SkAutoBitmapShaderInstall;
};

#endif

