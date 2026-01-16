diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn test_return_b() -> bool {
  {
    loop {
      {
        return true;
      }
      continuing {
        break if true;
      }
    }
  }
  return bool();
}
fn test_break_b() -> bool {
  {
    loop {
      {
        break;
      }
      continuing {
        break if true;
      }
    }
    return true;
  }
}
fn test_continue_b() -> bool {
  {
    loop {
      {
        continue;
      }
      continuing {
        break if true;
      }
    }
    return true;
  }
}
fn test_if_return_b() -> bool {
  {
    loop {
      {
        if _globalUniforms.colorGreen.y > 0.0 {
          {
            return true;
          }
        } else {
          {
            break;
          }
        }
        continue;
      }
      continuing {
        break if true;
      }
    }
    return false;
  }
}
fn test_if_break_b() -> bool {
  {
    loop {
      {
        if _globalUniforms.colorGreen.y > 0.0 {
          {
            break;
          }
        } else {
          {
            continue;
          }
        }
      }
      continuing {
        break if true;
      }
    }
    return true;
  }
}
fn test_else_b() -> bool {
  {
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
        break if true;
      }
    }
  }
  return bool();
}
fn test_loop_return_b() -> bool {
  {
    return true;
  }
}
fn test_loop_break_b() -> bool {
  {
    {
      var x: i32 = 0;
      loop {
        {
          break;
        }
        continuing {
          x = x + i32(1);
          break if x > 1;
        }
      }
    }
    return true;
  }
}
fn _skslMain(xy: vec2<f32>) -> vec4<f32> {
  {
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    var _skTemp2: bool;
    var _skTemp3: bool;
    var _skTemp4: bool;
    var _skTemp5: bool;
    var _skTemp6: bool;
    var _skTemp7: bool;
    if test_return_b() {
      _skTemp7 = test_break_b();
    } else {
      _skTemp7 = false;
    }
    if _skTemp7 {
      _skTemp6 = test_continue_b();
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      _skTemp5 = test_if_return_b();
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      _skTemp4 = test_if_break_b();
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      _skTemp3 = test_else_b();
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      _skTemp2 = test_loop_return_b();
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      _skTemp1 = test_loop_break_b();
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
