diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  testInputs: vec4<f16>,
  colorBlack: vec4<f16>,
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    let _0_v: vec4<f16> = _globalUniforms.testInputs;
    let _1_i: vec4<i32> = vec4<i32>(_globalUniforms.colorBlack);
    let _skTemp0 = _1_i.x;
    let _2_x: f16 = _0_v[_skTemp0];
    let _skTemp1 = _1_i.y;
    let _3_y: f16 = _0_v[_skTemp1];
    let _skTemp2 = _1_i.z;
    let _4_z: f16 = _0_v[_skTemp2];
    let _skTemp3 = _1_i.w;
    let _5_w: f16 = _0_v[_skTemp3];
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(all(vec4<f16>(_2_x, _3_y, _4_z, _5_w) == vec4<f16>(-1.25h, -1.25h, -1.25h, 0.0h))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
