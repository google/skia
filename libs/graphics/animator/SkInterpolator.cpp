#include "SkInterpolator.h"
#include "SkTSearch.h"

SkInterpolatorBase::SkInterpolatorBase()
{
	fStorage	= nil;
	fTimes		= nil;
	SkDEBUGCODE(fTimesArray	= nil;)
}

SkInterpolatorBase::~SkInterpolatorBase()
{
	if (fStorage)
		sk_free(fStorage);
}

void SkInterpolatorBase::reset(int elemCount, int frameCount)
{
	fFlags = 0;
	fElemCount = SkToU8(elemCount);
	fFrameCount = SkToS16(frameCount);
	fRepeat = SK_Scalar1;
	if (fStorage) {
		sk_free(fStorage);
		fStorage = nil;
		fTimes = nil;
		SkDEBUGCODE(fTimesArray = nil);
	}
}

/*	Each value[] run is formated as:
		<time (in msec)>
		<blend>
		<data[fElemCount]>

	Totaling fElemCount+2 entries per keyframe
*/

bool SkInterpolatorBase::getDuration(SkMSec* startTime, SkMSec* endTime) const
{
	if (fFrameCount == 0)
		return false;

	if (startTime)
		*startTime = fTimes[0].fTime;
	if (endTime)
		*endTime = fTimes[fFrameCount - 1].fTime;
	return true;
}

SkScalar SkInterpolatorBase::ComputeRelativeT(SkMSec time, SkMSec prevTime, SkMSec nextTime, SkScalar blend)
{
	SkASSERT(time > prevTime && time < nextTime);
	SkASSERT(blend >= 0);

	SkScalar	t = SkScalarDiv((SkScalar)(time - prevTime), (SkScalar)(nextTime - prevTime));
	return Blend(t, blend);
}

SkScalar SkInterpolatorBase::Blend(SkScalar t, SkScalar blend)
{
	// f(t) = -2(1-blend)t^3 + 3(1 - blend)t^2 + blend*t
	return SkScalarMul(SkScalarMul(SkScalarMul(-2*(SK_Scalar1 - blend), t) + 3*(SK_Scalar1 - blend), t) + blend, t);
}

SkInterpolatorBase::Result SkInterpolatorBase::timeToT(SkMSec time, SkScalar* T, int* indexPtr, SkBool* exactPtr) const
{
	SkASSERT(fFrameCount > 0);
	Result	result = kNormal_Result;
	if (fRepeat != SK_Scalar1)
	{
		SkMSec startTime, endTime;
		this->getDuration(&startTime, &endTime);
		SkMSec totalTime = endTime - startTime;
		SkMSec offsetTime = time - startTime;
		endTime = SkScalarMulFloor(fRepeat, totalTime);
		if (offsetTime >= endTime)
		{
			SkScalar fraction = SkScalarFraction(fRepeat);
			offsetTime = fraction == 0 && fRepeat > 0 ? totalTime :
				SkScalarMulFloor(fraction, totalTime);
			result = kFreezeEnd_Result;
		}
		else
		{
			int mirror = fFlags & kMirror;
			offsetTime = offsetTime % (totalTime << mirror);
			if (offsetTime > totalTime)	// can only be true if fMirror is true
				offsetTime = (totalTime << 1) - offsetTime;
		}
		time = offsetTime + startTime;
	}

	int index = SkTSearch<SkMSec>(&fTimes[0].fTime, fFrameCount, time, sizeof(SkTimeCode));

	bool	exact = true;

	if (index < 0)
	{
		index = ~index;
		if (index == 0)
			result = kFreezeStart_Result;
		else if (index == fFrameCount)
		{
			if (fFlags & kReset)
				index = 0;
			else
				index -= 1;
			result = kFreezeEnd_Result;
		}
		else
			exact = false;
	}
	SkASSERT(index < fFrameCount);
	const SkTimeCode* nextTime = &fTimes[index];
	SkMSec	 nextT = nextTime[0].fTime;
	if (exact)
		*T = 0;
	else {
		SkMSec prevT = nextTime[-1].fTime;
		*T = ComputeRelativeT(time, prevT, nextT, nextTime[-1].fBlend);
	}
	*indexPtr = index;
	*exactPtr = exact;
	return result;
}


