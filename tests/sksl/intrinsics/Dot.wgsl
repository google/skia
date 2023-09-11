diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testMatrix4x4: mat4x4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var inputA: vec4<f32> = _globalUniforms.testMatrix4x4[0];
    var inputB: vec4<f32> = _globalUniforms.testMatrix4x4[1];
    var expected: vec4<f32> = vec4<f32>(5.0, 17.0, 38.0, 70.0);
    let _skTemp0 = dot(inputA.xy, inputB.xy);
    let _skTemp1 = dot(inputA.xyz, inputB.xyz);
    let _skTemp2 = dot(inputA, inputB);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((((inputA.x * inputB.x) == expected.x) && (_skTemp0 == expected.y)) && (_skTemp1 == expected.z)) && (_skTemp2 == expected.w)) && (5.0 == expected.x)) && (17.0 == expected.y)) && (38.0 == expected.z)) && (70.0 == expected.w)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
