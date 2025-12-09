### Compilation failed:

error: Tint compilation failed.

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
    let inputVal: vec4<f32> = vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1]) * vec4<f32>(1.0, 1.0, -1.0, -1.0);
    const expectedB: vec4<i32> = vec4<i32>(1065353216, 1073741824, -1069547520, -1065353216);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((inputVal.x == intBitsToFloat(expectedB.x)) && all(inputVal.xy == intBitsToFloat(expectedB.xy))) && all(inputVal.xyz == intBitsToFloat(expectedB.xyz))) && all(inputVal == intBitsToFloat(expectedB))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}

1 error
