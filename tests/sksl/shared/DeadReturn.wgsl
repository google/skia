diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
var<private> scratchVar: i32 = 0;
fn test_flat_b() -> bool {
  {
    return true;
  }
}
fn test_if_b() -> bool {
  {
    if _globalUniforms.colorGreen.y > 0.0h {
      {
        return true;
      }
    } else {
      {
        scratchVar = scratchVar + i32(1);
      }
    }
    scratchVar = scratchVar + i32(1);
    return false;
  }
}
fn test_else_b() -> bool {
  {
    if _globalUniforms.colorGreen.y == 0.0h {
      {
        return false;
      }
    } else {
      {
        return true;
      }
    }
  }
  return bool();
}
fn test_loop_if_b() -> bool {
  {
    {
      var x: i32 = 0;
      loop {
        {
          if _globalUniforms.colorGreen.y == 0.0h {
            {
              return false;
            }
          } else {
            {
              return true;
            }
          }
        }
        continuing {
          x = x + i32(1);
          break if x > 1;
        }
      }
    }
    scratchVar = scratchVar + i32(1);
    return true;
  }
}
fn _skslMain(xy: vec2<f32>) -> vec4<f16> {
  {
    var _skTemp0: vec4<f16>;
    var _skTemp1: bool;
    var _skTemp2: bool;
    var _skTemp3: bool;
    if test_flat_b() {
      _skTemp3 = test_if_b();
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      _skTemp2 = test_else_b();
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      _skTemp1 = test_loop_if_b();
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
