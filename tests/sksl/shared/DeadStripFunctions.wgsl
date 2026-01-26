diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn unpremul_h4h4(color: vec4<f16>) -> vec4<f16> {
  {
    return vec4<f16>(color.xyz / max(color.w, 0.0001h), color.w);
  }
}
fn live_fn_h4h4h4(a: vec4<f16>, b: vec4<f16>) -> vec4<f16> {
  {
    return a + b;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var a: vec4<f16>;
    var b: vec4<f16>;
    {
      a = live_fn_h4h4h4(vec4<f16>(3.0h), vec4<f16>(-5.0h));
    }
    {
      b = unpremul_h4h4(vec4<f16>(1.0h));
    }
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(any(a != vec4<f16>(0.0h)) && any(b != vec4<f16>(0.0h))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
