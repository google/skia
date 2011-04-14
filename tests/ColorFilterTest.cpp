#include "Test.h"
#include "SkColor.h"
#include "SkColorFilter.h"
#include "SkRandom.h"
#include "SkXfermode.h"

static SkFlattenable* reincarnate_flattenable(SkFlattenable* obj) {
    SkFlattenable::Factory fact = obj->getFactory();
    if (NULL == fact) {
        return NULL;
    }

    SkFlattenableWriteBuffer wb(1024);
    obj->flatten(wb);

    size_t size = wb.size();
    SkAutoSMalloc<1024> storage(size);
    // make a copy into storage
    wb.flatten(storage.get());

    SkFlattenableReadBuffer rb(storage.get(), size);
    return fact(rb);
}

template <typename T> T* reincarnate(T* obj) {
    return (T*)reincarnate_flattenable(obj);
}

///////////////////////////////////////////////////////////////////////////////

#define ILLEGAL_MODE    ((SkXfermode::Mode)-1)

static void test_asColorMode(skiatest::Reporter* reporter) {
    SkRandom rand;

    for (int mode = 0; mode <= SkXfermode::kLastMode; mode++) {
        SkColor color = rand.nextU();

        // ensure we always get a filter, by avoiding the possibility of a
        // special case that would return NULL (if color's alpha is 0 or 0xFF)
        color = SkColorSetA(color, 0x7F);

        SkColorFilter* cf = SkColorFilter::CreateModeFilter(color,
                                                        (SkXfermode::Mode)mode);

        // allow for no filter if we're in Dst mode (its a no op)
        if (SkXfermode::kDst_Mode == mode && NULL == cf) {
            continue;
        }

        SkAutoUnref aur(cf);
        REPORTER_ASSERT(reporter, cf);

        SkColor c = ~color;
        SkXfermode::Mode m = ILLEGAL_MODE;

        SkColor expectedColor = color;
        SkXfermode::Mode expectedMode = (SkXfermode::Mode)mode;

//        SkDebugf("--- mc [%d %x] ", mode, color);

        REPORTER_ASSERT(reporter, cf->asColorMode(&c, &m));
        // handle special-case folding by the factory
        if (SkXfermode::kClear_Mode == mode) {
            if (c != expectedColor) {
                expectedColor = 0;
            }
            if (m != expectedMode) {
                expectedMode = SkXfermode::kSrc_Mode;
            }
        } 

//        SkDebugf("--- got [%d %x] expected [%d %x]\n", m, c, expectedMode, expectedColor);

        REPORTER_ASSERT(reporter, c == expectedColor);
        REPORTER_ASSERT(reporter, m == expectedMode);
        
        {
            SkColorFilter* cf2 = reincarnate(cf);
            SkAutoUnref aur2(cf2);
            REPORTER_ASSERT(reporter, cf2);

            SkColor c2 = ~color;
            SkXfermode::Mode m2 = ILLEGAL_MODE;
            REPORTER_ASSERT(reporter, cf2->asColorMode(&c2, &m2));
            REPORTER_ASSERT(reporter, c2 == expectedColor);
            REPORTER_ASSERT(reporter, m2 == expectedMode);
        }
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("ColorFilter", ColorFilterTestClass, test_asColorMode)
