diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn flatten_compound_constructor_b() -> bool {
  {
    var x: vec4<i32> = vec4<i32>(vec3<i32>(vec2<i32>(1, 2), 3), 4);
    var y: vec4<i32> = vec4<i32>(1, vec3<i32>(2, vec2<i32>(3, 4)));
    return all(x == y);
  }
}
fn flatten_known_if_b() -> bool {
  {
    var value: i32;
    if true {
      {
        value = 1;
      }
    } else {
      {
        value = 2;
      }
    }
    return value == 1;
  }
}
fn eliminate_empty_if_else_b() -> bool {
  {
    var check: bool = false;
    check = !check;
    if check {
      {
      }
    } else {
      {
      }
    }
    return check;
  }
}
fn eliminate_empty_else_b() -> bool {
  {
    var check: bool = true;
    if check {
      {
        return true;
      }
    } else {
      {
      }
    }
    return false;
  }
}
fn flatten_matching_ternary_b() -> bool {
  {
    var check: bool = true;
    return select(true, true, check);
  }
}
fn flatten_expr_without_side_effects_b() -> bool {
  {
    var check: bool = true;
    return check;
  }
}
fn eliminate_no_op_arithmetic_b() -> bool {
  {
    const ONE: i32 = 1;
    var a1: array<i32, 1>;
    var a2: array<i32, 1>;
    var x: i32 = ONE;
    x = x + 0;
    x = x * 1;
    return x == 1;
  }
}
fn flatten_switch_b() -> bool {
  {
    switch 1 {
      case 0 {
        return false;
      }
      case 1 {
        return true;
      }
      case 2 {
        return false;
      }
      case default {}
    }
    return false;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    var _skTemp2: bool;
    var _skTemp3: bool;
    var _skTemp4: bool;
    var _skTemp5: bool;
    var _skTemp6: bool;
    var _skTemp7: bool;
    let _skTemp8 = flatten_compound_constructor_b();
    if _skTemp8 {
      let _skTemp9 = flatten_known_if_b();
      _skTemp7 = _skTemp9;
    } else {
      _skTemp7 = false;
    }
    if _skTemp7 {
      let _skTemp10 = eliminate_empty_if_else_b();
      _skTemp6 = _skTemp10;
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      let _skTemp11 = eliminate_empty_else_b();
      _skTemp5 = _skTemp11;
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      let _skTemp12 = flatten_matching_ternary_b();
      _skTemp4 = _skTemp12;
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      let _skTemp13 = flatten_expr_without_side_effects_b();
      _skTemp3 = _skTemp13;
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      let _skTemp14 = eliminate_no_op_arithmetic_b();
      _skTemp2 = _skTemp14;
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      let _skTemp15 = flatten_switch_b();
      _skTemp1 = _skTemp15;
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
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
