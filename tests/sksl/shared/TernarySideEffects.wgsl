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
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var x: f16 = 1.0h;
    var y: f16 = 1.0h;
    var _skTemp0: f16;
    if x == y {
      x = x + 1.0h;
      _skTemp0 = x;
    } else {
      y = y + 1.0h;
      _skTemp0 = y;
    }
    var _skTemp1: f16;
    if x == y {
      x = x + 3.0h;
      _skTemp1 = x;
    } else {
      y = y + 3.0h;
      _skTemp1 = y;
    }
    var _skTemp2: f16;
    if x < y {
      x = x + 5.0h;
      _skTemp2 = x;
    } else {
      y = y + 5.0h;
      _skTemp2 = y;
    }
    var _skTemp3: f16;
    if y >= x {
      x = x + 9.0h;
      _skTemp3 = x;
    } else {
      y = y + 9.0h;
      _skTemp3 = y;
    }
    var _skTemp4: f16;
    if x != y {
      x = x + 1.0h;
      _skTemp4 = x;
    } else {
      _skTemp4 = y;
    }
    var _skTemp5: f16;
    if x == y {
      x = x + 2.0h;
      _skTemp5 = x;
    } else {
      _skTemp5 = y;
    }
    var _skTemp6: f16;
    if x != y {
      _skTemp6 = x;
    } else {
      y = y + 3.0h;
      _skTemp6 = y;
    }
    var _skTemp7: f16;
    if x == y {
      _skTemp7 = x;
    } else {
      y = y + 4.0h;
      _skTemp7 = y;
    }
    var b: bool = true;
    var _skTemp8: bool;
    b = false;
    if b {
      _skTemp8 = false;
    } else {
      _skTemp8 = b;
    }
    let c: bool = _skTemp8;
    var _skTemp9: vec4<f16>;
    if c {
      _skTemp9 = _globalUniforms.colorRed;
    } else {
      _skTemp9 = select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((x == 8.0h) && (y == 17.0h)));
    }
    return _skTemp9;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
