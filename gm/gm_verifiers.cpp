/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm_verifiers.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkImageFilters.h"

/** Checks the given VerifierResult. If it is not ok, returns it. */
#define RETURN_NOT_OK(res)  if (!(res).ok()) return (res)

using namespace skiagm;

VerifierResult::VerifierResult() : VerifierResult(Code::kOk, SkString("Ok")) {}

VerifierResult::VerifierResult(VerifierResult::Code code, const SkString& msg)
    : fCode(code), fMessage(msg) {}

bool VerifierResult::ok() const {
    return fCode == Code::kOk;
}

const SkString& VerifierResult::message() const {
    return fMessage;
}

VerifierResult VerifierResult::Ok() {
    return VerifierResult(Code::kOk, SkString("Ok"));
}

VerifierResult VerifierResult::Fail(const SkString& msg) {
    return VerifierResult(Code::kFail, msg);
}

void RelaxNearEdges::init(const SkBitmap& bmp) {
    fBitmap.reset(new SkBitmap);
    edgeDetect(bmp, fBitmap.get());
}

bool RelaxNearEdges::relaxPixel(int x, int y) {
    // return true if the pixel is near an edge.

    const SkIRect bounds = fBitmap->bounds();
    const int minX = std::max(x - kPixelRadius, bounds.fLeft),
        maxX = std::min(x + kPixelRadius, bounds.fRight - 1),
        minY = std::max(y - kPixelRadius, bounds.fTop),
        maxY = std::min(y + kPixelRadius, bounds.fBottom - 1);

    for (int i = minY; i <= maxY; i++) {
        for (int j = minX; j <= maxX; j++) {
            if (fBitmap->getColor(j, i) != SK_ColorBLACK) {
                return true;
            }
        }
    }

    return false;
}

