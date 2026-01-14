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
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  var coords = _skParam0;
  {
    const negativeVal: vec4<f32> = vec4<f32>(-1.0, -4.0, -16.0, -64.0);
    let _skTemp0 = negativeVal;
    coords = sqrt(_skTemp0).xy;
    let inputVal: vec4<f32> = vec4<f32>(_skUnpacked__globalUniforms_testMatrix2x2[0], _skUnpacked__globalUniforms_testMatrix2x2[1]) + vec4<f32>(0.0, 2.0, 6.0, 12.0);
    const expected: vec4<f32> = vec4<f32>(1.0, 2.0, 3.0, 4.0);
    const allowedDelta: vec4<f32> = vec4<f32>(0.05);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((abs(sqrt(inputVal.x) - 1.0) < 0.05) && all((abs(sqrt(inputVal.xy) - vec2<f32>(1.0, 2.0)) < vec2<f32>(0.05)))) && all((abs(sqrt(inputVal.xyz) - vec3<f32>(1.0, 2.0, 3.0)) < vec3<f32>(0.05)))) && all((abs(sqrt(inputVal) - expected) < allowedDelta))));
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
