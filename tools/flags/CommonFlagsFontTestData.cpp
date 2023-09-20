/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkFontMgrPriv.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/flags/CommonFlags.h"
#include "tools/fonts/TestFontMgr.h"

#if defined(SK_BUILD_FOR_WIN)
#include "include/ports/SkTypeface_win.h"
#endif

extern std::atomic<const char*> gFontTestDataBasePath;

namespace CommonFlags {

static DEFINE_string(fontTestDataPath,
                     "",
                     "Path to extracted data from googlefonts_testdata CIPD file.");

void SetFontTestDataDirectory() {
    if (FLAGS_fontTestDataPath.isEmpty()) {
        return;
    }
    if (strlen(FLAGS_fontTestDataPath[0])) {
        gFontTestDataBasePath = FLAGS_fontTestDataPath[0];
    }
}

}  // namespace CommonFlags
