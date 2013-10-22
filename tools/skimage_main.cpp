/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm_expectations.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkRandom.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "SkTemplates.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_string(config, "None", "Preferred config to decode into. [None|8888|565|A8]");
DEFINE_string(createExpectationsPath, "", "Path to write JSON expectations.");
DEFINE_string(mismatchPath, "", "Folder to write mismatched images to.");
DEFINE_string2(readPath, r, "", "Folder(s) and files to decode images. Required.");
DEFINE_string(readExpectationsPath, "", "Path to read JSON expectations from.");
DEFINE_bool(reencode, true, "Reencode the images to test encoding.");
DEFINE_int32(sampleSize, 1, "Set the sampleSize for decoding.");
DEFINE_bool(skip, false, "Skip writing zeroes.");
DEFINE_bool(testSubsetDecoding, true, "Test decoding subsets of images.");
DEFINE_bool(writeChecksumBasedFilenames, false,  "When writing out actual images, use checksum-"
            "based filenames, as rebaseline.py will use when downloading them from Google Storage");
DEFINE_string2(writePath, w, "",  "Write rendered images into this directory.");

struct Format {
    SkImageEncoder::Type    fType;
    SkImageDecoder::Format  fFormat;
    const char*             fSuffix;
};

static const Format gFormats[] = {
    { SkImageEncoder::kBMP_Type, SkImageDecoder::kBMP_Format, ".bmp" },
    { SkImageEncoder::kGIF_Type, SkImageDecoder::kGIF_Format, ".gif" },
    { SkImageEncoder::kICO_Type, SkImageDecoder::kICO_Format, ".ico" },
    { SkImageEncoder::kJPEG_Type, SkImageDecoder::kJPEG_Format, ".jpg" },
    { SkImageEncoder::kPNG_Type, SkImageDecoder::kPNG_Format, ".png" },
    { SkImageEncoder::kWBMP_Type, SkImageDecoder::kWBMP_Format, ".wbmp" },
    { SkImageEncoder::kWEBP_Type, SkImageDecoder::kWEBP_Format, ".webp" }
};

static SkImageEncoder::Type format_to_type(SkImageDecoder::Format format) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gFormats); i++) {
        if (gFormats[i].fFormat == format) {
            return gFormats[i].fType;
        }
    }
    return SkImageEncoder::kUnknown_Type;
}

static const char* suffix_for_type(SkImageEncoder::Type type) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gFormats); i++) {
        if (gFormats[i].fType == type) {
            return gFormats[i].fSuffix;
        }
    }
    return "";
}

static SkImageDecoder::Format guess_format_from_suffix(const char suffix[]) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gFormats); i++) {
        if (strcmp(suffix, gFormats[i].fSuffix) == 0) {
            return gFormats[i].fFormat;
        }
    }
    return SkImageDecoder::kUnknown_Format;
}

static void make_outname(SkString* dst, const char outDir[], const char src[],
                         const char suffix[]) {
    SkString basename = SkOSPath::SkBasename(src);
    dst->set(SkOSPath::SkPathJoin(outDir, basename.c_str()));
    dst->append(suffix);
}

// Store the names of the filenames to report later which ones failed, succeeded, and were
// invalid.
// FIXME: Add more arrays, for more specific types of errors, and make the output simpler.
// If each array holds one type of error, the output can change from:
//
// Failures:
//      <image> failed for such and such reason
//      <image> failed for some different reason
//
// to:
//
// Such and such failures:
//      <image>
//
// Different reason failures:
//      <image>
//
static SkTArray<SkString, false> gInvalidStreams;
static SkTArray<SkString, false> gMissingCodecs;
static SkTArray<SkString, false> gDecodeFailures;
static SkTArray<SkString, false> gEncodeFailures;
static SkTArray<SkString, false> gSuccessfulDecodes;
static SkTArray<SkString, false> gSuccessfulSubsetDecodes;
static SkTArray<SkString, false> gFailedSubsetDecodes;
// Files/subsets that do not have expectations. Not reported as a failure of the test so
// the bots will not turn red with each new image test.
static SkTArray<SkString, false> gMissingExpectations;
static SkTArray<SkString, false> gMissingSubsetExpectations;
// For files that are expected to fail.
static SkTArray<SkString, false> gKnownFailures;
static SkTArray<SkString, false> gKnownSubsetFailures;

