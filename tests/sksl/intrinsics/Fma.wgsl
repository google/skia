diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  testArray: array<_skArrayElement_f, 5>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let one: f32 = _skUnpacked__globalUniforms_testArray[0];
    let two: f32 = _skUnpacked__globalUniforms_testArray[1];
    let three: f32 = _skUnpacked__globalUniforms_testArray[2];
    let four: f32 = f32(_skUnpacked__globalUniforms_testArray[3]);
    let five: f32 = f32(_skUnpacked__globalUniforms_testArray[4]);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((fma(one, two, three) == 5.0) && (fma(f32(three), four, five) == 17.0)));
  }
}
@fragment fn main() -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
struct _skArrayElement_f {
  @align(16) e : f32
};
var<private> _skUnpacked__globalUniforms_testArray: array<f32, 5>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__globalUniforms_testArray = array<f32, 5>(_globalUniforms.testArray[0].e, _globalUniforms.testArray[1].e, _globalUniforms.testArray[2].e, _globalUniforms.testArray[3].e, _globalUniforms.testArray[4].e);
}
