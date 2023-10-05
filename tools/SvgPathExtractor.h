/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PathExtractor_DEFINED
#define PathExtractor_DEFINED

#include "tools/ToolUtils.h"

#include <functional>

class SkMatrix;
class SkPaint;
class SkPath;

namespace ToolUtils {

void ExtractPathsFromSVG(const char filepath[], std::function<PathSniffCallback>);

}  // namespace ToolUtils

#endif
