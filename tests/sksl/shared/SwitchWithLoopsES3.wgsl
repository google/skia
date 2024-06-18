diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn switch_with_continue_in_while_loop_bi(x: i32) -> bool {
  {
    var val: i32 = 0;
    var i: i32 = 0;
    switch x {
      case 1, default {
        var _skTemp0: bool = false;
        if x == 1 {
          loop {
            if i < 10 {
              {
                i = i + i32(1);
                val = val + i32(1);
                continue;
              }
            } else {
              break;
            }
          }
          // fallthrough
        }
        val = val + i32(1);
      }
    }
    return val == 11;
  }
}
fn while_loop_with_break_in_switch_bi(x: i32) -> bool {
  {
    var val: i32 = 0;
    var i: i32 = 0;
    loop {
      if i < 10 {
        {
          i = i + i32(1);
          switch x {
            case 1 {
              val = val + i32(1);
              break;
            }
            case default {
              return false;
            }
          }
          val = val + i32(1);
        }
      } else {
        break;
      }
    }
    return val == 20;
  }
}
fn switch_with_break_in_do_while_loop_bi(x: i32) -> bool {
  {
    var val: i32 = 0;
    var i: i32 = 0;
    switch x {
      case 1, default {
        var _skTemp1: bool = false;
        if x == 1 {
          loop {
            {
              i = i + i32(1);
              val = val + i32(1);
              break;
            }
            continuing {
              break if i >= 10;
            }
          }
          // fallthrough
        }
        val = val + i32(1);
      }
    }
    return val == 2;
  }
}
fn switch_with_continue_in_do_while_loop_bi(x: i32) -> bool {
  {
    var val: i32 = 0;
    var i: i32 = 0;
    switch x {
      case 1, default {
        var _skTemp2: bool = false;
        if x == 1 {
          loop {
            {
              i = i + i32(1);
              val = val + i32(1);
              continue;
            }
            continuing {
              break if i >= 10;
            }
          }
          // fallthrough
        }
        val = val + i32(1);
      }
    }
    return val == 11;
  }
}
fn do_while_loop_with_break_in_switch_bi(x: i32) -> bool {
  {
    var val: i32 = 0;
    var i: i32 = 0;
    loop {
      {
        i = i + i32(1);
        switch x {
          case 1 {
            val = val + i32(1);
            break;
          }
          case default {
            return false;
          }
        }
        val = val + i32(1);
      }
      continuing {
        break if i >= 10;
      }
    }
    return val == 20;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var x: i32 = i32(_globalUniforms.colorGreen.y);
    var _0_val: i32 = 0;
    var _1_i: i32 = 0;
    switch x {
      case 1, default {
        var _skTemp3: bool = false;
        if x == 1 {
          loop {
            if _1_i < 10 {
              {
                _1_i = _1_i + i32(1);
                _0_val = _0_val + i32(1);
                break;
              }
            } else {
              break;
            }
          }
          // fallthrough
        }
        _0_val = _0_val + i32(1);
      }
    }
    var _skTemp4: vec4<f32>;
    var _skTemp5: bool;
    var _skTemp6: bool;
    var _skTemp7: bool;
    var _skTemp8: bool;
    var _skTemp9: bool;
    if _0_val == 2 {
      let _skTemp10 = switch_with_continue_in_while_loop_bi(x);
      _skTemp9 = _skTemp10;
    } else {
      _skTemp9 = false;
    }
    if _skTemp9 {
      let _skTemp11 = while_loop_with_break_in_switch_bi(x);
      _skTemp8 = _skTemp11;
    } else {
      _skTemp8 = false;
    }
    if _skTemp8 {
      let _skTemp12 = switch_with_break_in_do_while_loop_bi(x);
      _skTemp7 = _skTemp12;
    } else {
      _skTemp7 = false;
    }
    if _skTemp7 {
      let _skTemp13 = switch_with_continue_in_do_while_loop_bi(x);
      _skTemp6 = _skTemp13;
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      let _skTemp14 = do_while_loop_with_break_in_switch_bi(x);
      _skTemp5 = _skTemp14;
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      _skTemp4 = _globalUniforms.colorGreen;
    } else {
      _skTemp4 = _globalUniforms.colorRed;
    }
    return _skTemp4;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
