#ifndef SkInterpolator_DEFINED
#define SkInterpolator_DEFINED

#include "SkMath.h"

class SkInterpolatorBase {
public:
	enum Result {
		kNormal_Result,
		kFreezeStart_Result,
		kFreezeEnd_Result
	};
	static SkScalar Blend(SkScalar t, SkScalar blend);
protected:
	SkInterpolatorBase();
	~SkInterpolatorBase();
public:
	void	reset(int elemCount, int frameCount);

	/**	Return the start and end time for this interpolator.
		If there are no key frames, return false.
		@param	startTime	If no nil, returns the time (in milliseconds) of the
							first keyframe. If there are no keyframes, this parameter
							is ignored (left unchanged).
		@param	endTime		If no nil, returns the time (in milliseconds) of the
							last keyframe. If there are no keyframes, this parameter
							is ignored (left unchanged).
		@return	True if there are key frames, or false if there are none.
	*/
	bool	getDuration(SkMSec* startTime, SkMSec* endTime) const;


	/**	Set the whether the repeat is mirrored.
		@param If true, the odd repeats interpolate from the last key frame and the first.
	*/
	void	setMirror(bool mirror) { fFlags = SkToU8(fFlags & ~kMirror | (int) mirror); }

	/**	Set the repeat count. The repeat count may be fractional.
		@param repeatCount Multiplies the total time by this scalar.
	*/
	void	setRepeatCount(SkScalar repeatCount) { fRepeat = repeatCount; }

	/**	Set the whether the repeat is mirrored.
		@param If true, the odd repeats interpolate from the last key frame and the first.
	*/
	void	setReset(bool reset) { fFlags = SkToU8(fFlags & ~kReset | (int) reset); }

	Result	timeToT(SkMSec time, SkScalar* T, int* index, SkBool* exact) const;
protected:
	enum Flags {
		kMirror = 1,
		kReset = 2
	};
	static SkScalar ComputeRelativeT(SkMSec time, SkMSec prevTime, SkMSec nextTime, SkScalar blend);
	S16 fFrameCount;
	U8 fElemCount;
	U8 fFlags;
	SkScalar fRepeat;
	struct SkTimeCode {
		SkMSec fTime;
		SkScalar fBlend;
	};
	SkTimeCode* fTimes;		// pointer into fStorage
	void* fStorage;
#ifdef SK_DEBUG
	SkTimeCode(* fTimesArray)[10];
#endif
};

class SkInterpolator : public SkInterpolatorBase {
public:
	SkInterpolator();
	SkInterpolator(int elemCount, int frameCount);
	void	reset(int elemCount, int frameCount);

	/**	Add or replace a key frame, copying the values[] data into the interpolator.
		@param index	The index of this frame (frames must be ordered by time)
		@param time	The millisecond time for this frame
		@param values	The array of values [elemCount] for this frame. The data is copied
						into the interpolator.
		@param blend	A positive scalar specifying how to blend between this and the next key frame.
						[0...1) is a cubic lag/log/lag blend (slow to change at the beginning and end)
						1 is a linear blend (default)
	*/
	bool	setKeyFrame(int index, SkMSec time, const SkScalar values[], SkScalar blend = SK_Scalar1);
	Result timeToValues(SkMSec time, SkScalar values[]) const;
	SkDEBUGCODE(static void UnitTest();)
private:
	SkScalar* fValues; 	// pointer into fStorage
#ifdef SK_DEBUG
	SkScalar(* fScalarsArray)[10];
#endif
	typedef SkInterpolatorBase INHERITED;
};


#endif

