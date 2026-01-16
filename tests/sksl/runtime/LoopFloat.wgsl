diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
const kZero: f32 = 0.0;
fn return_loop_ff(five: f32) -> f32 {
  {
    {
      var i: f32 = kZero;
      loop {
        {
          if i == five {
            {
              return i;
            }
          }
        }
        continuing {
          i = i + f32(1);
          break if i >= 10.0;
        }
      }
    }
    return 0.0;
  }
}
const kTen: f32 = 10.0;
fn continue_loop_ff(five: f32) -> f32 {
  {
    var sum: f32 = 0.0;
    {
      var i: f32 = 0.0;
      loop {
        {
          if i < five {
            {
              continue;
            }
          }
          sum = sum + i;
        }
        continuing {
          i = i + f32(1);
          break if i >= kTen;
        }
      }
    }
    return sum;
  }
}
fn break_loop_ff(five: f32) -> f32 {
  {
    var sum: f32 = 0.0;
    const kOne: f32 = 1.0;
    {
      var i: f32 = 0.0;
      loop {
        {
          if i > five {
            {
              break;
            }
          }
          sum = sum + i;
        }
        continuing {
          i = i + kOne;
          break if i >= 10.0;
        }
      }
    }
    return sum;
  }
}
fn float_loop_f() -> f32 {
  {
    var sum: f32 = 0.0;
    {
      var i: f32 = 0.123;
      loop {
        {
          sum = sum + i;
        }
        continuing {
          i = i + 0.111;
          break if i >= 0.6;
        }
      }
    }
    return sum - 1.725;
  }
}
fn loop_operator_le_b() -> bool {
  {
    var result: vec4<f32> = vec4<f32>(9.0);
    {
      var i: f32 = 1.0;
      loop {
        {
          result = vec4<f32>(result.yzw, i);
        }
        continuing {
          i = i + f32(1);
          break if i > 3.0;
        }
      }
    }
    return all(result == vec4<f32>(9.0, 1.0, 2.0, 3.0));
  }
}
fn loop_operator_lt_b() -> bool {
  {
    var result: vec4<f32> = vec4<f32>(9.0);
    {
      var i: f32 = 1.0;
      loop {
        {
          result = vec4<f32>(result.yzw, i);
        }
        continuing {
          i = i + f32(1);
          break if i >= 4.0;
        }
      }
    }
    return all(result == vec4<f32>(9.0, 1.0, 2.0, 3.0));
  }
}
fn loop_operator_ge_b() -> bool {
  {
    var result: vec4<f32> = vec4<f32>(9.0);
    {
      var i: f32 = 3.0;
      loop {
        {
          result = vec4<f32>(result.yzw, i);
        }
        continuing {
          i = i - f32(1);
          break if i < 1.0;
        }
      }
    }
    return all(result == vec4<f32>(9.0, 3.0, 2.0, 1.0));
  }
}
fn loop_operator_gt_b() -> bool {
  {
    var result: vec4<f32> = vec4<f32>(9.0);
    {
      var i: f32 = 3.0;
      loop {
        {
          result = vec4<f32>(result.yzw, i);
        }
        continuing {
          i = i - f32(1);
          break if i <= 0.0;
        }
      }
    }
    return all(result == vec4<f32>(9.0, 3.0, 2.0, 1.0));
  }
}
fn loop_operator_ne_b() -> bool {
  {
    var result: vec4<f32> = vec4<f32>(9.0);
    {
      var i: f32 = 1.0;
      loop {
        {
          result = vec4<f32>(result.yzw, i);
        }
        continuing {
          i = i + f32(1);
          break if i >= 4.0;
        }
      }
    }
    return all(result == vec4<f32>(9.0, 1.0, 2.0, 3.0));
  }
}
fn loop_operator_eq_b() -> bool {
  {
    var result: vec4<f32> = vec4<f32>(9.0);
    {
      var i: f32 = 1.0;
      loop {
        {
          result = vec4<f32>(result.yzw, i);
        }
        continuing {
          i = i + f32(1);
          break if i != 1.0;
        }
      }
    }
    return all(result == vec4<f32>(9.0, 9.0, 9.0, 1.0));
  }
}
fn _skslMain(pos: vec2<f32>) -> vec4<f32> {
  {
    let five: f32 = clamp(pos.x, f32(_globalUniforms.colorGreen.y), f32(_globalUniforms.colorGreen.w)) * 5.0;
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    var _skTemp2: bool;
    var _skTemp3: bool;
    var _skTemp4: bool;
    var _skTemp5: bool;
    var _skTemp6: bool;
    var _skTemp7: bool;
    var _skTemp8: bool;
    var _skTemp9: bool;
    if return_loop_ff(five) == 5.0 {
      _skTemp9 = (continue_loop_ff(five) == 35.0);
    } else {
      _skTemp9 = false;
    }
    if _skTemp9 {
      _skTemp8 = (break_loop_ff(five) == 15.0);
    } else {
      _skTemp8 = false;
    }
    if _skTemp8 {
      _skTemp7 = abs(float_loop_f()) < 0.025;
    } else {
      _skTemp7 = false;
    }
    if _skTemp7 {
      _skTemp6 = loop_operator_le_b();
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      _skTemp5 = loop_operator_lt_b();
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      _skTemp4 = loop_operator_ge_b();
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      _skTemp3 = loop_operator_gt_b();
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      _skTemp2 = loop_operator_eq_b();
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      _skTemp1 = loop_operator_ne_b();
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
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
