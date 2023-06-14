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
fn switch_fallthrough_twice_bi(_skParam0: i32) -> bool {
  let value = _skParam0;
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
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
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
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    if _0_ok {
      let _skTemp2 = switch_fallthrough_twice_bi(x);
      _skTemp1 = _skTemp2;
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
