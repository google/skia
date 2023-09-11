diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  src: vec4<f32>,
  dst: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn blend_overlay_component_Qhh2h2(s: vec2<f32>, d: vec2<f32>) -> f32 {
  {
    return select(s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x), (2.0 * s.x) * d.x, (2.0 * d.x) <= d.y);
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp0 = blend_overlay_component_Qhh2h2(_globalUniforms.src.xw, _globalUniforms.dst.xw);
    let _skTemp1 = blend_overlay_component_Qhh2h2(_globalUniforms.src.yw, _globalUniforms.dst.yw);
    let _skTemp2 = blend_overlay_component_Qhh2h2(_globalUniforms.src.zw, _globalUniforms.dst.zw);
    var _0_result: vec4<f32> = vec4<f32>(_skTemp0, _skTemp1, _skTemp2, _globalUniforms.src.w + (1.0 - _globalUniforms.src.w) * _globalUniforms.dst.w);
    _0_result = vec4<f32>((_0_result.xyz + (_globalUniforms.dst.xyz * (1.0 - _globalUniforms.src.w) + _globalUniforms.src.xyz * (1.0 - _globalUniforms.dst.w))), _0_result.w).xyzw;
    (*_stageOut).sk_FragColor = _0_result;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
