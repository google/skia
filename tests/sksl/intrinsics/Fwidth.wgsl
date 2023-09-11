diagnostic(off, derivative_uniformity);
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
    var expected: vec4<f32> = vec4<f32>(0.0);
    let _skTemp0 = dpdx(_globalUniforms.testInputs.x);
    let _skTemp1 = dpdx(_globalUniforms.testInputs.xy);
    let _skTemp2 = dpdx(_globalUniforms.testInputs.xyz);
    let _skTemp3 = dpdx(_globalUniforms.testInputs);
    let _skTemp4 = fwidth(coords.xx);
    let _skTemp5 = sign(_skTemp4);
    let _skTemp6 = fwidth(vec2<f32>(coords.x, 1.0));
    let _skTemp7 = sign(_skTemp6);
    let _skTemp8 = fwidth(coords.yy);
    let _skTemp9 = sign(_skTemp8);
    let _skTemp10 = fwidth(vec2<f32>(0.0, coords.y));
    let _skTemp11 = sign(_skTemp10);
    let _skTemp12 = fwidth(coords);
    let _skTemp13 = sign(_skTemp12);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((((_skTemp0 == expected.x) && all(_skTemp1 == expected.xy)) && all(_skTemp2 == expected.xyz)) && all(_skTemp3 == expected)) && all(_skTemp5 == vec2<f32>(1.0))) && all(_skTemp7 == vec2<f32>(1.0, 0.0))) && all(_skTemp9 == vec2<f32>(1.0))) && all(_skTemp11 == vec2<f32>(0.0, 1.0))) && all(_skTemp13 == vec2<f32>(1.0))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
