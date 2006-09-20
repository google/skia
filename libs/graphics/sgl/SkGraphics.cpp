#include "SkGraphics.h"

#include "Sk64.h"
#include "SkBlitter.h"
#include "SkCanvas.h"
#include "SkDeque.h"
#include "SkDOM.h"
#include "SkFloat.h"
#include "SkGeometry.h"
#include "SkGlobals.h"
#include "SkMath.h"
#include "SkMatrix.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkPathMeasure.h"
#include "SkRefCnt.h"
#include "SkShader.h"
#include "SkStream.h"
#include "SkTSearch.h"
#include "SkUtils.h"
#include "SkXfermode.h"

#define typesizeline(type)	{ #type , sizeof(type) }
#define unittestline(type)	{ #type , type::UnitTest }


#ifdef BUILD_EMBOSS_TABLE
	extern void SkEmbossMask_BuildTable();
#endif

#ifdef BUILD_RADIALGRADIENT_TABLE
	extern void SkRadialGradient_BuildTable();
#endif

void SkGraphics::Init(bool runUnitTests)
{
	SkGlobals::Init();

#ifdef BUILD_EMBOSS_TABLE
	SkEmbossMask_BuildTable();
#endif
#ifdef BUILD_RADIALGRADIENT_TABLE
	SkRadialGradient_BuildTable();
#endif

#ifdef SK_SUPPORT_UNITTEST
	if (runUnitTests == false)
		return;
	int i;

	static const struct {
		const char*	fTypeName;
		size_t		fSizeOf;
	} gTypeSize[] = {
		typesizeline(char),
		typesizeline(short),
		typesizeline(int),
		typesizeline(long),
		typesizeline(size_t),
		typesizeline(void*),

		typesizeline(S8),
		typesizeline(U8),
		typesizeline(S16),
		typesizeline(U16),
		typesizeline(S32),
		typesizeline(U32),
		typesizeline(S8CPU),
		typesizeline(U8CPU),
		typesizeline(S16CPU),
		typesizeline(U16CPU),

		typesizeline(SkPoint),
		typesizeline(SkRect),
		typesizeline(SkMatrix),
		typesizeline(SkPath),
		typesizeline(SkRefCnt),

		typesizeline(SkPaint),
		typesizeline(SkCanvas),
		typesizeline(SkBlitter),
		typesizeline(SkShader),
		typesizeline(SkXfermode),
		typesizeline(SkPathEffect)
	};

	{
		char	test = (char)(0-1);	// use this subtract to avoid truncation warnings (in VC7 at least)
		if (test < 0)
			SkDebugf("SkGraphics: char is signed\n");
		else
			SkDebugf("SkGraphics: char is unsigned\n");
	}
	for (i = 0; i < (int)SK_ARRAY_COUNT(gTypeSize); i++)
		SkDebugf("SkGraphics: sizeof(%s) = %d\n", gTypeSize[i].fTypeName, gTypeSize[i].fSizeOf);

	static const struct {
		const char*	fTypeName;
		void (*fUnitTest)();
	} gUnitTests[] = {
		unittestline(Sk64),
		unittestline(SkMath),
		unittestline(SkUtils),
		unittestline(SkString),
		unittestline(SkFloat),
		unittestline(SkMatrix),
		unittestline(SkGeometry),
        unittestline(SkDeque),
		unittestline(SkPath),
		unittestline(SkPathMeasure)
	};

	for (i = 0; i < (int)SK_ARRAY_COUNT(gUnitTests); i++)
	{
		SkDebugf("SkGraphics: Running UnitTest for %s\n", gUnitTests[i].fTypeName);
		gUnitTests[i].fUnitTest();
		SkDebugf("SkGraphics: End UnitTest for %s\n", gUnitTests[i].fTypeName);
	}
	SkQSort_UnitTest();

#endif
}

////////////////////////////////////////////////////////////////////////////

#include "SkGlyphCache.h"
#include "SkImageDecoder.h"

void SkGraphics::Term()
{
	SkBitmapRef::PurgeCacheAll();
	SkGraphics::FreeCaches(SK_MaxS32);
	SkGlobals::Term();
}

bool SkGraphics::FreeCaches(size_t bytesNeeded)
{
	bool didSomething = SkBitmapRef::PurgeCacheOne();

	return SkGlyphCache::FreeCache(bytesNeeded) || didSomething;
}


