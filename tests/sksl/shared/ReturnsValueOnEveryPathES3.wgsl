diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  unknownInput: f32,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn inside_while_loop_b() -> bool {
  {
    loop {
      if _globalUniforms.unknownInput == 123.0 {
        {
          return false;
        }
      } else {
        break;
      }
    }
    return true;
  }
}
fn inside_infinite_do_loop_b() -> bool {
  {
    loop {
      {
        return true;
      }
      continuing {
        break if false;
      }
    }
  }
  return bool();
}
fn inside_infinite_while_loop_b() -> bool {
  {
    loop {
      if true {
        {
          return true;
        }
      } else {
        break;
      }
    }
  }
  return bool();
}
fn after_do_loop_b() -> bool {
  {
    loop {
      {
        break;
      }
      continuing {
        break if false;
      }
    }
    return true;
  }
}
fn after_while_loop_b() -> bool {
  {
    loop {
      if true {
        {
          break;
        }
      } else {
        break;
      }
    }
    return true;
  }
}
fn switch_with_all_returns_b() -> bool {
  {
    let _skTemp0 = i32(_globalUniforms.unknownInput);
    switch _skTemp0 {
      case 1 {
        return true;
      }
      case 2 {
        return false;
      }
      case default {
        return false;
      }
    }
  }
}
fn switch_fallthrough_b() -> bool {
  {
    let _skTemp1 = i32(_globalUniforms.unknownInput);
    switch _skTemp1 {
      case 1 {
        return true;
      }
      case 2, default {
        return false;
      }
    }
  }
}
fn switch_fallthrough_twice_b() -> bool {
  {
    let _skTemp2 = i32(_globalUniforms.unknownInput);
    switch _skTemp2 {
      case 1, 2, default {
        return true;
      }
    }
  }
}
fn switch_with_break_in_loop_b() -> bool {
  {
    let _skTemp3 = i32(_globalUniforms.unknownInput);
    switch _skTemp3 {
      case 1, default {
        var _skTemp4: bool = false;
        if _skTemp3 == 1 {
          {
            var x: i32 = 0;
            loop {
              {
                break;
              }
              continuing {
                x = x + i32(1);
                break if x > 10;
              }
            }
          }
        }
        return true;
      }
    }
  }
}
fn switch_with_continue_in_loop_b() -> bool {
  {
    let _skTemp5 = i32(_globalUniforms.unknownInput);
    switch _skTemp5 {
      case 1, default {
        var _skTemp6: bool = false;
        if _skTemp5 == 1 {
          {
            var x: i32 = 0;
            loop {
              {
                continue;
              }
              continuing {
                x = x + i32(1);
                break if x > 10;
              }
            }
          }
        }
        return true;
      }
    }
  }
}
fn switch_with_if_that_returns_b() -> bool {
  {
    let _skTemp7 = i32(_globalUniforms.unknownInput);
    switch _skTemp7 {
      case 1, default {
        var _skTemp8: bool = false;
        if _skTemp7 == 1 {
          if _globalUniforms.unknownInput == 123.0 {
            return false;
          } else {
            return true;
          }
        }
        return true;
      }
    }
  }
}
fn switch_with_one_sided_if_then_fallthrough_b() -> bool {
  {
    let _skTemp9 = i32(_globalUniforms.unknownInput);
    switch _skTemp9 {
      case 1, default {
        var _skTemp10: bool = false;
        if _skTemp9 == 1 {
          if _globalUniforms.unknownInput == 123.0 {
            return false;
          }
        }
        return true;
      }
    }
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _skTemp11: vec4<f32>;
    var _skTemp12: bool;
    var _skTemp13: bool;
    var _skTemp14: bool;
    var _skTemp15: bool;
    var _skTemp16: bool;
    var _skTemp17: bool;
    var _skTemp18: bool;
    var _skTemp19: bool;
    var _skTemp20: bool;
    var _skTemp21: bool;
    var _skTemp22: bool;
    if inside_while_loop_b() {
      _skTemp22 = inside_infinite_do_loop_b();
    } else {
      _skTemp22 = false;
    }
    if _skTemp22 {
      _skTemp21 = inside_infinite_while_loop_b();
    } else {
      _skTemp21 = false;
    }
    if _skTemp21 {
      _skTemp20 = after_do_loop_b();
    } else {
      _skTemp20 = false;
    }
    if _skTemp20 {
      _skTemp19 = after_while_loop_b();
    } else {
      _skTemp19 = false;
    }
    if _skTemp19 {
      _skTemp18 = switch_with_all_returns_b();
    } else {
      _skTemp18 = false;
    }
    if _skTemp18 {
      _skTemp17 = switch_fallthrough_b();
    } else {
      _skTemp17 = false;
    }
    if _skTemp17 {
      _skTemp16 = switch_fallthrough_twice_b();
    } else {
      _skTemp16 = false;
    }
    if _skTemp16 {
      _skTemp15 = switch_with_break_in_loop_b();
    } else {
      _skTemp15 = false;
    }
    if _skTemp15 {
      _skTemp14 = switch_with_continue_in_loop_b();
    } else {
      _skTemp14 = false;
    }
    if _skTemp14 {
      _skTemp13 = switch_with_if_that_returns_b();
    } else {
      _skTemp13 = false;
    }
    if _skTemp13 {
      _skTemp12 = switch_with_one_sided_if_then_fallthrough_b();
    } else {
      _skTemp12 = false;
    }
    if _skTemp12 {
      _skTemp11 = _globalUniforms.colorGreen;
    } else {
      _skTemp11 = _globalUniforms.colorRed;
    }
    return _skTemp11;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
