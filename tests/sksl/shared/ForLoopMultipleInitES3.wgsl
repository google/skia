/*

:93:3 warning: code is unreachable
  return vec4<f32>();
  ^^^^^^

*/

diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var sumA: f32 = 0.0;
    var sumB: f32 = 0.0;
    {
      var a: f32 = 0.0;
      var b: f32 = 10.0;
      loop {
        if (a < 10.0) && (b > 0.0) {
          {
            sumA = sumA + a;
            sumB = sumB + b;
          }
        } else {
          break;
        }
        continuing {
          a = a + f32(1);
          b = b - f32(1);
        }
      }
    }
    if (sumA != 45.0) || (sumB != 55.0) {
      {
        return _globalUniforms.colorRed;
      }
    }
    var sumC: i32 = 0;
    {
      var c: i32 = 0;
      loop {
        if c < 10 {
          {
            sumC = sumC + c;
          }
        } else {
          break;
        }
        continuing {
          c = c + i32(1);
        }
      }
    }
    if sumC != 45 {
      {
        return _globalUniforms.colorRed;
      }
    }
    var sumE: f32 = 0.0;
    {
      var d: array<f32, 2> = array<f32, 2>(0.0, 10.0);
      var e: array<f32, 4> = array<f32, 4>(1.0, 2.0, 3.0, 4.0);
      loop {
        if d[0] < d[1] {
          {
            sumE = sumE + f32(f32(e[0]));
          }
        } else {
          break;
        }
        continuing {
          d[0] = d[0] + f32(1);
        }
      }
    }
    if sumE != 10.0 {
      {
        return _globalUniforms.colorRed;
      }
    }
    {
      loop {
        break;
      }
    }
    {
      ;

      loop {
        return _globalUniforms.colorGreen;
      }
    }
  }
  return vec4<f32>();
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
