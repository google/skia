diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  testMatrixArray: array<mat3x3<f32>, 3>,
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    {
      var index: i32 = 0;
      for (; index < 3; index = index + i32(1)) {
        {
          if _globalUniforms.testMatrixArray[index][index][index] != 1.0 {
            {
              return _globalUniforms.colorRed;
            }
          }
        }
      }
    }
    return _globalUniforms.colorGreen;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
