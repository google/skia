diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
fn _skslMain(xy: vec2<f32>) -> vec4<f32> {
  {
    var i: i32 = 0;
    return vec4<f32>(f32(i));
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
