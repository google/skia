diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var result: vec4<f32> = vec4<f32>(0.0);
    {
      var a: f32 = 0.0;
      var b: f32 = 0.0;
      loop {
        if (a < 10.0) && (b < 10.0) {
          {
            result.x = result.x + a;
            result.y = result.y + b;
          }
        } else {
          break;
        }
        continuing {
          a = a + f32(1);
          b = b + f32(1);
        }
      }
    }
    {
      var c: i32 = 0;
      loop {
        if c < 10 {
          {
            result.z = result.z + 1.0;
          }
        } else {
          break;
        }
        continuing {
          c = c + i32(1);
        }
      }
    }
    {
      var d: array<f32, 2> = array<f32, 2>(0.0, 10.0);
      var e: array<f32, 4> = array<f32, 4>(1.0, 2.0, 3.0, 4.0);
      var f: f32 = 9.0;
      loop {
        if d[0] < d[1] {
          {
            result.w = f32(e[0] * f);
          }
        } else {
          break;
        }
        continuing {
          d[0] = d[0] + f32(1);
        }
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
        break;
      }
    }
    return result;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
