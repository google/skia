/* libs/graphics/sgl/SkGraphics.cpp
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

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
#include "SkRandom.h"
#include "SkRefCnt.h"
#include "SkShader.h"
#include "SkStream.h"
#include "SkTSearch.h"
#include "SkTime.h"
#include "SkUtils.h"
#include "SkXfermode.h"

#define typesizeline(type)  { #type , sizeof(type) }
#define unittestline(type)  { #type , type::UnitTest }


#ifdef BUILD_EMBOSS_TABLE
    extern void SkEmbossMask_BuildTable();
#endif

#ifdef BUILD_RADIALGRADIENT_TABLE
    extern void SkRadialGradient_BuildTable();
#endif

#define BIG_LOOP_COUNT  1000000

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
        const char* fTypeName;
        size_t      fSizeOf;
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
        char    test = (char)(0-1); // use this subtract to avoid truncation warnings (in VC7 at least)
        if (test < 0)
            SkDebugf("SkGraphics: char is signed\n");
        else
            SkDebugf("SkGraphics: char is unsigned\n");
    }
    for (i = 0; i < (int)SK_ARRAY_COUNT(gTypeSize); i++)
        SkDebugf("SkGraphics: sizeof(%s) = %d\n", gTypeSize[i].fTypeName, gTypeSize[i].fSizeOf);

    static const struct {
        const char* fTypeName;
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

    if (false)  // test asm fixmul
    {
        int j;
        SkMSec now = SkTime::GetMSecs();
        for (j = 0; j < BIG_LOOP_COUNT; j++) {
            (void)SkFixedMul_portable(0x8000, 0x150000);
        }
        SkMSec now2 = SkTime::GetMSecs();
        printf("-------- SkFixedMul_portable = %d\n", now2 - now);

        for (j = 0; j < BIG_LOOP_COUNT; j++) {
            (void)SkFixedMul(0x8000, 0x150000);
        }
        printf("-------- SkFixedMul = %d\n", SkTime::GetMSecs() - now2);

        SkRandom rand;
        for (j = 0; j < 10000; j++) {
            SkFixed a = rand.nextS() >> 8;
            SkFixed b = rand.nextS() >> 8;
            SkFixed c1 = SkFixedMul_portable(a, b);
            SkFixed c2 = SkFixedMul(a, b);
            if (SkAbs32(c1 - c2) > 1)
                printf("------ FixMul disagreement: (%x %x) slow=%x fast=%x\n", a, b, c1, c2);
        }
    }
    
    if (false)  // test asm fractmul
    {
        int j;
        SkMSec now = SkTime::GetMSecs();
        for (j = 0; j < BIG_LOOP_COUNT; j++) {
            (void)SkFractMul_portable(0x800000, 0x1500000);
        }
        SkMSec now2 = SkTime::GetMSecs();
        printf("-------- SkFractMul_portable = %d\n", now2 - now);

        for (j = 0; j < BIG_LOOP_COUNT; j++) {
            (void)SkFractMul(0x800000, 0x1500000);
        }
        printf("-------- SkFractMul = %d\n", SkTime::GetMSecs() - now2);

        SkRandom rand;
        for (j = 0; j < 10000; j++) {
            SkFixed a = rand.nextS() >> 1;
            SkFixed b = rand.nextS() >> 1;
            SkFixed c1 = SkFractMul_portable(a, b);
            SkFixed c2 = SkFractMul(a, b);
            if (SkAbs32(c1 - c2) > 1)
                printf("------ FractMul disagreement: (%x %x) slow=%x fast=%x\n", a, b, c1, c2);
        }
    }
    
    if (false)   // test asm clz
    {
        int j;
        SkMSec now = SkTime::GetMSecs();
        for (j = 0; j < BIG_LOOP_COUNT; j++) {
            (void)SkCLZ_portable(now);
        }
        SkMSec now2 = SkTime::GetMSecs();
        printf("-------- SkCLZ_portable = %d\n", now2 - now);

        for (j = 0; j < BIG_LOOP_COUNT; j++) {
            (void)SkCLZ(now);
        }
        printf("-------- SkCLZ = %d\n", SkTime::GetMSecs() - now2);

        SkRandom rand;
        for (j = 0; j < 10000; j++) {
            uint32_t a = rand.nextU();
            int c1 = SkCLZ_portable(a);
            int c2 = SkCLZ(a);
            if (c1 != c2)
                printf("------ CLZ disagreement: (%x) slow=%x fast=%x\n", a, c1, c2);
        }
    }
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


