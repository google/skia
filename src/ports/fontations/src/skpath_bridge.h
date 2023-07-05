// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkPathBridge_DEFINED
#define SkPathBridge_DEFINED

#include <cstddef>
#include <cstdint>

namespace fontations_ffi {

/** C++ pure virtual interface type, exposed to Rust side to be able to write
 * from Skrifa path output functions to an SkPath type to capture and convert a
 * glyph path. */
class PathWrapper {
public:
    virtual ~PathWrapper() = default;
    virtual void move_to(float x, float y) = 0;
    virtual void line_to(float x, float y) = 0;
    virtual void quad_to(float cx0, float cy0, float x, float y) = 0;
    virtual void curve_to(float cx0, float cy0, float cx1, float cy1, float x, float y) = 0;
    virtual void close() = 0;
};

/** C++ pure virtual interface type, exposed to Rust side to be able to write
 * out variation design parameters to the caller-side allocated
 * SkFontParameters::Variation::Axis. A direct cast or mapping between a shared
 * C++/Rust struct and a Skia side struct is not possible because the
 * hidden-axis flag is private on SkFontParameters::Variation::Axis.  */
class AxisWrapper {
public:
    virtual ~AxisWrapper() = default;
    virtual bool populate_axis(
            size_t i, uint32_t axisTag, float min, float def, float max, bool hidden) = 0;
    virtual size_t size() const = 0;
};

}  // namespace fontations_ffi

#endif
