#ifndef SkMatrix_DEFINED
#define SkMatrix_DEFINED

#include "SkRect.h"

/**	\class SkMatrix

	The SkMatrix class holds a 3x3 matrix for transforming coordinates.
	SkMatrix does not have a constructor, so it must be explicitly initialized
	using either reset() - to construct an identity matrix, or one of the set...()
	functions (e.g. setTranslate, setRotate, etc.).
*/
class SkMatrix {
public:
	/**	Bit fields used to identify the characteristics of the matrix.
		See TypeMask for the corresponding mask values.
	*/
	enum TypeShift {
		kTranslate_Shift,
		kScale_Shift,
		kAffine_Shift,
		kPerspective_Shift,

		kShiftCount
	};

	/**	Enum of bit fields for the mask return by getType().
		Use this to identify the complexity of the matrix.
	*/
	enum TypeMask {
		kIdentity_Mask		= 0,						//!< type is 0 iff the matrix is the identiy
		kTranslate_Mask		= 1 << kTranslate_Shift,	//!< set if the matrix has non-zero translation
		kScale_Mask			= 1 << kScale_Shift,		//!< set if the matrix has X or Y scale different from 1.0
		kAffine_Mask		= 1 << kAffine_Shift,		//!< set if the matrix skews or rotates
		kPerspective_Mask	= 1 << kPerspective_Shift	//!< set if the matrix is in perspective
	};

	/**	Returns true if the mask represents a matrix that will only scale
		or translate (i.e., will map a rectangle into another rectangle).
	*/
	static bool RectStaysRect(TypeMask mask)
	{
		return (mask & (kAffine_Mask | kPerspective_Mask)) == 0;
	}

	/**	Returns a mask bitfield describing the types of transformations
		that the matrix will perform. This information is used by routines
		like mapPoints, to optimize its inner loops to only perform as much
		arithmetic as is necessary.
	*/
	TypeMask getType() const;

	/**	Returns true if the matrix is identity.
		This is faster than testing if (getType() == kIdentity_Mask)
	*/
	bool isIdentity() const;

	/**	Returns true if the matrix that will only scale
		or translate (i.e., will map a rectangle into another rectangle).
	*/
	bool rectStaysRect() const { return RectStaysRect(this->getType()); }

	SkScalar	getScaleX() const { return fMat[0]; }
	SkScalar	getScaleY() const { return fMat[4]; }
	SkScalar	getSkewY() const { return fMat[3]; }
	SkScalar	getSkewX() const { return fMat[1]; }
	SkScalar	getTranslateX() const { return fMat[2]; }
	SkScalar	getTranslateY() const { return fMat[5]; }
	SkScalar	getPerspX() const { return fMat[6]; }
	SkScalar	getPerspY() const { return fMat[7]; }

	void	setScaleX(SkScalar v) { fMat[0] = v; }
	void	setScaleY(SkScalar v) { fMat[4] = v; }
	void	setSkewY(SkScalar v) { fMat[3] = v; }
	void	setSkewX(SkScalar v) { fMat[1] = v; }
	void	setTranslateX(SkScalar v) { fMat[2] = v; }
	void	setTranslateY(SkScalar v) { fMat[5] = v; }
#ifdef SK_SCALAR_IS_FIXED
	void	setPerspX(SkFract v) { fMat[6] = v; }
	void	setPerspY(SkFract v) { fMat[7] = v; }
#else
	void	setPerspX(SkScalar v) { fMat[6] = v; }
	void	setPerspY(SkScalar v) { fMat[7] = v; }
#endif
	/**	Set the matrix to identity
	*/
	void	reset();
    
    void    set(const SkMatrix& other) { *this = other; }
    
