#include "Test.h"
#include "TestClassDef.h"

#include "SkPixelRef.h"
#include "SkMallocPixelRef.h"

static void test_info(skiatest::Reporter* reporter) {
    static const struct {
        SkBitmap::Config    fConfig;
        SkAlphaType         fAlphaType;
        SkColorType         fExpectedColorType;
        bool                fExpectedSuccess;
    } gRec[] = {
        { SkBitmap::kNo_Config,         kPremul_SkAlphaType,    kPMColor_SkColorType,   false },
        { SkBitmap::kARGB_8888_Config,  kPremul_SkAlphaType,    kPMColor_SkColorType,   true },
        { SkBitmap::kARGB_8888_Config,  kOpaque_SkAlphaType,    kPMColor_SkColorType,   true },
        { SkBitmap::kRGB_565_Config,    kOpaque_SkAlphaType,    kRGB_565_SkColorType,   true },
        { SkBitmap::kARGB_4444_Config,  kPremul_SkAlphaType,    kARGB_4444_SkColorType, true },
        { SkBitmap::kARGB_4444_Config,  kOpaque_SkAlphaType,    kARGB_4444_SkColorType, true },
        { SkBitmap::kA8_Config,         kPremul_SkAlphaType,    kAlpha_8_SkColorType,   true },
        { SkBitmap::kA8_Config,         kOpaque_SkAlphaType,    kAlpha_8_SkColorType,   true },
        { SkBitmap::kIndex8_Config,     kPremul_SkAlphaType,    kIndex_8_SkColorType,   true },
        { SkBitmap::kIndex8_Config,     kOpaque_SkAlphaType,    kIndex_8_SkColorType,   true },
    };

    SkBitmap bitmap;
    SkImageInfo info;

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        bool success = bitmap.setConfig(gRec[i].fConfig, 10, 10, 0, gRec[i].fAlphaType);
        REPORTER_ASSERT(reporter, success);
        success = bitmap.asImageInfo(&info);
        REPORTER_ASSERT(reporter, success == gRec[i].fExpectedSuccess);
        if (gRec[i].fExpectedSuccess) {
            REPORTER_ASSERT(reporter, info.fAlphaType == gRec[i].fAlphaType);
            REPORTER_ASSERT(reporter, info.fColorType == gRec[i].fExpectedColorType);
        }
    }
}

namespace {

class TestListener : public SkPixelRef::GenIDChangeListener {
public:
    explicit TestListener(int* ptr) : fPtr(ptr) {}
    void onChange() SK_OVERRIDE { (*fPtr)++; }
private:
    int* fPtr;
};

}  // namespace

DEF_TEST(PixelRef_GenIDChange, r) {
    SkImageInfo info = { 10, 10, kPMColor_SkColorType, kPremul_SkAlphaType };

    SkAutoTUnref<SkPixelRef> pixelRef(SkMallocPixelRef::NewAllocate(info, 0, NULL));

    // Register a listener.
    int count = 0;
    pixelRef->addGenIDChangeListener(SkNEW_ARGS(TestListener, (&count)));
    REPORTER_ASSERT(r, 0 == count);

    // No one has looked at our pixelRef's generation ID, so invalidating it doesn't make sense.
    // (An SkPixelRef tree falls in the forest but there's nobody around to hear it.  Do we care?)
    pixelRef->notifyPixelsChanged();
    REPORTER_ASSERT(r, 0 == count);

    // Force the generation ID to be calculated.
    REPORTER_ASSERT(r, 0 != pixelRef->getGenerationID());

    // Our listener was dropped in the first call to notifyPixelsChanged().  This is a no-op.
    pixelRef->notifyPixelsChanged();
    REPORTER_ASSERT(r, 0 == count);

    // Force the generation ID to be recalculated, then add a listener.
    REPORTER_ASSERT(r, 0 != pixelRef->getGenerationID());
    pixelRef->addGenIDChangeListener(SkNEW_ARGS(TestListener, (&count)));
    pixelRef->notifyPixelsChanged();
    REPORTER_ASSERT(r, 1 == count);

    // Quick check that NULL is safe.
    REPORTER_ASSERT(r, 0 != pixelRef->getGenerationID());
    pixelRef->addGenIDChangeListener(NULL);
    pixelRef->notifyPixelsChanged();

    test_info(r);
}
