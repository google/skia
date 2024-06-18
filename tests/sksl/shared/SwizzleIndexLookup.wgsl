diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  testMatrix3x3: mat3x3<f32>,
  testMatrix4x4: mat4x4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn test3x3_b() -> bool {
  {
    var expected: vec3<f32> = vec3<f32>(3.0, 2.0, 1.0);
    {
      var c: i32 = 0;
      loop {
        {
          var vec: vec3<f32> = _globalUniforms.testMatrix3x3[c];
          {
            var r: i32 = 0;
            loop {
              {
                if vec.zyx[r] != expected[r] {
                  {
                    return false;
                  }
                }
              }
              continuing {
                r = r + i32(1);
                break if r >= 3;
              }
            }
          }
          expected = expected + 3.0;
        }
        continuing {
          c = c + i32(1);
          break if c >= 3;
        }
      }
    }
    return true;
  }
}
fn test4x4_b() -> bool {
  {
    var expected: vec4<f32> = vec4<f32>(4.0, 3.0, 2.0, 1.0);
    {
      var c: i32 = 0;
      loop {
        {
          var vec: vec4<f32> = _globalUniforms.testMatrix4x4[c];
          {
            var r: i32 = 0;
            loop {
              {
                if vec.wzyx[r] != expected[r] {
                  {
                    return false;
                  }
                }
              }
              continuing {
                r = r + i32(1);
                break if r >= 4;
              }
            }
          }
          expected = expected + 4.0;
        }
        continuing {
          c = c + i32(1);
          break if c >= 4;
        }
      }
    }
    return true;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    let _skTemp2 = test3x3_b();
    if _skTemp2 {
      let _skTemp3 = test4x4_b();
      _skTemp1 = _skTemp3;
    } else {
      _skTemp1 = false;
    }
    if _skTemp1 {
      _skTemp0 = _globalUniforms.colorGreen;
    } else {
      _skTemp0 = _globalUniforms.colorRed;
    }
    return _skTemp0;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