static SkBitmap::Config gPrefConfig(SkBitmap::kNo_Config);

// Expections read from a file specified by readExpectationsPath. The expectations must have been
// previously written using createExpectationsPath.
SkAutoTUnref<skiagm::JsonExpectationsSource> gJsonExpectations;

/**
 *  Encode the bitmap to a file, written one of two ways, depending on
 *  FLAGS_writeChecksumBasedFilenames. If true, the final image will be
 *  written to:
 *      outDir/hashType/src/digestValue.png
 *  If false, the final image will be written out to:
 *      outDir/src.png
 *  The function returns whether the file was successfully written.
 */
static bool write_bitmap(const char outDir[], const char src[],
                         const skiagm::BitmapAndDigest& bitmapAndDigest) {
    SkString filename;
    if (FLAGS_writeChecksumBasedFilenames) {
        // First create the directory for the hashtype.
        const SkString hashType = bitmapAndDigest.fDigest.getHashType();
        const SkString hashDir = SkOSPath::SkPathJoin(outDir, hashType.c_str());
        if (!sk_mkdir(hashDir.c_str())) {
            return false;
        }

        // Now create the name of the folder specific to this image.
        SkString basename = SkOSPath::SkBasename(src);
        const SkString imageDir = SkOSPath::SkPathJoin(hashDir.c_str(), basename.c_str());
        if (!sk_mkdir(imageDir.c_str())) {
            return false;
        }

        // Name the file <digest>.png
        SkString checksumBasedName = bitmapAndDigest.fDigest.getDigestValue();
        checksumBasedName.append(".png");

        filename = SkOSPath::SkPathJoin(imageDir.c_str(), checksumBasedName.c_str());
    } else {
        make_outname(&filename, outDir, src, ".png");
    }

    const SkBitmap& bm = bitmapAndDigest.fBitmap;
    if (SkImageEncoder::EncodeFile(filename.c_str(), bm, SkImageEncoder::kPNG_Type, 100)) {
        return true;
    }

    if (bm.config() == SkBitmap::kARGB_8888_Config) {
        // First attempt at encoding failed, and the bitmap was already 8888. Making
        // a copy is not going to help.
        return false;
    }

    // Encoding failed. Copy to 8888 and try again.
    SkBitmap bm8888;
    if (!bm.copyTo(&bm8888, SkBitmap::kARGB_8888_Config)) {
        return false;
    }
    return SkImageEncoder::EncodeFile(filename.c_str(), bm8888, SkImageEncoder::kPNG_Type, 100);
}

/**
 *  Return a random SkIRect inside the range specified.
 *  @param rand Random number generator.
 *  @param maxX Exclusive maximum x-coordinate. SkIRect's fLeft and fRight will be
 *      in the range [0, maxX)
 *  @param maxY Exclusive maximum y-coordinate. SkIRect's fTop and fBottom will be
 *      in the range [0, maxY)
 *  @return SkIRect Non-empty, non-degenerate rectangle.
 */
static SkIRect generate_random_rect(SkRandom* rand, int32_t maxX, int32_t maxY) {
    SkASSERT(maxX > 1 && maxY > 1);
    int32_t left = rand->nextULessThan(maxX);
    int32_t right = rand->nextULessThan(maxX);
    int32_t top = rand->nextULessThan(maxY);
    int32_t bottom = rand->nextULessThan(maxY);
    SkIRect rect = SkIRect::MakeLTRB(left, top, right, bottom);
    rect.sort();
    // Make sure rect is not empty.
    if (rect.fLeft == rect.fRight) {
        if (rect.fLeft > 0) {
            rect.fLeft--;
        } else {
            rect.fRight++;
            // This branch is only taken if 0 == rect.fRight, and
            // maxX must be at least 2, so it must still be in
            // range.
            SkASSERT(rect.fRight < maxX);
        }
    }
    if (rect.fTop == rect.fBottom) {
        if (rect.fTop > 0) {
            rect.fTop--;
        } else {
            rect.fBottom++;
            // Again, this must be in range.
            SkASSERT(rect.fBottom < maxY);
        }
    }
    return rect;
}

