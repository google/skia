/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "picture_utils.h"
#include "SkBitmap.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkString.h"

#include "sk_tool_utils.h"

namespace sk_tools {

bool write_bitmap_to_disk(const SkBitmap& bm, const SkString& dirPath,
                          const char *subdirOrNull, const SkString& baseName) {
    SkString partialPath;
    if (subdirOrNull) {
        partialPath = SkOSPath::Join(dirPath.c_str(), subdirOrNull);
        sk_mkdir(partialPath.c_str());
    } else {
        partialPath.set(dirPath);
    }
    SkString fullPath = SkOSPath::Join(partialPath.c_str(), baseName.c_str());
    if (sk_tool_utils::EncodeImageToFile(fullPath.c_str(), bm, SkEncodedImageFormat::kPNG, 100)) {
        return true;
    } else {
        SkDebugf("Failed to write the bitmap to %s.\n", fullPath.c_str());
        return false;
    }
}

} // namespace sk_tools
