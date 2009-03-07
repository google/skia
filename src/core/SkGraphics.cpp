/* libs/graphics/sgl/SkGraphics.cpp
**
** Copyright 2006, The Android Open Source Project
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
#include "SkFloat.h"
#include "SkGeometry.h"
#include "SkGlobals.h"
#include "SkMath.h"
#include "SkMatrix.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkRandom.h"
#include "SkRefCnt.h"
#include "SkScalerContext.h"
#include "SkShader.h"
#include "SkStream.h"
#include "SkTSearch.h"
#include "SkTime.h"
#include "SkUtils.h"
#include "SkXfermode.h"

#if 0

#define SK_SORT_TEMPLATE_TYPE       int
#define SK_SORT_TEMPLATE_NAME       sort_int
#define SK_SORT_TEMPLATE_CMP(a, b)   ((a) - (b))
#include "SkSortTemplate.h"

#define SK_SORT_TEMPLATE_TYPE       int*
#define SK_SORT_TEMPLATE_NAME       sort_intptr
#define SK_SORT_TEMPLATE_CMP(a, b)   (*(a) - *(b))
#include "SkSortTemplate.h"

static void test_sort()
{
    int array[] = { 4, 3, 7, 5, 2, 5, 1, 2, 9, 6, 7, 4, 5, 3, 1, 0 };
    int* ptr[SK_ARRAY_COUNT(array)];
    int i, N = SK_ARRAY_COUNT(array) - 1;

    for (i = 0; i < N; i++)
        printf(" %d", array[i]);
    printf("\n");
    
    for (i = 0; i < N; i++)
        ptr[i] = &array[i];
    sort_intptr(ptr, N);
    for (i = 0; i < N; i++)
        printf(" %d", *ptr[i]);
    printf("\n");

    sort_int(array, N);
    for (i = 0; i < N; i++)
        printf(" %d", array[i]);
    printf("\n");

}
#endif

#define SPEED_TESTx

#define typesizeline(type)  { #type , sizeof(type) }


#ifdef BUILD_EMBOSS_TABLE
    extern void SkEmbossMask_BuildTable();
#endif

#ifdef BUILD_RADIALGRADIENT_TABLE
    extern void SkRadialGradient_BuildTable();
#endif

#define BIG_LOOP_COUNT  1000000
#define TEXT_LOOP_COUNT 1000

#ifdef SPEED_TEST
static int test_s64(int i)
{
    Sk64    a, b, c;
    
    c.set(0);
    a.set(i);
    b.setMul(i, i);
    a.add(b);
    a.add(c);
    return c.getFixed();
}

static int test_native_64(int i)
{
    int16_t    a, b, c;
    
    c = 0;
    a = i;
    b = (int64_t)i * i;
    a += b;
    a += c;
    return (int)(c >> 16);
}

static void test_drawText(SkBitmap::Config config, SkColor color)
{
    SkBitmap    bm;
    
    bm.setConfig(config, 320, 240);
    bm.allocPixels();
    
    SkCanvas canvas(bm);
    SkPaint  paint;
    
    paint.setAntiAlias(true);
    paint.setTextSize(SkIntToScalar(12));
    paint.setColor(color);
    
    SkScalar x = SkIntToScalar(20);
    SkScalar y = SkIntToScalar(100);
    const char* text = "Hamburgefons";
    size_t      len = strlen(text);

    // draw once to populate the cache
    canvas.drawText(text, len, x, y, paint);
    
    SkMSec now = SkTime::GetMSecs();
    for (int i = 0; i < TEXT_LOOP_COUNT; i++)
        canvas.drawText(text, len, x, y, paint);
    printf("----------- Config: %d, Color=%x, CPS = %g\n", config, color,
           len * TEXT_LOOP_COUNT * 1000.0 / (SkTime::GetMSecs() - now));
}

#endif

#ifdef SK_CAN_USE_FLOAT
#include "SkFloatBits.h"

static inline float fast_inc(float x) {
    SkFloatIntUnion data;
    data.fFloat = x;
    data.fSignBitInt += 1;
    return data.fFloat;
}

extern float dummy();
int time_math() {
    SkMSec now;
    int i;
    int sum = 0;
    const int repeat = 1000000;
    float f;

    f = dummy();
    now = SkTime::GetMSecs();
    for (i = repeat - 1; i >= 0; --i) {
        sum += (int)f; f = fast_inc(f);
        sum += (int)f; f = fast_inc(f);
        sum += (int)f; f = fast_inc(f);
        sum += (int)f; f = fast_inc(f);
    }
    SkDebugf("---- native cast %d\n", SkTime::GetMSecs() - now);

    f = dummy();
    now = SkTime::GetMSecs();
    for (i = repeat - 1; i >= 0; --i) {
        sum += SkFloatToIntCast(f); f = fast_inc(f);
        sum += SkFloatToIntCast(f); f = fast_inc(f);
        sum += SkFloatToIntCast(f); f = fast_inc(f);
        sum += SkFloatToIntCast(f); f = fast_inc(f);
    }
    SkDebugf("---- hack cast %d\n", SkTime::GetMSecs() - now);

    f = dummy();
    now = SkTime::GetMSecs();
    for (i = repeat - 1; i >= 0; --i) {
        sum += (int)floorf(f + 0.5f); f = fast_inc(f);
        sum += (int)floorf(f + 0.5f); f = fast_inc(f);
        sum += (int)floorf(f + 0.5f); f = fast_inc(f);
        sum += (int)floorf(f + 0.5f); f = fast_inc(f);
    }
    SkDebugf("---- native round %d\n", SkTime::GetMSecs() - now);
    
    f = dummy();
    now = SkTime::GetMSecs();
    for (i = repeat - 1; i >= 0; --i) {
        sum += SkFloatToIntRound(f); f = fast_inc(f);
        sum += SkFloatToIntRound(f); f = fast_inc(f);
        sum += SkFloatToIntRound(f); f = fast_inc(f);
        sum += SkFloatToIntRound(f); f = fast_inc(f);
    }
    SkDebugf("---- hack round %d\n", SkTime::GetMSecs() - now);
    
    f = dummy();
    now = SkTime::GetMSecs();
    for (i = repeat - 1; i >= 0; --i) {
        sum += SkFloat2Bits(floorf(f)); f = fast_inc(f);
        sum += SkFloat2Bits(floorf(f)); f = fast_inc(f);
        sum += SkFloat2Bits(floorf(f)); f = fast_inc(f);
        sum += SkFloat2Bits(floorf(f)); f = fast_inc(f);
    }
    SkDebugf("---- native floor %d\n", SkTime::GetMSecs() - now);
    
    f = dummy();
    now = SkTime::GetMSecs();
    for (i = repeat - 1; i >= 0; --i) {
        sum += SkFloatToIntFloor(f); f = fast_inc(f);
        sum += SkFloatToIntFloor(f); f = fast_inc(f);
        sum += SkFloatToIntFloor(f); f = fast_inc(f);
        sum += SkFloatToIntFloor(f); f = fast_inc(f);
    }
    SkDebugf("---- hack floor %d\n", SkTime::GetMSecs() - now);
    
    return sum;
}

#if 0
static float time_intToFloat() {
    const int repeat = 1000000;
    int i, n;
    SkMSec now;
    float sum = 0;
    
    n = (int)dummy();
    now = SkTime::GetMSecs();
    for (i = repeat - 1; i >= 0; --i) {
        sum += (float)n; n += 1;
        sum += (float)n; n += 1;
        sum += (float)n; n += 1;
        sum += (float)n; n += 1;
    }
    SkDebugf("---- native i2f %d\n", SkTime::GetMSecs() - now);
    
    n = (int)dummy();
    now = SkTime::GetMSecs();
    for (i = repeat - 1; i >= 0; --i) {
        sum += SkIntToFloatCast(n); n += 1;
        sum += SkIntToFloatCast(n); n += 1;
        sum += SkIntToFloatCast(n); n += 1;
        sum += SkIntToFloatCast(n); n += 1;
    }
    SkDebugf("---- check i2f %d\n", SkTime::GetMSecs() - now);

    n = (int)dummy();
    now = SkTime::GetMSecs();
    for (i = repeat - 1; i >= 0; --i) {
        sum += SkIntToFloatCast_NoOverflowCheck(n); n += 1;
        sum += SkIntToFloatCast_NoOverflowCheck(n); n += 1;
        sum += SkIntToFloatCast_NoOverflowCheck(n); n += 1;
        sum += SkIntToFloatCast_NoOverflowCheck(n); n += 1;
    }
    SkDebugf("---- nocheck i2f %d\n", SkTime::GetMSecs() - now);

    return sum;
}
#endif
#endif

void SkGraphics::Init()
{
    SkGlobals::Init();

#ifdef SK_CAN_USE_FLOAT
//    time_math();
//    time_intToFloat();
#endif
    
#ifdef BUILD_EMBOSS_TABLE
    SkEmbossMask_BuildTable();
#endif
#ifdef BUILD_RADIALGRADIENT_TABLE
    SkRadialGradient_BuildTable();
#endif

#ifdef SK_DEBUGx
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

        typesizeline(S8CPU),
        typesizeline(U8CPU),
        typesizeline(S16CPU),
        typesizeline(U16CPU),

        typesizeline(SkPoint),
        typesizeline(SkRect),
        typesizeline(SkMatrix),
        typesizeline(SkPath),
        typesizeline(SkGlyph),
        typesizeline(SkRefCnt),

        typesizeline(SkPaint),
        typesizeline(SkCanvas),
        typesizeline(SkBlitter),
        typesizeline(SkShader),
        typesizeline(SkXfermode),
        typesizeline(SkPathEffect)
    };

#ifdef SK_CPU_BENDIAN
    SkDebugf("SkGraphics: big-endian\n");
#else
    SkDebugf("SkGraphics: little-endian\n");
#endif

    {
        char    test = 0xFF;
        int     itest = test;   // promote to int, see if it sign-extended
        if (itest < 0)
            SkDebugf("SkGraphics: char is signed\n");
        else
            SkDebugf("SkGraphics: char is unsigned\n");
    }
    for (i = 0; i < (int)SK_ARRAY_COUNT(gTypeSize); i++) {
        SkDebugf("SkGraphics: sizeof(%s) = %d\n",
                 gTypeSize[i].fTypeName, gTypeSize[i].fSizeOf);
    }

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
    
#ifdef SPEED_TEST
    if (false) {
        int i;
        int (*proc)(int);

        static const struct {
            int (*proc)(int);
            const char* name;
        } gList[] = {
            { test_s64, "Sk64" },
            { test_native_64, "native" }
        };

        for (size_t j = 0; j < SK_ARRAY_COUNT(gList); j++) {
            SkMSec now = SkTime::GetMSecs();
            proc = gList[j].proc;
            for (i = 0; i < BIG_LOOP_COUNT; i++) {
                proc(i);
            }
            printf("-------- %s = %d\n", gList[j].name, SkTime::GetMSecs() - now);
        }
    }
#endif

    if (false) {
        size_t i, size = 480;
        char* buffer = (char*)sk_malloc_throw(size);
        uint16_t* buffer16 = (uint16_t*)buffer;
        uint32_t* buffer32 = (uint32_t*)buffer;

        SkMSec now = SkTime::GetMSecs();
        for (i = 0; i < 100000; i++) {
            sk_memset16(buffer16, (uint16_t)i, size >> 1);
        }
        SkMSec now2 = SkTime::GetMSecs();
        for (i = 0; i < 100000; i++) {
            sk_memset16_portable(buffer16, (uint16_t)i, size >> 1);
        }
        SkMSec now3 = SkTime::GetMSecs();
        printf("----------- memset16: native %d, portable %d\n", now2 - now, now3 - now2);

        now = SkTime::GetMSecs();
        for (i = 0; i < 100000; i++) {
            sk_memset32(buffer32, i, size >> 2);
        }
        now2 = SkTime::GetMSecs();
        for (i = 0; i < 100000; i++) {
            sk_memset32_portable(buffer32, i, size >> 2);
        }
        now3 = SkTime::GetMSecs();
        printf("----------- memset32: native %d, portable %d\n", now2 - now, now3 - now2);
        
        sk_free(buffer);
    }
    
#ifdef SPEED_TEST
    if (false) {
        test_drawText(SkBitmap::kARGB_8888_Config, SK_ColorBLACK);
        test_drawText(SkBitmap::kARGB_8888_Config, SK_ColorRED);
        test_drawText(SkBitmap::kRGB_565_Config, SK_ColorBLACK);
        test_drawText(SkBitmap::kRGB_565_Config, SK_ColorRED);
    }
#endif
    
//    if (true) {
//        test_sort();
//    }
}

////////////////////////////////////////////////////////////////////////////

#include "SkGlyphCache.h"

void SkGraphics::Term() {
    SkGraphics::SetFontCacheUsed(0);
    SkGlobals::Term();
}

size_t SkGraphics::GetFontCacheUsed() {
    return SkGlyphCache::GetCacheUsed();
}

bool SkGraphics::SetFontCacheUsed(size_t usageInBytes) {
    return SkGlyphCache::SetCacheUsed(usageInBytes);
}

float dummy() { return 1.25f; }
