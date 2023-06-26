// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkPathBridge_DEFINED
#define SkPathBridge_DEFINED

#include <memory>
#include "include/core/SkFontParameters.h"
#include "include/core/SkPath.h"

namespace fontations_ffi {
class SkPathWrapper {
public:
    SkPathWrapper();
    void move_to(float x, float y);
    void line_to(float x, float y);
    void quad_to(float cx0, float cy0, float x, float y);
    void curve_to(float cx0, float cy0, float cx1, float cy1, float x, float y);
    void close();
    void dump();
    SkPath into_inner() &&;

private:
    SkPath path_;
};

/** C++ type opaque to Rust side to be able to write out variation design
 * parameters to the caller-side allocated SkFontParameters::Variation::Axis. A
 * direct cast between a shared C++/Rust struct and a Skia side struct is not
 * possible because the hidden-axis flag is private on
 * SkFontParameters::Variation::Axis.  */
class SkAxisWrapper {
public:
    SkAxisWrapper(SkFontParameters::Variation::Axis axisArray[], size_t axisCount);
    SkAxisWrapper() = delete;
    bool populate_axis(size_t i, uint32_t axisTag, float min, float def, float max, bool hidden);
    size_t size() const;

private:
    SkFontParameters::Variation::Axis* fAxisArray;
    size_t fAxisCount;
};

}  // namespace fontations_ffi

#endif
