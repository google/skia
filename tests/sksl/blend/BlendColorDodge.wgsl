diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  src: vec4<f16>,
  dst: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
const sk_PrivkGuardedDivideEpsilon: f16 = f16(select(0.0, 1e-08, false));
const sk_PrivkMinNormalHalf: f16 = 6.10351562e-05h;
fn guarded_divide_Qhhh(n: f16, d: f16) -> f16 {
  {
    return n / (d + sk_PrivkGuardedDivideEpsilon);
  }
}
fn color_dodge_component_Qhh2h2(s: vec2<f16>, d: vec2<f16>) -> f16 {
  {
    let dxScale: f16 = f16(select(1, 0, d.x == 0.0h));
    var _skTemp0: f16;
    if abs(s.y - s.x) >= sk_PrivkMinNormalHalf {
      _skTemp0 = guarded_divide_Qhhh(d.x * s.y, s.y - s.x);
    } else {
      _skTemp0 = d.y;
    }
    let delta: f16 = dxScale * min(d.y, _skTemp0);
    return (delta * s.y + s.x * (1.0h - d.y)) + d.x * (1.0h - s.y);
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = vec4<f16>(color_dodge_component_Qhh2h2(_globalUniforms.src.xw, _globalUniforms.dst.xw), color_dodge_component_Qhh2h2(_globalUniforms.src.yw, _globalUniforms.dst.yw), color_dodge_component_Qhh2h2(_globalUniforms.src.zw, _globalUniforms.dst.zw), _globalUniforms.src.w + (1.0h - _globalUniforms.src.w) * _globalUniforms.dst.w);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
