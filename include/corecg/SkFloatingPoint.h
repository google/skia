#ifndef SkFloatingPoint_DEFINED
#define SkFloatingPoint_DEFINED

#include "SkTypes.h"

#ifdef SK_CAN_USE_FLOAT

#include <math.h>
#include <float.h>

#ifdef SK_BUILD_FOR_WINCE
	#define	sk_float_sqrt(x)		(float)::sqrt(x)
	#define	sk_float_sin(x)			(float)::sin(x)
	#define	sk_float_cos(x)			(float)::cos(x)
	#define	sk_float_tan(x)			(float)::tan(x)
	#define sk_float_acos(x)		(float)::acos(x)
	#define sk_float_asin(x)		(float)::asin(x)
	#define sk_float_atan2(y,x)		(float)::atan2(y,x)
	#define sk_float_abs(x)			(float)::fabs(x)
	#define sk_float_mod(x,y)		(float)::fmod(x,y)
	#define sk_float_exp(x)		(float)::exp(x)
	#define sk_float_log(x)		(float)::log(x)
#else
	#define	sk_float_sqrt(x)		sqrtf(x)
	#define	sk_float_sin(x)			sinf(x)
	#define	sk_float_cos(x)			cosf(x)
	#define	sk_float_tan(x)			tanf(x)
#ifdef SK_BUILD_FOR_MAC
	#define sk_float_acos(x)		acos(x)
	#define sk_float_asin(x)		asin(x)
#else
	#define sk_float_acos(x)		acosf(x)
	#define sk_float_asin(x)		asinf(x)
#endif
	#define sk_float_atan2(y,x)	atan2f(y,x)
	#define sk_float_abs(x)			fabsf(x)
	#define sk_float_mod(x,y)		fmodf(x,y)
	#define sk_float_exp(x)			expf(x)
	#define sk_float_log(x)			logf(x)
	#define sk_float_isNaN(x)		_isnan(x)
#endif

#endif
#endif
