diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
  testMatrix3x3: mat3x3<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn GetTestMatrix_f33() -> mat3x3<f32> {
  {
    return _globalUniforms.testMatrix3x3;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var expected: f32 = 0.0;
    {
      var i: i32 = 0;
      for (; i < 3; i = i + i32(1)) {
        {
          {
            var j: i32 = 0;
            for (; j < 3; j = j + i32(1)) {
              {
                expected = expected + 1.0;
                if GetTestMatrix_f33()[i][j] != expected {
                  {
                    return _globalUniforms.colorRed;
                  }
                }
              }
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
