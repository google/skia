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

DEFINE_string(createExpectationsPath, "", "Path to write JSON expectations.");
DEFINE_string(mismatchPath, "", "Folder to write mismatched images to.");
DEFINE_string2(readPath, r, "", "Folder(s) and files to decode images. Required.");
DEFINE_string(readExpectationsPath, "", "Path to read JSON expectations from.");
DEFINE_bool(reencode, true, "Reencode the images to test encoding.");
DEFINE_bool(testSubsetDecoding, true, "Test decoding subsets of images.");
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
    if (!dst->endsWith(suffix)) {
        const char* cstyleDst = dst->c_str();
        const char* dot = strrchr(cstyleDst, '.');
        if (dot != NULL) {
            int32_t index = SkToS32(dot - cstyleDst);
            dst->remove(index, dst->size() - index);
        }
        dst->append(suffix);
    }
}

// Store the names of the filenames to report later which ones failed, succeeded, and were
// invalid.
static SkTArray<SkString, false> gInvalidStreams;
static SkTArray<SkString, false> gMissingCodecs;
static SkTArray<SkString, false> gDecodeFailures;
static SkTArray<SkString, false> gEncodeFailures;
static SkTArray<SkString, false> gSuccessfulDecodes;
static SkTArray<SkString, false> gSuccessfulSubsetDecodes;
static SkTArray<SkString, false> gFailedSubsetDecodes;

// Expections read from a file specified by readExpectationsPath. The expectations must have been
// previously written using createExpectationsPath.
SkAutoTUnref<skiagm::JsonExpectationsSource> gJsonExpectations;

