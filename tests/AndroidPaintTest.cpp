// SkPaints only have an SkPaintOptionsAndroid if SK_BUILD_FOR_ANDROID is true.
#ifdef SK_BUILD_FOR_ANDROID

#include "SkPaintOptionsAndroid.h"
#include "SkOrderedReadBuffer.h"
#include "SkOrderedWriteBuffer.h"
#include "SkPaint.h"
#include "Test.h"
#include "TestClassDef.h"

static size_t Reconstruct(const SkPaint& src, SkPaint* dst) {
    SkOrderedWriteBuffer writer(64 /*arbitrary*/);
    src.flatten(writer);

    const size_t size = writer.bytesWritten();
    SkAutoMalloc bytes(size);
    writer.writeToMemory(bytes.get());

    SkOrderedReadBuffer reader(bytes.get(), size);
    dst->unflatten(reader);

    return size;
}

static void android_options_serialization(skiatest::Reporter* reporter) {
    // We want to make sure that Android's paint options survive a flatten/unflatten round trip.
    // These are all non-default options.
    SkPaintOptionsAndroid options;
    options.setLanguage("ja-JP");
    options.setFontVariant(SkPaintOptionsAndroid::kElegant_Variant);
    options.setUseFontFallbacks(true);

    SkPaint paint;
    paint.setPaintOptionsAndroid(options);

    SkPaint reconstructed;
    Reconstruct(paint, &reconstructed);

    REPORTER_ASSERT(reporter, options == reconstructed.getPaintOptionsAndroid());
}
DEFINE_TESTCLASS_SHORT(android_options_serialization);

static void android_options_serialization_reverse(skiatest::Reporter* reporter) {
    // Opposite test of above: make sure the serialized default values of a paint overwrite
    // non-default values on the paint we're unflattening into.
    const SkPaint defaultOptions;

    SkPaintOptionsAndroid options;
    options.setLanguage("ja-JP");
    options.setFontVariant(SkPaintOptionsAndroid::kElegant_Variant);
    options.setUseFontFallbacks(true);
    SkPaint nonDefaultOptions;
    nonDefaultOptions.setPaintOptionsAndroid(options);

    Reconstruct(defaultOptions, &nonDefaultOptions);

    REPORTER_ASSERT(reporter,
            defaultOptions.getPaintOptionsAndroid() ==
            nonDefaultOptions.getPaintOptionsAndroid());
}
DEFINE_TESTCLASS_SHORT(android_options_serialization_reverse);

static void android_options_size(skiatest::Reporter* reporter) {
    // A paint with default android options should serialize to something smaller than
    // a paint with non-default android options.

    SkPaint defaultOptions;

    SkPaintOptionsAndroid options;
    options.setUseFontFallbacks(true);
    SkPaint nonDefaultOptions;
    nonDefaultOptions.setPaintOptionsAndroid(options);

    SkPaint dummy;

    REPORTER_ASSERT(reporter,
                    Reconstruct(defaultOptions, &dummy) < Reconstruct(nonDefaultOptions, &dummy));
}
DEFINE_TESTCLASS_SHORT(android_options_size);

#endif  // SK_BUILD_FOR_ANDROID
