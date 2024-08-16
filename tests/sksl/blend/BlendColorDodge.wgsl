diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  src: vec4<f32>,
  dst: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
const sk_PrivkGuardedDivideEpsilon: f32 = f32(select(0.0, 1e-08, false));
const sk_PrivkMinNormalHalf: f32 = 6.10351562e-05;
fn color_dodge_component_Qhh2h2(s: vec2<f32>, d: vec2<f32>) -> f32 {
  {
    let dxScale: f32 = f32(select(1, 0, d.x == 0.0));
    let _skTemp0 = abs(s.y - s.x);
    let _skTemp1 = select(d.y, (d.x * s.y) / ((s.y - s.x) + sk_PrivkGuardedDivideEpsilon), _skTemp0 >= sk_PrivkMinNormalHalf);
    let _skTemp2 = min(d.y, _skTemp1);
    let delta: f32 = dxScale * _skTemp2;
    return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp3 = color_dodge_component_Qhh2h2(_globalUniforms.src.xw, _globalUniforms.dst.xw);
    let _skTemp4 = color_dodge_component_Qhh2h2(_globalUniforms.src.yw, _globalUniforms.dst.yw);
    let _skTemp5 = color_dodge_component_Qhh2h2(_globalUniforms.src.zw, _globalUniforms.dst.zw);
    (*_stageOut).sk_FragColor = vec4<f32>(_skTemp3, _skTemp4, _skTemp5, _globalUniforms.src.w + (1.0 - _globalUniforms.src.w) * _globalUniforms.dst.w);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
