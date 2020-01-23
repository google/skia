/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "gm/gm_verifiers.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/encode/SkPngEncoder.h"
#include "include/private/SkTHash.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTaskGroup.h"
#include "src/utils/SkOSPath.h"
#include "tools/Resources.h"
#include "tools/flags/CommandLineFlags.h"

#include <iostream>

static DEFINE_bool2(quiet, q, false, "if true, don't print status updates.");

static DEFINE_string(inputDir, "",
                     "Path to a directory containing one directory per test name. Each test-named "
                     "directory can contain a set of images. Each image will be verified against "
                     "the \"golden\" verifier-produced image.");

static DEFINE_string(writePath, "",
                     "Path to a directory containing one directory per test name. Each test-named "
                     "directory can contain a set of images. Each image will be verified against "
                     "the \"golden\" verifier-produced image.");

struct GmResults {
    int numTested = 0;
    int numPassed = 0;
};

template<typename... Args>
static void info(const char* fmt, Args&& ... args) {
    if (!FLAGS_quiet) {
        printf(fmt, args...);
    }
}

static void info(const char* fmt) {
    if (!FLAGS_quiet) {
        printf("%s", fmt);  // Clang warns printf(fmt) is insecure.
    }
}

static SkBitmap draw_golden_image(std::unique_ptr<skiagm::GM>& gm) {
    const SkColorType colorType = kN32_SkColorType;
    const SkISize size = gm->getISize();

    // If there's an appropriate alpha type for this color type, use it, otherwise use premul.
    SkAlphaType alphaType = kPremul_SkAlphaType;
    (void)SkColorTypeValidateAlphaType(colorType, alphaType, &alphaType);

    SkBitmap gold_bmp;
    gold_bmp.allocPixelsFlags(SkImageInfo::Make(size, colorType, alphaType, nullptr),
                              SkBitmap::kZeroPixels_AllocFlag);

    SkCanvas canvas(gold_bmp);
    gm->draw(&canvas);
    return gold_bmp;
}

static SkTArray<std::pair<SkBitmap,
                          SkString>> read_test_images(const std::unique_ptr<skiagm::GM>& gm) {
    SkTArray<std::pair<SkBitmap, SkString>> result;

    const SkString dirPath = SkOSPath::Join(FLAGS_inputDir[0], gm->getName());
    SkOSFile::Iter it(dirPath.c_str(), nullptr);
    for (SkString file; it.next(&file);) {
        const SkString filePath = SkOSPath::Join(dirPath.c_str(), file.c_str());
        if (auto codec = SkCodec::MakeFromData(SkData::MakeFromFileName(filePath.c_str()))) {
            SkBitmap bmp;
            bmp.allocN32Pixels(codec->dimensions().fWidth, codec->dimensions().fHeight);
            if (SkCodec::kSuccess == codec->getPixels(bmp.pixmap())) {
                result.push_back({bmp, file});
            } else {
                info("bad file: %s\n", filePath.c_str());
            }
        } else {
            info("bad file: %s\n", filePath.c_str());
        }
    }

    return result;
}

static void write_png(const SkBitmap& bmp, const SkString& path) {
    SkPixmap pm;
    SkAssertResult(bmp.peekPixels(&pm));

    SkFILEWStream fileStream(path.c_str());
    SkASSERT(fileStream.isValid());

    SkPngEncoder::Options options;
    options.fZLibLevel = 0;
    options.fFilterFlags = SkPngEncoder::FilterFlag::kNone;
    SkPngEncoder::Encode(&fileStream, pm, options);
    fileStream.flush();
    info("wrote png to %s\n", path.c_str());
}

static void write_pngs(const std::unique_ptr<skiagm::GM>& gm,
                       const SkBitmap* gold,
                       const SkBitmap* actual,
                       const SkString& actualFilename) {
    if (FLAGS_writePath.isEmpty()) {
        return;
    } else if (!sk_isdir(FLAGS_writePath[0])) {
        sk_mkdir(FLAGS_writePath[0]);
    }

    const SkString dirPath = SkOSPath::Join(FLAGS_writePath[0], gm->getName());
    const SkString goldPath = SkOSPath::Join(dirPath.c_str(), "gold.png");
    const SkString actualPath = SkOSPath::Join(dirPath.c_str(), actualFilename.c_str());

    if (!sk_isdir(dirPath.c_str())) {
        sk_mkdir(dirPath.c_str());
    }

    if (gold && !sk_exists(goldPath.c_str())) {
        write_png(*gold, goldPath);
    }

    if (actual) {
        write_png(*actual, actualPath);
    }
}

static GmResults run_gm(std::unique_ptr<skiagm::GM>& gm) {
    GmResults results = {0, 0};
    std::unique_ptr<skiagm::GMVerifiers> verifiers = gm->getVerifiers();

    if (verifiers) {
        SkBitmap goldBmp = draw_golden_image(gm);
        SkTArray<std::pair<SkBitmap, SkString>> testBmps = read_test_images(gm);
        for (const auto& pair : testBmps) {
            const SkBitmap& testBmp = pair.first;
            const SkString& origFilename = pair.second;
            skiagm::VerifierResult result = verifiers->verify(goldBmp, testBmp);
            results.numTested++;
            if (result.ok()) {
                results.numPassed++;
            } else {
                info("%s failed: %s\n", gm->getName(), result.message().c_str());
                write_pngs(gm, &goldBmp, verifiers->imgUnderTest(), origFilename);
            }
        }
    }

    return results;
}

int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);
    if (FLAGS_inputDir.isEmpty()) {
        std::cerr << "--inputDir argument is required" << std::endl;
        return 1;
    }

    // Get test names by traversing inputDir.
    SkTHashSet<SkString> test_names;
    SkOSFile::Iter it(FLAGS_inputDir[0], nullptr);
    for (SkString file; it.next(&file, true);) {
        test_names.add(file);
    }

    // Get matching GMs
    SkTArray<std::unique_ptr<skiagm::GM>> gms;
    for (skiagm::GMFactory f : skiagm::GMRegistry::Range()) {
        std::unique_ptr<skiagm::GM> gm(f());
        if (test_names.contains(SkString(gm->getName()))) {
            gms.push_back(std::move(gm));
        }
    }
    info("Collected %d test names from %s\n", gms.size(), FLAGS_inputDir[0]);

    SkTaskGroup::Enabler enabled(-1);
    SkTaskGroup parallel;
    std::vector<GmResults> results(gms.size());
    for (size_t i = 0; i < gms.size(); i++) {
        std::unique_ptr<skiagm::GM>& gm = gms[i];
        parallel.add([i, &results, &gm]() {
            results[i] = run_gm(gm);
        });
    }
    parallel.wait();

    int numGMsWithNoTestImages = 0, numGMsPassed = 0;
    for (const auto& r : results) {
        if (r.numTested == 0) {
            numGMsWithNoTestImages++;
        } else if (r.numPassed == r.numTested) {
            numGMsPassed++;
        }
    }

    info("\nReport:\n");
    info("%d GMs\n", gms.size());
    info("%d GMs had no test images (or were GPU-only)\n", numGMsWithNoTestImages);
    info("%d GMs passed verification for all test images\n", numGMsPassed);

    // Histogram of num test images
    {
        std::vector<int> numTestsImages(6, 0);
        for (const auto& r : results) {
            numTestsImages[r.numTested]++;
        }

        for (size_t i = 0; i < numTestsImages.size(); i++) {
            info("%4d GMs had %d test images\n", numTestsImages[i], i);
        }
    }

    return 0;
}
