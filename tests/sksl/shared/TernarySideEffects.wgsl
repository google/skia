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
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var x: f32 = 1.0;
    var y: f32 = 1.0;
    var _skTemp0: f32;
    if x == y {
      x = x + 1.0;
      _skTemp0 = x;
    } else {
      y = y + 1.0;
      _skTemp0 = y;
    }
    var _skTemp1: f32;
    if x == y {
      x = x + 3.0;
      _skTemp1 = x;
    } else {
      y = y + 3.0;
      _skTemp1 = y;
    }
    var _skTemp2: f32;
    if x < y {
      x = x + 5.0;
      _skTemp2 = x;
    } else {
      y = y + 5.0;
      _skTemp2 = y;
    }
    var _skTemp3: f32;
    if y >= x {
      x = x + 9.0;
      _skTemp3 = x;
    } else {
      y = y + 9.0;
      _skTemp3 = y;
    }
    var _skTemp4: f32;
    if x != y {
      x = x + 1.0;
      _skTemp4 = x;
    } else {
      _skTemp4 = y;
    }
    var _skTemp5: f32;
    if x == y {
      x = x + 2.0;
      _skTemp5 = x;
    } else {
      _skTemp5 = y;
    }
    var _skTemp6: f32;
    if x != y {
      _skTemp6 = x;
    } else {
      y = y + 3.0;
      _skTemp6 = y;
    }
    var _skTemp7: f32;
    if x == y {
      _skTemp7 = x;
    } else {
      y = y + 4.0;
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
    var c: bool = _skTemp8;
    return select(select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((x == 8.0) && (y == 17.0))), _globalUniforms.colorRed, vec4<bool>(c));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
