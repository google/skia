diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorRed: vec4<f16>,
  testMatrix2x2: _skMatrix22h,
  testArray: array<_skArrayElement_h, 5>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
const globalArray: array<f16, 5> = array<f16, 5>(1.0h, 1.0h, 1.0h, 1.0h, 1.0h);
const globalVector: vec2<f16> = vec2<f16>(1.0h);
const globalMatrix: mat2x2<f16> = mat2x2<f16>(1.0h, 1.0h, 1.0h, 1.0h);
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f16> {
  {
    const localArray: array<f16, 5> = array<f16, 5>(0.0h, 1.0h, 2.0h, 3.0h, 4.0h);
    const localVector: vec2<f16> = vec2<f16>(1.0h);
    const localMatrix: mat2x2<f16> = mat2x2<f16>(0.0h, 1.0h, 2.0h, 3.0h);
    if ((((((globalArray[0] == _skUnpacked__globalUniforms_testArray[0]) && (globalArray[1] == _skUnpacked__globalUniforms_testArray[1]) && (globalArray[2] == _skUnpacked__globalUniforms_testArray[2]) && (globalArray[3] == _skUnpacked__globalUniforms_testArray[3]) && (globalArray[4] == _skUnpacked__globalUniforms_testArray[4])) || all(globalVector == _globalUniforms.colorRed.xy)) || (all(globalMatrix[0] == _skUnpacked__globalUniforms_testMatrix2x2[0]) && all(globalMatrix[1] == _skUnpacked__globalUniforms_testMatrix2x2[1]))) || ((localArray[0] == _skUnpacked__globalUniforms_testArray[0]) && (localArray[1] == _skUnpacked__globalUniforms_testArray[1]) && (localArray[2] == _skUnpacked__globalUniforms_testArray[2]) && (localArray[3] == _skUnpacked__globalUniforms_testArray[3]) && (localArray[4] == _skUnpacked__globalUniforms_testArray[4]))) || all(localVector == _globalUniforms.colorRed.xy)) || (all(localMatrix[0] == _skUnpacked__globalUniforms_testMatrix2x2[0]) && all(localMatrix[1] == _skUnpacked__globalUniforms_testMatrix2x2[1])) {
      {
        return _globalUniforms.colorRed;
      }
    }
    return vec4<f16>(0.0h, 1.0h, 0.0h, 1.0h);
  }
}
@fragment fn main() -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
struct _skArrayElement_h {
  @align(16) e : f16
};
var<private> _skUnpacked__globalUniforms_testArray: array<f16, 5>;
struct _skRow2h {
  @align(16) r : vec2<f16>
};
struct _skMatrix22h {
  c : array<_skRow2h, 2>
};
var<private> _skUnpacked__globalUniforms_testMatrix2x2: mat2x2<f16>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__globalUniforms_testArray = array<f16, 5>(_globalUniforms.testArray[0].e, _globalUniforms.testArray[1].e, _globalUniforms.testArray[2].e, _globalUniforms.testArray[3].e, _globalUniforms.testArray[4].e);
  _skUnpacked__globalUniforms_testMatrix2x2 = mat2x2<f16>(_globalUniforms.testMatrix2x2.c[0].r, _globalUniforms.testMatrix2x2.c[1].r);
}
