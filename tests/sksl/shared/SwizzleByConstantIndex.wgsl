diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testInputs: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_v: vec4<f32> = _globalUniforms.testInputs;
    var _1_x: f32 = _0_v.x;
    var _2_y: f32 = _0_v.y;
    var _3_z: f32 = _0_v.z;
    var _4_w: f32 = _0_v.w;
    var a: vec4<f32> = vec4<f32>(_1_x, _2_y, _3_z, _4_w);
    var _9_x: f32 = _globalUniforms.testInputs.x;
    var _10_y: f32 = _globalUniforms.testInputs.y;
    var _11_z: f32 = _globalUniforms.testInputs.z;
    var _12_w: f32 = _globalUniforms.testInputs.w;
    var b: vec4<f32> = vec4<f32>(_9_x, _10_y, _11_z, _12_w);
    var c: vec4<f32> = vec4<f32>(0.0, 1.0, 2.0, 3.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((all(a == vec4<f32>(-1.25, 0.0, 0.75, 2.25)) && all(b == vec4<f32>(-1.25, 0.0, 0.75, 2.25))) && all(c == vec4<f32>(0.0, 1.0, 2.0, 3.0))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
