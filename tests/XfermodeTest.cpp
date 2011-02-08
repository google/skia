#include "Test.h"
#include "SkColor.h"
#include "SkXfermode.h"

SkPMColor bogusXfermodeProc(SkPMColor src, SkPMColor dst) {
    return 42;
}

static void test_asMode(skiatest::Reporter* reporter) {
    for (int mode = 0; mode <= SkXfermode::kLastMode; mode++) {
        SkXfermode* xfer = SkXfermode::Create((SkXfermode::Mode) mode);
        SkXfermode::Mode reportedMode = (SkXfermode::Mode) -1;
        REPORTER_ASSERT(reporter,
                        xfer != NULL || mode == SkXfermode::kSrcOver_Mode);
        if (xfer) {
          REPORTER_ASSERT(reporter, xfer->asMode(&reportedMode));
          REPORTER_ASSERT(reporter, reportedMode == mode);
          xfer->unref();
        }
    }

    SkXfermode* bogusXfer = new SkProcXfermode(bogusXfermodeProc);
    SkXfermode::Mode reportedMode = (SkXfermode::Mode) -1;
    REPORTER_ASSERT(reporter, !bogusXfer->asMode(&reportedMode));
    REPORTER_ASSERT(reporter, reportedMode == -1);
    bogusXfer->unref();
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Xfermode", XfermodeTestClass, test_asMode)