/**
 *  Return a string which includes the name of the file and the preferred config,
 *  as specified by "--config". The resulting string will match the pattern of
 *  gm_json.py's IMAGE_FILENAME_PATTERN: "filename_config.png"
 */
static SkString create_json_key(const char* filename) {
    SkASSERT(FLAGS_config.count() == 1);
    return SkStringPrintf("%s_%s.png", filename, FLAGS_config[0]);
}

// Stored expectations to be written to a file if createExpectationsPath is specified.
static Json::Value gExpectationsToWrite;

/**
 *  If expectations are to be recorded, record the bitmap expectations into the global
 *  expectations array.
 *  As is the case with reading expectations, the key used will combine the filename
 *  parameter with the preferred config, as specified by "--config", matching the
 *  pattern of gm_json.py's IMAGE_FILENAME_PATTERN: "filename_config.png"
 */
static void write_expectations(const skiagm::BitmapAndDigest& bitmapAndDigest,
                               const char* filename) {
    const SkString name_config = create_json_key(filename);
    if (!FLAGS_createExpectationsPath.isEmpty()) {
        // Creates an Expectations object, and add it to the list to write.
        skiagm::Expectations expectation(bitmapAndDigest);
        Json::Value value = expectation.asJsonValue();
        gExpectationsToWrite[name_config.c_str()] = value;
    }
}

/**
 *  If --readExpectationsPath is set, compare this bitmap to the json expectations
 *  provided.
 *
 *  @param digest GmResultDigest, computed from the decoded bitmap, to compare to
 *         the existing expectation.
 *  @param filename String used to find the expected value. Will be combined with the
 *         preferred config, as specified by "--config", to match the pattern of
 *         gm_json.py's IMAGE_FILENAME_PATTERN: "filename_config.png". The resulting
 *         key will be used to find the proper expectations.
 *  @param failureArray Array to add a failure message to on failure.
 *  @param missingArray Array to add failure message to when missing image
 *          expectation.
 *  @param ignoreArray Array to add failure message to when the image does not match
 *          the expectation, but this is a failure we can ignore.
 *  @return bool True in any of these cases:
 *                  - the bitmap matches the expectation.
 *               False in any of these cases:
 *                  - there is no expectations file.
 *                  - there is an expectations file, but no expectation for this bitmap.
 *                  - there is an expectation for this bitmap, but it did not match.
 *                  - expectation could not be computed from the bitmap.
 */
static bool compare_to_expectations_if_necessary(const skiagm::GmResultDigest& digest,
                                                 const char* filename,
                                                 SkTArray<SkString, false>* failureArray,
                                                 SkTArray<SkString, false>* missingArray,
                                                 SkTArray<SkString, false>* ignoreArray) {
    // For both writing and reading, the key for this entry will include the name
    // of the file and the pref config, matching the pattern of gm_json.py's
    // IMAGE_FILENAME_PATTERN: "name_config.png"
    const SkString name_config = create_json_key(filename);

    if (!digest.isValid()) {
        if (failureArray != NULL) {
            failureArray->push_back().printf("decoded %s, but could not create a GmResultDigest.",
                                             filename);
        }
        return false;
    }

    if (NULL == gJsonExpectations.get()) {
        return false;
    }

    skiagm::Expectations jsExpectation = gJsonExpectations->get(name_config.c_str());
    if (jsExpectation.empty()) {
        if (missingArray != NULL) {
            missingArray->push_back().printf("decoded %s, but could not find expectation.",
                                             filename);
        }
        return false;
    }

    if (jsExpectation.match(digest)) {
        return true;
    }

    if (jsExpectation.ignoreFailure()) {
        ignoreArray->push_back().printf("%s does not match expectation, but this is known.",
                                        filename);
    } else if (failureArray != NULL) {
        failureArray->push_back().printf("decoded %s, but the result does not match "
                                         "expectations.",
                                         filename);
    }
    return false;
}

