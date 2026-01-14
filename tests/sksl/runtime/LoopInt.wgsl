diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
const kZero: i32 = 0;
fn return_loop_ii(five: i32) -> i32 {
  {
    {
      var i: i32 = kZero;
      loop {
        {
          if i == five {
            {
              return i;
            }
          }
        }
        continuing {
          i = i + i32(1);
          break if i >= 10;
        }
      }
    }
    return 0;
  }
}
const kTen: i32 = 10;
fn continue_loop_ii(five: i32) -> i32 {
  {
    var sum: i32 = 0;
    {
      var i: i32 = 0;
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
          i = i + i32(1);
          break if i >= kTen;
        }
      }
    }
    return sum;
  }
}
fn break_loop_ii(five: i32) -> i32 {
  {
    var sum: i32 = 0;
    const kOne: i32 = 1;
    {
      var i: i32 = 0;
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
          break if i >= 10;
        }
      }
    }
    return sum;
  }
}
fn loop_operator_le_b() -> bool {
  {
    var result: vec4<i32> = vec4<i32>(8);
    {
      var i: i32 = 0;
      loop {
        {
          result = result + vec4<i32>(1);
        }
        continuing {
          i = i + i32(1);
          break if i > 0;
        }
      }
    }
    {
      var i: i32 = 1;
      loop {
        {
          result = vec4<i32>(result.yzw, i);
        }
        continuing {
          i = i + i32(1);
          break if i > 3;
        }
      }
    }
    return all(result == vec4<i32>(9, 1, 2, 3));
  }
}
fn loop_operator_lt_b() -> bool {
  {
    var result: vec4<i32> = vec4<i32>(8);
    {
      var i: i32 = 0;
      loop {
        {
          result = result + vec4<i32>(1);
        }
        continuing {
          i = i + i32(1);
          break if i >= 1;
        }
      }
    }
    {
      var i: i32 = 1;
      loop {
        {
          result = vec4<i32>(result.yzw, i);
        }
        continuing {
          i = i + i32(1);
          break if i >= 4;
        }
      }
    }
    return all(result == vec4<i32>(9, 1, 2, 3));
  }
}
fn loop_operator_ge_b() -> bool {
  {
    var result: vec4<i32> = vec4<i32>(8);
    {
      var i: i32 = 0;
      loop {
        {
          result = result + vec4<i32>(1);
        }
        continuing {
          i = i - i32(1);
          break if i < 0;
        }
      }
    }
    {
      var i: i32 = 3;
      loop {
        {
          result = vec4<i32>(result.yzw, i);
        }
        continuing {
          i = i - i32(1);
          break if i < 1;
        }
      }
    }
    return all(result == vec4<i32>(9, 3, 2, 1));
  }
}
fn loop_operator_gt_b() -> bool {
  {
    var result: vec4<i32> = vec4<i32>(8);
    {
      var i: i32 = 1;
      loop {
        {
          result = result + vec4<i32>(1);
        }
        continuing {
          i = i - i32(1);
          break if i <= 0;
        }
      }
    }
    {
      var i: i32 = 3;
      loop {
        {
          result = vec4<i32>(result.yzw, i);
        }
        continuing {
          i = i - i32(1);
          break if i <= 0;
        }
      }
    }
    return all(result == vec4<i32>(9, 3, 2, 1));
  }
}
fn loop_operator_ne_b() -> bool {
  {
    var result: vec4<i32> = vec4<i32>(8);
    {
      var i: i32 = 1;
      loop {
        {
          result = result + vec4<i32>(1);
        }
        continuing {
          i = i + i32(1);
          break if i == 2;
        }
      }
    }
    {
      var i: i32 = 1;
      loop {
        {
          result = vec4<i32>(result.yzw, i);
        }
        continuing {
          i = i + i32(1);
          break if i == 4;
        }
      }
    }
    return all(result == vec4<i32>(9, 1, 2, 3));
  }
}
fn loop_operator_eq_b() -> bool {
  {
    var result: vec4<i32> = vec4<i32>(9);
    {
      var i: i32 = 1;
      loop {
        {
          result = vec4<i32>(result.yzw, i);
        }
        continuing {
          i = i + i32(1);
          break if i != 1;
        }
      }
    }
    return all(result == vec4<i32>(9, 9, 9, 1));
  }
}
fn _skslMain(pos: vec2<f32>) -> vec4<f32> {
  {
    let five: i32 = i32(clamp(pos.x, f32(_globalUniforms.colorGreen.y), f32(_globalUniforms.colorGreen.w))) * 5;
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    var _skTemp2: bool;
    var _skTemp3: bool;
    var _skTemp4: bool;
    var _skTemp5: bool;
    var _skTemp6: bool;
    var _skTemp7: bool;
    var _skTemp8: bool;
    if return_loop_ii(five) == 5 {
      _skTemp8 = (continue_loop_ii(five) == 35);
    } else {
      _skTemp8 = false;
    }
    if _skTemp8 {
      _skTemp7 = (break_loop_ii(5) == 15);
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
