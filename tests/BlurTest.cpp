#include "Test.h"
#include "SkBlurMaskFilter.h"
#include "SkRandom.h"

///////////////////////////////////////////////////////////////////////////////

#define ILLEGAL_MODE    ((SkXfermode::Mode)-1)

static void test_blur(skiatest::Reporter* reporter) {
    SkScalar radius = SkIntToScalar(2);

    for (int i = 0; i < SkBlurMaskFilter::kBlurStyleCount; ++i) {
        SkMaskFilter* filter;
        SkMaskFilter::BlurInfo info;

        uint32_t flags = i & 3;

        filter = SkBlurMaskFilter::Create(radius, (SkBlurMaskFilter::BlurStyle)i,
                                          flags);

        sk_bzero(&info, sizeof(info));
        SkMaskFilter::BlurType type = filter->asABlur(&info);
        REPORTER_ASSERT(reporter, type == (SkMaskFilter::BlurType)(i + 1));
        REPORTER_ASSERT(reporter, info.fRadius == radius);
        REPORTER_ASSERT(reporter, info.fIgnoreTransform ==
                        SkToBool(flags & SkBlurMaskFilter::kIgnoreTransform_BlurFlag));
        REPORTER_ASSERT(reporter, info.fHighQuality ==
                        SkToBool(flags & SkBlurMaskFilter::kHighQuality_BlurFlag));

        filter->unref();
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("BlurMaskFilter", BlurTestClass, test_blur)
