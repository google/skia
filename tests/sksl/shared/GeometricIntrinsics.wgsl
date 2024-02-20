diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_x: f32 = 1.0;
    let _skTemp0 = length(_0_x);
    _0_x = _skTemp0;
    let _skTemp1 = distance(_0_x, 2.0);
    _0_x = _skTemp1;
    _0_x = _0_x * 2.0;
    let _skTemp2 = sign(_0_x);
    _0_x = _skTemp2;
    var _1_x: vec2<f32> = vec2<f32>(1.0, 2.0);
    let _skTemp3 = length(_1_x);
    _1_x = vec2<f32>(_skTemp3);
    let _skTemp4 = distance(_1_x, vec2<f32>(3.0, 4.0));
    _1_x = vec2<f32>(_skTemp4);
    let _skTemp5 = dot(_1_x, vec2<f32>(3.0, 4.0));
    _1_x = vec2<f32>(_skTemp5);
    let _skTemp6 = normalize(_1_x);
    _1_x = _skTemp6;
    return _globalUniforms.colorGreen;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
