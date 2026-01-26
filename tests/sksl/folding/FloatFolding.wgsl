diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct _GlobalUniforms {
  colorRed: vec4<f16>,
  colorGreen: vec4<f16>,
  unknownInput: f32,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    let _0_unknown: f16 = f16(_globalUniforms.unknownInput);
    var _1_ok: bool = true;
    var _2_x: f16 = 34.0h;
    _1_ok = _1_ok && (_2_x == 34.0h);
    _2_x = 30.0h;
    _1_ok = _1_ok && (_2_x == 30.0h);
    _2_x = 64.0h;
    _1_ok = _1_ok && (_2_x == 64.0h);
    _2_x = 16.0h;
    _1_ok = _1_ok && (_2_x == 16.0h);
    _2_x = 19.0h;
    _1_ok = _1_ok && (_2_x == 19.0h);
    _2_x = 1.0h;
    _1_ok = _1_ok && (_2_x == 1.0h);
    _2_x = -2.0h;
    _1_ok = _1_ok && (_2_x == -2.0h);
    _2_x = 3.0h;
    _1_ok = _1_ok && (_2_x == 3.0h);
    _2_x = -4.0h;
    _1_ok = _1_ok && (_2_x == -4.0h);
    _2_x = 5.0h;
    _1_ok = _1_ok && (_2_x == 5.0h);
    _2_x = -6.0h;
    _1_ok = _1_ok && (_2_x == -6.0h);
    _2_x = 7.0h;
    _1_ok = _1_ok && (_2_x == 7.0h);
    _2_x = -8.0h;
    _1_ok = _1_ok && (_2_x == -8.0h);
    _2_x = 9.0h;
    _1_ok = _1_ok && (_2_x == 9.0h);
    _2_x = -10.0h;
    _1_ok = _1_ok && (_2_x == -10.0h);
    _2_x = 11.0h;
    _1_ok = _1_ok && (_2_x == 11.0h);
    _2_x = -12.0h;
    _1_ok = _1_ok && (_2_x == -12.0h);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = 0.0h;
    _1_ok = _1_ok && (_2_x == 0.0h);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = 0.0h;
    _1_ok = _1_ok && (_2_x == 0.0h);
    _2_x = _0_unknown;
    _1_ok = _1_ok && (_2_x == _0_unknown);
    _2_x = 0.0h / _0_unknown;
    _1_ok = _1_ok && (_2_x == 0.0h);
    _2_x = _2_x + 1.0h;
    _1_ok = _1_ok && (_2_x == 1.0h);
    _1_ok = _1_ok && (_2_x == 1.0h);
    _2_x = _2_x - 2.0h;
    _1_ok = _1_ok && (_2_x == -1.0h);
    _1_ok = _1_ok && (_2_x == -1.0h);
    _1_ok = _1_ok && (_2_x == -1.0h);
    _2_x = _2_x * 2.0h;
    _1_ok = _1_ok && (_2_x == -2.0h);
    _1_ok = _1_ok && (_2_x == -2.0h);
    _2_x = _2_x * 0.5h;
    _1_ok = _1_ok && (_2_x == -1.0h);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(_1_ok));
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f16> {
  return _skslMain(_coords);
}
