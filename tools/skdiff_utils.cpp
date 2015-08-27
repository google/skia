/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "skdiff.h"
#include "skdiff_utils.h"
#include "SkBitmap.h"
#include "SkData.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkStream.h"
#include "SkTypes.h"

#include <memory>

bool are_buffers_equal(SkData* skdata1, SkData* skdata2) {
    if ((nullptr == skdata1) || (nullptr == skdata2)) {
        return false;
    }
    if (skdata1->size() != skdata2->size()) {
        return false;
    }
    return (0 == memcmp(skdata1->data(), skdata2->data(), skdata1->size()));
}

SkData* read_file(const char* file_path) {
    SkData* data = SkData::NewFromFileName(file_path);
    if (!data) {
        SkDebugf("WARNING: could not open file <%s> for reading\n", file_path);
    }
    return data;
}

bool get_bitmap(SkData* fileBits, DiffResource& resource, SkImageDecoder::Mode mode) {
    SkMemoryStream stream(fileBits->data(), fileBits->size());

    // In debug, the DLL will automatically be unloaded when this is deleted,
    // but that shouldn't be a problem in release mode.
    std::unique_ptr<SkImageDecoder> codec(SkImageDecoder::Factory(&stream));
    if (nullptr == codec) {
        SkDebugf("ERROR: no codec found for <%s>\n", resource.fFullPath.c_str());
        resource.fStatus = DiffResource::kCouldNotDecode_Status;
        return false;
    }

    stream.rewind();
    if (!codec->decode(&stream, &resource.fBitmap, kN32_SkColorType, mode)) {
        SkDebugf("ERROR: codec failed for basePath <%s>\n", resource.fFullPath.c_str());
        resource.fStatus = DiffResource::kCouldNotDecode_Status;
        return false;
    }

    resource.fStatus = DiffResource::kDecoded_Status;
    return true;
}

/** Thanks to PNG, we need to force all pixels 100% opaque. */
static void force_all_opaque(const SkBitmap& bitmap) {
   SkAutoLockPixels lock(bitmap);
   for (int y = 0; y < bitmap.height(); y++) {
       for (int x = 0; x < bitmap.width(); x++) {
           *bitmap.getAddr32(x, y) |= (SK_A32_MASK << SK_A32_SHIFT);
       }
   }
}

bool write_bitmap(const SkString& path, const SkBitmap& bitmap) {
    SkBitmap copy;
    bitmap.copyTo(&copy, kN32_SkColorType);
    force_all_opaque(copy);
    return SkImageEncoder::EncodeFile(path.c_str(), copy,
                                      SkImageEncoder::kPNG_Type, 100);
}

/// Return a copy of the "input" string, within which we have replaced all instances
/// of oldSubstring with newSubstring.
///
/// TODO: If we like this, we should move it into the core SkString implementation,
/// adding more checks and ample test cases, and paying more attention to efficiency.
static SkString replace_all(const SkString &input,
                            const char oldSubstring[], const char newSubstring[]) {
    SkString output;
    const char *input_cstr = input.c_str();
    const char *first_char = input_cstr;
    const char *match_char;
    size_t oldSubstringLen = strlen(oldSubstring);
    while ((match_char = strstr(first_char, oldSubstring))) {
        output.append(first_char, (match_char - first_char));
        output.append(newSubstring);
        first_char = match_char + oldSubstringLen;
    }
    output.append(first_char);
    return output;
}

static SkString filename_to_derived_filename(const SkString& filename, const char *suffix) {
    SkString diffName (filename);
    const char* cstring = diffName.c_str();
    size_t dotOffset = strrchr(cstring, '.') - cstring;
    diffName.remove(dotOffset, diffName.size() - dotOffset);
    diffName.append(suffix);

    // In case we recursed into subdirectories, replace slashes with something else
    // so the diffs will all be written into a single flat directory.
    diffName = replace_all(diffName, PATH_DIV_STR, "_");
    return diffName;
}

SkString filename_to_diff_filename(const SkString& filename) {
    return filename_to_derived_filename(filename, "-diff.png");
}

SkString filename_to_white_filename(const SkString& filename) {
    return filename_to_derived_filename(filename, "-white.png");
}

void create_and_write_diff_image(DiffRecord* drp,
                                 DiffMetricProc dmp,
                                 const int colorThreshold,
                                 const SkString& outputDir,
                                 const SkString& filename) {
    const int w = drp->fBase.fBitmap.width();
    const int h = drp->fBase.fBitmap.height();

    if (w != drp->fComparison.fBitmap.width() || h != drp->fComparison.fBitmap.height()) {
        drp->fResult = DiffRecord::kDifferentSizes_Result;
    } else {
        drp->fDifference.fBitmap.allocN32Pixels(w, h);

        drp->fWhite.fBitmap.allocN32Pixels(w, h);

        SkASSERT(DiffRecord::kUnknown_Result == drp->fResult);
        compute_diff(drp, dmp, colorThreshold);
        SkASSERT(DiffRecord::kUnknown_Result != drp->fResult);
    }

    if (outputDir.isEmpty()) {
        drp->fDifference.fStatus = DiffResource::kUnspecified_Status;
        drp->fWhite.fStatus = DiffResource::kUnspecified_Status;

    } else {
        drp->fDifference.fFilename = filename_to_diff_filename(filename);
        drp->fDifference.fFullPath = outputDir;
        drp->fDifference.fFullPath.append(drp->fDifference.fFilename);
        drp->fDifference.fStatus = DiffResource::kSpecified_Status;

        drp->fWhite.fFilename = filename_to_white_filename(filename);
        drp->fWhite.fFullPath = outputDir;
        drp->fWhite.fFullPath.append(drp->fWhite.fFilename);
        drp->fWhite.fStatus = DiffResource::kSpecified_Status;

        if (DiffRecord::kDifferentPixels_Result == drp->fResult) {
            if (write_bitmap(drp->fDifference.fFullPath, drp->fDifference.fBitmap)) {
                drp->fDifference.fStatus = DiffResource::kExists_Status;
            } else {
                drp->fDifference.fStatus = DiffResource::kDoesNotExist_Status;
            }
            if (write_bitmap(drp->fWhite.fFullPath, drp->fWhite.fBitmap)) {
                drp->fWhite.fStatus = DiffResource::kExists_Status;
            } else {
                drp->fWhite.fStatus = DiffResource::kDoesNotExist_Status;
            }
        }
    }
}
