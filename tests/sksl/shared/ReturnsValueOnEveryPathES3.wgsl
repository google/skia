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
  unknownInput: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
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
          // fallthrough
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
          // fallthrough
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
          // fallthrough
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
          // fallthrough
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
    let _skTemp23 = inside_while_loop_b();
    if _skTemp23 {
      let _skTemp24 = inside_infinite_do_loop_b();
      _skTemp22 = _skTemp24;
    } else {
      _skTemp22 = false;
    }
    if _skTemp22 {
      let _skTemp25 = inside_infinite_while_loop_b();
      _skTemp21 = _skTemp25;
    } else {
      _skTemp21 = false;
    }
    if _skTemp21 {
      let _skTemp26 = after_do_loop_b();
      _skTemp20 = _skTemp26;
    } else {
      _skTemp20 = false;
    }
    if _skTemp20 {
      let _skTemp27 = after_while_loop_b();
      _skTemp19 = _skTemp27;
    } else {
      _skTemp19 = false;
    }
    if _skTemp19 {
      let _skTemp28 = switch_with_all_returns_b();
      _skTemp18 = _skTemp28;
    } else {
      _skTemp18 = false;
    }
    if _skTemp18 {
      let _skTemp29 = switch_fallthrough_b();
      _skTemp17 = _skTemp29;
    } else {
      _skTemp17 = false;
    }
    if _skTemp17 {
      let _skTemp30 = switch_fallthrough_twice_b();
      _skTemp16 = _skTemp30;
    } else {
      _skTemp16 = false;
    }
    if _skTemp16 {
      let _skTemp31 = switch_with_break_in_loop_b();
      _skTemp15 = _skTemp31;
    } else {
      _skTemp15 = false;
    }
    if _skTemp15 {
      let _skTemp32 = switch_with_continue_in_loop_b();
      _skTemp14 = _skTemp32;
    } else {
      _skTemp14 = false;
    }
    if _skTemp14 {
      let _skTemp33 = switch_with_if_that_returns_b();
      _skTemp13 = _skTemp33;
    } else {
      _skTemp13 = false;
    }
    if _skTemp13 {
      let _skTemp34 = switch_with_one_sided_if_then_fallthrough_b();
      _skTemp12 = _skTemp34;
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
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
