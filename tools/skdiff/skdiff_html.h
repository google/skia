/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skdiff_html_DEFINED
#define skdiff_html_DEFINED

#include "tools/skdiff/skdiff.h"
class SkString;

void print_diff_page(const int matchCount,
                     const int colorThreshold,
                     const RecordArray& differences,
                     const SkString& baseDir,
                     const SkString& comparisonDir,
                     const SkString& outputDir);

#endif
