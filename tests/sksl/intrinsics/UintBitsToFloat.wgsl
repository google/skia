### Compilation failed:

error: :16:20 error: unresolved call target 'uintBitsToFloat'
    let _skTemp0 = uintBitsToFloat(expectedB.x);
                   ^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
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
    var expectedB: vec4<u32> = vec4<u32>(1065353216u, 1073741824u, 3225419776u, 3229614080u);
    let _skTemp0 = uintBitsToFloat(expectedB.x);
    let _skTemp1 = uintBitsToFloat(expectedB.xy);
    let _skTemp2 = uintBitsToFloat(expectedB.xyz);
    let _skTemp3 = uintBitsToFloat(expectedB);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((inputVal.x == _skTemp0) && all(inputVal.xy == _skTemp1)) && all(inputVal.xyz == _skTemp2)) && all(inputVal == _skTemp3)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}

1 error
