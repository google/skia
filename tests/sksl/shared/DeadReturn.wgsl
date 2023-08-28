/*

:47:3 warning: code is unreachable
  return bool();
  ^^^^^^

*/

diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
var<private> scratchVar: i32 = 0;
fn test_flat_b() -> bool {
  {
    return true;
  }
}
fn test_if_b() -> bool {
  {
    if _globalUniforms.colorGreen.y > 0.0 {
      {
        return true;
      }
    } else {
      {
        scratchVar = scratchVar + i32(1);
      }
    }
    scratchVar = scratchVar + i32(1);
    return false;
  }
}
fn test_else_b() -> bool {
  {
    if _globalUniforms.colorGreen.y == 0.0 {
      {
        return false;
      }
    } else {
      {
        return true;
      }
    }
  }
  return bool();
}
fn test_loop_if_b() -> bool {
  {
    {
      var x: i32 = 0;
      loop {
        {
          if _globalUniforms.colorGreen.y == 0.0 {
            {
              return false;
            }
          } else {
            {
              return true;
            }
          }
        }
        continuing {
          x = x + i32(1);
          break if x > 1;
        }
      }
    }
    scratchVar = scratchVar + i32(1);
    return true;
  }
}
fn _skslMain(xy: vec2<f32>) -> vec4<f32> {
  {
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    var _skTemp2: bool;
    var _skTemp3: bool;
    let _skTemp4 = test_flat_b();
    if _skTemp4 {
      let _skTemp5 = test_if_b();
      _skTemp3 = _skTemp5;
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      let _skTemp6 = test_else_b();
      _skTemp2 = _skTemp6;
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      let _skTemp7 = test_loop_if_b();
      _skTemp1 = _skTemp7;
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
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
