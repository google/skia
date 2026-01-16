diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testArray: array<_skArrayElement_f, 5>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    {
      var index: i32 = 0;
      loop {
        {
          if _skUnpacked__globalUniforms_testArray[index] != f32(index + 1) {
            {
              return _globalUniforms.colorRed;
            }
          }
        }
        continuing {
          index = index + i32(1);
          break if index >= 5;
        }
      }
    }
    return _globalUniforms.colorGreen;
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
