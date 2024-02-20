diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
  unknownInput: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_unknown: i32 = i32(_globalUniforms.unknownInput);
    var _1_ok: bool = true;
    var _2_x: i32 = 34;
    _1_ok = _1_ok && (_2_x == 34);
    _2_x = 30;
    _1_ok = _1_ok && (_2_x == 30);
    _2_x = 64;
    _1_ok = _1_ok && (_2_x == 64);
    _2_x = 16;
    _1_ok = _1_ok && (_2_x == 16);
    _2_x = 1;
    _1_ok = _1_ok && (_2_x == 1);
    _2_x = -2;
    _1_ok = _1_ok && (_2_x == -2);
    _2_x = 3;
    _1_ok = _1_ok && (_2_x == 3);
    _2_x = -4;
    _1_ok = _1_ok && (_2_x == -4);
    _2_x = 5;
    _1_ok = _1_ok && (_2_x == 5);
    _2_x = -6;
    _1_ok = _1_ok && (_2_x == -6);
    _2_x = 7;
    _1_ok = _1_ok && (_2_x == 7);
    _2_x = -8;
    _1_ok = _1_ok && (_2_x == -8);
    _2_x = 9;
    _1_ok = _1_ok && (_2_x == 9);
    _2_x = -10;
    _1_ok = _1_ok && (_2_x == -10);
    _2_x = 11;
    _1_ok = _1_ok && (_2_x == 11);
    _2_x = -12;
    _1_ok = _1_ok && (_2_x == -12);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = 0;
    _1_ok = _1_ok && (_2_x == 0);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = 0;
    _1_ok = _1_ok && (_2_x == 0);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = 0 / _0_unknown;
    _1_ok = _1_ok && (_2_x == 0);
    _2_x = _2_x + 1;
    _1_ok = _1_ok && (_2_x == 1);
    _1_ok = _1_ok && (_2_x == 1);
    _2_x = _2_x - 2;
    _1_ok = _1_ok && (_2_x == -1);
    _1_ok = _1_ok && (_2_x == -1);
    _1_ok = _1_ok && (_2_x == -1);
    _2_x = _2_x * 2;
    _1_ok = _1_ok && (_2_x == -2);
    _1_ok = _1_ok && (_2_x == -2);
    _2_x = _2_x / 2;
    _1_ok = _1_ok && (_2_x == -1);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(_1_ok));
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
