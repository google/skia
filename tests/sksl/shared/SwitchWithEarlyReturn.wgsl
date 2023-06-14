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
fn return_in_one_case_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    var val: i32 = 0;
    switch x {
      case 1 {
        val = val + i32(1);
        return false;
      }
      case default {
        val = val + i32(1);
      }
    }
    return val == 1;
  }
}
fn return_in_default_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    switch x {
      case default {
        return true;
      }
    }
  }
}
fn return_in_every_case_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    switch x {
      case 1 {
        return false;
      }
      case default {
        return true;
      }
    }
  }
}
fn return_in_every_case_no_default_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    var val: i32 = 0;
    switch x {
      case 1 {
        return false;
      }
      case 2 {
        return true;
      }
      case default {}
    }
    val = val + i32(1);
    return val == 1;
  }
}
fn case_has_break_before_return_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    var val: i32 = 0;
    switch x {
      case 1 {
        break;
      }
      case 2 {
        return true;
      }
      case default {
        return true;
      }
    }
    val = val + i32(1);
    return val == 1;
  }
}
fn case_has_break_after_return_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    switch x {
      case 1 {
        return false;
      }
      case 2 {
        return true;
      }
      case default {
        return true;
      }
    }
  }
}
fn no_return_in_default_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    var val: i32 = 0;
    switch x {
      case 1 {
        return false;
      }
      case 2 {
        return true;
      }
      case default {
        break;
      }
    }
    val = val + i32(1);
    return val == 1;
  }
}
fn empty_default_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    var val: i32 = 0;
    switch x {
      case 1 {
        return false;
      }
      case 2 {
        return true;
      }
      case default {
        ;
      }
    }
    val = val + i32(1);
    return val == 1;
  }
}
fn return_with_fallthrough_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    switch x {
      case 1, 2 {
        return true;
      }
      case default {
        return false;
      }
    }
  }
}
fn fallthrough_ends_in_break_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    var val: i32 = 0;
    switch x {
      case 1, 2 {
        break;
      }
      case default {
        return false;
      }
    }
    val = val + i32(1);
    return val == 1;
  }
}
fn fallthrough_to_default_with_break_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    var val: i32 = 0;
    switch x {
      case 1, 2, default {
        break;
      }
    }
    val = val + i32(1);
    return val == 1;
  }
}
fn fallthrough_to_default_with_return_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    switch x {
      case 1, 2, default {
        return true;
      }
    }
  }
}
fn fallthrough_with_loop_break_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    var val: i32 = 0;
    switch x {
      case default {}
      // cases missing due to fallthrough: 1, 2, default
    }
  }
  return bool();
}
fn fallthrough_with_loop_continue_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    var val: i32 = 0;
    switch x {
      case default {}
      // cases missing due to fallthrough: 1, 2, default
    }
  }
  return bool();
}
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var x: i32 = i32(_globalUniforms.colorGreen.y);
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
    var _skTemp10: bool;
    var _skTemp11: bool;
    var _skTemp12: bool;
    var _skTemp13: bool;
    let _skTemp14 = return_in_one_case_bi(x);
    if _skTemp14 {
      let _skTemp15 = return_in_default_bi(x);
      _skTemp13 = _skTemp15;
    } else {
      _skTemp13 = false;
    }
    if _skTemp13 {
      let _skTemp16 = return_in_every_case_bi(x);
      _skTemp12 = _skTemp16;
    } else {
      _skTemp12 = false;
    }
    if _skTemp12 {
      let _skTemp17 = return_in_every_case_no_default_bi(x);
      _skTemp11 = _skTemp17;
    } else {
      _skTemp11 = false;
    }
    if _skTemp11 {
      let _skTemp18 = case_has_break_before_return_bi(x);
      _skTemp10 = _skTemp18;
    } else {
      _skTemp10 = false;
    }
    if _skTemp10 {
      let _skTemp19 = case_has_break_after_return_bi(x);
      _skTemp9 = _skTemp19;
    } else {
      _skTemp9 = false;
    }
    if _skTemp9 {
      let _skTemp20 = no_return_in_default_bi(x);
      _skTemp8 = _skTemp20;
    } else {
      _skTemp8 = false;
    }
    if _skTemp8 {
      let _skTemp21 = empty_default_bi(x);
      _skTemp7 = _skTemp21;
    } else {
      _skTemp7 = false;
    }
    if _skTemp7 {
      let _skTemp22 = return_with_fallthrough_bi(x);
      _skTemp6 = _skTemp22;
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      let _skTemp23 = fallthrough_ends_in_break_bi(x);
      _skTemp5 = _skTemp23;
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      let _skTemp24 = fallthrough_to_default_with_break_bi(x);
      _skTemp4 = _skTemp24;
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      let _skTemp25 = fallthrough_to_default_with_return_bi(x);
      _skTemp3 = _skTemp25;
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      let _skTemp26 = fallthrough_with_loop_break_bi(x);
      _skTemp2 = _skTemp26;
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      let _skTemp27 = fallthrough_with_loop_continue_bi(x);
      _skTemp1 = _skTemp27;
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
