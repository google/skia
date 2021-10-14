// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "tools/flags/CommonFlags.h"

namespace CommonFlags {
bool CollectImages(CommandLineFlags::StringArray images, SkTArray<SkString>* output) {
    SkASSERT(output);

    static const char* const exts[] = {
        "bmp",
        "gif",
        "jpg",
        "jpeg",
        "png",
        "webp",
        "ktx",
        "astc",
        "wbmp",
        "ico",
#if !defined(SK_BUILD_FOR_WIN)
        "BMP",
        "GIF",
        "JPG",
        "JPEG",
        "PNG",
        "WEBP",
        "KTX",
        "ASTC",
        "WBMP",
        "ICO",
#endif
#ifdef SK_HAS_HEIF_LIBRARY
        "heic",
#if !defined(SK_BUILD_FOR_WIN)
        "HEIC",
#endif
#endif
#ifdef SK_CODEC_DECODES_RAW
        "arw",
        "cr2",
        "dng",
        "nef",
        "nrw",
        "orf",
        "raf",
        "rw2",
        "pef",
        "srw",
#if !defined(SK_BUILD_FOR_WIN)
        "ARW",
        "CR2",
        "DNG",
        "NEF",
        "NRW",
        "ORF",
        "RAF",
        "RW2",
        "PEF",
        "SRW",
#endif
#endif
    };

    for (int i = 0; i < images.count(); ++i) {
        const char* flag = images[i];
        if (!sk_exists(flag)) {
            SkDebugf("%s does not exist!\n", flag);
            return false;
        }

        if (sk_isdir(flag)) {
            // If the value passed in is a directory, add all the images
            bool foundAnImage = false;
            for (const char* ext : exts) {
                SkOSFile::Iter it(flag, ext);
                SkString file;
                while (it.next(&file)) {
                    foundAnImage = true;
                    output->push_back() = SkOSPath::Join(flag, file.c_str());
                }
            }
            if (!foundAnImage) {
                SkDebugf("No supported images found in %s!\n", flag);
                return false;
            }
        } else {
            // Also add the value if it is a single image
            output->push_back() = flag;
        }
    }
    return true;
}

}  // namespace CommonFlags
