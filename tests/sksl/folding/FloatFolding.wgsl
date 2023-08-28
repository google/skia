diagnostic(off, derivative_uniformity);
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
  unknownInput: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_unknown: f32 = f32(_globalUniforms.unknownInput);
    var _1_ok: bool = true;
    var _2_x: f32 = 34.0;
    _1_ok = _1_ok && (_2_x == 34.0);
    _2_x = 30.0;
    _1_ok = _1_ok && (_2_x == 30.0);
    _2_x = 64.0;
    _1_ok = _1_ok && (_2_x == 64.0);
    _2_x = 16.0;
    _1_ok = _1_ok && (_2_x == 16.0);
    _2_x = 19.0;
    _1_ok = _1_ok && (_2_x == 19.0);
    _2_x = 1.0;
    _1_ok = _1_ok && (_2_x == 1.0);
    _2_x = -2.0;
    _1_ok = _1_ok && (_2_x == -2.0);
    _2_x = 3.0;
    _1_ok = _1_ok && (_2_x == 3.0);
    _2_x = -4.0;
    _1_ok = _1_ok && (_2_x == -4.0);
    _2_x = 5.0;
    _1_ok = _1_ok && (_2_x == 5.0);
    _2_x = -6.0;
    _1_ok = _1_ok && (_2_x == -6.0);
    _2_x = 7.0;
    _1_ok = _1_ok && (_2_x == 7.0);
    _2_x = -8.0;
    _1_ok = _1_ok && (_2_x == -8.0);
    _2_x = 9.0;
    _1_ok = _1_ok && (_2_x == 9.0);
    _2_x = -10.0;
    _1_ok = _1_ok && (_2_x == -10.0);
    _2_x = 11.0;
    _1_ok = _1_ok && (_2_x == 11.0);
    _2_x = -12.0;
    _1_ok = _1_ok && (_2_x == -12.0);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = 0.0;
    _1_ok = _1_ok && (_2_x == 0.0);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = 0.0;
    _1_ok = _1_ok && (_2_x == 0.0);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = 0.0 / _0_unknown;
    _1_ok = _1_ok && (_2_x == 0.0);
    _2_x = _2_x + 1.0;
    _1_ok = _1_ok && (_2_x == 1.0);
    _1_ok = _1_ok && (_2_x == 1.0);
    _2_x = _2_x - 2.0;
    _1_ok = _1_ok && (_2_x == -1.0);
    _1_ok = _1_ok && (_2_x == -1.0);
    _1_ok = _1_ok && (_2_x == -1.0);
    _2_x = _2_x * 2.0;
    _1_ok = _1_ok && (_2_x == -2.0);
    _1_ok = _1_ok && (_2_x == -2.0);
    _2_x = _2_x * 0.5;
    _1_ok = _1_ok && (_2_x == -1.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(_1_ok));
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
