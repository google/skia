diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
struct S {
  a: i32,
  b: i32,
  c: i32,
};
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    const _6_two: i32 = 2;
    var _8_flatten1: i32 = _6_two;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(_8_flatten1 == 2));
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
