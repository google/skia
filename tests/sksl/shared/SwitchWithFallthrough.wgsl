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
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn switch_fallthrough_twice_bi(value: i32) -> bool {
  {
    var ok: bool = false;
    switch value {
      case 0 {
        break;
      }
      case 1, 2, 3 {
        ok = true;
        break;
      }
      case default {
        break;
      }
    }
    return ok;
  }
}
fn switch_fallthrough_groups_bi(value: i32) -> bool {
  {
    var ok: bool = false;
    switch value {
      case -1, 0 {
        var _skTemp0: bool = false;
        if value == -1 {
          ok = false;
          // fallthrough
        }
        return false;
      }
      case 1, 2, 3 {
        var _skTemp1: bool = false;
        if value == 1 {
          ok = true;
          _skTemp1 = true;  // fallthrough
        }
        if _skTemp1 || value == 2 {
          ;
          // fallthrough
        }
        break;
      }
      case 4, 5, 6, 7, default {
        var _skTemp2: bool = false;
        if value == 4 {
          ok = false;
          _skTemp2 = true;  // fallthrough
        }
        if _skTemp2 || value == 5 {
          ;
          _skTemp2 = true;  // fallthrough
        }
        if _skTemp2 || value == 6 {
          ;
          _skTemp2 = true;  // fallthrough
        }
        if _skTemp2 || value == 7 {
          ;
          // fallthrough
        }
        break;
      }
    }
    return ok;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var x: i32 = i32(_globalUniforms.colorGreen.y);
    var _0_ok: bool = false;
    switch x {
      case 2 {
        break;
      }
      case 1, 0 {
        _0_ok = true;
        break;
      }
      case default {
        break;
      }
    }
    var _skTemp3: vec4<f32>;
    var _skTemp4: bool;
    var _skTemp5: bool;
    if _0_ok {
      let _skTemp6 = switch_fallthrough_twice_bi(x);
      _skTemp5 = _skTemp6;
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      let _skTemp7 = switch_fallthrough_groups_bi(x);
      _skTemp4 = _skTemp7;
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      _skTemp3 = _globalUniforms.colorGreen;
    } else {
      _skTemp3 = _globalUniforms.colorRed;
    }
    return _skTemp3;
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
