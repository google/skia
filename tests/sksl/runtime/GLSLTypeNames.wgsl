diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
var<private> f: f32;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var fv: vec4<f32> = vec4<f32>(f);
    return fv;
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
