diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn switch_with_continue_in_loop_bi(x: i32) -> bool {
  {
    var val: i32 = 0;
    switch x {
      case 1, default {
        var _skTemp0: bool = false;
        if x == 1 {
          {
            var i: i32 = 0;
            loop {
              {
                val = val + i32(1);
                continue;
              }
              continuing {
                i = i + i32(1);
                break if i >= 10;
              }
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
fn loop_with_break_in_switch_bi(x: i32) -> bool {
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
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var x: i32 = i32(_globalUniforms.colorGreen.y);
    var _0_val: i32 = 0;
    switch x {
      case 1, default {
        var _skTemp1: bool = false;
        if x == 1 {
          {
            var _1_i: i32 = 0;
            loop {
              {
                _0_val = _0_val + i32(1);
                break;
              }
              continuing {
                _1_i = _1_i + i32(1);
                break if _1_i >= 10;
              }
            }
          }
          // fallthrough
        }
        _0_val = _0_val + i32(1);
      }
    }
    var _skTemp2: vec4<f32>;
    var _skTemp3: bool;
    var _skTemp4: bool;
    if _0_val == 2 {
      let _skTemp5 = switch_with_continue_in_loop_bi(x);
      _skTemp4 = _skTemp5;
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      let _skTemp6 = loop_with_break_in_switch_bi(x);
      _skTemp3 = _skTemp6;
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      _skTemp2 = _globalUniforms.colorGreen;
    } else {
      _skTemp2 = _globalUniforms.colorRed;
    }
    return _skTemp2;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
