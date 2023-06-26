// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "src/ports/fontations/src/skpath_bridge.h"

namespace fontations_ffi {
SkPathWrapper::SkPathWrapper() {}

void SkPathWrapper::move_to(float x, float y) { path_.moveTo(x, y); }

void SkPathWrapper::line_to(float x, float y) { path_.lineTo(x, y); }

void SkPathWrapper::quad_to(float cx0, float cy0, float x, float y) {
    path_.quadTo(cx0, cy0, x, y);
}
void SkPathWrapper::curve_to(float cx0, float cy0, float cx1, float cy1, float x, float y) {
    path_.cubicTo(cx0, cy0, cx1, cy1, x, y);
}

void SkPathWrapper::close() { path_.close(); }

void SkPathWrapper::dump() { path_.dump(); }

SkPath SkPathWrapper::into_inner() && { return std::move(path_); }

SkAxisWrapper::SkAxisWrapper(SkFontParameters::Variation::Axis axisArray[], size_t axisCount)
        : fAxisArray(axisArray), fAxisCount(axisCount) {}

bool SkAxisWrapper::populate_axis(
        size_t i, uint32_t axisTag, float min, float def, float max, bool hidden) {
    if (i >= fAxisCount) {
        return false;
    }
    SkFontParameters::Variation::Axis& axis = fAxisArray[i];
    axis.tag = axisTag;
    axis.min = min;
    axis.def = def;
    axis.max = max;
    axis.setHidden(hidden);
    return true;
}

size_t SkAxisWrapper::size() const { return fAxisCount; }

}  // namespace fontations_ffi
