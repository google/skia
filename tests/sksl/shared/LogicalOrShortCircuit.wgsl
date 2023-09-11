/*

:31:3 warning: code is unreachable
  return bool();
  ^^^^^^

:54:3 warning: code is unreachable
  return bool();
  ^^^^^^

:77:3 warning: code is unreachable
  return bool();
  ^^^^^^

*/

diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn TrueFalse_b() -> bool {
  {
    var x: i32 = 1;
    var y: i32 = 1;
    var _skTemp0: bool;
    if x == 1 {
      _skTemp0 = true;
    } else {
      y = y + 1;
      _skTemp0 = (y == 3);
    }
    if _skTemp0 {
      {
        return (x == 1) && (y == 1);
      }
    } else {
      {
        return false;
      }
    }
  }
  return bool();
}
fn FalseTrue_b() -> bool {
  {
    var x: i32 = 1;
    var y: i32 = 1;
    var _skTemp1: bool;
    if x == 2 {
      _skTemp1 = true;
    } else {
      y = y + 1;
      _skTemp1 = (y == 2);
    }
    if _skTemp1 {
      {
        return (x == 1) && (y == 2);
      }
    } else {
      {
        return false;
      }
    }
  }
  return bool();
}
fn FalseFalse_b() -> bool {
  {
    var x: i32 = 1;
    var y: i32 = 1;
    var _skTemp2: bool;
    if x == 2 {
      _skTemp2 = true;
    } else {
      y = y + 1;
      _skTemp2 = (y == 3);
    }
    if _skTemp2 {
      {
        return false;
      }
    } else {
      {
        return (x == 1) && (y == 2);
      }
    }
  }
  return bool();
}
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  {
    var _0_TrueTrue: bool;
    var _2_y: i32 = 1;
    {
      _0_TrueTrue = (_2_y == 1);
    }
    var _skTemp3: vec4<f32>;
    var _skTemp4: bool;
    var _skTemp5: bool;
    var _skTemp6: bool;
    if _0_TrueTrue {
      let _skTemp7 = TrueFalse_b();
      _skTemp6 = _skTemp7;
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      let _skTemp8 = FalseTrue_b();
      _skTemp5 = _skTemp8;
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      let _skTemp9 = FalseFalse_b();
      _skTemp4 = _skTemp9;
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
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