static bool write_bitmap(const char outName[], const SkBitmap& bm) {
    return SkImageEncoder::EncodeFile(outName, bm, SkImageEncoder::kPNG_Type, 100);
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

// Stored expectations to be written to a file if createExpectationsPath is specified.
static Json::Value gExpectationsToWrite;

/**
 *  If expectations are to be recorded, record the bitmap expectations into global
 *  expectations array.
 */
static void write_expectations(const SkBitmap& bitmap, const char* filename) {
    if (!FLAGS_createExpectationsPath.isEmpty()) {
        // Creates an Expectations object, and add it to the list to write.
        skiagm::Expectations expectation(bitmap);
        Json::Value value = expectation.asJsonValue();
        gExpectationsToWrite[filename] = value;
    }
}

/**
 *  Compare against an expectation for this filename, if there is one.
 *  @param bitmap SkBitmap to compare to the expected value.
 *  @param filename String used to find the expected value.
 *  @return bool True in any of these cases:
 *                  - the bitmap matches the expectation.
 *               False in any of these cases:
 *                  - there is no expectations file.
 *                  - there is an expectations file, but no expectation for this bitmap.
 *                  - there is an expectation for this bitmap, but it did not match.
 *                  - expectation could not be computed from the bitmap.
 */
static bool compare_to_expectations_if_necessary(const SkBitmap& bitmap, const char* filename,
                                                 SkTArray<SkString, false>* failureArray) {
    skiagm::GmResultDigest resultDigest(bitmap);
    if (!resultDigest.isValid()) {
        if (failureArray != NULL) {
            failureArray->push_back().printf("decoded %s, but could not create a GmResultDigest.",
                                             filename);
        }
        return false;
    }

    if (NULL == gJsonExpectations.get()) {
        return false;
    }

    skiagm::Expectations jsExpectation = gJsonExpectations->get(filename);
    if (jsExpectation.empty()) {
        if (failureArray != NULL) {
            failureArray->push_back().printf("decoded %s, but could not find expectation.",
                                             filename);
        }
        return false;
    }

    if (jsExpectation.match(resultDigest)) {
        return true;
    }

    if (failureArray != NULL) {
        failureArray->push_back().printf("decoded %s, but the result does not match "
                                         "expectations.",
                                         filename);
    }
    return false;
}

/**
 *  Helper function to write a bitmap subset to a file. Only called if subsets were created
 *  and a writePath was provided. Creates a subdirectory called 'subsets' and writes a PNG to
 *  that directory. Also creates a subdirectory called 'extracted' and writes a bitmap created
 *  using extractSubset to a PNG in that directory. Both files will represent the same
 *  subrectangle and have the same name for comparison.
 *  @param writePath Parent directory to hold the folders for the PNG files to write. Must
 *      not be NULL.
 *  @param filename Basename of the original file. Used to name the new files. Must not be
 *      NULL.
 *  @param subsetDim String representing the dimensions of the subset. Used to name the new
 *      files. Must not be NULL.
 *  @param bitmapFromDecodeSubset Pointer to SkBitmap created by SkImageDecoder::DecodeSubset,
 *      using rect as the area to decode.
 *  @param rect Rectangle of the area decoded into bitmapFromDecodeSubset. Used to call
 *      extractSubset on originalBitmap to create a bitmap with the same dimensions/pixels as
 *      bitmapFromDecodeSubset (assuming decodeSubset worked properly).
 *  @param originalBitmap SkBitmap decoded from the same stream as bitmapFromDecodeSubset,
 *      using SkImageDecoder::decode to get the entire image. Used to create a PNG file for
 *      comparison to the PNG created by bitmapFromDecodeSubset.
 *  @return bool Whether the function succeeded at drawing the decoded subset and the extracted
 *      subset to files.
 */
static bool write_subset(const char* writePath, const char* filename, const char* subsetDim,
                          SkBitmap* bitmapFromDecodeSubset, SkIRect rect,
                          const SkBitmap& originalBitmap) {
    // All parameters must be valid.
    SkASSERT(writePath != NULL);
    SkASSERT(filename != NULL);
    SkASSERT(subsetDim != NULL);
    SkASSERT(bitmapFromDecodeSubset != NULL);

    // Create a subdirectory to hold the results of decodeSubset.
    SkString dir = SkOSPath::SkPathJoin(writePath, "subsets");
    if (!sk_mkdir(dir.c_str())) {
        gFailedSubsetDecodes.push_back().printf("Successfully decoded %s from %s, but failed to "
                                                "create a directory to write to.", subsetDim,
                                                 filename);
        return false;
    }

    // Write the subset to a file whose name includes the dimensions.
    SkString suffix = SkStringPrintf("_%s.png", subsetDim);
    SkString outPath;
    make_outname(&outPath, dir.c_str(), filename, suffix.c_str());
    SkAssertResult(write_bitmap(outPath.c_str(), *bitmapFromDecodeSubset));
    gSuccessfulSubsetDecodes.push_back().printf("\twrote %s", outPath.c_str());

    // Also use extractSubset from the original for visual comparison.
    // Write the result to a file in a separate subdirectory.
    SkBitmap extractedSubset;
    if (!originalBitmap.extractSubset(&extractedSubset, rect)) {
        gFailedSubsetDecodes.push_back().printf("Successfully decoded %s from %s, but failed to "
                                                "extract a similar subset for comparison.",
                                                subsetDim, filename);
        return false;
    }

    SkString dirExtracted = SkOSPath::SkPathJoin(writePath, "extracted");
    if (!sk_mkdir(dirExtracted.c_str())) {
        gFailedSubsetDecodes.push_back().printf("Successfully decoded %s from %s, but failed to "
                                                "create a directory for extractSubset comparison.",
                                                subsetDim, filename);
        return false;
    }

    make_outname(&outPath, dirExtracted.c_str(), filename, suffix.c_str());
    SkAssertResult(write_bitmap(outPath.c_str(), extractedSubset));
    return true;
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

    stream.rewind();
    if (!codec->decode(&stream, &bitmap, SkBitmap::kARGB_8888_Config,
                       SkImageDecoder::kDecodePixels_Mode)) {
        gDecodeFailures.push_back().set(srcPath);
        return;
    }

    // Create a string representing just the filename itself, for use in json expectations.
    SkString basename = SkOSPath::SkBasename(srcPath);
    const char* filename = basename.c_str();

    if (compare_to_expectations_if_necessary(bitmap, filename, &gDecodeFailures)) {
        gSuccessfulDecodes.push_back().printf("%s [%d %d]", srcPath, bitmap.width(),
                                              bitmap.height());
    } else if (!FLAGS_mismatchPath.isEmpty()) {
        SkString outPath;
        make_outname(&outPath, FLAGS_mismatchPath[0], srcPath, ".png");
        if (write_bitmap(outPath.c_str(), bitmap)) {
            gSuccessfulDecodes.push_back().appendf("\twrote %s", outPath.c_str());
        } else {
            gEncodeFailures.push_back().set(outPath);
        }
    }

    if (writePath != NULL) {
        SkString outPath;
        make_outname(&outPath, writePath->c_str(), srcPath, ".png");
        if (write_bitmap(outPath.c_str(), bitmap)) {
            gSuccessfulDecodes.push_back().appendf("\twrote %s", outPath.c_str());
        } else {
            gEncodeFailures.push_back().set(outPath);
        }
    }

    write_expectations(bitmap, filename);

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
                if (codec->decodeSubset(&bitmapFromDecodeSubset, rect, SkBitmap::kNo_Config)) {
                    SkString subsetName = SkStringPrintf("%s_%s", filename, subsetDim.c_str());
                    if (compare_to_expectations_if_necessary(bitmapFromDecodeSubset,
                                                             subsetName.c_str(),
                                                             &gFailedSubsetDecodes)) {
                        gSuccessfulSubsetDecodes.push_back().printf("Decoded subset %s from %s",
                                                              subsetDim.c_str(), srcPath);
                    } else if (!FLAGS_mismatchPath.isEmpty()) {
                        write_subset(FLAGS_mismatchPath[0], filename, subsetDim.c_str(),
                                     &bitmapFromDecodeSubset, rect, bitmap);
                    }

                    write_expectations(bitmapFromDecodeSubset, subsetName.c_str());
                    if (writePath != NULL) {
                        write_subset(writePath->c_str(), filename, subsetDim.c_str(),
                                     &bitmapFromDecodeSubset, rect, bitmap);
                    }
                } else {
                    gFailedSubsetDecodes.push_back().printf("Failed to decode region %s from %s",
                                                            subsetDim.c_str(), srcPath);
                }
            }
        }
    }

    if (FLAGS_reencode) {
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
            make_outname(&outPath, writePath->c_str(), srcPath, suffix_for_type(type));
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
        if (SkImageDecoder::DecodeStream(&memStream, &redecodedBitmap, SkBitmap::kNo_Config,
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

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage("Decode files, and optionally write the results to files.");
    SkCommandLineFlags::Parse(argc, argv);

    if (FLAGS_readPath.count() < 1) {
        SkDebugf("Folder(s) or image(s) to decode are required.\n");
        return -1;
    }


    SkAutoGraphics ag;

    if (!FLAGS_readExpectationsPath.isEmpty()) {
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

    for (int i = 0; i < FLAGS_readPath.count(); i++) {
        if (strlen(FLAGS_readPath[i]) < 1) {
            break;
        }
        SkOSFile::Iter iter(FLAGS_readPath[i]);
        SkString filename;
        if (iter.next(&filename)) {
            SkString directory(FLAGS_readPath[i]);
            append_path_separator_if_necessary(&directory);
            do {
                SkString fullname(directory);
                fullname.append(filename);
                decodeFileAndWrite(fullname.c_str(), outDirPtr);
            } while (iter.next(&filename));
        } else {
            // FLAGS_readPath[i] is an empty or nonexistent directory,
            // so do nothing.
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

    if (FLAGS_testSubsetDecoding) {
        failed |= print_strings("Failed subset decodes", gFailedSubsetDecodes);
        print_strings("Decoded subsets", gSuccessfulSubsetDecodes);
    }

    return failed ? -1 : 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
