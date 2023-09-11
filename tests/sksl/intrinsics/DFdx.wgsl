diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
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
    let _skTemp4 = dpdx(coords.xx);
    let _skTemp5 = sign(_skTemp4);
    let _skTemp6 = dpdx(coords.yy);
    let _skTemp7 = sign(_skTemp6);
    let _skTemp8 = dpdx(coords);
    let _skTemp9 = sign(_skTemp8);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((_skTemp0 == expected.x) && all(_skTemp1 == expected.xy)) && all(_skTemp2 == expected.xyz)) && all(_skTemp3 == expected)) && all(_skTemp5 == vec2<f32>(1.0))) && all(_skTemp7 == vec2<f32>(0.0))) && all(_skTemp9 == vec2<f32>(1.0, 0.0))));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
