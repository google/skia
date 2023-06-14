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
fn switch_with_continue_in_while_loop_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    var val: i32 = 0;
    var i: i32 = 0;
    switch x {
      case default {}
      // cases missing due to fallthrough: 1, default
    }
    return val == 11;
  }
}
fn while_loop_with_break_in_switch_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
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
fn switch_with_break_in_do_while_loop_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    var val: i32 = 0;
    var i: i32 = 0;
    switch x {
      case default {}
      // cases missing due to fallthrough: 1, default
    }
    return val == 2;
  }
}
fn switch_with_continue_in_do_while_loop_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    var val: i32 = 0;
    var i: i32 = 0;
    switch x {
      case default {}
      // cases missing due to fallthrough: 1, default
    }
    return val == 11;
  }
}
fn do_while_loop_with_break_in_switch_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
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
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var x: i32 = i32(_globalUniforms.colorGreen.y);
    var _0_val: i32 = 0;
    var _1_i: i32 = 0;
    switch x {
      case default {}
      // cases missing due to fallthrough: 1, default
    }
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    var _skTemp2: bool;
    var _skTemp3: bool;
    var _skTemp4: bool;
    var _skTemp5: bool;
    if _0_val == 2 {
      let _skTemp6 = switch_with_continue_in_while_loop_bi(x);
      _skTemp5 = _skTemp6;
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      let _skTemp7 = while_loop_with_break_in_switch_bi(x);
      _skTemp4 = _skTemp7;
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      let _skTemp8 = switch_with_break_in_do_while_loop_bi(x);
      _skTemp3 = _skTemp8;
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      let _skTemp9 = switch_with_continue_in_do_while_loop_bi(x);
      _skTemp2 = _skTemp9;
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      let _skTemp10 = do_while_loop_with_break_in_switch_bi(x);
      _skTemp1 = _skTemp10;
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
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