/**
 *  Helper function to write a bitmap subset to a file. Only called if subsets were created
 *  and a writePath was provided. Behaves differently depending on
 *  FLAGS_writeChecksumBasedFilenames. If true:
 *      Writes the image to a PNG file named according to the digest hash, as described in
 *      write_bitmap.
 *  If false:
 *      Creates a subdirectory called 'subsets' and writes a PNG to that directory. Also
 *      creates a subdirectory called 'extracted' and writes a bitmap created using
 *      extractSubset to a PNG in that directory. Both files will represent the same
 *      subrectangle and have the same name for convenient comparison. In this case, the
 *      digest is ignored.
 *
 *  @param writePath Parent directory to hold the folders for the PNG files to write. Must
 *      not be NULL.
 *  @param subsetName Basename of the original file, with the dimensions of the subset tacked
 *      on. Used to name the new file/folder.
 *  @param bitmapAndDigestFromDecodeSubset SkBitmap (with digest) created by
 *      SkImageDecoder::DecodeSubset, using rect as the area to decode.
 *  @param rect Rectangle of the area decoded into bitmapFromDecodeSubset. Used to call
 *      extractSubset on originalBitmap to create a bitmap with the same dimensions/pixels as
 *      bitmapFromDecodeSubset (assuming decodeSubset worked properly).
 *  @param originalBitmap SkBitmap decoded from the same stream as bitmapFromDecodeSubset,
 *      using SkImageDecoder::decode to get the entire image. Used to create a PNG file for
 *      comparison to the PNG created by bitmapAndDigestFromDecodeSubset's bitmap.
 *  @return bool Whether the function succeeded at drawing the decoded subset and the extracted
 *      subset to files.
 */
static bool write_subset(const char* writePath, const SkString& subsetName,
                          const skiagm::BitmapAndDigest bitmapAndDigestFromDecodeSubset,
                          SkIRect rect, const SkBitmap& originalBitmap) {
    // All parameters must be valid.
    SkASSERT(writePath != NULL);

    SkString subsetPath;
    if (FLAGS_writeChecksumBasedFilenames) {
        subsetPath.set(writePath);
    } else {
        // Create a subdirectory to hold the results of decodeSubset.
        subsetPath = SkOSPath::SkPathJoin(writePath, "subsets");
        if (!sk_mkdir(subsetPath.c_str())) {
            gFailedSubsetDecodes.push_back().printf("Successfully decoded subset %s, but "
                                                    "failed to create a directory to write to.",
                                                     subsetName.c_str());
            return false;
        }
    }
    SkAssertResult(write_bitmap(subsetPath.c_str(), subsetName.c_str(),
                                bitmapAndDigestFromDecodeSubset));
    gSuccessfulSubsetDecodes.push_back().printf("\twrote %s", subsetName.c_str());

    if (!FLAGS_writeChecksumBasedFilenames) {
        // FIXME: The goal of extracting the subset is for visual comparison/using skdiff/skpdiff.
        // Currently disabling for writeChecksumBasedFilenames since it will be trickier to
        // determine which files to compare.

        // Also use extractSubset from the original for visual comparison.
        // Write the result to a file in a separate subdirectory.
        SkBitmap extractedSubset;
        if (!originalBitmap.extractSubset(&extractedSubset, rect)) {
            gFailedSubsetDecodes.push_back().printf("Successfully decoded subset %s, but failed "
                                                    "to extract a similar subset for comparison.",
                                                    subsetName.c_str());
            return false;
        }

        SkString dirExtracted = SkOSPath::SkPathJoin(writePath, "extracted");
        if (!sk_mkdir(dirExtracted.c_str())) {
            gFailedSubsetDecodes.push_back().printf("Successfully decoded subset%s, but failed "
                                                    "to create a directory for extractSubset "
                                                    "comparison.",
                                                    subsetName.c_str());
            return false;
        }

        skiagm::BitmapAndDigest bitmapAndDigestFromExtractSubset(extractedSubset);
        SkAssertResult(write_bitmap(dirExtracted.c_str(), subsetName.c_str(),
                                    bitmapAndDigestFromExtractSubset));
    }
    return true;
}

// FIXME: This test could be run on windows/mac once we remove their dependence on
// getLength. See https://code.google.com/p/skia/issues/detail?id=1570
#if defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_UNIX)

/**
 *  Dummy class for testing to ensure that a stream without a length decodes the same
 *  as a stream with a length.
 */
class FILEStreamWithoutLength : public SkFILEStream {
public:
    FILEStreamWithoutLength(const char path[])
    : INHERITED(path) {}

