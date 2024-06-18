diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var zero: i32 = i32(_globalUniforms.colorGreen.x);
    var one: i32 = i32(_globalUniforms.colorGreen.y);
    {
      var x: i32 = zero;
      loop {
        if x < 100 {
          {
            {
              var y: i32 = one;
              loop {
                if y < 100 {
                  {
                    var _0_x: i32 = x;
                    var _1_result: i32 = 0;
                    loop {
                      if _0_x >= y {
                        {
                          _1_result = _1_result + i32(1);
                          _0_x = _0_x - y;
                        }
                      } else {
                        break;
                      }
                    }
                    if (x / y) != _1_result {
                      {
                        return vec4<f32>(1.0, f32(f32(x) * 0.003921569), f32(f32(y) * 0.003921569), 1.0);
                      }
                    }
                  }
                } else {
                  break;
                }
                continuing {
                  y = y + i32(1);
                }
              }
            }
          }
        } else {
          break;
        }
        continuing {
          x = x + i32(1);
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