void RelaxNearEdges::edgeDetect(const SkBitmap& input_bmp, SkBitmap* result_bmp) {
    const auto make_filter = [&input_bmp](const SkScalar* kernel) {
        return SkImageFilters::MatrixConvolution(SkISize::Make(3, 3),
                                                 kernel,
                                                 1,
                                                 0,
                                                 {1, 1},
                                                 SkTileMode::kRepeat,
                                                 false,
                                                 SkImageFilters::Image(SkImage::MakeFromBitmap(
                                                     input_bmp)));
    };

    SkBitmap img_x;
    img_x.allocPixelsFlags(input_bmp.info(), SkBitmap::kZeroPixels_AllocFlag);
    {
        const SkScalar k_gx[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
        SkCanvas canvas(img_x);
        SkPaint paint;
        paint.setImageFilter(make_filter(k_gx));
        canvas.drawBitmap(input_bmp, 0, 0, &paint);
    }

    SkBitmap img_y;
    img_y.allocPixelsFlags(input_bmp.info(), SkBitmap::kZeroPixels_AllocFlag);
    {
        const SkScalar k_gy[] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
        SkCanvas canvas(img_y);
        SkPaint paint;
        paint.setImageFilter(make_filter(k_gy));
        canvas.drawBitmap(input_bmp, 0, 0, &paint);
    }

    result_bmp->allocPixelsFlags(input_bmp.info(), SkBitmap::kZeroPixels_AllocFlag);
    for (int y = 0; y < result_bmp->height(); y++) {
        for (int x = 0; x < result_bmp->width(); x++) {
            const SkColor cx = img_x.getColor(x, y), cy = img_y.getColor(x, y);
            const auto r = (uint32_t)sqrt(
                (double)SkColorGetR(cx) * SkColorGetR(cx) + SkColorGetR(cy) * SkColorGetR(cy));
            const auto g = (uint32_t)sqrt(
                (double)SkColorGetG(cx) * SkColorGetG(cx) + SkColorGetG(cy) * SkColorGetG(cy));
            const auto b = (uint32_t)sqrt(
                (double)SkColorGetB(cx) * SkColorGetB(cx) + SkColorGetB(cy) * SkColorGetB(cy));
            const uint8_t m = (uint8_t)std::min(std::max(r, std::max(g, b)), (uint32_t)0xff);
            SkColor c = SkColorSetARGB(0xff, m, m, m);
            result_bmp->erase(c, SkIRect::MakeLTRB(x, y, x + 1, y + 1));
        }
    }
}

GMVerifier::GMVerifier() : fHighlightChangesIfPossible(false) {}

GMVerifier::~GMVerifier() {}

VerifierResult GMVerifier::verify(const SkBitmap& gold, const SkBitmap& actual) {
    if (fRelaxation) {
        fRelaxation->init(gold);
    }

    return verifyImpl(gold, actual);
}

void GMVerifier::setRelaxation(std::unique_ptr<VerifierRelaxation> relax) {
    fRelaxation = std::move(relax);
}

const SkBitmap* GMVerifier::goldImg() const {
    return fCurrentGoldImg.get();
}

const SkBitmap* GMVerifier::imgUnderTest() const {
    return fCurrentImgUnderTest.get();
}

std::unique_ptr<SkBitmap> GMVerifier::duplicate(const SkBitmap& bmp) {
    const SkISize input_size = bmp.info().bounds().size();
    const SkImageInfo image_info = bmp.info();

    std::unique_ptr<SkBitmap> result(new SkBitmap);
    result->allocPixelsFlags(image_info, SkBitmap::kZeroPixels_AllocFlag);

    const SkPixmap& pixmap = bmp.pixmap();
    for (int y = 0; y < input_size.fHeight; y++) {
        for (int x = 0; x < input_size.fWidth; x++) {
            *result->getAddr32(x, y) = *pixmap.addr32(x, y);
        }
    }

    return result;
}

std::unique_ptr<SkBitmap> GMVerifier::mask(const SkBitmap& bmp) {
    const SkISize input_size = bmp.info().bounds().size();
    const SkImageInfo
        image_info = SkImageInfo::Make(input_size, bmp.colorType(), bmp.alphaType(), nullptr);

    std::unique_ptr<SkBitmap> result(new SkBitmap);
    result->allocPixelsFlags(image_info, SkBitmap::kZeroPixels_AllocFlag);

    SkCanvas canvas(*result);
    canvas.clear(SK_ColorWHITE);
    for (int y = 0; y < input_size.fHeight; y++) {
        for (int x = 0; x < input_size.fWidth; x++) {
            SkColor c = bmp.getColor(x, y);
            if (c != SK_ColorWHITE && SkColorGetA(c) > 0) {
                result->erase(SK_ColorBLACK, SkIRect::MakeLTRB(x, y, x + 1, y + 1));
            }
        }
    }

    return result;
}

void GMVerifier::addRegion(const SkIRect& rect) {
    fRegions.emplace_back(rect);
}

bool GMVerifier::relaxPixel(int x, int y) const {
    if (fRelaxation) {
        return fRelaxation->relaxPixel(x, y);
    } else {
        return false;
    }
}

VerifierResult GMVerifier::makeError(const SkString& msg) const {
    return VerifierResult::Fail(SkStringPrintf("[%s] %s", name().c_str(), msg.c_str()));
}

SkString GMVerifier::toString(const SkIRect& r) {
    return SkStringPrintf("%d, %d, %d, %d", r.fLeft, r.fTop, r.fRight, r.fBottom);
}

SkString GMVerifier::toString(const SkColor& c) {
    return SkStringPrintf("ARGB 0x%0.2x%0.2x%0.2x%.02x",
                          SkColorGetA(c),
                          SkColorGetR(c),
                          SkColorGetG(c),
                          SkColorGetB(c));
}

bool GMVerifier::colorInNeighborhood(const SkBitmap& bitmap,
                                     int x,
                                     int y,
                                     SkColor color,
                                     int n,
                                     uint32_t dist) {
    const SkIRect bounds = bitmap.bounds();
    const int minX = std::max(x - n, bounds.fLeft),
        maxX = std::min(x + n, bounds.fRight - 1),
        minY = std::max(y - n, bounds.fTop),
        maxY = std::min(y + n, bounds.fBottom - 1);

    for (int i = minY; i <= maxY; i++) {
        for (int j = minX; j <= maxX; j++) {
            if (maxChannelDist(color, bitmap.getColor(j, i)) <= dist) {
                return true;
            }
        }
    }

    return false;
}

uint32_t GMVerifier::colorDist(SkColor a, SkColor b) {
    const auto abs_delta = [](uint8_t x, uint8_t y) {
        return x < y ? y - x : x - y;
    };

    const uint8_t a_a = SkColorGetA(a),
        a_r = SkColorGetR(a),
        a_g = SkColorGetG(a),
        a_b = SkColorGetB(a);
    const uint8_t b_a = SkColorGetA(b),
        b_r = SkColorGetR(b),
        b_g = SkColorGetG(b),
        b_b = SkColorGetB(b);

    const uint32_t da = abs_delta(a_a, b_a);
    const uint32_t dr = abs_delta(a_r, b_r);
    const uint32_t dg = abs_delta(a_g, b_g);
    const uint32_t db = abs_delta(a_b, b_b);

    return da + dr + dg + db;
}

uint32_t GMVerifier::maxChannelDist(SkColor a, SkColor b) {
    const auto abs_delta = [](uint8_t x, uint8_t y) {
        return x < y ? y - x : x - y;
    };

    const uint8_t a_a = SkColorGetA(a),
        a_r = SkColorGetR(a),
        a_g = SkColorGetG(a),
        a_b = SkColorGetB(a);
    const uint8_t b_a = SkColorGetA(b),
        b_r = SkColorGetR(b),
        b_g = SkColorGetG(b),
        b_b = SkColorGetB(b);

    const uint32_t da = abs_delta(a_a, b_a);
    const uint32_t dr = abs_delta(a_r, b_r);
    const uint32_t dg = abs_delta(a_g, b_g);
    const uint32_t db = abs_delta(a_b, b_b);

    return std::max(std::max(std::max(da, dr), dg), db);
}

SkString ExactPixelMatch::name() const {
    return SkString("ExactPixelMatch");
}

VerifierResult ExactPixelMatch::verifyImpl(const SkBitmap& gold, const SkBitmap& actual) {
    fCurrentGoldImg = duplicate(gold);
    fCurrentImgUnderTest = duplicate(actual);
    return bitmapsEqual(gold, actual);
}

VerifierResult ExactPixelMatch::bitmapsEqual(const SkBitmap& a, const SkBitmap& b) {
    const auto checkRegion = [this, &a, &b](const SkIRect& region) {
        if (!a.bounds().contains(region) || !b.bounds().contains(region)) {
            SkString msg = SkStringPrintf("region (%s) exceeds bitmap bounds ((%s) or (%s))",
                                          toString(region).c_str(),
                                          toString(a.bounds()).c_str(),
                                          toString(b.bounds()).c_str());
            return makeError(msg);
        }

        for (int y = region.fTop; y < region.fBottom; y++) {
            for (int x = region.fLeft; x < region.fRight; x++) {
                if (a.getColor(x, y) != b.getColor(x, y) && !relaxPixel(x, y)) {
                    SkString
                        msg = SkStringPrintf("expected pixel %d,%d to be %s, got %s (dist = %d)",
                                             x,
                                             y,
                                             toString(a.getColor(x, y)).c_str(),
                                             toString(b.getColor(x, y)).c_str(),
                                             colorDist(a.getColor(x, y), b.getColor(x, y)));
                    return makeError(msg);
                }
            }
        }

        return VerifierResult::Ok();
    };

    if (fRegions.empty()) {
        RETURN_NOT_OK(checkRegion(a.bounds()));
    } else {
        for (const SkIRect& region : fRegions) {
            RETURN_NOT_OK(checkRegion(region));
        }
    }

    return VerifierResult::Ok();
}

SkString CompareToMask::name() const {
    return SkString("CompareToMask");
}

VerifierResult CompareToMask::verifyImpl(const SkBitmap& gold, const SkBitmap& actual) {
    fCurrentGoldImg = mask(gold);
    fCurrentImgUnderTest = duplicate(actual);
    return pixelsAreInMask(*fCurrentImgUnderTest, *fCurrentGoldImg);
}

VerifierResult CompareToMask::pixelsAreInMask(SkBitmap& a, SkBitmap& mask) {
    const auto checkRegion = [this, &a, &mask](const SkIRect& region) {
        uint32_t num_failed_pixels = 0;
        for (int y = region.fTop; y < region.fBottom; y++) {
            for (int x = region.fLeft; x < region.fRight; x++) {
                const SkColor c = a.getColor(x, y);
                const uint32_t distance_from_bg = colorDist(c, kBackgroundColor);
                if (distance_from_bg > kBackgroundColorDistanceThreshold &&
                    !colorInNeighborhood(mask, x, y, SK_ColorBLACK) && !relaxPixel(x, y)) {
                    // Test image drew a pixel that gold image did not (approximately).
                    num_failed_pixels++;
                    if (fHighlightChangesIfPossible) {
                        a.erase(SK_ColorRED, SkIRect::MakeLTRB(x, y, x + 1, y + 1));
                    }
                }
            }
        }

        return num_failed_pixels;
    };

    uint32_t num_failed_pixels = 0, failed_pixel_threshold = 0;
    if (fRegions.empty()) {
        num_failed_pixels = checkRegion(a.bounds());
        failed_pixel_threshold =
            static_cast<uint32_t>(kPercentPixelDifferenceThreshold * a.bounds().size().area());
    } else {
        for (const SkIRect& region : fRegions) {
            num_failed_pixels += checkRegion(region);
            failed_pixel_threshold +=
                static_cast<uint32_t>(kPercentPixelDifferenceThreshold * region.size().area());
        }
    }

    return num_failed_pixels < failed_pixel_threshold ? VerifierResult::Ok()
                                                      : makeError(SkStringPrintf(
            "%d failed pixels is above threshold of %d",
            num_failed_pixels,
            failed_pixel_threshold));
}

SkString CheckPixelColorNearby::name() const {
    return SkString("CheckPixelColorNearby");
}

VerifierResult CheckPixelColorNearby::verifyImpl(const SkBitmap& gold, const SkBitmap& actual) {
    fCurrentGoldImg = duplicate(gold);
    fCurrentImgUnderTest = duplicate(actual);
    return pixelColorsAreNearby(*fCurrentGoldImg, *fCurrentImgUnderTest);
}

VerifierResult CheckPixelColorNearby::pixelColorsAreNearby(SkBitmap& gold, SkBitmap& actual) {
    const auto checkRegion = [this, &gold, &actual](const SkIRect& region) {
        uint32_t num_failed_pixels = 0;
        for (int y = region.fTop; y < region.fBottom; y++) {
            for (int x = region.fLeft; x < region.fRight; x++) {
                const SkColor c = actual.getColor(x, y);
                if (!colorInNeighborhood(gold,
                                         x,
                                         y,
                                         c,
                                         kNeighborhoodSize,
                                         kColorDistanceThreshold) &&
                    !relaxPixel(x, y)) {
                    // Test image drew a pixel that gold image did not (approximately).
                    num_failed_pixels++;
                    if (fHighlightChangesIfPossible) {
                        actual.erase(SK_ColorRED, SkIRect::MakeLTRB(x, y, x + 1, y + 1));
                    }
                }
            }
        }

        return num_failed_pixels;
    };

    uint32_t num_failed_pixels = 0, failed_pixel_threshold = 0;
    if (fRegions.empty()) {
        num_failed_pixels = checkRegion(actual.bounds());
        failed_pixel_threshold =
            static_cast<uint32_t>(kPercentPixelDifferenceThreshold * actual.bounds().size().area());
    } else {
        for (const SkIRect& region : fRegions) {
            num_failed_pixels += checkRegion(region);
            failed_pixel_threshold +=
                static_cast<uint32_t>(kPercentPixelDifferenceThreshold * region.size().area());
        }
    }

    return num_failed_pixels < failed_pixel_threshold ? VerifierResult::Ok()
                                                      : makeError(SkStringPrintf(
            "%d failed pixels is above threshold of %d",
            num_failed_pixels,
            failed_pixel_threshold));
}

CheckNoPixelsOutsideRegion::CheckNoPixelsOutsideRegion() : fInverted(false) {
}

SkString CheckNoPixelsOutsideRegion::name() const {
    return SkString("CheckNoPixelsOutsideRegion");
}

VerifierResult CheckNoPixelsOutsideRegion::verifyImpl(const SkBitmap& gold,
                                                      const SkBitmap& actual) {
    fCurrentGoldImg = duplicate(gold);
    fCurrentImgUnderTest = duplicate(actual);

    if (fRegions.empty()) {
        RETURN_NOT_OK(checkRegion(*fCurrentImgUnderTest, actual.bounds()));
    } else {
        for (const SkIRect& region : fRegions) {
            RETURN_NOT_OK(checkRegion(*fCurrentImgUnderTest, region));
        }
    }

    return VerifierResult::Ok();
}

void CheckNoPixelsOutsideRegion::toggleInverted() {
    fInverted = !fInverted;
}

VerifierResult CheckNoPixelsOutsideRegion::checkRegion(SkBitmap& actual, const SkIRect& region) {
    uint32_t num_failed_pixels = 0;
    const auto fail_pixel = [this, &actual, &num_failed_pixels](int x, int y) {
        num_failed_pixels++;
        if (fHighlightChangesIfPossible) {
            actual.erase(SK_ColorRED, SkIRect::MakeLTRB(x, y, x + 1, y + 1));
        }
    };

    if (fInverted) {
        // No pixels should appear within the region.
        for (int y = region.fTop; y < region.fBottom; y++) {
            for (int x = region.fLeft; x < region.fRight; x++) {
                const SkColor c = actual.getColor(x, y);
                if (c != kBackgroundColor && !relaxPixel(x, y)) {
                    fail_pixel(x, y);
                }
            }
        }
    } else {
        // No pixels should appear outside the region.
        for (int y = 0; y < actual.bounds().fBottom; y++) {
            for (int x = 0; x < actual.bounds().fRight; x++) {
                const SkColor c = actual.getColor(x, y);
                if (!region.contains(x, y) && c != kBackgroundColor && !relaxPixel(x, y)) {
                    fail_pixel(x, y);
                }
            }
        }
    }

    if (num_failed_pixels == 0) {
        return VerifierResult::Ok();
    } else {
        return makeError(SkStringPrintf("%d pixels drawn %s region (%s)",
                                        num_failed_pixels,
                                        fInverted ? "inside" : "outside",
                                        toString(region).c_str()));
    }
}

void GMVerifiers::add(std::unique_ptr<GMVerifier> verifier) {
    fVerifiers.push_back(std::move(verifier));
}

VerifierResult GMVerifiers::verify(const SkBitmap& gold, const SkBitmap& actual) {
    fFailedVerifier = nullptr;
    for (const auto& v : fVerifiers) {
        fFailedVerifier = v.get();
        RETURN_NOT_OK(v->verify(gold, actual));
    }
    return VerifierResult::Ok();
}

const SkBitmap* GMVerifiers::goldImg() const {
    return fFailedVerifier ? fFailedVerifier->goldImg() : nullptr;
}

const SkBitmap* GMVerifiers::imgUnderTest() const {
    return fFailedVerifier ? fFailedVerifier->imgUnderTest() : nullptr;
}
