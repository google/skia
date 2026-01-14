diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_x: f32 = 1.0;
    _0_x = length(_0_x);
    _0_x = distance(_0_x, 2.0);
    _0_x = _0_x * 2.0;
    _0_x = sign(_0_x);
    var _1_x: vec2<f32> = vec2<f32>(1.0, 2.0);
    _1_x = vec2<f32>(length(_1_x));
    _1_x = vec2<f32>(distance(_1_x, vec2<f32>(3.0, 4.0)));
    _1_x = vec2<f32>(dot(_1_x, vec2<f32>(3.0, 4.0)));
    _1_x = normalize(_1_x);
    return _globalUniforms.colorGreen;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
