### Compilation failed:

error: :18:20 error: unresolved call target 'intBitsToFloat'
    let _skTemp0 = intBitsToFloat(expectedB.x);
                   ^^^^^^^^^^^^^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testMatrix2x2: mat2x2<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var inputVal: vec4<f32> = vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1]) * vec4<f32>(1.0, 1.0, -1.0, -1.0);
    var expectedB: vec4<i32> = vec4<i32>(1065353216, 1073741824, -1069547520, -1065353216);
    let _skTemp0 = intBitsToFloat(expectedB.x);
    let _skTemp1 = intBitsToFloat(expectedB.xy);
    let _skTemp2 = intBitsToFloat(expectedB.xyz);
    let _skTemp3 = intBitsToFloat(expectedB);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((inputVal.x == _skTemp0) && all(inputVal.xy == _skTemp1)) && all(inputVal.xyz == _skTemp2)) && all(inputVal == _skTemp3)));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}

1 error
