diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
const gInitialized: f32 = -1.0;
var<private> gInitializedFromOther: f32 = 1.0;
var<private> gUninitialized: f32;
fn init_globals_v() {
  {
    gUninitialized = 1.0;
  }
}
fn _skslMain(xy: vec2<f32>) -> vec4<f32> {
  {
    init_globals_v();
    return vec4<f32>(0.0, gInitializedFromOther, 0.0, gUninitialized);
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