	/**	Set the matrix to translate by (dx, dy).
	*/
	void	setTranslate(SkScalar dx, SkScalar dy);
	/**	Set the matrix to scale by sx and sy, with a pivot point at (px, py).
		The pivot point is the coordinate that should remain unchanged by the
		specified transformation.
	*/
	void	setScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py);
	/**	Set the matrix to rotate by the specified number of degrees, with a pivot point at (px, py).
		The pivot point is the coordinate that should remain unchanged by the
		specified transformation.
	*/
	void	setRotate(SkScalar degrees, SkScalar px, SkScalar py);
	/**	Set the matrix to rotate by the specified sine and cosine values, with a pivot point at (px, py).
		The pivot point is the coordinate that should remain unchanged by the
		specified transformation.
	*/
	void	setSinCos(SkScalar sinValue, SkScalar cosValue, SkScalar px, SkScalar py);
	/**	Set the matrix to skew by sx and sy, with a pivot point at (px, py).
		The pivot point is the coordinate that should remain unchanged by the
		specified transformation.
	*/
	void	setSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py);
	/**	Set the matrix to the concatenation of the two specified matrices, returning
		true if the the result can be represented. Either of the two matrices may
		also be the target matrix. *this = a * b;
	*/
	bool	setConcat(const SkMatrix& a, const SkMatrix& b);

	/**	Preconcats the matrix with the specified translation.
		M' = M * T(dx, dy)
	*/
	bool	preTranslate(SkScalar dx, SkScalar dy);
	/**	Preconcats the matrix with the specified scale.
		M' = M * S(sx, sy, px, py)
	*/
	bool	preScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py);
	/**	Preconcats the matrix with the specified rotation.
		M' = M * R(degrees, px, py)
	*/
	bool	preRotate(SkScalar degrees, SkScalar px, SkScalar py);
	/**	Preconcats the matrix with the specified skew.
		M' = M * K(kx, ky, px, py)
	*/
	bool	preSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py);
	/**	Preconcats the matrix with the specified matrix.
		M' = M * other
	*/
	bool	preConcat(const SkMatrix& other);

	/**	Postconcats the matrix with the specified translation.
		M' = T(dx, dy) * M
	*/
	bool	postTranslate(SkScalar dx, SkScalar dy);
	/**	Postconcats the matrix with the specified scale.
		M' = S(sx, sy, px, py) * M
	*/
	bool	postScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py);
	/**	Postconcats the matrix with the specified rotation.
		M' = R(degrees, px, py) * M
	*/
	bool	postRotate(SkScalar degrees, SkScalar px, SkScalar py);
	/**	Postconcats the matrix with the specified skew.
		M' = K(kx, ky, px, py) * M
	*/
	bool	postSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py);
	/**	Postconcats the matrix with the specified matrix.
		M' = other * M
	*/
	bool	postConcat(const SkMatrix& other);

	enum ScaleToFit {
		kFill_ScaleToFit,		//!< scale in X and Y independently
		kStart_ScaleToFit,		//!< uniform scale in X/Y, align along left/top
		kCenter_ScaleToFit,		//!< uniform scale in X/Y, align along center
		kEnd_ScaleToFit			//!< uniform scale in X/Y, align along right/bottom
	};
	/**	Set the matrix to the scale and translate values that map the source rectangle
		to the destination rectangle, returning true if the the result can be represented.
		@param src the source rectangle to map from.
		@param dst the destination rectangle to map to.
		@param stf the ScaleToFit option
		@return true if the matrix can be represented by the rectangle mapping.
	*/
	bool	setRectToRect(const SkRect& src, const SkRect& dst, ScaleToFit stf = kFill_ScaleToFit);
	/**	Set the matrix such that the specified src points would map to the
		specified dst points. count must be withing [0..4].
	*/
	bool	setPolyToPoly(const SkPoint dst[], const SkPoint src[], int count);


	/**	If this matrix can be inverted, return true and if inverse is not nil, set inverse
		to be the inverse of this matrix. If this matrix cannot be inverted, ignore inverse
		and return false
	*/
	bool	invert(SkMatrix* inverse) const;

	/**	Apply this matrix to the array of points specified by src, and write the transformed
		points into the array of points specified by dst.
		dst[] = M * src[]
		@param dst	Where the transformed coordinates are written. It must contain at least count entries
		@param src	The original coordinates that are to be transformed. It must contain at least count entries
		@param count The number of points in src to read, and then transform into dst.
		@param typeMask The mask bits returned by getType() for this matrix.
	*/
	bool	mapPoints(SkPoint dst[], const SkPoint src[], int count, TypeMask typeMask) const;
	/**	Apply this matrix to the array of vectors specified by src, and write the transformed
		vectors into the array of points specified by dst. This is similar to mapPoints, but
		ignores any translation in the matrix.
		@param dst	Where the transformed coordinates are written. It must contain at least count entries
		@param src	The original coordinates that are to be transformed. It must contain at least count entries
		@param count The number of vectors in src to read, and then transform into dst.
		@param typeMask The mask bits returned by getType() for this matrix.
	*/
	bool	mapVectors(SkVector dst[], const SkVector src[], int count, TypeMask typeMask) const;
	/**	Apply this matrix to the src rectangle, and write the transformed rectangle into
		dst. This is accomplished by transforming the 4 corners of src, and then setting
		dst to the bounds of those points.
		@param dst	Where the transformed rectangle is written.
		@param src	The original rectangle to be transformed.
		@param typeMask The mask bits returned by getType() for this matrix.
	*/
	bool	mapRect(SkRect* dst, const SkRect& src, TypeMask typeMask) const;

	/**	Helper method for mapPoints() where the TypeMask needs to be computed.
	*/
	bool mapPoints(SkPoint dst[], const SkPoint src[], int count) const
	{
		return this->mapPoints(dst, src, count, this->getType());
	}
	/**	Helper method for mapPoints() where the src and dst arrays are the
		same, and the TypeMask needs to be computed.
	*/
	bool mapPoints(SkPoint pts[], int count) const
	{
		return this->mapPoints(pts, pts, count, this->getType());
	}
	/**	Helper method for mapVectors() where the TypeMask needs to be computed.
	*/
	bool mapVectors(SkVector dst[], const SkVector src[], int count) const
	{
		return this->mapVectors(dst, src, count, this->getType());
	}
	/**	Helper method for mapVectors() where the src and dst arrays are the
		same, and the TypeMask needs to be computed.
	*/
	bool mapVectors(SkVector vecs[], int count) const
	{
		return this->mapVectors(vecs, vecs, count, this->getType());
	}
	/**	Helper method for mapRect() where the TypeMask needs to be computed.
	*/
	bool mapRect(SkRect* dst, const SkRect& src) const
	{
		return this->mapRect(dst, src, this->getType());
	}
	/**	Helper method for mapRect() where the TypeMask needs to be computed
		and the src and dst rects are the same (i.e. map in place)
	*/
	bool mapRect(SkRect* rect) const
	{
		return this->mapRect(rect, *rect, this->getType());
	}

	/**	Return the mean radius of a circle after it has been mapped by
		this matrix. NOTE: in perspective this value assumes the circle
		has its center at the origin.
	*/
	SkScalar mapRadius(SkScalar radius) const;

	typedef void (*MapPtProc)(const SkMatrix& mat, SkScalar x, SkScalar y, SkPoint* result);
	MapPtProc	getMapPtProc() const;

	/**	If the matrix can be stepped in X (not complex perspective)
		then return true and if step[XY] is not nil, return the step[XY] value.
		If it cannot, return false and ignore step.
	*/
	bool fixedStepInX(SkScalar y, SkFixed* stepX, SkFixed* stepY) const;

	friend bool operator==(const SkMatrix& a, const SkMatrix& b)
	{
		return memcmp(a.fMat, b.fMat, sizeof(a)) == 0;
	}

#ifdef SK_DEBUG
  /** @cond UNIT_TEST */
  void dump() const;

	static void UnitTest();
  /** @endcond */
#endif

private:
	SkScalar fMat[9];

	static void Map2Pt(const SkPoint srcPt[], SkMatrix* dst, SkScalar scale);
	static void Map3Pt(const SkPoint srcPt[], SkMatrix* dst, SkScalar scaleX, SkScalar scaleY);
	static void Map4Pt(const SkPoint srcPt[], SkMatrix* dst, SkScalar scaleX, SkScalar scaleY);

	static void Perspective_ptProc(const SkMatrix&, SkScalar, SkScalar, SkPoint*);
	static void Affine_ptProc(const SkMatrix&, SkScalar, SkScalar, SkPoint*);
	static void Scale_ptProc(const SkMatrix&, SkScalar, SkScalar, SkPoint*);
	static void Translate_ptProc(const SkMatrix&, SkScalar, SkScalar, SkPoint*);
	static void Identity_ptProc(const SkMatrix&, SkScalar, SkScalar, SkPoint*);
};

#endif

