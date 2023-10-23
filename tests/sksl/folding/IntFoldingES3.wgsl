diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_ok: bool = true;
    var _1_x: i32 = 14;
    _0_ok = _0_ok && (_1_x == 14);
    _1_x = 6;
    _0_ok = _0_ok && (_1_x == 6);
    _1_x = 5;
    _0_ok = _0_ok && (_1_x == 5);
    _1_x = 16;
    _0_ok = _0_ok && (_1_x == 16);
    _1_x = ~_1_x;
    _0_ok = _0_ok && (_1_x == -17);
    _0_ok = _0_ok && (_1_x == -17);
    _1_x = -8;
    _0_ok = _0_ok && (_1_x == -8);
    _1_x = 32;
    _0_ok = _0_ok && (_1_x == 32);
    _1_x = 33;
    _0_ok = _0_ok && (_1_x == 33);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(_0_ok));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
