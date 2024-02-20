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
    var expected: vec4<i32> = vec4<i32>(-1, 0, 0, 1);
    let _skTemp0 = sign(i32(_globalUniforms.testInputs.x));
    let _skTemp1 = sign(vec2<i32>(_globalUniforms.testInputs.xy));
    let _skTemp2 = sign(vec3<i32>(_globalUniforms.testInputs.xyz));
    let _skTemp3 = sign(vec4<i32>(_globalUniforms.testInputs));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((_skTemp0 == expected.x) && all(_skTemp1 == expected.xy)) && all(_skTemp2 == expected.xyz)) && all(_skTemp3 == expected)) && (-1 == expected.x)) && all(vec2<i32>(-1, 0) == expected.xy)) && all(vec3<i32>(-1, 0, 0) == expected.xyz)) && all(vec4<i32>(-1, 0, 0, 1) == expected)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
