diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testMatrix2x2: _skMatrix22,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let inputVal: vec4<f32> = vec4<f32>(_skUnpacked__globalUniforms_testMatrix2x2[0], _skUnpacked__globalUniforms_testMatrix2x2[1]) * vec4<f32>(1.0, 1.0, -1.0, -1.0);
    const expectedB: vec4<i32> = vec4<i32>(1065353216, 1073741824, -1069547520, -1065353216);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((inputVal.x == bitcast<f32>(expectedB.x)) && all(inputVal.xy == bitcast<vec2<f32>>(expectedB.xy))) && all(inputVal.xyz == bitcast<vec3<f32>>(expectedB.xyz))) && all(inputVal == bitcast<vec4<f32>>(expectedB))));
  }
}
@fragment fn main() -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
struct _skRow2 {
  @align(16) r : vec2<f32>
};
struct _skMatrix22 {
  c : array<_skRow2, 2>
};
var<private> _skUnpacked__globalUniforms_testMatrix2x2: mat2x2<f32>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__globalUniforms_testMatrix2x2 = mat2x2<f32>(_globalUniforms.testMatrix2x2.c[0].r, _globalUniforms.testMatrix2x2.c[1].r);
}
