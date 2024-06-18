diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testInputs: vec4<f32>,
  colorBlack: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_v: vec4<f32> = _globalUniforms.testInputs;
    var _1_i: vec4<i32> = vec4<i32>(_globalUniforms.colorBlack);
    let _skTemp0 = _1_i.x;
    var _2_x: f32 = _0_v[_skTemp0];
    let _skTemp1 = _1_i.y;
    var _3_y: f32 = _0_v[_skTemp1];
    let _skTemp2 = _1_i.z;
    var _4_z: f32 = _0_v[_skTemp2];
    let _skTemp3 = _1_i.w;
    var _5_w: f32 = _0_v[_skTemp3];
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(all(vec4<f32>(_2_x, _3_y, _4_z, _5_w) == vec4<f32>(-1.25, -1.25, -1.25, 0.0))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
