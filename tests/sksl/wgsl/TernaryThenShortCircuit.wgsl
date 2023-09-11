/*

:57:3 warning: code is unreachable
  return bool();
  ^^^^^^

:90:3 warning: code is unreachable
  return bool();
  ^^^^^^

:123:3 warning: code is unreachable
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
fn Increment_ii(y: ptr<function, i32>) -> i32 {
  {
    (*y) = (*y) + 1;
    return (*y);
  }
}
fn True_b() -> bool {
  {
    return true;
  }
}
fn False_b() -> bool {
  {
    return false;
  }
}
fn TrueFalse_b() -> bool {
  {
    var x: i32 = 1;
    var y: i32 = 1;
    var _skTemp0: bool;
    var _skTemp1: bool;
    if x == 1 {
      let _skTemp2 = True_b();
      _skTemp1 = _skTemp2;
    } else {
      let _skTemp3 = False_b();
      _skTemp1 = _skTemp3;
    }
    if _skTemp1 {
      _skTemp0 = true;
    } else {
      var _skTemp4: i32 = y;
      let _skTemp5 = Increment_ii(&_skTemp4);
      y = _skTemp4;
      _skTemp0 = (_skTemp5 == 3);
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
    var _skTemp6: bool;
    var _skTemp7: bool;
    if x == 2 {
      let _skTemp8 = True_b();
      _skTemp7 = _skTemp8;
    } else {
      let _skTemp9 = False_b();
      _skTemp7 = _skTemp9;
    }
    if _skTemp7 {
      _skTemp6 = true;
    } else {
      var _skTemp10: i32 = y;
      let _skTemp11 = Increment_ii(&_skTemp10);
      y = _skTemp10;
      _skTemp6 = (_skTemp11 == 2);
    }
    if _skTemp6 {
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
    var _skTemp12: bool;
    var _skTemp13: bool;
    if x == 2 {
      let _skTemp14 = True_b();
      _skTemp13 = _skTemp14;
    } else {
      let _skTemp15 = False_b();
      _skTemp13 = _skTemp15;
    }
    if _skTemp13 {
      _skTemp12 = true;
    } else {
      var _skTemp16: i32 = y;
      let _skTemp17 = Increment_ii(&_skTemp16);
      y = _skTemp16;
      _skTemp12 = (_skTemp17 == 3);
    }
    if _skTemp12 {
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
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_TrueTrue: bool;
    var _2_y: i32 = 1;
    var _skTemp18: bool;
    let _skTemp19 = True_b();
    if _skTemp19 {
      _skTemp18 = true;
    } else {
      var _skTemp20: i32 = _2_y;
      let _skTemp21 = Increment_ii(&_skTemp20);
      _2_y = _skTemp20;
      _skTemp18 = (_skTemp21 == 2);
    }
    if _skTemp18 {
      {
        _0_TrueTrue = (_2_y == 1);
      }
    } else {
      {
        _0_TrueTrue = false;
      }
    }
    var _skTemp22: vec4<f32>;
    var _skTemp23: bool;
    var _skTemp24: bool;
    var _skTemp25: bool;
    if _0_TrueTrue {
      let _skTemp26 = TrueFalse_b();
      _skTemp25 = _skTemp26;
    } else {
      _skTemp25 = false;
    }
    if _skTemp25 {
      let _skTemp27 = FalseTrue_b();
      _skTemp24 = _skTemp27;
    } else {
      _skTemp24 = false;
    }
    if _skTemp24 {
      let _skTemp28 = FalseFalse_b();
      _skTemp23 = _skTemp28;
    } else {
      _skTemp23 = false;
    }
    if _skTemp23 {
      _skTemp22 = _globalUniforms.colorGreen;
    } else {
      _skTemp22 = _globalUniforms.colorRed;
    }
    return _skTemp22;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