    virtual bool hasLength() const SK_OVERRIDE {
        return false;
    }

private:
    typedef SkFILEStream INHERITED;
};

/**
 *  Test that decoding a stream which reports to not have a length still results in the
 *  same image as if it did report to have a length. Assumes that codec was used to
 *  successfully decode the file using SkFILEStream.
 *  @param srcPath The path to the file, for recreating the length-less stream.
 *  @param codec The SkImageDecoder originally used to decode srcPath, which will be used
 *      again to decode the length-less stream.
 *  @param digest GmResultDigest computed from decoding the stream the first time.
 *      Decoding the length-less stream is expected to result in a matching digest.
 */
static void test_stream_without_length(const char srcPath[], SkImageDecoder* codec,
                                       const skiagm::GmResultDigest& digest) {
    if (!digest.isValid()) {
        // An error was already reported.
        return;
    }
    SkASSERT(srcPath);
    SkASSERT(codec);
    FILEStreamWithoutLength stream(srcPath);
    // This will only be called after a successful decode. Creating a stream from the same
    // path should never fail.
    SkASSERT(stream.isValid());
    SkBitmap bm;
    if (!codec->decode(&stream, &bm, gPrefConfig, SkImageDecoder::kDecodePixels_Mode)) {
        gDecodeFailures.push_back().appendf("Without using getLength, %s failed to decode\n",
                                            srcPath);
        return;
    }
    skiagm::GmResultDigest lengthLessDigest(bm);
    if (!lengthLessDigest.isValid()) {
        gDecodeFailures.push_back().appendf("Without using getLength, %s failed to build "
                                            "a digest\n", srcPath);
        return;
    }
    if (!lengthLessDigest.equals(digest)) {
        gDecodeFailures.push_back().appendf("Without using getLength, %s did not match digest "
                                            "that uses getLength\n", srcPath);
    }
}
#endif // defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_UNIX)

/**
 *  Replace all instances of oldChar with newChar in str.
 *  TODO: Add this function to SkString and write tests for it.
 */
static void replace_char(SkString* str, const char oldChar, const char newChar) {
    if (NULL == str) {
        return;
    }
    for (size_t i = 0; i < str->size(); ++i) {
        if (oldChar == str->operator[](i)) {
            str->operator[](i) = newChar;
        }
    }
}

