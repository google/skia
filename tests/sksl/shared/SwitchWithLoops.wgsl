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
fn switch_with_continue_in_loop_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    var val: i32 = 0;
    switch x {
      case default {}
      // cases missing due to fallthrough: 1, default
    }
    return val == 11;
  }
}
fn loop_with_break_in_switch_bi(_skParam0: i32) -> bool {
  let x = _skParam0;
  {
    var val: i32 = 0;
    {
      var i: i32 = 0;
      loop {
        {
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
          i = i + i32(1);
          break if i >= 10;
        }
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
    switch x {
      case default {}
      // cases missing due to fallthrough: 1, default
    }
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    var _skTemp2: bool;
    if _0_val == 2 {
      let _skTemp3 = switch_with_continue_in_loop_bi(x);
      _skTemp2 = _skTemp3;
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      let _skTemp4 = loop_with_break_in_switch_bi(x);
      _skTemp1 = _skTemp4;
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
