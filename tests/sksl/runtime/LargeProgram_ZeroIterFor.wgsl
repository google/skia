diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
fn _skslMain(xy: vec2<f32>) -> vec4<f16> {
  {
    const i: i32 = 0;
    return vec4<f16>(f16(i));
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f16> {
  return _skslMain(_coords);
}
