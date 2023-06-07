struct _GlobalUniforms {
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn const_after_in_vf2(_skParam0: vec2<f32>) {
  let x = _skParam0;
  {
  }
}
fn inout_after_high_precision_vf2(_skParam0: ptr<function, vec2<f32>>) {
  let x = _skParam0;
  {
  }
}
fn out_after_high_precision_vf2(_skParam0: ptr<function, vec2<f32>>) {
  let x = _skParam0;
  {
    (*x) = vec2<f32>(0.0);
  }
}
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
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
@fragment fn runtimeShaderMain(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return main(_coords);
}
