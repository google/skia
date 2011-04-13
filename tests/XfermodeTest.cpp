#include "Test.h"
#include "SkColor.h"
#include "SkXfermode.h"

SkPMColor bogusXfermodeProc(SkPMColor src, SkPMColor dst) {
    return 42;
}

#define ILLEGAL_MODE    ((SkXfermode::Mode)-1)

static void test_asMode(skiatest::Reporter* reporter) {
    for (int mode = 0; mode <= SkXfermode::kLastMode; mode++) {
        SkXfermode* xfer = SkXfermode::Create((SkXfermode::Mode) mode);

        SkXfermode::Mode reportedMode = ILLEGAL_MODE;
        REPORTER_ASSERT(reporter, reportedMode != mode);

        // test IsMode
        REPORTER_ASSERT(reporter, SkXfermode::IsMode(xfer, &reportedMode));
        REPORTER_ASSERT(reporter, reportedMode == mode);

        // repeat that test, but with asMode instead
        if (xfer) {
            reportedMode = (SkXfermode::Mode) -1;
            REPORTER_ASSERT(reporter, xfer->asMode(&reportedMode));
            REPORTER_ASSERT(reporter, reportedMode == mode);
            xfer->unref();
        } else {
            REPORTER_ASSERT(reporter, SkXfermode::kSrcOver_Mode == mode); 
        }
    }

    SkXfermode* bogusXfer = new SkProcXfermode(bogusXfermodeProc);
    SkXfermode::Mode reportedMode = (SkXfermode::Mode) -1;
    REPORTER_ASSERT(reporter, !bogusXfer->asMode(&reportedMode));
    REPORTER_ASSERT(reporter, reportedMode == -1);
    REPORTER_ASSERT(reporter, !SkXfermode::IsMode(bogusXfer, &reportedMode));
    REPORTER_ASSERT(reporter, reportedMode == -1);
    bogusXfer->unref();
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Xfermode", XfermodeTestClass, test_asMode)