static void decodeFileAndWrite(const char srcPath[], const SkString* writePath) {
    SkBitmap bitmap;
    SkFILEStream stream(srcPath);
    if (!stream.isValid()) {
        gInvalidStreams.push_back().set(srcPath);
        return;
    }

    SkImageDecoder* codec = SkImageDecoder::Factory(&stream);
    if (NULL == codec) {
        gMissingCodecs.push_back().set(srcPath);
        return;
    }

    SkAutoTDelete<SkImageDecoder> ad(codec);

    codec->setSkipWritingZeroes(FLAGS_skip);
    codec->setSampleSize(FLAGS_sampleSize);
    stream.rewind();

    // Create a string representing just the filename itself, for use in json expectations.
    SkString basename = SkOSPath::SkBasename(srcPath);
    // Replace '_' with '-', so that the names can fit gm_json.py's IMAGE_FILENAME_PATTERN
    replace_char(&basename, '_', '-');
    // Replace '.' with '-', so the output filename can still retain the original file extension,
    // but still end up with only one '.', which denotes the actual extension of the final file.
    replace_char(&basename, '.', '-');
    const char* filename = basename.c_str();

    if (!codec->decode(&stream, &bitmap, gPrefConfig,
                       SkImageDecoder::kDecodePixels_Mode)) {
        if (NULL != gJsonExpectations.get()) {
            const SkString name_config = create_json_key(filename);
            skiagm::Expectations jsExpectations = gJsonExpectations->get(name_config.c_str());
            if (jsExpectations.ignoreFailure()) {
                // This is a known failure.
                gKnownFailures.push_back().appendf(
                    "failed to decode %s, which is a known failure.", srcPath);
                return;
            }
            if (jsExpectations.empty()) {
                // This is a failure, but it is a new file. Mark it as missing, with
                // a note that it should be marked failing.
                gMissingExpectations.push_back().appendf(
                    "new file %s (with no expectations) FAILED to decode.", srcPath);
                return;
            }
        }

        // If there was a failure, and either there was no expectations file, or
        // the expectations file listed a valid expectation, report the failure.
        gDecodeFailures.push_back().set(srcPath);
        return;
    }

    // Test decoding just the bounds. The bounds should always match.
    {
        stream.rewind();
        SkBitmap dim;
        if (!codec->decode(&stream, &dim, SkImageDecoder::kDecodeBounds_Mode)) {
            SkString failure = SkStringPrintf("failed to decode bounds for %s", srcPath);
            gDecodeFailures.push_back() = failure;
        } else {
            // Now check that the bounds match:
            if (dim.width() != bitmap.width() || dim.height() != bitmap.height()) {
                SkString failure = SkStringPrintf("bounds do not match for %s", srcPath);
                gDecodeFailures.push_back() = failure;
            }
        }
    }

    skiagm::BitmapAndDigest bitmapAndDigest(bitmap);
    if (compare_to_expectations_if_necessary(bitmapAndDigest.fDigest, filename, &gDecodeFailures,
                                             &gMissingExpectations, &gKnownFailures)) {
        gSuccessfulDecodes.push_back().printf("%s [%d %d]", srcPath, bitmap.width(),
                                              bitmap.height());
    } else if (!FLAGS_mismatchPath.isEmpty()) {
        if (write_bitmap(FLAGS_mismatchPath[0], filename, bitmapAndDigest)) {
            gSuccessfulDecodes.push_back().appendf("\twrote %s", filename);
        } else {
            gEncodeFailures.push_back().set(filename);
        }
    }

// FIXME: This test could be run on windows/mac once we remove their dependence on
// getLength. See https://code.google.com/p/skia/issues/detail?id=1570
#if defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_UNIX)
    test_stream_without_length(srcPath, codec, bitmapAndDigest.fDigest);
#endif

    if (writePath != NULL) {
        if (write_bitmap(writePath->c_str(), filename, bitmapAndDigest)) {
            gSuccessfulDecodes.push_back().appendf("\twrote %s", filename);
        } else {
            gEncodeFailures.push_back().set(filename);
        }
    }

    write_expectations(bitmapAndDigest, filename);

    if (FLAGS_testSubsetDecoding) {
        SkDEBUGCODE(bool couldRewind =) stream.rewind();
        SkASSERT(couldRewind);
        int width, height;
        // Build the tile index for decoding subsets. If the image is 1x1, skip subset
        // decoding since there are no smaller subsets.
        if (codec->buildTileIndex(&stream, &width, &height) && width > 1 && height > 1) {
            SkASSERT(bitmap.width() == width && bitmap.height() == height);
            // Call decodeSubset multiple times:
            SkRandom rand(0);
            for (int i = 0; i < 5; i++) {
                SkBitmap bitmapFromDecodeSubset;
                // FIXME: Come up with a more representative set of rectangles.
                SkIRect rect = generate_random_rect(&rand, width, height);
                SkString subsetDim = SkStringPrintf("[%d,%d,%d,%d]", rect.fLeft, rect.fTop,
                                                    rect.fRight, rect.fBottom);
                if (codec->decodeSubset(&bitmapFromDecodeSubset, rect, gPrefConfig)) {
                    SkString subsetName = SkStringPrintf("%s-%s", filename, subsetDim.c_str());
                    skiagm::BitmapAndDigest subsetBitmapAndDigest(bitmapFromDecodeSubset);
                    if (compare_to_expectations_if_necessary(subsetBitmapAndDigest.fDigest,
                                                             subsetName.c_str(),
                                                             &gFailedSubsetDecodes,
                                                             &gMissingSubsetExpectations,
                                                             &gKnownSubsetFailures)) {
                        gSuccessfulSubsetDecodes.push_back().printf("Decoded subset %s from %s",
                                                              subsetDim.c_str(), srcPath);
                    } else if (!FLAGS_mismatchPath.isEmpty()) {
                        write_subset(FLAGS_mismatchPath[0], subsetName,
                                     subsetBitmapAndDigest, rect, bitmap);
                    }

                    write_expectations(subsetBitmapAndDigest, subsetName.c_str());

                    if (writePath != NULL) {
                        write_subset(writePath->c_str(), subsetName,
                                     subsetBitmapAndDigest, rect, bitmap);
                    }
                } else {
                    gFailedSubsetDecodes.push_back().printf("Failed to decode region %s from %s",
                                                            subsetDim.c_str(), srcPath);
                }
            }
        }
    }

    // Do not attempt to re-encode A8, since our image encoders do not support encoding to A8.
    if (FLAGS_reencode && bitmap.config() != SkBitmap::kA8_Config) {
        // Encode to the format the file was originally in, or PNG if the encoder for the same
        // format is unavailable.
        SkImageDecoder::Format format = codec->getFormat();
        if (SkImageDecoder::kUnknown_Format == format) {
            if (stream.rewind()) {
                format = SkImageDecoder::GetStreamFormat(&stream);
            }
            if (SkImageDecoder::kUnknown_Format == format) {
                const char* dot = strrchr(srcPath, '.');
                if (NULL != dot) {
                    format = guess_format_from_suffix(dot);
                }
                if (SkImageDecoder::kUnknown_Format == format) {
                    SkDebugf("Could not determine type for '%s'\n", srcPath);
                    format = SkImageDecoder::kPNG_Format;
                }

            }
        } else {
            SkASSERT(!stream.rewind() || SkImageDecoder::GetStreamFormat(&stream) == format);
        }
        SkImageEncoder::Type type = format_to_type(format);
        // format should never be kUnknown_Format, so type should never be kUnknown_Type.
        SkASSERT(type != SkImageEncoder::kUnknown_Type);

        SkImageEncoder* encoder = SkImageEncoder::Create(type);
        if (NULL == encoder) {
            type = SkImageEncoder::kPNG_Type;
            encoder = SkImageEncoder::Create(type);
            SkASSERT(encoder);
        }
        SkAutoTDelete<SkImageEncoder> ade(encoder);
        // Encode to a stream.
        SkDynamicMemoryWStream wStream;
        if (!encoder->encodeStream(&wStream, bitmap, 100)) {
            gEncodeFailures.push_back().printf("Failed to reencode %s to type '%s'", srcPath,
                                               suffix_for_type(type));
            return;
        }

        SkAutoTUnref<SkData> data(wStream.copyToData());
        if (writePath != NULL && type != SkImageEncoder::kPNG_Type) {
            // Write the encoded data to a file. Do not write to PNG, which was already written.
            SkString outPath;
            make_outname(&outPath, writePath->c_str(), filename, suffix_for_type(type));
            SkFILEWStream file(outPath.c_str());
            if(file.write(data->data(), data->size())) {
                gSuccessfulDecodes.push_back().appendf("\twrote %s", outPath.c_str());
            } else {
                gEncodeFailures.push_back().printf("Failed to write %s", outPath.c_str());
            }
        }
        // Ensure that the reencoded data can still be decoded.
        SkMemoryStream memStream(data);
        SkBitmap redecodedBitmap;
        SkImageDecoder::Format formatOnSecondDecode;
        if (SkImageDecoder::DecodeStream(&memStream, &redecodedBitmap, gPrefConfig,
                                          SkImageDecoder::kDecodePixels_Mode,
                                          &formatOnSecondDecode)) {
            SkASSERT(format_to_type(formatOnSecondDecode) == type);
        } else {
            gDecodeFailures.push_back().printf("Failed to redecode %s after reencoding to '%s'",
                                               srcPath, suffix_for_type(type));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

// If strings is not empty, print title, followed by each string on its own line starting
// with a tab.
// @return bool True if strings had at least one entry.
static bool print_strings(const char* title, const SkTArray<SkString, false>& strings) {
    if (strings.count() > 0) {
        SkDebugf("%s:\n", title);
        for (int i = 0; i < strings.count(); i++) {
            SkDebugf("\t%s\n", strings[i].c_str());
        }
        SkDebugf("\n");
        return true;
    }
    return false;
}

/**
 *  If directory is non null and does not end with a path separator, append one.
 *  @param directory SkString representing the path to a directory. If the last character is not a
 *      path separator (specific to the current OS), append one.
 */
static void append_path_separator_if_necessary(SkString* directory) {
    if (directory != NULL && directory->c_str()[directory->size() - 1] != SkPATH_SEPARATOR) {
        directory->appendf("%c", SkPATH_SEPARATOR);
    }
}

/**
 *  Return true if the filename represents an image.
 */
static bool is_image_file(const char* filename) {
    const char* gImageExtensions[] = {
        ".png", ".PNG", ".jpg", ".JPG", ".jpeg", ".JPEG", ".bmp", ".BMP",
        ".webp", ".WEBP", ".ico", ".ICO", ".wbmp", ".WBMP", ".gif", ".GIF"
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gImageExtensions); ++i) {
        if (SkStrEndsWith(filename, gImageExtensions[i])) {
            return true;
        }
    }
    return false;
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage("Decode files, and optionally write the results to files.");
    SkCommandLineFlags::Parse(argc, argv);

    if (FLAGS_readPath.count() < 1) {
        SkDebugf("Folder(s) or image(s) to decode are required.\n");
        return -1;
    }


    SkAutoGraphics ag;

    if (!FLAGS_readExpectationsPath.isEmpty() && sk_exists(FLAGS_readExpectationsPath[0])) {
        gJsonExpectations.reset(SkNEW_ARGS(skiagm::JsonExpectationsSource,
                                           (FLAGS_readExpectationsPath[0])));
    }

    SkString outDir;
    SkString* outDirPtr;

    if (FLAGS_writePath.count() == 1) {
        outDir.set(FLAGS_writePath[0]);
        append_path_separator_if_necessary(&outDir);
        outDirPtr = &outDir;
    } else {
        outDirPtr = NULL;
    }

    if (FLAGS_config.count() == 1) {
        // Only consider the first config specified on the command line.
        const char* config = FLAGS_config[0];
        if (0 == strcmp(config, "8888")) {
            gPrefConfig = SkBitmap::kARGB_8888_Config;
        } else if (0 == strcmp(config, "565")) {
            gPrefConfig = SkBitmap::kRGB_565_Config;
        } else if (0 == strcmp(config, "A8")) {
            gPrefConfig = SkBitmap::kA8_Config;
        } else if (0 != strcmp(config, "None")) {
            SkDebugf("Invalid preferred config\n");
            return -1;
        }
    }

    for (int i = 0; i < FLAGS_readPath.count(); i++) {
        const char* readPath = FLAGS_readPath[i];
        if (strlen(readPath) < 1) {
            break;
        }
        if (sk_isdir(readPath)) {
            const char* dir = readPath;
            SkOSFile::Iter iter(dir);
            SkString filename;
            while (iter.next(&filename)) {
                if (!is_image_file(filename.c_str())) {
                    continue;
                }
                SkString fullname = SkOSPath::SkPathJoin(dir, filename.c_str());
                decodeFileAndWrite(fullname.c_str(), outDirPtr);
            }
        } else if (sk_exists(readPath) && is_image_file(readPath)) {
            decodeFileAndWrite(readPath, outDirPtr);
        }
    }

    if (!FLAGS_createExpectationsPath.isEmpty()) {
        // Use an empty value for everything besides expectations, since the reader only cares
        // about the expectations.
        Json::Value nullValue;
        Json::Value root = skiagm::CreateJsonTree(gExpectationsToWrite, nullValue, nullValue,
                                                  nullValue, nullValue);
        std::string jsonStdString = root.toStyledString();
        SkFILEWStream stream(FLAGS_createExpectationsPath[0]);
        stream.write(jsonStdString.c_str(), jsonStdString.length());
    }
    // Add some space, since codecs may print warnings without newline.
    SkDebugf("\n\n");

    bool failed = print_strings("Invalid files", gInvalidStreams);
    failed |= print_strings("Missing codec", gMissingCodecs);
    failed |= print_strings("Failed to decode", gDecodeFailures);
    failed |= print_strings("Failed to encode", gEncodeFailures);
    print_strings("Decoded", gSuccessfulDecodes);
    print_strings("Missing expectations", gMissingExpectations);

    if (FLAGS_testSubsetDecoding) {
        failed |= print_strings("Failed subset decodes", gFailedSubsetDecodes);
        print_strings("Decoded subsets", gSuccessfulSubsetDecodes);
        print_strings("Missing subset expectations", gMissingSubsetExpectations);
        print_strings("Known subset failures", gKnownSubsetFailures);
    }

    print_strings("Known failures", gKnownFailures);

    return failed ? -1 : 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
