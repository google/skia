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

struct ColorStop;
struct BridgeColorStops;
struct Transform;
struct FillLinearParams;
struct FillRadialParams;
struct FillSweepParams;

/** C++ pure virtual interface, exposed to Rust side for receiving COLRv0/COLRv1 drawing callback
 * matching Skrifa's ColorPainter trait. */
class ColorPainterWrapper {
public:
    virtual ~ColorPainterWrapper() = default;
    virtual void push_transform(const Transform& transform) = 0;
    virtual void pop_transform() = 0;
    virtual void push_clip_glyph(uint16_t glyph_id) = 0;
    virtual void push_clip_rectangle(float x_min, float y_min, float x_max, float y_max) = 0;
    virtual void pop_clip() = 0;

    // Paint*Gradient equivalents:
    virtual void fill_solid(uint16_t palette_index, float alpha) = 0;
    virtual void fill_linear(const FillLinearParams& fill_linear_params,
                             BridgeColorStops& stops,
                             uint8_t extend_mode) = 0;
    virtual void fill_radial(const FillRadialParams& fill_radial_params,
                             BridgeColorStops& stops,
                             uint8_t extend_mode) = 0;
    virtual void fill_sweep(const FillSweepParams&,
                            BridgeColorStops& stops,
                            uint8_t extend_mode) = 0;

    // Optimized calls that allow a SkCanvas::drawPath() call.
    virtual void fill_glyph_solid(uint16_t glyph_id, uint16_t palette_index, float alpha) = 0;
    virtual void fill_glyph_radial(uint16_t glyph_id,
                                   const fontations_ffi::Transform& transform,
                                   const fontations_ffi::FillRadialParams& fill_radial_params,
                                   fontations_ffi::BridgeColorStops& stops,
                                   uint8_t) = 0;
    virtual void fill_glyph_linear(uint16_t glyph_id,
                                   const fontations_ffi::Transform& transform,
                                   const fontations_ffi::FillLinearParams& fill_linear_params,
                                   fontations_ffi::BridgeColorStops& stops,
                                   uint8_t) = 0;
    virtual void fill_glyph_sweep(uint16_t glyph_id,
                                  const fontations_ffi::Transform& transform,
                                  const fontations_ffi::FillSweepParams& fill_sweep_params,
                                  fontations_ffi::BridgeColorStops& stops,
                                  uint8_t) = 0;

    virtual void push_layer(uint8_t colrV1CompositeMode) = 0;
    virtual void pop_layer() = 0;
};

}  // namespace fontations_ffi

#endif
