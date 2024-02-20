diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  testMatrix3x3: mat3x3<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn GetTestMatrix_f33() -> mat3x3<f32> {
  {
    return _globalUniforms.testMatrix3x3;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var expected: f32 = 0.0;
    {
      var i: i32 = 0;
      loop {
        {
          {
            var j: i32 = 0;
            loop {
              {
                expected = expected + 1.0;
                let _skTemp0 = GetTestMatrix_f33();
                if _skTemp0[i][j] != expected {
                  {
                    return _globalUniforms.colorRed;
                  }
                }
              }
              continuing {
                j = j + i32(1);
                break if j >= 3;
              }
            }
          }
        }
        continuing {
          i = i + i32(1);
          break if i >= 3;
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
