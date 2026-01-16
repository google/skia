diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  unknownInput: f32,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn return_on_both_sides_b() -> bool {
  {
    if _globalUniforms.unknownInput == 1.0 {
      return true;
    } else {
      return true;
    }
  }
  return bool();
}
fn for_inside_body_b() -> bool {
  {
    {
      var x: i32 = 0;
      loop {
        {
          return true;
        }
        continuing {
          x = x + i32(1);
          break if x > 10;
        }
      }
    }
  }
  return bool();
}
fn after_for_body_b() -> bool {
  {
    {
      var x: i32 = 0;
      loop {
        {
        }
        continuing {
          x = x + i32(1);
          break if x > 10;
        }
      }
    }
    return true;
  }
}
fn for_with_double_sided_conditional_return_b() -> bool {
  {
    {
      var x: i32 = 0;
      loop {
        {
          if _globalUniforms.unknownInput == 1.0 {
            return true;
          } else {
            return true;
          }
        }
        continuing {
          x = x + i32(1);
          break if x > 10;
        }
      }
    }
  }
  return bool();
}
fn if_else_chain_b() -> bool {
  {
    if _globalUniforms.unknownInput == 1.0 {
      return true;
    } else {
      if _globalUniforms.unknownInput == 2.0 {
        return false;
      } else {
        if _globalUniforms.unknownInput == 3.0 {
          return true;
        } else {
          if _globalUniforms.unknownInput == 4.0 {
            return false;
          } else {
            return true;
          }
        }
      }
    }
  }
  return bool();
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    var _skTemp2: bool;
    var _skTemp3: bool;
    var _skTemp4: bool;
    var _skTemp5: bool;
    if true {
      _skTemp5 = return_on_both_sides_b();
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      _skTemp4 = for_inside_body_b();
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      _skTemp3 = after_for_body_b();
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      _skTemp2 = for_with_double_sided_conditional_return_b();
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      _skTemp1 = if_else_chain_b();
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
