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
fn blend_overlay_component_Qhh2h2(s: vec2<f32>, d: vec2<f32>) -> f32 {
  {
    var _skTemp0: f32;
    if (2.0 * d.x) <= d.y {
      _skTemp0 = (2.0 * s.x) * d.x;
    } else {
      _skTemp0 = s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
    }
    return _skTemp0;
  }
}
fn blend_overlay_h4h4h4(src: vec4<f32>, dst: vec4<f32>) -> vec4<f32> {
  {
    let _skTemp1 = blend_overlay_component_Qhh2h2(src.xw, dst.xw);
    let _skTemp2 = blend_overlay_component_Qhh2h2(src.yw, dst.yw);
    let _skTemp3 = blend_overlay_component_Qhh2h2(src.zw, dst.zw);
    var result: vec4<f32> = vec4<f32>(_skTemp1, _skTemp2, _skTemp3, src.w + (1.0 - src.w) * dst.w);
    result = vec4<f32>((result.xyz + (dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w))), result.w);
    return result;
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp4 = blend_overlay_h4h4h4(_globalUniforms.dst, _globalUniforms.src);
    (*_stageOut).sk_FragColor = _skTemp4;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
