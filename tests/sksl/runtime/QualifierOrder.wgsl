diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn const_after_in_vf2(x: vec2<f32>) {
  {
  }
}
fn inout_after_high_precision_vf2(x: ptr<function, vec2<f32>>) {
  {
  }
}
fn out_after_high_precision_vf2(x: ptr<function, vec2<f32>>) {
  {
    (*x) = vec2<f32>(0.0);
  }
}
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  var coords = _skParam0;
  {
    const_after_in_vf2(coords);
    var _skTemp0: vec2<f32> = coords;
    inout_after_high_precision_vf2(&_skTemp0);
    coords = _skTemp0;
    var _skTemp1: vec2<f32>;
    out_after_high_precision_vf2(&_skTemp1);
    coords = _skTemp1;
    return vec4<f32>(_globalUniforms.colorGreen);
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
