#include "tests/Test.h"
#include "tools/Resources.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"

#include <utility>

// Return a pair of {size_to_crop, size_to_then_scale} based on original
// image size.
static std::pair<SkISize, SkISize> get_sizes(const sk_sp<SkImage>& image) {
    // For landscape images, as suggested:
    if (image->width() == 4032 && image->height() == 3024) {
        return {{4032,2268}, {1280,720}};
    }

    // For portrait, I'm just switching them.
    if (image->width() == 3024 && image->height() == 4032) {
        return {{2268, 4032}, {720, 1280}};
    }

    return {{0, 0}, {0, 0}};
}

static void save(SkISize origSize, SkISize scaledSize, const SkPaint& paint, SkString baseName,
        const char* qual, skiatest::Reporter* r) {
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::Make(scaledSize, kN32_SkColorType, kPremul_SkAlphaType));

    SkCanvas canvas(bm);
    SkRect srcRect = SkRect::Make(origSize),
           dstRect = SkRect::Make(scaledSize);
    auto matrix = SkMatrix::MakeRectToRect(srcRect, dstRect, SkMatrix::kFill_ScaleToFit);
    canvas.setMatrix(matrix);
    canvas.drawIRect(SkIRect::MakeSize(origSize), paint);

    baseName.append("_");
    baseName.append(qual);
    baseName.append(".png");
    SkFILEWStream stream(baseName.c_str());
    if (!SkEncodeImage(&stream, bm, SkEncodedImageFormat::kPNG, 100)) {
        ERRORF(r, "Failed to encode %s\n", baseName.c_str());
    }
}

SkString base_name(const char* name, skiatest::Reporter* r) {
    const char* start = strstr(name, "/");
    if (!start) {
        ERRORF(r, "missing / in %s\n", name);
        return SkString();
    }
    start++;
    const char* dot = strstr(start, ".");
    if (!dot) {
        ERRORF(r, "missing . in %s\n", name);
        return SkString(start);
    }
    return SkString(start, dot - start);
}

DEF_TEST(filtering, r) {
    for (auto* name : { "images/portrait.jpg",
                        "images/landscape.jpg",
                        "images/landscape2.jpg" }) {
        auto image = GetResourceAsImage(name);
        if (!image) {
            ERRORF(r, "Failed to get %s\n", name);
            continue;
        }

        auto [croppedSize, scaledSize] = get_sizes(image);
        if (croppedSize.isEmpty() || scaledSize.isEmpty()) {
            ERRORF(r, "Skipping image %s with unexpected size %i x %i\n",
                     name, image->width(), image->height());
            continue;
        }

        SkString baseName = base_name(name, r);

        auto cropped = image->makeSubset(SkIRect::MakeSize(croppedSize));
        image.reset();  // No longer need the original.
        if (!cropped) {
            ERRORF(r, "Failed to crop %s\n", name);
            continue;
        }

        SkPaint paint;
        auto no_filtering = cropped->makeShader(SkTileMode::kDecal, SkTileMode::kDecal,
                nullptr, kNone_SkFilterQuality);
        paint.setShader(std::move(no_filtering));
        save(croppedSize, croppedSize, paint, baseName, "cropped", r);

        auto nearest = cropped->makeShader(SkTileMode::kDecal, SkTileMode::kDecal,
                {SkSamplingMode::kNearest, SkMipmapMode::kNone});
        if (!nearest) {
            ERRORF(r, "Failed to make nearest shader from %s\n", name);
            continue;
        }
        paint.setShader(std::move(nearest));

        save(croppedSize, scaledSize, paint, baseName, "nearest", r);

        SkFilterOptions bilinearOptions {SkSamplingMode::kLinear, SkMipmapMode::kNone};
        auto bilinear = cropped->makeShader(SkTileMode::kDecal, SkTileMode::kDecal,
                                            bilinearOptions);
        if (!bilinear) {
            ERRORF(r, "Failed to make bilinear shader from %s\n", name);
            continue;
        }

        paint.setShader(std::move(bilinear));

        save(croppedSize, scaledSize, paint, baseName, "bilinear", r);

        using Cubic = std::pair<SkImage::CubicResampler, const char*>;
        for (Cubic resampler : {
                Cubic{{ 1.0f / 3.0f, 1.0f / 3.0f },  "Mitchell" },
                Cubic{{ 0.0f, .5f },                 "Catmull-Rom" },
        }){
            auto shader = cropped->makeShader(SkTileMode::kDecal, SkTileMode::kDecal,
                                              resampler.first);
            if (shader) {
                paint.setShader(std::move(shader));

                save(croppedSize, scaledSize, paint, baseName, resampler.second, r);
            } else {
                ERRORF(r, "Failed to create %s sampler for %s\n", resampler.second, name);
            }
        }
    }
}
