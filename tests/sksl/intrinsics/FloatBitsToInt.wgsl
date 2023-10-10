### Compilation failed:

error: :15:20 error: unresolved call target 'floatBitsToInt'
    let _skTemp0 = floatBitsToInt(inputVal.x);
                   ^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
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
    const expectedB: vec4<i32> = vec4<i32>(1065353216, 1073741824, -1069547520, -1065353216);
    let _skTemp0 = floatBitsToInt(inputVal.x);
    let _skTemp1 = floatBitsToInt(inputVal.xy);
    let _skTemp2 = floatBitsToInt(inputVal.xyz);
    let _skTemp3 = floatBitsToInt(inputVal);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((_skTemp0 == 1065353216) && all(_skTemp1 == vec2<i32>(1065353216, 1073741824))) && all(_skTemp2 == vec3<i32>(1065353216, 1073741824, -1069547520))) && all(_skTemp3 == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}

1 error
