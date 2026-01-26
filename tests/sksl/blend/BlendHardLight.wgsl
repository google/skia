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
fn blend_overlay_component_Qhh2h2(s: vec2<f16>, d: vec2<f16>) -> f16 {
  {
    var _skTemp0: f16;
    if (2.0h * d.x) <= d.y {
      _skTemp0 = (2.0h * s.x) * d.x;
    } else {
      _skTemp0 = s.y * d.y - (2.0h * (d.y - d.x)) * (s.y - s.x);
    }
    return _skTemp0;
  }
}
fn blend_overlay_h4h4h4(src: vec4<f16>, dst: vec4<f16>) -> vec4<f16> {
  {
    var result: vec4<f16> = vec4<f16>(blend_overlay_component_Qhh2h2(src.xw, dst.xw), blend_overlay_component_Qhh2h2(src.yw, dst.yw), blend_overlay_component_Qhh2h2(src.zw, dst.zw), src.w + (1.0h - src.w) * dst.w);
    result = vec4<f16>((result.xyz + (dst.xyz * (1.0h - src.w) + src.xyz * (1.0h - dst.w))), result.w);
    return result;
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = blend_overlay_h4h4h4(_globalUniforms.dst, _globalUniforms.src);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
