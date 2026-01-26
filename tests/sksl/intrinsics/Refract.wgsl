diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  a: f16,
  b: f16,
  c: f16,
  d: vec4<f16>,
  e: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f16> {
  {
    let _skTemp0 = 65504.0h;
    var result: vec4<f16> = vec4<f16>(refract(vec2<f16>(_skTemp0 * 2.0h, 0), vec2<f16>(2.0h, 0), 2.0h).x);
    result.x = refract(vec2<f16>(_globalUniforms.a, 0), vec2<f16>(_globalUniforms.b, 0), _globalUniforms.c).x;
    result = refract(_globalUniforms.d, _globalUniforms.e, _globalUniforms.c);
    result = vec4<f16>((vec2<f16>(0.5h, -0.8660254h)), result.zw);
    result = vec4<f16>((vec3<f16>(0.5h, 0.0h, -0.8660254h)), result.w);
    result = vec4<f16>(0.5h, 0.0h, 0.0h, -0.8660254h);
    return result;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