SkInterpolator::SkInterpolator() {
	INHERITED::reset(0, 0);
	fValues = nil;
	SkDEBUGCODE(fScalarsArray = nil;)
}

SkInterpolator::SkInterpolator(int elemCount, int frameCount)
{
	SkASSERT(elemCount > 0);
	this->reset(elemCount, frameCount);
}

void SkInterpolator::reset(int elemCount, int frameCount) {
	INHERITED::reset(elemCount, frameCount);
	fStorage = sk_malloc_throw((sizeof(SkScalar) * elemCount + sizeof(SkTimeCode)) * frameCount);
	fTimes = (SkTimeCode*) fStorage;
	fValues = (SkScalar*) ((char*) fStorage + sizeof(SkTimeCode) * frameCount);
#ifdef SK_DEBUG
	fTimesArray = (SkTimeCode(*)[10]) fTimes;
	fScalarsArray = (SkScalar(*)[10]) fValues;
#endif
}

bool SkInterpolator::setKeyFrame(int index, SkMSec time, const SkScalar values[], SkScalar blend)
{
	SkASSERT(values != nil);
	blend = SkScalarPin(blend, 0, SK_Scalar1);

	bool success = ~index == SkTSearch<SkMSec>(&fTimes->fTime, index, time, sizeof(SkTimeCode));
	SkASSERT(success);
	if (success) {
		SkTimeCode* timeCode = &fTimes[index];
		timeCode->fTime = time;
		timeCode->fBlend = blend;
		SkScalar* dst = &fValues[fElemCount * index];
		memcpy(dst, values, fElemCount * sizeof(SkScalar));
	}
	return success;
}

SkInterpolator::Result SkInterpolator::timeToValues(SkMSec time, SkScalar values[]) const
{
	SkScalar T;
	int index;
	SkBool exact;
	Result result = timeToT(time, &T, &index, &exact);
	if (values)
	{
		const SkScalar* nextSrc = &fValues[index * fElemCount];

		if (exact)
			memcpy(values, nextSrc, fElemCount * sizeof(SkScalar));
		else
		{
			SkASSERT(index > 0);

			const SkScalar* prevSrc = nextSrc - fElemCount;

			for (int i = fElemCount - 1; i >= 0; --i)
				values[i] = SkScalarInterp(prevSrc[i], nextSrc[i], T);
		}
	}
	return result;
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

#ifdef SK_SUPPORT_UNITTEST
	static SkScalar* iset(SkScalar array[3], int a, int b, int c)
	{
		array[0] = SkIntToScalar(a);
		array[1] = SkIntToScalar(b);
		array[2] = SkIntToScalar(c);
		return array;
	}
#endif

void SkInterpolator::UnitTest()
{
#ifdef SK_SUPPORT_UNITTEST
	SkInterpolator	inter(3, 2);
	SkScalar		v1[3], v2[3], v[3], vv[3];
	Result			result;

	inter.setKeyFrame(0, 100, iset(v1, 10, 20, 30), 0);
	inter.setKeyFrame(1, 200, iset(v2, 110, 220, 330));

	result = inter.timeToValues(0, v);
	SkASSERT(result == kFreezeStart_Result);
	SkASSERT(memcmp(v, v1, sizeof(v)) == 0);

	result = inter.timeToValues(99, v);
	SkASSERT(result == kFreezeStart_Result);
	SkASSERT(memcmp(v, v1, sizeof(v)) == 0);

	result = inter.timeToValues(100, v);
	SkASSERT(result == kNormal_Result);
	SkASSERT(memcmp(v, v1, sizeof(v)) == 0);

	result = inter.timeToValues(200, v);
	SkASSERT(result == kNormal_Result);
	SkASSERT(memcmp(v, v2, sizeof(v)) == 0);

	result = inter.timeToValues(201, v);
	SkASSERT(result == kFreezeEnd_Result);
	SkASSERT(memcmp(v, v2, sizeof(v)) == 0);

	result = inter.timeToValues(150, v);
	SkASSERT(result == kNormal_Result);
	SkASSERT(memcmp(v, iset(vv, 60, 120, 180), sizeof(v)) == 0);

	result = inter.timeToValues(125, v);
	SkASSERT(result == kNormal_Result);
	result = inter.timeToValues(175, v);
	SkASSERT(result == kNormal_Result);
#endif
}

#endif

